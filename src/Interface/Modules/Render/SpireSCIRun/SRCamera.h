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
/// \date   February 2013

#ifndef SPIRE_APPSPECIFIC_SCIRUN_SRUNCAMERA_H
#define SPIRE_APPSPECIFIC_SCIRUN_SRUNCAMERA_H

#include <Interface/Modules/Render/SpireSCIRun/SRInterface.h>

#include <spire/src/Common.h>
#include <arc-look-at/ArcLookAt.hpp>

namespace SCIRun {
namespace Gui {

/// Basic camera class for spire, mimicking SCIRun v4.
class SRCamera
{
public:
  SRCamera(SRInterface& iface);
  virtual ~SRCamera();

  // V  = View matrix
  // IV = Inverse view matrix
  // P  = Projection matrix
  // m  = multiplication
  const spire::M44& getWorldToProjection() const  {return mPIV;}
  const spire::M44& getWorldToView() const        {return mIV;}
  const spire::M44& getViewToWorld() const        {return mV;}
  const spire::M44& getViewToProjection() const   {return mP;}

  /// Sets this camera to use a perspective projection transformation.
  void setAsPerspective();

  /// Sets this camera to use an orthographic projection transformation.
  void setAsOrthographic(float halfWidth, float halfHeight);

  /// Sets the current view transform (view to world space).
  /// \xxx This should be removed.
  void applyTransform();

  /// Handle mouse down.
  void mouseDownEvent(const glm::ivec2& pos, SRInterface::MouseButton btn);

  /// Handle mouse movement.
  void mouseMoveEvent(const glm::ivec2& pos, SRInterface::MouseButton btn);

  /// Handle mouse wheel event.
  void mouseWheelEvent(int32_t delta);

  /// Perform autoview.
  void doAutoView(const Core::Geometry::BBox& bbox);

  /// Default camera settings
  /// @{
  static float getDefaultFOVY()   {return 32.0f * (spire::PI / 180.0f);}
  static float getDefaultZNear()  {return 0.1f;}
  static float getDefaultZFar()   {return 100000.0f;}
  /// @}

private:

  void buildTransform();
  spire::V2 calculateScreenSpaceCoords(const glm::ivec2& mousePos);

  spire::M44            mPIV;         ///< Projection * Inverse View transformation.
  spire::M44            mIV;          ///< Inverse view transformation.
  spire::M44            mV;           ///< View matrix.
  spire::M44            mP;           ///< Projection transformation.
  size_t                mTrafoSeq;    ///< Current sequence of the view transform.
                                      ///< Helps us determine when a camera is 'dirty'.

  bool                  mPerspective; ///< True if we are using a perspective 
                                      ///< transformation. 
  float                 mFOV;         ///< Field of view.
  float                 mZNear;       ///< Position of near plane along view vec.
  float                 mZFar;        ///< Position of far plane along view vec.

  SRInterface&          mInterface;   ///< SRInterface.
  
  std::shared_ptr<CPM_LOOK_AT_NS::ArcLookAt>  mArcLookAt;

};

} // namespace Gui
} // namespace SCIRun 

#endif 
