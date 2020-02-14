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


#ifndef SPIRE_ARC_BALL_H
#define SPIRE_ARC_BALL_H

#include <es-log/trace-log.h>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spire/scishare.h>

namespace spire {

/// A reimplementation of Ken Shoemake's arcball camera. SCIRun 4's camera
/// system is based off of Ken's code. The Code appears in Graphics Gems 4,
/// III.1.
/// Unless specified otherwise, all calculations and variables stored in this
/// class are relative to the target coordinate system (TCS) for which there is
/// a transformation from screen space to TCS given by the screenToTCS
/// constructor parameter.
/// If the screenToTCS parameter in the constructor is left as the identity
/// matrix then all values are given in screen coordinates.
/// Screen coordinates are (x \in [-1,1]) and (y \in [-1,1]) where (0,0) is the
/// center of the screen.
class SCISHARE ArcBall
{
public:
  /// \param center         Center of the arcball in TCS (screen coordinates if
  ///                       screenToTCS = identity). Generally this will
  ///                       always be (0,0,0). But you may move the center
  ///                       in and out of the screen plane to various effect.
  /// \param radius         Radius in TCS. For screen coordinates, a good
  ///                       default is 0.75.
  /// \param screenToTCS    Transformation from screen coordinates
  ///                       to TCS. \p center and \p radius are given in TCS.
  ArcBall(const glm::vec3& center, glm::float_t radius, bool inverted = false,
          const glm::mat4& screenToTCS = glm::mat4());

  /// Initiate an arc ball drag given the mouse click in screen coordinates.
  /// \param mouseScreenCoords  Mouse screen coordinates.
  void beginDrag(const glm::vec2& mouseScreenCoords);

  /// Informs the arcball when the mouse has been dragged.
  /// \param mouseScreenCoords  Mouse screen coordinates.
  void drag(const glm::vec2& mouseScreenCoords);

  /// Sets the camera to a specific location and up
  void setLocationOnSphere(glm::vec3 location, glm::vec3 up);

  /// Retrieves the current transformation in TCS.
  /// Obtains full transformation of object in question. If the arc ball is
  /// being used to control camera rotation, then this will contain all
  /// concatenated camera transformations. The current state of the camera
  /// is stored in the quaternions mQDown and mQNow. mMatNow is calculated
  /// from mQNow.
  glm::mat4 getTransformation() const;

  glm::quat getQuat() const {return mQNow;}
  void setQuat(const glm::quat q) {mQNow = q;}

private:

  /// Calculates our position on the ArcBall from 2D mouse position.
  /// \param tscMouse   TSC coordinates of mouse click.
  glm::vec3 mouseOnSphere(const glm::vec3& tscMouse);

  /// Construct a unit quaternion from two points on the unit sphere.
  static glm::quat quatFromUnitSphere(const glm::vec3& from, const glm::vec3& to);

  /// Transform from screen coordinates to the target coordinate system.
  glm::mat4     mScreenToTCS;
  glm::vec3     mCenter;        ///< Center of the arcball in target coordinate system.
  glm::float_t  mRadius;        ///< Radius of the arcball in target coordinate system.
  bool  invertHemisphere;

  glm::vec3     mVSphereDown;   ///< vDown mapped to the sphere of 'mRadius' centered at 'mCenter' in TCS.
  glm::quat     mQDown;         ///< State of the rotation since mouse down.
  glm::quat     mQNow;          ///< Current state of the rotation taking into account mouse.
                                ///< Essentially QDrag * QDown (QDown is a applied first, just
                                ///< as in matrix multiplication).

};

} // namespace spire

#endif
