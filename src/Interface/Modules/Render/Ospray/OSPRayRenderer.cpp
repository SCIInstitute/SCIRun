/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

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

#include "OSPRayRenderer.h"
#include <Core/GeometryPrimitives/BBox.h>

#ifdef __APPLE__
  #define GL_SILENCE_DEPRECATION
  #include <OpenGL/glu.h>
#else
#ifdef _WIN32
  #include <gl-platform/GLPlatform.hpp>
#else
  #include <GL/glu.h>
#endif
#endif

#include <cstdio>

using namespace SCIRun;
using namespace Render;
using namespace Core::Datatypes;
using namespace Core::Geometry;

#ifdef WITH_OSPRAY
//int OSPRayRenderer::osprayRendererInstances = 0;
OSPRayDataManager OSPRayRenderer::dataManager;

OSPRayRenderer::OSPRayRenderer()
{
  frameBuffer_ = ospNewFrameBuffer(width_, height_, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
  renderer_ = ospNewRenderer("scivis");
  camera_ = new OSPRayCamera();
  world_ = ospNewWorld();

  float backgroundColor[] = {0.0, 0.0, 0.0};
  ospSetParam(renderer_, "backgroundColor", OSP_VEC3F, backgroundColor);
  ospCommit(renderer_);
  lights_.clear();

  // TODO make these into UI parameters
  auto white = glm::vec3(1,1,1);
  auto light_dir = glm::vec3(0,-1,-1);
  double ambient_intensity = 0.1;
  addDirectionalLight(white, light_dir);
  addAmbientLight(white, ambient_intensity);
  // addSphereLight(white, glm::vec3(0,0,60), 0.2, 500.0);
  // addQuadLight(white, glm::vec3(0,45,0), glm::vec3(25,0,0), glm::vec3(0,0,25), 2);
  setLightsAsObject();
}

OSPRayRenderer::~OSPRayRenderer()
{
  if(frameBuffer_) ospRelease(frameBuffer_);
  if(renderer_) ospRelease(renderer_);
  if(camera_) delete camera_;
  if(world_) ospRelease(world_);
}

//Rendering-----------------------------------------------------------------------------------------
void OSPRayRenderer::renderFrame()
{
  if(framesAccumulated == 0)
    parentCamera_ = camera_->getOSPCamera();
  if(framesAccumulated < 64)
  {
    OSPFuture fut = ospRenderFrame(frameBuffer_, renderer_, parentCamera_, world_);
    ospWait(fut);
    ++framesAccumulated;
  }

  uint32_t* mappedFrameBuffer = (uint32_t*)ospMapFrameBuffer(frameBuffer_);
  glDrawPixels(width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, mappedFrameBuffer);
  ospUnmapFrameBuffer(mappedFrameBuffer, frameBuffer_);
}



//Interaction---------------------------------------------------------------------------------------
void OSPRayRenderer::resize(uint32_t width, uint32_t height)
{
  width_ = width;
  this->height_ = height;

  if(frameBuffer_) ospRelease(frameBuffer_);
  frameBuffer_ = ospNewFrameBuffer(width_, height_, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM | OSP_FB_VARIANCE);
  framesAccumulated = 0;

  camera_->setAspect(static_cast<float>(width_) / height_);
}

void OSPRayRenderer::mousePress(float x, float y, MouseButton btn)
{
  camera_->mousePress(x, y, btn);
}

void OSPRayRenderer::mouseMove(float x, float y, MouseButton btn)
{
  camera_->mouseMove(x, y, btn);
  ospResetAccumulation(frameBuffer_);
  framesAccumulated = 0;
}

void OSPRayRenderer::mouseRelease()
{
  camera_->mouseRelease();
}

void OSPRayRenderer::mouseWheel(int delta)
{
  camera_->mouseWheel(delta);
  ospResetAccumulation(frameBuffer_);
  framesAccumulated = 0;
}

void OSPRayRenderer::autoView()
{
  camera_->autoView();
  ospResetAccumulation(frameBuffer_);
  framesAccumulated = 0;
}



//Data----------------------------------------------------------------------------------------------
void OSPRayRenderer::updateGeometries(const std::vector<OsprayGeometryObjectHandle>& geometries)
{
  BBox bbox;
  for(auto& geometry : geometries)
  {
    bbox.extend(geometry->box);
    //printf("ID: %d\n", geometry->id);
    switch(geometry->type)
    {
      case GeometryType::TRI_SURFACE:
      {
        //printf("TRI_SURFACE\n");
        addMeshToGroup(&*geometry, 3);
        addInstaceOfGroup();
        break;
      }
      case GeometryType::QUAD_SURFACE:
      {
        //printf("QUAD_SURFACE\n");
        addMeshToGroup(&*geometry, 4);
        addInstaceOfGroup();
        break;
      }
      case GeometryType::STRUCTURED_VOLUME:
      {
        //printf("STRUCTURED_VOLUME\n");
        addStructuredVolumeToGroup(&*geometry);
        addInstaceOfGroup();
        break;
      }
      default:
      {
        //printf("NOT_SUPPORTED\n");
        break;
      }
    }
  }
  camera_->setSceneBoundingBox(bbox);

  ospResetAccumulation(frameBuffer_);
  framesAccumulated = 0;
  ospCommit(frameBuffer_);
}

void OSPRayRenderer::addInstaceOfGroup()
{
  OSPInstance instance = ospNewInstance(group_);
  ospCommit(instance);

  ospSetObjectAsData(world_, "instance", OSP_INSTANCE, instance);
  ospCommit(world_);
  ospRelease(instance);
}

void OSPRayRenderer::addGroup()
{
  if(group_) ospRelease(group_);
  group_ = ospNewGroup();
}

void OSPRayRenderer::addMaterial(OSPGeometricModel model, OsprayGeometryObject::Material& mat)
{
  OSPMaterial material;
  if(isScivis)
  {
    float ks[] = {0.8, 0.8, 0.8};
    float ns = (1.0f - mat.roughness);
    ns = ns * ns * 20.0f + 2.0f;
    material = ospNewMaterial("scivis", "obj");
    ospSetParam(material, "kd", OSP_VEC3F, mat.albedo);
    ospSetParam(material, "ks", OSP_VEC3F, ks);
    ospSetParam(material, "ns", OSP_FLOAT, &ns);
    ospSetParam(material, "d", OSP_FLOAT, &mat.opacity);
    ospCommit(material);
  }
  else
  {
    printf("MATERIAL NOT SUPPORTED!\n");
    return;
  }

  ospSetObject(model, "material", material);
  ospCommit(model);
  ospRelease(material);
}

void OSPRayRenderer::addTransferFunction(OSPVolumetricModel model, OsprayGeometryObject::TransferFunc& tfn)
{
  size_t numColors = tfn.colors.size()/3;
  OSPData colorDataTemp = ospNewSharedData(tfn.colors.data(), OSP_VEC3F, numColors);
  OSPData colorData = ospNewData(OSP_VEC3F, numColors);
  ospCopyData(colorDataTemp, colorData);
  ospRelease(colorDataTemp);

  OSPData opacityDataTemp = ospNewSharedData(tfn.opacities.data(), OSP_FLOAT, tfn.opacities.size());
  OSPData opacityData = ospNewData(OSP_FLOAT, tfn.opacities.size());
  ospCopyData(opacityDataTemp, opacityData);
  ospRelease(opacityDataTemp);

  OSPTransferFunction transferFunction = ospNewTransferFunction("piecewiseLinear");
  ospSetParam(transferFunction, "valueRange", OSP_VEC2F, tfn.range.data());
  ospSetParam(transferFunction, "color", OSP_DATA, &colorData);
  ospSetParam(transferFunction, "opacity", OSP_DATA, &opacityData);
  ospCommit(transferFunction);
  ospRelease(colorData);
  ospRelease(opacityData);

  ospSetObject(model, "transferFunction", transferFunction);
  ospCommit(model);
  ospRelease(transferFunction);
}

void OSPRayRenderer::addMeshToGroup(OsprayGeometryObject* geometryObject, uint32_t vertsPerPoly)
{
  if(!group_) addGroup();

  OSPGeometry geometry = dataManager.updateAndGetMesh(geometryObject, vertsPerPoly);

  OSPGeometricModel model = ospNewGeometricModel(geometry);
  addMaterial(model, geometryObject->material); //also commits changes

  ospSetObjectAsData(group_, "geometry", OSP_GEOMETRIC_MODEL, model);
  ospCommit(group_);
  ospRelease(model);
}

void OSPRayRenderer::addStructuredVolumeToGroup(OsprayGeometryObject* geometryObject)
{
  if(!group_) addGroup();

  OSPVolume volume = dataManager.updateAndgetStructuredVolume(geometryObject);

  OSPVolumetricModel model = ospNewVolumetricModel(volume);
  addTransferFunction(model, geometryObject->tfn); //also commits changes

  ospSetObjectAsData(group_, "volume", OSP_VOLUMETRIC_MODEL, model);
  ospCommit(group_);
  ospRelease(model);
}

void OSPRayRenderer::addDirectionalLight(glm::vec3 col, glm::vec3 dir)
{
  OSPLight light = ospNewLight("distant");
  ospSetParam(light, "color", OSP_VEC3F, &col);
  ospSetParam(light, "direction", OSP_VEC3F, &dir);
  ospCommit(light);
  lights_.push_back(light);
}

void OSPRayRenderer::addAmbientLight(glm::vec3 col, float intensity)
{
  OSPLight light = ospNewLight("ambient");
  ospSetParam(light, "color", OSP_VEC3F, &col);
  ospSetParam(light, "intensity", OSP_FLOAT, &intensity);
  ospCommit(light);
  lights_.push_back(light);
}

void OSPRayRenderer::addSphereLight(glm::vec3 col, glm::vec3 position, float radius, float intensity)
{
  OSPLight light = ospNewLight("sphere");
  ospSetParam(light, "color", OSP_VEC3F, &col);
  ospSetParam(light, "position", OSP_VEC3F, &position);
  ospSetParam(light, "radius", OSP_FLOAT, &radius);
  ospSetParam(light, "intensity", OSP_FLOAT, &intensity);
  ospCommit(light);
  lights_.push_back(light);
}

void OSPRayRenderer::addQuadLight(glm::vec3 col, glm::vec3 position, glm::vec3 edge1, glm::vec3 edge2, float intensity)
{
  OSPLight light = ospNewLight("quad");
  ospSetParam(light, "color", OSP_VEC3F, &col);
  ospSetParam(light, "position", OSP_VEC3F, &position);
  ospSetParam(light, "edge1", OSP_VEC3F, &edge1);
  ospSetParam(light, "edge2", OSP_VEC3F, &edge2);
  ospSetParam(light, "intensity", OSP_FLOAT, &intensity);
  ospCommit(light);
  lights_.push_back(light);
}

void OSPRayRenderer::setLightsAsObject()
{
  auto data = ospNewSharedData(lights_.data(), OSP_LIGHT, lights_.size());
  ospSetObject(world_, "light", data);
  for (auto light : lights_)
    ospRelease(light);
  ospCommit(world_);
}

#endif
