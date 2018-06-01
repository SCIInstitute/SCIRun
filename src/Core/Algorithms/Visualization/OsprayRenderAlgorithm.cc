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

#include <Core/Algorithms/Visualization/OsprayRenderAlgorithm.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace SCIRun;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Visualization;
using namespace Core::Datatypes;

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

#ifdef WITH_OSPRAY

#include <ospray/ospray.h>

namespace detail
{
  class OsprayImpl
  {
  private:
    static bool initialized_;
    static Core::Thread::Mutex lock_;
    static void initialize(AlgorithmBase* algo);

    Core::Thread::Guard guard_;
    Core::Geometry::BBox imageBox_;
    osp::vec2i imgSize_;
    OSPCamera camera_;
    OSPModel world_;
    std::vector<OSPGeometry> meshes_;
    OSPRenderer renderer_;
    OSPFrameBuffer framebuffer_;
    AlgorithmBase* algo_;


    float toFloat(const Name& name) const;
    OsprayGeometryObjectHandle makeObject(FieldHandle field);
    std::array<float, 3> toArray(const Vector& v) const;
    void writePPM(const char *fileName, const osp::vec2i &size, const uint32_t *pixel) const;
    OsprayGeometryObjectHandle fillDataBuffers(FieldHandle field, ColorMapHandle colorMap);
    void visualizeScalarField(OsprayGeometryObjectHandle obj);
    void visualizeStreamline(OsprayGeometryObjectHandle obj);
    void adjustCameraPosition(const BBox& bbox);
    Vector getCameraUp(float* newDir, float* cam_up);

  public:
    explicit OsprayImpl(AlgorithmBase* algo);
    void setup();
    void render(const CompositeOsprayGeometryObject& objList);
    void writeImage(const std::string& filename);
  };


  void OsprayImpl::initialize(AlgorithmBase* algo)
  {
    if (!initialized_)
    {
      const char* argv[] = { "" };
      int argc = 0;
      int init_error = ospInit(&argc, argv);
      if (init_error != OSP_NO_ERROR)
      {
        THROW_ALGORITHM_INPUT_ERROR_WITH(algo, "Ospray initialization error code: " + std::to_string(init_error));
      }
      auto device = ospGetCurrentDevice();
      ospDeviceSetErrorMsgFunc(device, [](const char *msg) { std::cerr << msg << std::endl; });
      initialized_ = true;
    }
  }

  float OsprayImpl::toFloat(const Name& name) const
  {
    return static_cast<float>(algo_->get(name).toDouble());
  }

  std::array<float, 3> OsprayImpl::toArray(const Vector& v) const
  {
    return { static_cast<float>(v.x()), static_cast<float>(v.y()), static_cast<float>(v.z()) };
  }

  OsprayImpl::OsprayImpl(AlgorithmBase* algo) : guard_(lock_.get()), algo_(algo)
  {
    initialize(algo_);
  }

  void OsprayImpl::setup()
  {
    imgSize_.x = algo_->get(Parameters::ImageWidth).toInt();
    imgSize_.y = algo_->get(Parameters::ImageHeight).toInt();

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

  void OsprayImpl::adjustCameraPosition(const BBox& bbox)
  {
    if (algo_->get(Parameters::AutoCameraView).toBool())
    {
      imageBox_.extend(bbox);
      auto center = imageBox_.center();
      float position[] = { toFloat(Parameters::CameraPositionX), toFloat(Parameters::CameraPositionY), toFloat(Parameters::CameraPositionZ) };
      float cam_up[] = { toFloat(Parameters::CameraUpX), toFloat(Parameters::CameraUpY), toFloat(Parameters::CameraUpZ) };
      float newDir[] = { static_cast<float>(center.x()) - position[0],
          static_cast<float>(center.y()) - position[1],
          static_cast<float>(center.z()) - position[2]};

      algo_->set(Parameters::CameraViewX, center.x());
      algo_->set(Parameters::CameraViewY, center.y());
      algo_->set(Parameters::CameraViewZ, center.z());
      ospSet3fv(camera_, "dir", newDir);
      auto newUp = getCameraUp(newDir, cam_up);
      algo_->set(Parameters::CameraUpX, newUp.x());
      algo_->set(Parameters::CameraUpY, newUp.y());
      algo_->set(Parameters::CameraUpZ, newUp.z());
      ospSet3fv(camera_, "up", &toArray(newUp)[0]);
      ospCommit(camera_);
    }
  }

  Vector OsprayImpl::getCameraUp(float* newDir, float* cam_up)
  {
    Vector side(newDir[1]*cam_up[2] - newDir[2]*cam_up[1],
      newDir[2]*cam_up[0] - newDir[0]*cam_up[2],
      newDir[0]*cam_up[1] - newDir[1]*cam_up[0]);
    auto norm_side = side.length();
    if (norm_side <= 1e-3)
    {
      side = Vector(newDir[1], -newDir[0], 0.0);
      norm_side = side.length();
      if (norm_side <= 1e-3)
      {
        side = Vector(-newDir[2], 0.0, newDir[0]);
        norm_side = side.length();
      }
    }
    side /= norm_side;

    return Vector(side[1]*newDir[2] - side[2]*newDir[1],
      side[2]*newDir[0] - side[0]*newDir[2],
      side[0]*newDir[1] - side[1]*newDir[0]);
  }


  void OsprayImpl::visualizeScalarField(OsprayGeometryObjectHandle obj)
  {
    const auto& fieldData = obj->data;
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

  void OsprayImpl::visualizeStreamline(OsprayGeometryObjectHandle obj)
  {
    auto& fieldData = obj->data;
    const auto& vertex = fieldData.vertex;
    const auto& color = fieldData.color;

    auto& index = fieldData.index;

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

  void OsprayImpl::render(const CompositeOsprayGeometryObject& objList)
  {
    for (auto& obj : objList.objects())
    {
      if (obj->isStreamline)
        visualizeStreamline(obj);
      else
        visualizeScalarField(obj);

      adjustCameraPosition(obj->box);
    }
    renderer_ = ospNewRenderer("scivis"); // choose Scientific Visualization renderer

    // create and setup light for Ambient Occlusion
    OSPLight light = ospNewLight(renderer_, algo_->get(Parameters::LightType).toString().c_str());
    OSPData lights;
    if (light)
    {
      float lightColor[] = { toFloat(Parameters::LightColorR), toFloat(Parameters::LightColorG), toFloat(Parameters::LightColorB) };
      ospSet3fv(light, "color", lightColor);
      ospSet1f(light, "intensity", toFloat(Parameters::LightIntensity));
      ospSet1i(light, "isVisible", algo_->get(Parameters::LightVisible).toBool() ? 1 : 0);
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

    const int frameCount = algo_->get(Parameters::FrameCount).toInt();
    // render N more frames, which are accumulated to result in a better converged image
    for (int frames = 0; frames < frameCount-1; frames++)
      ospRenderFrame(framebuffer_, renderer_, OSP_FB_COLOR | OSP_FB_ACCUM);
  }

  void OsprayImpl::writeImage(const std::string& filename)
  {
    // access framebuffer and write its content as PPM file
    const uint32_t * fb = (uint32_t*)ospMapFrameBuffer(framebuffer_, OSP_FB_COLOR);
    writePPM(filename.c_str(), imgSize_, fb);
    ospUnmapFrameBuffer(fb, framebuffer_);
  }

  void OsprayImpl::writePPM(const char *fileName, const osp::vec2i &size, const uint32_t *pixel) const
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

  bool OsprayImpl::initialized_(false);
  Core::Thread::Mutex OsprayImpl::lock_("ospray lock");
}
#endif

OsprayRenderAlgorithm::OsprayRenderAlgorithm()
{
  addParameter(Parameters::ImageHeight, 768);
  addParameter(Parameters::ImageWidth, 1024);
  addParameter(Parameters::CameraPositionX, 5.0);
  addParameter(Parameters::CameraPositionY, 5.0);
  addParameter(Parameters::CameraPositionZ, 5.0);
  addParameter(Parameters::CameraUpX, 0.0);
  addParameter(Parameters::CameraUpY, 0.0);
  addParameter(Parameters::CameraUpZ, 1.0);
  addParameter(Parameters::CameraViewX, 0.0);
  addParameter(Parameters::CameraViewY, 0.0);
  addParameter(Parameters::CameraViewZ, 0.0);
  addParameter(Parameters::BackgroundColorR, 0.0);
  addParameter(Parameters::BackgroundColorG, 0.0);
  addParameter(Parameters::BackgroundColorB, 0.0);
  addParameter(Parameters::FrameCount, 10);
  addParameter(Parameters::ShowImageInWindow, true);
  addParameter(Parameters::LightColorR, 1.0);
  addParameter(Parameters::LightColorG, 1.0);
  addParameter(Parameters::LightColorB, 1.0);
  addParameter(Parameters::LightIntensity, 1.0);
  addParameter(Parameters::LightVisible, false);
  addParameter(Parameters::LightType, std::string("ambient"));
  addParameter(Parameters::AutoCameraView, true);
  addParameter(Parameters::StreamlineRadius, 0.1);
  addParameter(Variables::Filename, std::string(""));

#ifdef WITH_OSPRAY
  impl_.reset(new detail::OsprayImpl(this));
  impl_->setup();
#endif
}

#ifdef WITH_OSPRAY
void OsprayRenderAlgorithm::render(const AlgorithmInput& input) const
{
  auto geoms = input.getList<OsprayGeometryObject>(Name("OspraySceneGraph"));

  for (auto& geom : geoms)
  {
    auto g = boost::dynamic_pointer_cast<CompositeOsprayGeometryObject>(geom);
    if (g)
      impl_->render(*g);
  }
}

std::string OsprayRenderAlgorithm::writeImage() const
{
  auto isoString = boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::universal_time());
  auto filename = "scirunOsprayOutput_" + isoString + ".ppm";
  auto filePath = get(Variables::Filename).toString() / boost::filesystem::path(filename);
  auto filePathStr = filePath.string();
  remark("Saving output to " + filePathStr);

  impl_->writeImage(filePathStr);
  return filePathStr;
}

AlgorithmOutput OsprayRenderAlgorithm::run(const AlgorithmInput& input) const
{
  render(input);

  StringHandle fileStrObj(new String(writeImage()));
  AlgorithmOutput output;
  output[Variables::Filename] = fileStrObj;
  return output;
}
#else
AlgorithmOutput OsprayRenderAlgorithm::run(const AlgorithmInput& input) const
{
  THROW_ALGORITHM_PROCESSING_ERROR("Must compile WITH_OSPRAY to enable this module.");
}
#endif
