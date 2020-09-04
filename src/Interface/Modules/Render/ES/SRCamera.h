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
/// date   February 2013

#ifndef SPIRE_APPSPECIFIC_SCIRUN_SRUNCAMERA_H
#define SPIRE_APPSPECIFIC_SCIRUN_SRUNCAMERA_H

#include <Interface/Modules/Render/ES/RendererInterfaceFwd.h>
#include <arc-look-at/ArcLookAt.hpp>
#include <Interface/Modules/Render/share.h>

namespace SCIRun{
  namespace Render{

    /// Basic camera class for spire, mimicking SCIRun v4.
    class SCISHARE SRCamera
    {
    public:
      explicit SRCamera(const ScreenParameters* screen);

      /// Sets this camera to use a perspective projection transformation.
      void setAsPerspective();

      /// Sets this camera to use an orthographic projection transformation.
      void setAsOrthographic(float halfWidth, float halfHeight);

      /// Handle mouse down.
      void mouseDownEvent(MouseButton btn, const glm::vec2 &pos);

      /// Handle mouse movement.
      void mouseMoveEvent(MouseButton btn, const glm::vec2 &pos);

      /// Handle mouse wheel event.
      void mouseWheelEvent(int32_t delta, int zoomSpeed);

      //set zNear and zFar using scene bounding box
      void setSceneBoundingBox(const Core::Geometry::BBox& bbox);

      /// Perform autoview.
      void doAutoView();

      //set zNear and zFar using scene bounding box
      void setClippingPlanes();

      /// Sets the selected View of the window to given up axis and view axis
      void setView(const glm::vec3& view, const glm::vec3& up);

      /// Toggles the zoom controls on New Mouse Controls Inverted/Not Inverted
      void setZoomInverted(bool value);

      void tryAutoRotate();

      void rotate(glm::vec2);

      // P  = Projection matrix | IV = Inverse view matrix |  V  = View matrix
      const glm::mat4& getWorldToView() const        {return mV;}
      const glm::mat4& getViewToProjection() const   {return mP;}
      const glm::mat4 getWorldToProjection() const   {return mP * mV;}

      /// Default camera settings
      static float getDefaultFOVY()   {return 32.0f * (glm::pi<float>() / 180.0f);}
      static float getDefaultZNear()  {return 1.00f;}
      static float getDefaultZFar()   {return 10000.0f;}

      float getZFar()   {return mZFar;}
      float getZNear()  {return mZNear;}
      float getFOVY()   {return mFOVY;}
      float getAspect();

      float getDistance() const {return mArcLookAt->getDistance();}
      void setDistance(const float f) {mArcLookAt->setDistance(f); setClippingPlanes();}

      glm::vec3 getLookAt() const {return mArcLookAt->getLookAt();}
      void setLookAt(const glm::vec3 v) {mArcLookAt->setLookAt(v); setClippingPlanes();}

      glm::quat getRotation() const {return mArcLookAt->getRotation();}
      void setRotation(const glm::quat q) {mArcLookAt->setRotation(q); setClippingPlanes();}

      void setLockZoom(bool lock)     {lockZoom_ = lock;}
      void setLockPanning(bool lock)  {lockPanning_ = lock;}
      void setLockRotation(bool lock) {lockRotation_ = lock;}

    private:
      void buildTransform();

      bool                  mPerspective  {true};               ///< True if we are using a perspective
      bool                  lockRotation_ {false};
      bool                  lockZoom_     {false};
      bool                  lockPanning_  {false};

      int                   mInvertVal    {-1};                 ///< Invert multiplier
      float                 mFOVY         {getDefaultFOVY()};   ///< Field of view.
      float                 mZNear        {getDefaultZNear()};  ///< Position of near plane along view vec.
      float                 mZFar         {getDefaultZFar()};   ///< Position of far plane along view vec.
      float                 mRadius       {-1.0};

      glm::vec2             lastMousePos  {0.0, 0.0};
      glm::vec2             mouseMoveVec  {0.0, 0.0};
      glm::vec2             mouseMoveVecR {0.0, 0.0};
      glm::vec2             autoRotateVec {0.0, 0.0};

      glm::mat4             mVP           {};   ///< Projection * View transformation.
      glm::mat4             mV            {};   ///< View transformation.
      glm::mat4             mP            {};   ///< Projection transformation.

      const ScreenParameters*             screenParameters_{nullptr};
      std::shared_ptr<spire::ArcLookAt>   mArcLookAt{};
      Core::Geometry::BBox                mSceneBBox{};

    };

  } // namespace Render
} // namespace SCIRun

#endif
