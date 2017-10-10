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
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorA);
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
ALGORITHM_PARAMETER_DEF(Visualization, AutoCameraView);
ALGORITHM_PARAMETER_DEF(Visualization, StreamlineRadius);

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
  state->setValue(Parameters::DefaultColorA, 1.0);
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
  state->setValue(Parameters::LightType, std::string("ambient"));
  state->setValue(Parameters::AutoCameraView, true);
  state->setValue(Parameters::StreamlineRadius, 0.1);
  state->setValue(Variables::Filename, std::string(""));
}

namespace detail
{
  #ifdef WITH_OSPRAY
  class OsprayImpl
  {
  private:
    static bool initialized_;
    static Core::Thread::Mutex lock_;
    static void initialize()
    {
      if (!initialized_)
      {
        const char* argv[] = { "" };
        int argc = 0;
        int init_error = ospInit(&argc, argv);
        if (init_error != OSP_NO_ERROR)
          throw init_error;
        initialized_ = true;
      }
    }

    Core::Thread::Guard guard_;
    Core::Geometry::BBox imageBox_;
    ModuleStateHandle state_;
    osp::vec2i imgSize_;
    OSPCamera camera_;
    OSPModel world_;
    std::vector<OSPGeometry> meshes_;
    OSPRenderer renderer_;
    OSPFrameBuffer framebuffer_;

    float toFloat(const Name& name) const
    {
      return static_cast<float>(state_->getValue(name).toDouble());
    }

    struct FieldData
    {
      std::vector<float> vertex, color;
      std::vector<int32_t> index;
    };

    std::vector<FieldData> fieldData_;

  public:
    explicit OsprayImpl(ModuleStateHandle state) : guard_(lock_.get()), state_(state)
    {
      initialize();
    }

    void setup()
    {
      imgSize_.x = state_->getValue(Parameters::ImageWidth).toInt();
      imgSize_.y = state_->getValue(Parameters::ImageHeight).toInt();

      // camera
      float cam_pos[] = { toFloat(Parameters::CameraPositionX), toFloat(Parameters::CameraPositionY), toFloat(Parameters::CameraPositionZ) };
      float cam_up[] = { toFloat(Parameters::CameraUpX), toFloat(Parameters::CameraUpY), toFloat(Parameters::CameraUpZ) };
      float cam_view[] = { toFloat(Parameters::CameraViewX), toFloat(Parameters::CameraViewY), toFloat(Parameters::CameraViewZ) };

      // create and setup camera
      camera_ = ospNewCamera("perspective");
      ospSetf(camera_, "aspect", imgSize_.x / (float)imgSize_.y);
      ospSet3fv(camera_, "pos", cam_pos);
      ospSet3fv(camera_, "dir", cam_view);
      ospSet3fv(camera_, "up", cam_up);
      ospCommit(camera_); // commit each object to indicate modifications are done

      world_ = ospNewModel();
      ospCommit(world_);
    }

    void adjustCameraPosition(FieldHandle field)
    {
      if (state_->getValue(Parameters::AutoCameraView).toBool())
      {
        auto vmesh = field->vmesh();
        auto bbox = vmesh->get_bounding_box();
        imageBox_.extend(bbox);
        auto center = imageBox_.center();
        float position[] = { toFloat(Parameters::CameraPositionX), toFloat(Parameters::CameraPositionY), toFloat(Parameters::CameraPositionZ) };
        float cam_up[] = { toFloat(Parameters::CameraUpX), toFloat(Parameters::CameraUpY), toFloat(Parameters::CameraUpZ) };
        float newDir[] = { static_cast<float>(center.x()) - position[0],
           static_cast<float>(center.y()) - position[1],
           static_cast<float>(center.z()) - position[2]};
        //std::cout << "newDir " << newDir[0] << ", " << newDir[1] << ", " << newDir[2] << std::endl;
        state_->setValue(Parameters::CameraViewX, center.x());
        state_->setValue(Parameters::CameraViewY, center.y());
        state_->setValue(Parameters::CameraViewZ, center.z());
        ospSet3fv(camera_, "dir", newDir);
        //float newUp[] = { newDir[0] / newDir[2], -(newDir[0]*newDir[0] + newDir[2]*newDir[2])/(newDir[1]*newDir[2]) , 1.0f };
        float side[] = {newDir[1]*cam_up[2] - newDir[2]*cam_up[1],newDir[2]*cam_up[0] - newDir[0]*cam_up[2], newDir[0]*cam_up[1] - newDir[1]*cam_up[0], 1.0f };
        float norm_side = sqrt(side[0]*side[0]+side[1]*side[1]+side[2]*side[2]);
        if (norm_side <= 1e-3)
        {
          float side[] = {newDir[1] ,- newDir[0], 0.0f, 1.0f };
          norm_side = sqrt(side[0]*side[0]+side[1]*side[1]+side[2]*side[2]);
          if (norm_side <= 1e-3)
          {
            float side[] = {- newDir[2],0.0f, newDir[0], 1.0f };
            norm_side = sqrt(side[0]*side[0]+side[1]*side[1]+side[2]*side[2]);
          }
        }
        side[0] = side[0]/norm_side;
        side[1] = side[1]/norm_side;
        side[2] = side[2]/norm_side;
        
        float newUp[] = {side[1]*newDir[2] - side[2]*newDir[1],side[2]*newDir[0] - side[0]*newDir[2],side[0]*newDir[1] - side[1]*newDir[0],1.0f};
        
        state_->setValue(Parameters::CameraUpX, newUp[0]);
        state_->setValue(Parameters::CameraUpY, newUp[1]);
        state_->setValue(Parameters::CameraUpZ, newUp[2]);
        ospSet3fv(camera_, "up", newUp);
        ospCommit(camera_);
      }
    }

    void fillDataBuffers(FieldHandle field, ColorMapHandle colorMap)
    {
      auto facade(field->mesh()->getFacade());

      fieldData_.push_back({});
      auto& fieldData = fieldData_.back();
      auto& vertex = fieldData.vertex;
      auto& color = fieldData.color;

      auto vfield = field->vfield();

      {
        double value;
        ColorRGB nodeColor(state_->getValue(Parameters::DefaultColorR).toDouble(), state_->getValue(Parameters::DefaultColorG).toDouble(), state_->getValue(Parameters::DefaultColorB).toDouble());
        auto alpha = toFloat(Parameters::DefaultColorA);

        for (const auto& node : facade->nodes())
        {
          auto point = node.point();
          vertex.push_back(static_cast<float>(point.x()));
          vertex.push_back(static_cast<float>(point.y()));
          vertex.push_back(static_cast<float>(point.z()));
          vertex.push_back(0);

          vfield->get_value(value, node.index());
          if (colorMap)
          {
            nodeColor = colorMap->valueToColor(value);
          }
          color.push_back(static_cast<float>(nodeColor.r()));
          color.push_back(static_cast<float>(nodeColor.g()));
          color.push_back(static_cast<float>(nodeColor.b()));
          color.push_back(alpha);
        }
      }

      auto& index = fieldData.index;
      {
        for (const auto& face : facade->faces())
        {
          auto nodes = face.nodeIndices();
          index.push_back(static_cast<int32_t>(nodes[0]));
          index.push_back(static_cast<int32_t>(nodes[1]));
          index.push_back(static_cast<int32_t>(nodes[2]));
        }
      }

    }

    void addField(FieldHandle field, ColorMapHandle colorMap)
    {
      adjustCameraPosition(field);

      fillDataBuffers(field, colorMap);

      const auto& fieldData = fieldData_.back();
      const auto& vertex = fieldData.vertex;
      const auto& color = fieldData.color;
      const auto& index = fieldData.index;

      // create and setup model and mesh
      OSPGeometry mesh = ospNewGeometry("triangles");
      OSPData data = ospNewData(vertex.size() / 4, OSP_FLOAT3A, &vertex[0]); // OSP_FLOAT3 format is also supported for vertex positions
      ospCommit(data);
      ospSetData(mesh, "vertex", data);
      data = ospNewData(color.size() / 4, OSP_FLOAT4, &color[0]);
      ospCommit(data);
      ospSetData(mesh, "vertex.color", data);
      data = ospNewData(index.size() / 3, OSP_INT3, &index[0]); // OSP_INT4 format is also supported for triangle indices
      ospCommit(data);
      ospSetData(mesh, "index", data);
      ospCommit(mesh);

      meshes_.push_back(mesh);
      ospAddGeometry(world_, mesh);
      ospCommit(world_);
    }

    void addStreamline(FieldHandle field)
    {
      adjustCameraPosition(field);

      fillDataBuffers(field, nullptr);

      auto& fieldData = fieldData_.back();
      const auto& vertex = fieldData.vertex;
      const auto& color = fieldData.color;

      auto& index = fieldData.index;
      {
        auto facade(field->mesh()->getFacade());
        for (const auto& edge : facade->edges())
        {
          auto nodesFromEdge = edge.nodeIndices();
          index.push_back(nodesFromEdge[0]);
        }
      }

      OSPGeometry streamlines = ospNewGeometry("streamlines");
      OSPData data = ospNewData(vertex.size() / 4, OSP_FLOAT3A, &vertex[0]);
      ospCommit(data);
      ospSetData(streamlines, "vertex", data);

      data = ospNewData(color.size() / 4, OSP_FLOAT4, &color[0]);
      ospCommit(data);
      ospSetData(streamlines, "vertex.color", data);

      data = ospNewData(index.size(), OSP_INT, &index[0]);
      ospCommit(data);
      ospSetData(streamlines, "index", data);

      ospSet1f(streamlines, "radius", toFloat(Parameters::StreamlineRadius));

      ospCommit(streamlines);

      meshes_.push_back(streamlines);
      ospAddGeometry(world_, streamlines);
      ospCommit(world_);
    }

    void render()
    {
      renderer_ = ospNewRenderer("scivis"); // choose Scientific Visualization renderer

      // create and setup light for Ambient Occlusion
      OSPLight light = ospNewLight(renderer_, state_->getValue(Parameters::LightType).toString().c_str());
      OSPData lights;
      if (light)
      {
        float lightColor[] = { toFloat(Parameters::LightColorR), toFloat(Parameters::LightColorG), toFloat(Parameters::LightColorB) };
        ospSet3fv(light, "color", lightColor);
        ospSet1f(light, "intensity", toFloat(Parameters::LightIntensity));
        ospSet1i(light, "isVisible", state_->getValue(Parameters::LightVisible).toBool() ? 1 : 0);
        ospCommit(light);
        lights = ospNewData(1, OSP_LIGHT, &light);
        ospCommit(lights);
      }

      //material
      OSPMaterial material = ospNewMaterial(renderer_, "OBJMaterial");
      ospSet3f(material, "Kd", 0.2f, 0.2f, 0.2f);
      ospSet3f(material, "Ks", 0.4f, 0.4f, 0.4f);
      ospSet1f(material, "Ns", 100.0f);
      ospCommit(renderer_);

      // complete setup of renderer
      ospSet1i(renderer_, "aoSamples", 1);
      ospSet3f(renderer_, "bgColor", toFloat(Parameters::BackgroundColorR),
        toFloat(Parameters::BackgroundColorG),
        toFloat(Parameters::BackgroundColorB));
      ospSetObject(renderer_, "model", world_);
      ospSetObject(renderer_, "camera", camera_);
      if (light)
        ospSetObject(renderer_, "lights", lights);
      ospSetObject(renderer_, "material", material);
      ospCommit(renderer_);

      // create and setup framebuffer
      framebuffer_ = ospNewFrameBuffer(imgSize_, OSP_FB_SRGBA, OSP_FB_COLOR | /*OSP_FB_DEPTH |*/ OSP_FB_ACCUM);
      ospFrameBufferClear(framebuffer_, OSP_FB_COLOR | OSP_FB_ACCUM);

      // render one frame
      ospRenderFrame(framebuffer_, renderer_, OSP_FB_COLOR | OSP_FB_ACCUM);

      const int frameCount = state_->getValue(Parameters::FrameCount).toInt();
      // render N more frames, which are accumulated to result in a better converged image
      for (int frames = 0; frames < frameCount-1; frames++)
        ospRenderFrame(framebuffer_, renderer_, OSP_FB_COLOR | OSP_FB_ACCUM);
    }

    void writeImage(const std::string& filename)
    {
      // access framebuffer and write its content as PPM file
      const uint32_t * fb = (uint32_t*)ospMapFrameBuffer(framebuffer_, OSP_FB_COLOR);
      writePPM(filename.c_str(), imgSize_, fb);
      ospUnmapFrameBuffer(fb, framebuffer_);
    }
  private:
    void writePPM(const char *fileName, const osp::vec2i &size, const uint32_t *pixel) const
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
    }
  };

  bool OsprayImpl::initialized_(false);
  Core::Thread::Mutex OsprayImpl::lock_("ospray lock");

  #else
  class OsprayImpl {};
  #endif
}

InterfaceWithOspray::InterfaceWithOspray() : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(Field);
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(Streamlines);
  INITIALIZE_PORT(SceneGraph);
}

void InterfaceWithOspray::execute()
{
  #ifdef WITH_OSPRAY
  auto fields = getOptionalDynamicInputs(Field);
  auto colorMaps = getOptionalDynamicInputs(ColorMapObject);
  auto streamlines = getOptionalDynamicInputs(Streamlines);

  if (needToExecute())
  {
    detail::OsprayImpl ospray(get_state());
    ospray.setup();

    if (colorMaps.size() < fields.size())
      colorMaps.resize(fields.size());

    for (auto&& fieldColor : zip(fields, colorMaps))
    {
      FieldHandle field;
      ColorMapHandle color;
      boost::tie(field, color) = fieldColor;

      FieldInformation info(field);

      if (!info.is_trisurfmesh())
        THROW_INVALID_ARGUMENT("Module currently only works with trisurfs.");

      ospray.addField(field, color);
    }

    for (auto& streamline : streamlines)
    {
      FieldInformation info(streamline);

      if (!info.is_curvemesh())
        THROW_INVALID_ARGUMENT("Module currently only works with curvemesh streamlines.");

      ospray.addStreamline(streamline);
    }

    ospray.render();

    auto isoString = boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::universal_time());
    auto filename = "scirunOsprayOutput_" + isoString + ".ppm";
    auto filePath = get_state()->getValue(Variables::Filename).toString() / boost::filesystem::path(filename);
    ospray.writeImage(filePath.string());
    remark("Saving output to " + filePath.string());

    get_state()->setTransientValue(Variables::Filename, filePath.string());

    //auto geom = builder_->buildGeometryObject(field, colorMap, *this, this);
    //sendOutput(SceneGraph, geom);
  }
  #else
  error("Build SCIRun with WITH_OSPRAY set to true to enable this module.");
  #endif
}
