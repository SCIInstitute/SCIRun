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


/// author James Hughes
/// date   April 2013

#ifndef SPIRE_ARC_LOOK_AT_H
#define SPIRE_ARC_LOOK_AT_H

#include <es-log/trace-log.h>
#include <cstdint>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm-aabb/AABB.hpp>
#include <spire/scishare.h>

namespace spire {
  class ArcBall;
}

namespace spire {

/// Basic camera class that uses arc-ball to control view orientation.
/// Supports a number of utility functions such as ensuring a given
/// AABB is in the view of the frustum (zooming only).
/// All coordinates, unless otherwise specified, are in 2D screen space
/// coordinates.
class SCISHARE ArcLookAt
{
public:
  ArcLookAt();
  virtual ~ArcLookAt();

  /// Sets the reference point for upcoming operations.
  /// Should be be called before any of the 'do' function below, and acts
  /// as the 'mouse down' in the mouse input process. The 'do' functions below
  /// behave like 'mouse move'.
  void doReferenceDown(const glm::vec2& ssPos);

  /// Pans the camera based on the reference position, current orientation, and
  /// the current screen / space position.
  /// Note that this pans the lookat point by the same amount as the camera.
  void doPan(const glm::vec2& ssPos);

  /// Rotates about the look at position.
  void doRotation(const glm::vec2& ssPos);

  /// Dollys the camera towards(negative) / away(positive) from the look at
  /// point. With vairiable zoomspeed.
  void doZoom(glm::float_t camZoom, int zoomSpeed = 65);

  /// Uses a custom function to determine camera zoom. Downwards and to the
  /// right increases size, upwards or to the left decreases size
  void doZoom(const glm::vec2& ssPos);

  /// Implements autoview on a bounding box. Implemented just like SCIRun 4's
  /// autoview in Dataflow/Modules/Render/ViewWindow.cc.
  /// This function will change the lookat point to the center of the bounding
  /// box.
  /// \p fov is expected to be in radians.
  void autoview(const spire::AABB& aabb, float fov);

  /// Sets the view to the specified location and up
  void setView(const glm::vec3& view, const glm::vec3& up);

  /// Retrieves the camera's distance away from the look at point.
  glm::float_t getDistance() const {return mCamDistance;}
  void setDistance(const glm::float_t f) {mCamDistance = f;}

  /// Retrieves the current lookat point.
  glm::vec3 getLookAt() const {return mCamLookAt;}
  void setLookAt(const glm::vec3 v) {mCamLookAt = v;}

  glm::quat getRotation() const;
  void setRotation(const glm::quat q);

  /// Retrieves the world transformation for the camera (looking down
  /// negative z).
  glm::mat4 getWorldViewTransform() const;
  glm::vec3 getUp();
  glm::vec3 getPos();
  glm::vec3 getTarget() {return -mCamLookAt;}

private:
  std::unique_ptr<spire::ArcBall>  mArcBall;

  glm::vec3                 mCamLookAt   {0.0f};     ///< Current lookat position.
  glm::float_t              mCamDistance {3.0f};   ///< Distance from look-at.

  // The following are reference variables set when doReferenceDown is called.
  glm::vec2                 mReferenceScreenPos;
  glm::vec3                 mReferenceLookAt;
};

} // namespace spire

#endif
