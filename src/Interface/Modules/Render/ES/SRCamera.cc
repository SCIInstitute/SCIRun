/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

// author James Hughes
// date   March 2013

#include <gl-platform/GLPlatform.hpp>
#include <Interface/Modules/Render/ES/SRCamera.h>

namespace SCIRun {
  namespace Render {

    //----------------------------------------------------------------------------------------------
    SRCamera::SRCamera(SRInterface& iface) :
        mInterface(iface),
        mArcLookAt(new spire::ArcLookAt())
    {
      setAsPerspective();
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::buildTransform()
    {
      // todo fix this method to return mV instead of mIV
      mIV  = mArcLookAt->getWorldViewTransform();
      mV   = glm::affineInverse(mIV);
      mVP  = mP * mV;
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

    //----------------------------------------------------------------------------------------------
    void SRCamera::mouseDownEvent(const glm::ivec2& pos, SRInterface::MouseButton btn)
    {
      glm::vec2 screenSpace = calculateScreenSpaceCoords(pos);
      mArcLookAt->doReferenceDown(screenSpace);
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::mouseMoveEvent(const glm::ivec2& pos, SRInterface::MouseButton btn)
    {
      glm::vec2 screenSpace = calculateScreenSpaceCoords(pos);
      switch (mInterface.getMouseMode())
      {
        case SRInterface::MOUSE_OLDSCIRUN:
          if (btn == SRInterface::MOUSE_LEFT && !lockPanning_)    mArcLookAt->doPan(screenSpace);
          if (btn == SRInterface::MOUSE_RIGHT && !lockZoom_)      mArcLookAt->doZoom(screenSpace);
          if (btn == SRInterface::MOUSE_MIDDLE && !lockRotation_) mArcLookAt->doRotation(screenSpace);
          break;

        case SRInterface::MOUSE_NEWSCIRUN:
          if (btn == SRInterface::MOUSE_LEFT && !lockRotation_)   mArcLookAt->doRotation(screenSpace);
          if (btn == SRInterface::MOUSE_RIGHT && !lockPanning_)   mArcLookAt->doPan(screenSpace);
          break;
      }
      setClippingPlanes();
    }

    //----------------------------------------------------------------------------------------------
    void SRCamera::mouseWheelEvent(int32_t delta, int zoomSpeed)
    {
      if (mInterface.getMouseMode() != SRInterface::MOUSE_OLDSCIRUN && !lockZoom_)
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
      mRadius = (mSceneBBox.get_max() - mSceneBBox.get_min()).length() / 2.0f;

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

      setAsPerspective();
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
    glm::vec2 SRCamera::calculateScreenSpaceCoords(const glm::ivec2& mousePos)
    {
      float windowOriginX = 0.0f;
      float windowOriginY = 0.0f;

      // Transform incoming mouse coordinates into screen space.
      glm::vec2 mouseScreenSpace;
      mouseScreenSpace.x = 2.0f * (static_cast<float>(mousePos.x) - windowOriginX)
        / static_cast<float>(mInterface.getScreenWidthPixels()) - 1.0f;
      mouseScreenSpace.y = 2.0f * (static_cast<float>(mousePos.y) - windowOriginY)
        / static_cast<float>(mInterface.getScreenHeightPixels()) - 1.0f;

      // Rotation with flipped axes feels much more natural. It places it inside the
      // correct OpenGL coordinate system (with origin in the center of the screen).
      mouseScreenSpace.y = -mouseScreenSpace.y;

      return mouseScreenSpace;
    }

  } // namespace Render
} // namespace SCIRun
