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

#include "OSPRayCamera.h"
#include <Interface/Modules/Render/ES/RendererInterfaceCollaborators.h>

#include <glm/gtc/type_ptr.hpp>

using namespace SCIRun::Render;
using namespace SCIRun::Core::Geometry;

OSPRayCamera::OSPRayCamera()
{
  camera_ = ospNewCamera("perspective");
}

OSPRayCamera::~OSPRayCamera()
{
  ospRelease(camera_);
}

OSPCamera OSPRayCamera::getOSPCamera()
{
  glm::vec3 dir = target_ - pos_;
  float dist = glm::length(dir);
  dir = dir/dist;

  ospSetParam(camera_, "position",  OSP_VEC3F, glm::value_ptr(pos_));
  ospSetParam(camera_, "direction", OSP_VEC3F, glm::value_ptr(dir));
  ospSetParam(camera_, "up",        OSP_VEC3F, glm::value_ptr(up_));
  ospSetParam(camera_, "aspect",    OSP_FLOAT, &aspect_);
  ospSetParam(camera_, "fovy",      OSP_FLOAT, &fovy_);
  ospSetParam(camera_, "apertureRadius", OSP_FLOAT, &aperture_);
  ospSetParam(camera_, "focusDistance",  OSP_FLOAT, &dist);
  ospCommit(camera_);

  return camera_;
}

void OSPRayCamera::mousePress(float x, float y, MouseButton btn)
{
  switch(btn)
  {
    case MouseButton::LEFT:
    case MouseButton::RIGHT:
      arcLookAt_.doReferenceDown(glm::vec2(x, y));
    break;

    default:break;
  }
}

void OSPRayCamera::mouseMove(float x, float y, MouseButton btn)
{
  switch (btn)
  {
  case MouseButton::LEFT:
    arcLookAt_.doRotation(glm::vec2(x, y));
    pos_ = arcLookAt_.getPos();
    up_ = arcLookAt_.getUp();
    break;

  case MouseButton::RIGHT:
    arcLookAt_.doPan(glm::vec2(x, y));
    pos_ = arcLookAt_.getPos();
    target_ = arcLookAt_.getTarget();
    break;

  default:
    break;
  }
}

void OSPRayCamera::mouseRelease()
{

}

void OSPRayCamera::mouseWheel(int delta)
{
  arcLookAt_.doZoom(-delta/100.0f);
  pos_ = arcLookAt_.getPos();
}

float OSPRayCamera::toRadians(float v)
{
  const static float HALF_TURN_DEGREES = 180;
  const static float TO_RADIAN_OPERATOR = glm::pi<float>() / HALF_TURN_DEGREES;
  return TO_RADIAN_OPERATOR * v;
}

void OSPRayCamera::autoView()
{
  if(!sceneBBox_.valid()) return;

  // Convert core geom bbox to AABB.
  Core::Geometry::Point bboxMin = sceneBBox_.get_min();
  Core::Geometry::Point bboxMax = sceneBBox_.get_max();
  glm::vec3 min(bboxMin.x(), bboxMin.y(), bboxMin.z());
  glm::vec3 max(bboxMax.x(), bboxMax.y(), bboxMax.z());

  spire::AABB aabb(min, max);

  // The arcball class expects fov in radians
  arcLookAt_.autoview(aabb, toRadians(fovy_));
  pos_ = arcLookAt_.getPos();
  target_ = arcLookAt_.getTarget();
}

void OSPRayCamera::setSceneBoundingBox(const BBox& bbox)
{
  sceneBBox_ = bbox;
}
