/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

/// \author James Hughes
/// \date   April 2013

#ifndef SPIRE_APPSPECIFIC_SCIRUN_SCIBALL_H
#define SPIRE_APPSPECIFIC_SCIRUN_SCIBALL_H

#include <cstdint>

#include "../namespaces.h"

#include "spire/Interface.h"

namespace CPM_SPIRE_SCIRUN_NS {

/// A reimplementation of Ken Shoemake's arcball camera. SCIRun 4's camera
/// system is based completely off of Ken's code. The Code appears in
/// Graphics Gems 4, III.1.
/// 
/// Unless specified otherwise, all calculations and variables stored in this
/// class are relative to the target coordinate system (TCS) for which there is
/// a transformation from screen space to TCS given by the screenToTCS
/// constructor parameter.
///
/// If the screenToTCS parameter in the constructor is left as the identity
/// matrix then all values are given in screen coordinates.
/// Screen coordinates are (x \in [-1,1]) and (y \in [-1,1]) where (0,0) is the
/// center of the screen.
/// \todo Extend this class to include Mouse screen coords -> object space
///       calculations. That way we can rotate around a particular object.
///       May also want to visually represent the sphere when we perform this
///       calculation.
/// \todo Provide method of setting the default orientation of the object.
class SciBall
{
public:
  /// \param center         Center of the arcball in TCS (screen coordinates if 
  ///                       screenToTCS = identity). Generally this will 
  ///                       always be (0,0,0). But you may move the center
  ///                       in and out of the screen plane to various effect.
  /// \param radius         Radius in TCS. For screen coordinates, a good
  ///                       default is 0.75.
  /// \param screenToTCS  Transformation from screen coordinates
  ///                       to TCS. 'center' and 'radius' are given in TCS.
  SciBall(const spire::V3& center, float radius,
          const spire::M44& screenToTCS = spire::M44());
  virtual ~SciBall();
  
  /// Initiate an arc ball drag given the mouse click in screen coordinates.
  /// \param mouseScreenCoords  Mouse screen coordinates.
  void beginDrag(const spire::V2& mouseScreenCoords);

  /// Informs the arcball when the mouse has been dragged.
  /// \param mouseScreenCoords  Mouse screen coordinates.
  void drag(const spire::V2& mouseScreenCoords);

  /// Retrieves the current transformation in TCS.
  /// Obtains full transformation of object in question. If the arc ball is 
  /// being used to control camera rotation, then this will contain all
  /// concatenated camera transformations.
  spire::M44 getTransformation() const;

private:

  /// Calculates our position on the ArcBall from 2D mouse position.
  /// \param tscMouse   TSC coordinates of mouse click.
  spire::V3 mouseOnSphere(const spire::V3& tscMouse);

  /// Construct a unit quaternion from two points on the unit sphere.
  static spire::Quat quatFromUnitSphere(const spire::V3& from, const spire::V3& to);

  spire::V3   mCenter;        ///< Center of the arcball in target coordinate system.
  float       mRadius;        ///< Radius of the arcball in target coordinate system.

  /// \note Both mQNow and mQDown would need to be updated if we allowed
  ///       default transformations.

  spire::Quat mQNow;          ///< Current state of the rotation taking into account mouse.
                              ///< Essentially QDrag * QDown (QDown is a applied first, just
                              ///< as in matrix multiplication).
  spire::Quat mQDown;         ///< State of the rotation since mouse down.
  spire::Quat mQDrag;         ///< Dragged transform. Knows nothing of any prior 
                              ///< transformations.

  spire::V3   mVNow;          ///< Most current TCS position of mouse (during drag).
  spire::V3   mVDown;         ///< TCS position of mouse when the drag was begun.
  spire::V3   mVSphereFrom;   ///< vDown mapped to the sphere of 'mRadius' centered at 'mCenter' in TCS.
  spire::V3   mVSphereTo;     ///< vNow mapped to the sphere of 'mRadius' centered at 'mCenter' in TCS.

  spire::M44  mMatNow;        ///< Matrix representing the current rotation.
  spire::M44  mMatDown;       ///< Matrix representing the rotation when the mouse was first clicked.

  /// \todo Add in constraint sets (you can display handles and constrain
  ///       rotations along those handles).

  /// Transform from screen coordinates to the target coordinate system.
  spire::M44  mScreenToTCS;
};

} // namespace CPM_SPIRE_SCIRUN_NS

#endif 
