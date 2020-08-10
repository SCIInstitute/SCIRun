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

#ifdef __APPLE__
  #include <OpenGL/glu.h>
#else
  #include <GL/glu.h>
#endif

#include <cstdio>

using namespace SCIRun::Render;
using namespace SCIRun::Core::Datatypes;

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

  float dir[] = {0.0, -1.0, -1.0};
  float col[] = {0.0, 1.0, 1.0};
  OSPLight light = ospNewLight("distant");
  ospSetParam(light, "color", OSP_VEC3F, col);
  ospSetParam(light, "direction", OSP_VEC3F, dir);
  ospCommit(light);

  //ospSetObjectAsData(world_, "light", OSP_LIGHT, light);
  ospCommit(world_);
  ospRelease(light);
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
  if(framesAccumulated < 64)
  {
    OSPFuture fut = ospRenderFrame(frameBuffer_, renderer_, camera_->getOSPCamera(), world_);
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



//Data----------------------------------------------------------------------------------------------
void OSPRayRenderer::updateGeometries(const std::vector<OsprayGeometryObjectHandle>& geometries)
{
  for(auto& geometry : geometries)
  {
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
  printf("\n");

  ospResetAccumulation(frameBuffer_);
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
    printf("scivis mat\n");
    float ks[] = {0.8, 0.8, 0.8};
    float ns = (1.0f - mat.roughness);
    ns = ns * ns * 20.0f + 2.0f;
    material = ospNewMaterial("scivis", "obj");
    ospSetParam(model, "kd", OSP_VEC3F, mat.albedo);
    ospSetParam(model, "ks", OSP_VEC3F, ks);
    ospSetParam(model, "ns", OSP_FLOAT, &ns);
    ospSetParam(model, "d", OSP_FLOAT, &mat.opacity);
    ospCommit(material);
  }
  else
  {
    printf("MATERIAL NOT SUPPORTED!\n");
    return;
  }

  ospSetParam(model, "material", OSP_MATERIAL, &material);
  ospCommit(model);
  ospRelease(material);
}

void OSPRayRenderer::addTransferFunction(OSPVolumetricModel model, OsprayGeometryObject::TransferFunc& tnf)
{
  float valueRange[] = {0.0 , 1.0f};

  size_t numColors = tnf.colors.size()/3;
  OSPData colorDataTemp = ospNewSharedData(tnf.colors.data(), OSP_VEC3F, numColors);
  OSPData colorData = ospNewData(OSP_VEC3F, numColors);
  ospCopyData(colorDataTemp, colorData);
  ospRelease(colorDataTemp);

  OSPData opacityDataTemp = ospNewSharedData(tnf.opacities.data(), OSP_FLOAT, tnf.opacities.size());
  OSPData opacityData = ospNewData(OSP_FLOAT, tnf.opacities.size());
  ospCopyData(opacityDataTemp, opacityData);
  ospRelease(opacityDataTemp);

  OSPTransferFunction transferFunction = ospNewTransferFunction("piecewiseLinear");
  ospSetParam(transferFunction, "valueRange", OSP_VEC2F, valueRange);
  ospSetParam(transferFunction, "color", OSP_DATA, &colorData);
  ospSetParam(transferFunction, "opacity", OSP_DATA, &opacityData);
  ospCommit(transferFunction);
  ospRelease(colorData);
  ospRelease(opacityData);

  ospSetParam(model, "transferFunction", OSP_TRANSFER_FUNCTION, &transferFunction);
  ospCommit(model);
  ospRelease(transferFunction);
}

void OSPRayRenderer::addMeshToGroup(OsprayGeometryObject* geometryObject, uint32_t vertsPerPoly)
{
  if(!group_) addGroup();

  OsprayGeometryObject::FieldData& data = geometryObject->data;

  float* vertices   = data.vertex.size()   > 0 ? data.vertex.data()   : NULL;
  float* colors     = data.color.size()    > 0 ? data.color.data()    : NULL;
  float* normals    = data.normal.size()   > 0 ? data.normal.data()   : NULL;
  float* texCoords  = data.texCoord.size() > 0 ? data.texCoord.data() : NULL;
  uint32_t* indices = data.index.size()    > 0 ? data.index.data()    : NULL;

  uint32_t numVertices = data.vertex.size() / 3;
  size_t numPolygons = data.index.size() / vertsPerPoly;

  OSPGeometry geometry = dataManager.updateAndGetMesh(geometryObject->id, geometryObject->version,
    vertices, normals, colors, texCoords, indices, numVertices, numPolygons, vertsPerPoly);

  OSPGeometricModel model = ospNewGeometricModel(geometry);
  addMaterial(model, geometryObject->material); //also commits changes

  ospSetObjectAsData(group_, "geometry", OSP_GEOMETRIC_MODEL, model);
  ospCommit(group_);
  ospRelease(model);
}

void OSPRayRenderer::addStructuredVolumeToGroup(Core::Datatypes::OsprayGeometryObject* geometryObject)
{
  if(!group_) addGroup();

  OsprayGeometryObject::FieldData& data = geometryObject->data;

  OSPVolume volume = dataManager.updateAndgetStructuredVolume(geometryObject->id, geometryObject->version,
    data.origin, data.spacing, data.dim, data.color.data());

  OSPVolumetricModel model = ospNewVolumetricModel(volume);
  addTransferFunction(model, geometryObject->tfn); //also commits changes

  ospSetObjectAsData(group_, "volume", OSP_VOLUMETRIC_MODEL, model);
  ospCommit(group_);
  ospRelease(model);
}
