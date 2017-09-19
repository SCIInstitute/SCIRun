/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
University of Utah.

License for the specific language governing rights and limitations under
Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include <Modules/Visualization/InterfaceWithOspray.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef WITH_OSPRAY
#include <ospray/ospray.h>
#endif

using namespace SCIRun;
using namespace Dataflow::Networks;
using namespace Modules::Visualization;
using namespace Core;
using namespace Core::Algorithms;
using namespace Visualization;
using namespace Datatypes;

ALGORITHM_PARAMETER_DEF(Visualization, ImageHeight);
ALGORITHM_PARAMETER_DEF(Visualization, ImageWidth);
ALGORITHM_PARAMETER_DEF(Visualization, CameraPositionX);
ALGORITHM_PARAMETER_DEF(Visualization, CameraPositionY);
ALGORITHM_PARAMETER_DEF(Visualization, CameraPositionZ);
ALGORITHM_PARAMETER_DEF(Visualization, CameraUpX);
ALGORITHM_PARAMETER_DEF(Visualization, CameraUpY);
ALGORITHM_PARAMETER_DEF(Visualization, CameraUpZ);
ALGORITHM_PARAMETER_DEF(Visualization, CameraViewX);
ALGORITHM_PARAMETER_DEF(Visualization, CameraViewY);
ALGORITHM_PARAMETER_DEF(Visualization, CameraViewZ);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorR);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorG);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorB);
ALGORITHM_PARAMETER_DEF(Visualization, BackgroundColorR);
ALGORITHM_PARAMETER_DEF(Visualization, BackgroundColorG);
ALGORITHM_PARAMETER_DEF(Visualization, BackgroundColorB);
ALGORITHM_PARAMETER_DEF(Visualization, FrameCount);
ALGORITHM_PARAMETER_DEF(Visualization, ShowImageInWindow);
ALGORITHM_PARAMETER_DEF(Visualization, LightColorR);
ALGORITHM_PARAMETER_DEF(Visualization, LightColorG);
ALGORITHM_PARAMETER_DEF(Visualization, LightColorB);
ALGORITHM_PARAMETER_DEF(Visualization, LightIntensity);
ALGORITHM_PARAMETER_DEF(Visualization, LightVisible);
ALGORITHM_PARAMETER_DEF(Visualization, LightType);

MODULE_INFO_DEF(InterfaceWithOspray, Visualization, SCIRun)

void InterfaceWithOspray::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::ImageHeight, 768);
  state->setValue(Parameters::ImageWidth, 1024);
  state->setValue(Parameters::CameraPositionX, 5.0);
  state->setValue(Parameters::CameraPositionY, 5.0);
  state->setValue(Parameters::CameraPositionZ, 5.0);
  state->setValue(Parameters::CameraUpX, 0.0);
  state->setValue(Parameters::CameraUpY, 0.0);
  state->setValue(Parameters::CameraUpZ, 1.0);
  state->setValue(Parameters::CameraViewX, 0.0);
  state->setValue(Parameters::CameraViewY, 0.0);
  state->setValue(Parameters::CameraViewZ, 0.0);
  state->setValue(Parameters::DefaultColorR, 0.5);
  state->setValue(Parameters::DefaultColorG, 0.5);
  state->setValue(Parameters::DefaultColorB, 0.5);
  state->setValue(Parameters::BackgroundColorR, 0.0);
  state->setValue(Parameters::BackgroundColorG, 0.0);
  state->setValue(Parameters::BackgroundColorB, 0.0);
  state->setValue(Parameters::FrameCount, 10);
  state->setValue(Parameters::ShowImageInWindow, true);
  state->setValue(Parameters::LightColorR, 1.0);
  state->setValue(Parameters::LightColorG, 1.0);
  state->setValue(Parameters::LightColorB, 1.0);
  state->setValue(Parameters::LightIntensity, 1.0);
  state->setValue(Parameters::LightVisible, false);
  state->setValue(Parameters::LightType, std::string("none"));
}

namespace detail
{
  #ifdef WITH_OSPRAY
  class OsprayImpl
  {
  public:
    OsprayImpl()
    {
      const char* argv[] = { "" };
      int argc = 0;
      int init_error = ospInit(&argc, argv);
      if (init_error != OSP_NO_ERROR)
        throw init_error;
    }

    void writeImage(FieldHandle field, const std::string& filename, ModuleStateHandle state, boost::optional<ColorMapHandle> colorMap)
    {
      auto facade(field->mesh()->getFacade());

      // image size
      osp::vec2i imgSize;
      imgSize.x = state->getValue(Parameters::ImageWidth).toInt();
      imgSize.y = state->getValue(Parameters::ImageHeight).toInt();

      auto toFloat = [state](const Name& name) { return static_cast<float>(state->getValue(name).toDouble()); };

      // camera
      float cam_pos[] = { toFloat(Parameters::CameraPositionX), toFloat(Parameters::CameraPositionY), toFloat(Parameters::CameraPositionZ) };
      float cam_up[] = { toFloat(Parameters::CameraUpX), toFloat(Parameters::CameraUpY), toFloat(Parameters::CameraUpZ) };
      float cam_view[] = { toFloat(Parameters::CameraViewX), toFloat(Parameters::CameraViewY), toFloat(Parameters::CameraViewZ) };

      auto map = colorMap.value_or(nullptr);
      std::vector<float> vertex, color;
      auto vfield = field->vfield();

      {
        double value;
        for (const auto& node : facade->nodes())
        {
          auto point = node.point();
          vertex.push_back(static_cast<float>(point.x()));
          vertex.push_back(static_cast<float>(point.y()));
          vertex.push_back(static_cast<float>(point.z()));
          vertex.push_back(0);

          vfield->get_value(value, node.index());

          ColorRGB nodeColor(state->getValue(Parameters::DefaultColorR).toDouble(), state->getValue(Parameters::DefaultColorG).toDouble(), state->getValue(Parameters::DefaultColorB).toDouble());
          if (map)
          {
            nodeColor = map->valueToColor(value);
          }
          color.push_back(static_cast<float>(nodeColor.r()));
          color.push_back(static_cast<float>(nodeColor.g()));
          color.push_back(static_cast<float>(nodeColor.b()));
          color.push_back(1.0f);
        }
      }

      std::vector<int32_t> index;
      {
        for (const auto& face : facade->faces())
        {
          auto nodes = face.nodeIndices();
          index.push_back(static_cast<int32_t>(nodes[0]));
          index.push_back(static_cast<int32_t>(nodes[1]));
          index.push_back(static_cast<int32_t>(nodes[2]));
        }
      }

      // create and setup camera
      OSPCamera camera = ospNewCamera("perspective");
      ospSetf(camera, "aspect", imgSize.x / (float)imgSize.y);
      ospSet3fv(camera, "pos", cam_pos);
      ospSet3fv(camera, "dir", cam_view);
      ospSet3fv(camera, "up", cam_up);
      ospCommit(camera); // commit each object to indicate modifications are done

      // create and setup model and mesh
      OSPGeometry mesh = ospNewGeometry("triangles");
      OSPData data = ospNewData(vertex.size() / 4, OSP_FLOAT3A, &vertex[0]); // OSP_FLOAT3 format is also supported for vertex positions
      ospCommit(data);
      ospSetData(mesh, "vertex", data);

      data = ospNewData(vertex.size() / 4, OSP_FLOAT4, &color[0]);
      ospCommit(data);
      ospSetData(mesh, "vertex.color", data);

      data = ospNewData(index.size() / 3, OSP_INT3, &index[0]); // OSP_INT4 format is also supported for triangle indices
      ospCommit(data);
      ospSetData(mesh, "index", data);

      ospCommit(mesh);

      OSPModel world = ospNewModel();
      ospAddGeometry(world, mesh);
      ospCommit(world);

      // create renderer
      OSPRenderer renderer = ospNewRenderer("scivis"); // choose Scientific Visualization renderer

      // create and setup light for Ambient Occlusion
      OSPLight light = ospNewLight(renderer, state->getValue(Parameters::LightType).toString().c_str());
      OSPData lights;
      if (light)
      {
        float lightColor[] = { toFloat(Parameters::LightColorR), toFloat(Parameters::LightColorG), toFloat(Parameters::LightColorB) };
        ospSet3fv(light, "color", lightColor);
        ospSet1f(light, "intensity", toFloat(Parameters::LightIntensity));
        ospSet1i(light, "isVisible", state->getValue(Parameters::LightVisible).toBool() ? 1 : 0);
        ospCommit(light);
        lights = ospNewData(1, OSP_LIGHT, &light);
        ospCommit(lights);
      }

      //material
      OSPMaterial material = ospNewMaterial(renderer, "OBJMaterial");
      ospSet3f(material, "Kd", 0.2f, 0.2f, 0.2f);
      ospSet3f(material, "Ks", 0.4f, 0.4f, 0.4f);
      ospSet1f(material, "Ns", 100.0f);
      ospCommit(renderer);

      // complete setup of renderer
      ospSet1i(renderer, "aoSamples", 1);
      ospSet3f(renderer, "bgColor", toFloat(Parameters::BackgroundColorR),
        toFloat(Parameters::BackgroundColorG),
        toFloat(Parameters::BackgroundColorB));
      ospSetObject(renderer, "model", world);
      ospSetObject(renderer, "camera", camera);
      if (light)
        ospSetObject(renderer, "lights", lights);
      ospSetObject(renderer, "material", material);
      ospCommit(renderer);

      // create and setup framebuffer
      OSPFrameBuffer framebuffer = ospNewFrameBuffer(imgSize, OSP_FB_SRGBA, OSP_FB_COLOR | /*OSP_FB_DEPTH |*/ OSP_FB_ACCUM);
      ospFrameBufferClear(framebuffer, OSP_FB_COLOR | OSP_FB_ACCUM);

      // render one frame
      ospRenderFrame(framebuffer, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);

      const int frameCount = state->getValue(Parameters::FrameCount).toInt();
      // render N more frames, which are accumulated to result in a better converged image
      for (int frames = 0; frames < frameCount-1; frames++)
        ospRenderFrame(framebuffer, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);

      // access framebuffer and write its content as PPM file
      const uint32_t * fb = (uint32_t*)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR);
      writePPM(filename.c_str(), imgSize, fb);
      ospUnmapFrameBuffer(fb, framebuffer);
    }
  private:
    void writePPM(const char *fileName, const osp::vec2i &size, const uint32_t *pixel)
    {
      FILE *file = fopen(fileName, "wb");
      fprintf(file, "P6\n%i %i\n255\n", size.x, size.y);
      std::vector<unsigned char> out(3 * size.x);
      for (int y = 0; y < size.y; y++)
      {
        const unsigned char *in = (const unsigned char *)&pixel[(size.y - 1 - y)*size.x];
        for (int x = 0; x < size.x; x++)
        {
          out[3 * x + 0] = in[4 * x + 0];
          out[3 * x + 1] = in[4 * x + 1];
          out[3 * x + 2] = in[4 * x + 2];
        }
        fwrite(&out[0], 3 * size.x, sizeof(char), file);
      }
      fprintf(file, "\n");
      fclose(file);
      std::cout << "wrote file " << fileName << std::endl;
    }
  };
  #else
  class OsprayImpl {};
  #endif
}

boost::shared_ptr<detail::OsprayImpl> InterfaceWithOspray::impl_(new detail::OsprayImpl);

InterfaceWithOspray::InterfaceWithOspray() : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(Field);
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(SceneGraph);
}

void InterfaceWithOspray::execute()
{
  #ifdef WITH_OSPRAY
  auto field = getRequiredInput(Field);
  auto colorMap = getOptionalInput(ColorMapObject);

  if (needToExecute())
  {
    FieldInformation info(field);

    if (!info.is_trisurfmesh())
      THROW_INVALID_ARGUMENT("Module currently only works with trisurfs.");

    auto isoString = boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::universal_time());
    auto filename = "scirunOsprayOutput_" + isoString + ".ppm";
    remark("Saving output to " + filename);
    impl_->writeImage(field, filename, get_state(), colorMap);
    get_state()->setTransientValue(Variables::Filename, filename);

    //auto geom = builder_->buildGeometryObject(field, colorMap, *this, this);
    //sendOutput(SceneGraph, geom);
  }
  #else
  error("Build SCIRun with WITH_OSPRAY set to true to enable this module.");
  #endif
}
