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
/// date   March 2013

#include <gl-platform/GLPlatform.hpp>
#include <Interface/Modules/Render/ES/RendererInterface.h>
#include <Interface/Modules/Render/ES/SRCamera.h>

namespace SCIRun {
  namespace Render {

    //----------------------------------------------------------------------------------------------
    SRCamera::SRCamera(const ScreenParameters* screen) :
        screenParameters_(screen),
        mArcLookAt(new spire::ArcLookAt())
    {
      setAsPerspective();
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::buildTransform()
    {
      // todo fix this method to return mV instead of mIV
      mV  = mArcLookAt->getWorldViewTransform();
      mVP = mP * mV;
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::setAsPerspective()
    {
      mPerspective = true;
      mP = glm::perspective(mFOVY, getAspect(), mZNear, mZFar);
      buildTransform();
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::setAsOrthographic(float halfWidth, float halfHeight)
    {
      mPerspective = false;
    	mP = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, mZNear, mZFar);
      buildTransform();
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::setView(const glm::vec3& view, const glm::vec3& up)
    {
      mArcLookAt->setView(view, up);
      setClippingPlanes();
    }

    float SRCamera::getAspect() {return static_cast<float>(screenParameters_->getScreenWidthPixels()) /
                              static_cast<float>(screenParameters_->getScreenHeightPixels());}

    //----------------------------------------------------------------------------------------------
    void SRCamera::mouseDownEvent(MouseButton btn, const glm::vec2& pos)
    {
      mArcLookAt->doReferenceDown(pos);
      lastMousePos  = pos;
      mouseMoveVec  = glm::vec2(0.0, 0.0);
      mouseMoveVecR = glm::vec2(0.0, 0.0);
      autoRotateVec = glm::vec2(0.0, 0.0);
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::mouseMoveEvent(MouseButton btn, const glm::vec2& pos)
    {
      static const float avFac = 0.2f;
      switch (screenParameters_->getMouseMode())
      {
        case MouseMode::MOUSE_OLDSCIRUN:
          if (btn == MouseButton::MOUSE_LEFT && !lockPanning_)    mArcLookAt->doPan(pos);
          if (btn == MouseButton::MOUSE_RIGHT && !lockZoom_)      mArcLookAt->doZoom(pos);
          if (btn == MouseButton::MOUSE_MIDDLE && !lockRotation_)
          {
            mArcLookAt->doRotation(pos);
            mouseMoveVec = avFac * (pos - lastMousePos) + (1.0f - avFac) * mouseMoveVec;
            mouseMoveVecR = (1.0f - avFac) * (pos - lastMousePos) + avFac * mouseMoveVec;
            if(glm::length(mouseMoveVecR) < glm::length(mouseMoveVec)*0.7f)
              autoRotateVec = glm::vec2(0.0, 0.0);
            else
              autoRotateVec = mouseMoveVec;
            lastMousePos = pos;
          }
          break;

        case MouseMode::MOUSE_NEWSCIRUN:
          if (btn == MouseButton::MOUSE_LEFT && !lockRotation_)
          {
            mArcLookAt->doRotation(pos);
            mouseMoveVec = avFac * (pos - lastMousePos) + (1.0f - avFac) * mouseMoveVec;
            mouseMoveVecR = (1.0f - avFac) * (pos - lastMousePos) + avFac * mouseMoveVec;
            if(glm::length(mouseMoveVecR) < glm::length(mouseMoveVec)*0.7f)
              autoRotateVec = glm::vec2(0.0, 0.0);
            else
              autoRotateVec = mouseMoveVec;
            lastMousePos = pos;
          }
          if (btn == MouseButton::MOUSE_RIGHT && !lockPanning_)   mArcLookAt->doPan(pos);
          break;
      }
      setClippingPlanes();
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::mouseWheelEvent(int32_t delta, int zoomSpeed)
    {
      if (screenParameters_->getMouseMode() != MouseMode::MOUSE_OLDSCIRUN && !lockZoom_)
      {
        mArcLookAt->doZoom(mInvertVal*static_cast<float>(delta) / 100.0f, zoomSpeed);
        setClippingPlanes();
      }
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::setSceneBoundingBox(const Core::Geometry::BBox& bbox)
    {
      mSceneBBox = bbox;
      setClippingPlanes();
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::doAutoView()
    {
      if(mSceneBBox.valid())
      {
        // Convert core geom bbox to AABB.
        Core::Geometry::Point bboxMin = mSceneBBox.get_min();
        Core::Geometry::Point bboxMax = mSceneBBox.get_max();
        glm::vec3 min(bboxMin.x(), bboxMin.y(), bboxMin.z());
        glm::vec3 max(bboxMax.x(), bboxMax.y(), bboxMax.z());

        spire::AABB aabb(min, max);

        // todo Use real FOV-Y when we allow the user to change the FOV.
        mArcLookAt->autoview(aabb, mFOVY);
        setClippingPlanes();
      }
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::setClippingPlanes()
    {
      //we could divide by 2.0 for tightest bound but here we divide by 1.0 to accomidate
      //for the clipping plane visualization
      if (!mSceneBBox.valid()) return;
      mRadius = mSceneBBox.diagonal().length();

      if(mRadius > 0.0)
      {
        buildTransform();  // make sure matricies are up to date
        Core::Geometry::Point c =  Core::Geometry::Point(mSceneBBox.get_max() + mSceneBBox.get_min());
        glm::vec4 center(c.x()/2.0,c.y()/2.0,c.z()/2.0, 1.0);
        center = mV * center;

        mZFar = -center.z + mRadius;
        mZNear = std::max(mZFar/1000.0f, -center.z - mRadius);
      }
      else
      {
        mZFar = getDefaultZFar();
        mZNear = getDefaultZNear();
      }

      if(mPerspective)
        setAsPerspective();
      else
        setAsOrthographic(mRadius*0.5, mRadius*0.5*getAspect());
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::setZoomInverted(bool value)
    {
      if (value)
        mInvertVal = 1;
      else
        mInvertVal = -1;
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::tryAutoRotate()
    {
      mArcLookAt->doReferenceDown(lastMousePos);
      mArcLookAt->doRotation(lastMousePos + autoRotateVec);
      setClippingPlanes();
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::rotate(glm::vec2 vector)
    {
      mArcLookAt->doReferenceDown(glm::vec2(0.0, 0.0));
      mArcLookAt->doRotation(vector);
      setClippingPlanes();
    }

    ////----------------------------------------------------------------------------------------------
    //glm::vec2 SRCamera::calculateScreenSpaceCoords(const glm::ivec2& mousePos)
    //{
    //  float windowOriginX = 0.0f;
    //  float windowOriginY = 0.0f;

    //  // Transform incoming mouse coordinates into screen space.
    //  glm::vec2 mouseScreenSpace;
    //  mouseScreenSpace.x = 2.0f * (static_cast<float>(mousePos.x) - windowOriginX)
    //    / static_cast<float>(screenParameters_->getScreenWidthPixels()) - 1.0f;
    //  mouseScreenSpace.y = 2.0f * (static_cast<float>(mousePos.y) - windowOriginY)
    //    / static_cast<float>(screenParameters_->getScreenHeightPixels()) - 1.0f;

    //  // Rotation with flipped axes feels much more natural. It places it inside the
    //  // correct OpenGL coordinate system (with origin in the center of the screen).
    //  mouseScreenSpace.y = -mouseScreenSpace.y;

    //  return mouseScreenSpace;
    //}
  } // namespace Render
} // namespace SCIRun
