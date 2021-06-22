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

#include <Core/Datatypes/Feedback.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <arc-look-at/ArcLookAt.hpp>
#include <glm/glm.hpp>
#include <ospray/ospray.h>
#include <Interface/Modules/Render/Ospray/share.h>

namespace SCIRun { namespace Render {

class SCISHARE OSPRayCamera
{
public:
  OSPRayCamera();
  ~OSPRayCamera();

  void mousePress(float x, float y, MouseButton btn);
  void mouseMove(float x, float y, MouseButton btn);
  void mouseRelease();
  void mouseWheel(int delta);
  void autoView();

  OSPCamera getOSPCamera();

  void setAspect(float aspect) {aspect_ = aspect;}
  void setSceneBoundingBox(const Core::Geometry::BBox& bbox);

private:
  glm::vec3 pos_    {0.0f, 0.0f, 3.0f};
  glm::vec3 target_ {0.0f, 0.0f, 0.0f};
  glm::vec3 up_     {0.0f, 1.0f, 0.0f};
  float aspect_     { 1.0f};
  float fovy_       {60.0f};
  float aperture_   {0.0f};
  Core::Geometry::BBox sceneBBox_;

  spire::ArcLookAt lookat_ {       };
  OSPCamera camera_        {nullptr};

  float toRadians(float v);
};

} /*Render*/ } /*SCIRun*/
