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


#ifndef SPIRE_RENDER_COMPONENTS_STATIC_ARC_BALL_CAMERA_HPP
#define SPIRE_RENDER_COMPONENTS_STATIC_ARC_BALL_CAMERA_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <arc-look-at/ArcLookAt.hpp>
#include <spire/scishare.h>

namespace ren {

struct StaticArcBallCam
{
  enum CamerasAffected
  {
    PERSPECTIVE,
    ORTHO,
    BOTH
  };

  // -- Data --
  CamerasAffected           cams;
  spire::ArcLookAt arcLookAt;

  // -- Functions --
  StaticArcBallCam()
  {
    cams = PERSPECTIVE;
  }

  StaticArcBallCam(const StaticArcBallCam& other)
  {
    cams = other.cams;
  }

  StaticArcBallCam& operator=(const StaticArcBallCam& other)
  {
    // We don't care about the incoming object. We've already created oun own
    // shader man and will continue to use that.
    cams = other.cams;
    return *this;
  }

  static const char* getName() {return "ren:StaticArcBallCam";}

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Serialize camera settings.
    return true;
  }
};

} // namespace ren

#endif
