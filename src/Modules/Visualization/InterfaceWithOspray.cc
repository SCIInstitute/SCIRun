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
// #include <Core/Datatypes/Color.h>
#include <Core/Datatypes/ColorMap.h>
// #include <Core/GeometryPrimitives/Vector.h>
// #include <Core/GeometryPrimitives/Tensor.h>
// #include <Graphics/Glyphs/GlyphGeom.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>

#include <ospray/ospray.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Core;
using namespace Datatypes;
// using namespace Thread;
// using namespace Dataflow::Networks;
// using namespace Algorithms;
// using namespace Visualization;
// using namespace Geometry;
// using namespace Graphics;
// using namespace Graphics::Datatypes;

// ALGORITHM_PARAMETER_DEF(Visualization, CylinderRadius);

MODULE_INFO_DEF(InterfaceWithOspray, Visualization, SCIRun)

#if 0

  float faceTransparencyValue_ = 0.65f;
  float edgeTransparencyValue_ = 0.65f;
  float nodeTransparencyValue_ = 0.65f;
  std::string moduleId_;
  ModuleStateHandle state_;

#endif

void InterfaceWithOspray::setStateDefaults()
{
  // auto state = get_state();
  // state->setValue(NodesAvailable, true);
  // state->setValue(EdgesAvailable, true);
  // state->setValue(FacesAvailable, true);
  //
  // state->setValue(ShowNodes, false);
  // state->setValue(ShowEdges, true);
  // state->setValue(ShowFaces, true);
  // state->setValue(NodeTransparency, false);
  // state->setValue(EdgeTransparency, false);
  // state->setValue(FaceTransparency, false);
  // state->setValue(DefaultMeshColor, ColorRGB(0.5, 0.5, 0.5).toString());
  //
  // //state->setValue(NodeAsPoints, true); //not used
  // state->setValue(NodeAsSpheres, 0);
  // //state->setValue(EdgesAsLines, true); //not used
  // state->setValue(EdgesAsCylinders, 0);
  // state->setValue(FaceTransparencyValue, 0.65f);
  // state->setValue(EdgeTransparencyValue, 0.65f);
  // state->setValue(NodeTransparencyValue, 0.65f);
  // state->setValue(FacesColoring, 1);
  // state->setValue(NodesColoring, 1);
  // state->setValue(EdgesColoring, 1);
  // state->setValue(SphereScaleValue, 0.03);
  // state->setValue(SphereResolution, 5);
  // state->setValue(CylinderRadius, 0.1);
  // state->setValue(CylinderResolution, 5);
  //
  // state->setValue(DefaultTextColor, ColorRGB(1.0, 1.0, 1.0).toString());
  // state->setValue(ShowText, false);
  // state->setValue(ShowDataValues, true);
  // state->setValue(ShowNodeIndices, false);
  // state->setValue(ShowEdgeIndices, false);
  // state->setValue(ShowFaceIndices, false);
  // state->setValue(ShowCellIndices, false);
  // state->setValue(CullBackfacingText, false);
  // state->setValue(TextAlwaysVisible, false);
  // state->setValue(RenderAsLocation, false);
  // state->setValue(TextSize, 8);
  // state->setValue(TextPrecision, 3);
  // state->setValue(TextColoring, 0);
  //
  // state->setValue(UseFaceNormals, false);
  // state->setValue(FaceInvertNormals, false);
  //
  // state->setValue(FieldName, std::string());
  //
  // // NOTE: We need to add radio buttons for USE_DEFAULT_COLOR, COLORMAP, and
  // // COLOR_CONVERT. USE_DEFAULT_COLOR is selected by default. COLOR_CONVERT
  // // is more up in the air.
}

namespace detail
{
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

    void writeImage(FieldHandle field, const std::string& filename, float cameraSteps, boost::optional<ColorMapHandle> colorMap)
    {
      auto facade(field->mesh()->getFacade());

      std::cout << "hello ospray" << std::endl;
      // image size
      osp::vec2i imgSize;
      imgSize.x = 1024; // width
      imgSize.y = 768; // height

      // camera
      float cam_pos[] = { -1.f, 0.f, -5.f };
      cam_pos[0] += cameraSteps;
      float cam_up[] = { 0.f, 1.f, 0.f };
      float cam_view[] = { 0.5f, 0.5f, 1.f };

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

          ColorRGB nodeColor(0.6, 0.2, 0.2);
          if (map)
          {
            nodeColor = map->valueToColor(value);
          }
          color.push_back(nodeColor.r());
          color.push_back(nodeColor.g());
          color.push_back(nodeColor.b());
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
      //OSPData data = ospNewData(4, OSP_FLOAT3A, vertex_example); // OSP_FLOAT3 format is also supported for vertex positions
      ospCommit(data);
      ospSetData(mesh, "vertex", data);

      data = ospNewData(vertex.size() / 4, OSP_FLOAT4, &color[0]);
      //data = ospNewData(4, OSP_FLOAT4, color_example);
      ospCommit(data);
      ospSetData(mesh, "vertex.color", data);

      data = ospNewData(index.size() / 3, OSP_INT3, &index[0]); // OSP_INT4 format is also supported for triangle indices
      //data = ospNewData(2, OSP_INT3, index_example); // OSP_INT4 format is also supported for triangle indices
      ospCommit(data);
      ospSetData(mesh, "index", data);

      ospCommit(mesh);

      OSPModel world = ospNewModel();
      ospAddGeometry(world, mesh);
      ospCommit(world);

      // create renderer
      OSPRenderer renderer = ospNewRenderer("scivis"); // choose Scientific Visualization renderer

      // create and setup light for Ambient Occlusion
      OSPLight light = ospNewLight(renderer, "ambient");
      ospCommit(light);
      OSPData lights = ospNewData(1, OSP_LIGHT, &light);
      ospCommit(lights);

      // complete setup of renderer
      ospSet1i(renderer, "aoSamples", 1);
      ospSet1f(renderer, "bgColor", 1.0f); // white, transparent
      ospSetObject(renderer, "model", world);
      ospSetObject(renderer, "camera", camera);
      ospSetObject(renderer, "lights", lights);
      ospCommit(renderer);

      // create and setup framebuffer
      OSPFrameBuffer framebuffer = ospNewFrameBuffer(imgSize, OSP_FB_SRGBA, OSP_FB_COLOR | /*OSP_FB_DEPTH |*/ OSP_FB_ACCUM);
      ospFrameBufferClear(framebuffer, OSP_FB_COLOR | OSP_FB_ACCUM);

      // render one frame
      ospRenderFrame(framebuffer, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);

      // access framebuffer and write its content as PPM file
      const uint32_t * fb = (uint32_t*)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR);
      ospUnmapFrameBuffer(fb, framebuffer);

      // render 10 more frames, which are accumulated to result in a better converged image
      for (int frames = 0; frames < 10; frames++)
        ospRenderFrame(framebuffer, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);

      fb = (uint32_t*)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR);

      writePPM(filename.c_str(), imgSize, fb);
      ospUnmapFrameBuffer(fb, framebuffer);
    }
  private:
    void writePPM(const char *fileName, const osp::vec2i &size, const uint32_t *pixel)
    {
      FILE *file = fopen(fileName, "wb");
      fprintf(file, "P6\n%i %i\n255\n", size.x, size.y);
      unsigned char *out = (unsigned char *)alloca(3 * size.x);
      for (int y = 0; y < size.y; y++) {
        const unsigned char *in = (const unsigned char *)&pixel[(size.y - 1 - y)*size.x];
        for (int x = 0; x < size.x; x++) {
          out[3 * x + 0] = in[4 * x + 0];
          out[3 * x + 1] = in[4 * x + 1];
          out[3 * x + 2] = in[4 * x + 2];
        }
        fwrite(out, 3 * size.x, sizeof(char), file);
      }
      fprintf(file, "\n");
      fclose(file);
      std::cout << "wrote file " << fileName << std::endl;
    }
  };
}

InterfaceWithOspray::InterfaceWithOspray() : GeometryGeneratingModule(staticInfo_), impl_(new detail::OsprayImpl)
{
  INITIALIZE_PORT(Field);
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(SceneGraph);
}

void InterfaceWithOspray::execute()
{
  auto field = getRequiredInput(Field);
  auto colorMap = getOptionalInput(ColorMapObject);

  if (needToExecute())
  {
    // for (int inc : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10})
    // {
    //   osprayImpl::renderLatVol(latVol, inc*0.2, GetParam());
    // }
    impl_->writeImage(field, "scirunOsprayOutput.ppm", 0.2, colorMap);

    //updateAvailableRenderOptions(field);
    //auto geom = builder_->buildGeometryObject(field, colorMap, *this, this);
    //sendOutput(SceneGraph, geom);
  }
}

#if 0

const AlgorithmParameterName ShowField::FieldName("FieldName");
const AlgorithmParameterName ShowField::ShowNodes("ShowNodes");
const AlgorithmParameterName ShowField::ShowEdges("ShowEdges");
const AlgorithmParameterName ShowField::ShowFaces("ShowFaces");
const AlgorithmParameterName ShowField::NodesAvailable("NodesAvailable");
const AlgorithmParameterName ShowField::EdgesAvailable("EdgesAvailable");
const AlgorithmParameterName ShowField::FacesAvailable("FacesAvailable");
const AlgorithmParameterName ShowField::NodeTransparency("NodeTransparency");
const AlgorithmParameterName ShowField::EdgeTransparency("EdgeTransparency");
const AlgorithmParameterName ShowField::FaceTransparency("FaceTransparency");
const AlgorithmParameterName ShowField::FaceInvertNormals("FaceInvertNormals");
const AlgorithmParameterName ShowField::NodeAsPoints("NodeAsPoints");
const AlgorithmParameterName ShowField::NodeAsSpheres("NodeAsSpheres");
const AlgorithmParameterName ShowField::EdgesAsLines("EdgesAsLines");
const AlgorithmParameterName ShowField::EdgesAsCylinders("EdgesAsCylinders");
const AlgorithmParameterName ShowField::DefaultMeshColor("DefaultMeshColor");
const AlgorithmParameterName ShowField::FaceTransparencyValue("FaceTransparencyValue");
const AlgorithmParameterName ShowField::EdgeTransparencyValue("EdgeTransparencyValue");
const AlgorithmParameterName ShowField::NodeTransparencyValue("NodeTransparencyValue");
const AlgorithmParameterName ShowField::FacesColoring("FacesColoring");
const AlgorithmParameterName ShowField::NodesColoring("NodesColoring");
const AlgorithmParameterName ShowField::EdgesColoring("EdgesColoring");
const AlgorithmParameterName ShowField::SphereScaleValue("SphereScaleValue");
const AlgorithmParameterName ShowField::CylinderRadius("CylinderRadius");
const AlgorithmParameterName ShowField::CylinderResolution("CylinderResolution");
const AlgorithmParameterName ShowField::SphereResolution("SphereResolution");
const AlgorithmParameterName ShowField::DefaultTextColor("DefaultTextColor");
const AlgorithmParameterName ShowField::ShowText("ShowText");
const AlgorithmParameterName ShowField::ShowDataValues("ShowDataValues");
const AlgorithmParameterName ShowField::ShowNodeIndices("ShowNodeIndices");
const AlgorithmParameterName ShowField::ShowEdgeIndices("ShowEdgeIndices");
const AlgorithmParameterName ShowField::ShowFaceIndices("ShowFaceIndices");
const AlgorithmParameterName ShowField::ShowCellIndices("ShowCellIndices");
const AlgorithmParameterName ShowField::CullBackfacingText("CullBackfacingText");
const AlgorithmParameterName ShowField::TextAlwaysVisible("TextAlwaysVisible");
const AlgorithmParameterName ShowField::RenderAsLocation("RenderAsLocation");
const AlgorithmParameterName ShowField::TextSize("TextSize");
const AlgorithmParameterName ShowField::TextPrecision("TextPrecision");
const AlgorithmParameterName ShowField::TextColoring("TextColoring");
const AlgorithmParameterName ShowField::UseFaceNormals("UseFaceNormals");
#endif
