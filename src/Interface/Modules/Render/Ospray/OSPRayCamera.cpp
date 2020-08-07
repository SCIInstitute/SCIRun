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
    case MouseButton::MOUSE_LEFT:
    case MouseButton::MOUSE_RIGHT:
      lookat_.doReferenceDown(glm::vec2(x, -y));
    break;

    default:break;
  }
}

void OSPRayCamera::mouseMove(float x, float y, MouseButton btn)
{
  switch (btn)
  {
  case MouseButton::MOUSE_LEFT:
    lookat_.doRotation(glm::vec2(x, -y));
    pos_ = lookat_.getPos();
    up_ = lookat_.getUp();
    break;

  case MouseButton::MOUSE_RIGHT:
    lookat_.doPan(glm::vec2(x, -y));
    pos_ = lookat_.getPos();
    target_ = lookat_.getTarget();
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
  lookat_.doZoom(-delta/100.0f);
  pos_ = lookat_.getPos();
}
