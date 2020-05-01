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


int OSPRayRenderer::osprayRendererInstances = 0;

OSPRayRenderer::OSPRayRenderer()
{
  if(!OSPRayRenderer::osprayRendererInstances++) ospInit();

  frameBuffer = ospNewFrameBuffer(width, height, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
  renderer = ospNewRenderer("scivis");
  camera = ospNewCamera("perspective");
  world = ospNewWorld();

  float backgroundColor[] = {0.0, 0.0, 0.0};
  ospSetParam(renderer, "backgroundColor", OSP_VEC3F, backgroundColor);
  ospCommit(renderer);

  OSPLight ambientLight = ospNewLight("ambient");
  ospCommit(ambientLight);
  ospSetObjectAsData(world, "light", OSP_LIGHT, ambientLight);
  ospRelease(ambientLight);

  float fovy = 60.0f;
  float aspect =  width / (float)height;
  float camPos[] = {0.0f, 0.0f, 2.0f};
  float camDir[] = {0.0f, 0.0f, -1.0f};
  float camUp[] = {0.0f, 1.0f, 0.0f};
  ospSetParam(camera, "fovy", OSP_FLOAT, &fovy);
  ospSetParam(camera, "aspect", OSP_FLOAT, &aspect);
  ospSetParam(camera, "position", OSP_VEC3F, camPos);
  ospSetParam(camera, "direction", OSP_VEC3F, camDir);
  ospSetParam(camera, "up", OSP_VEC3F, camUp);
  ospCommit(camera);
}

OSPRayRenderer::~OSPRayRenderer()
{
  if(frameBuffer) ospRelease(frameBuffer);
  if(renderer) ospRelease(renderer);
  if(camera) ospRelease(camera);
  if(world) ospRelease(world);

  if(!--OSPRayRenderer::osprayRendererInstances) ospShutdown();
}

void OSPRayRenderer::renderFrame()
{
  OSPFuture fut = ospRenderFrame(frameBuffer, renderer, camera, world);
  ospWait(fut);

  uint32_t* mappedFrameBuffer = (uint32_t*)ospMapFrameBuffer(frameBuffer);
  glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, mappedFrameBuffer);
  ospUnmapFrameBuffer(mappedFrameBuffer, frameBuffer);
}

void OSPRayRenderer::resize(int width, int height)
{
  this->width = width;
  this->height = height;

  if(frameBuffer) ospRelease(frameBuffer);
  frameBuffer = ospNewFrameBuffer(width, height, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
  ospResetAccumulation(frameBuffer);

  ospSetFloat(camera, "aspect", width / (float)height);
  ospCommit(camera);
}

void OSPRayRenderer::addInstaceOfGroup()
{
  OSPInstance instance = ospNewInstance(group);
  ospCommit(instance);

  ospSetObjectAsData(world, "instance", OSP_INSTANCE, instance);
  ospCommit(world);
  ospRelease(instance);
}

void OSPRayRenderer::addGroup()
{
  if(group) ospRelease(group);
  group = ospNewGroup();
}

void OSPRayRenderer::addModelToGroup(float* vertexPositions, float* vertexColors, uint32_t vertexCount,
  uint32_t* indices, size_t trinagles)
{
  if(!group) addGroup();


  OSPGeometry geometry = ospNewGeometry("mesh");

  OSPData vpos = ospNewSharedData(vertexPositions, OSP_VEC3F, vertexCount);
  ospCommit(vpos);
  ospSetParam(geometry, "vertex.position", OSP_DATA, &vpos);
  ospRelease(vpos);

  OSPData vcol = ospNewSharedData(vertexColors, OSP_VEC3F, vertexCount);
  ospCommit(vcol);
  ospSetParam(geometry, "vertex.color", OSP_DATA, &vcol);
  ospRelease(vcol);

  OSPData ind = ospNewSharedData1D(indices, OSP_VEC3UI, trinagles);
  ospCommit(ind);
  ospSetParam(geometry, "index", OSP_DATA, &ind);
  ospRelease(ind);

  ospCommit(geometry);


  OSPMaterial material = ospNewMaterial("scivis", "obj");

  ospCommit(material);


  OSPGeometricModel model = ospNewGeometricModel(geometry);
  ospSetParam(model, "material", OSP_MATERIAL, &material);
  //float color[] = {0.5, 0.5, 0.5, 1.0};
  //ospSetParam(model, "color", OSP_VEC4F, color);
  ospCommit(model);
  ospRelease(geometry);
  ospRelease(material);


  ospSetObjectAsData(group, "geometry", OSP_GEOMETRIC_MODEL, model);
  ospCommit(group);
  ospRelease(model);
}
