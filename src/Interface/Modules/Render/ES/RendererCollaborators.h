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

#ifndef INTERFACE_MODULES_RENDER_SPIRESCIRUN_RENDERERCOLLABORATORS_H
#define INTERFACE_MODULES_RENDER_SPIRESCIRUN_RENDERERCOLLABORATORS_H

#include <Core/Datatypes/Feedback.h>
#include <Externals/spire/arc-ball/ArcBall.hpp>
#include <Graphics/Widgets/WidgetParameters.h>
#include <Interface/Modules/Render/ES/RendererInterfaceFwd.h>
#include <Interface/Modules/Render/ES/ObjectTransformCalculators.h>
#include <Interface/Modules/Render/share.h>

namespace SCIRun
{
  namespace Render
  {
    class SCISHARE FatalRendererError : public std::runtime_error
    {
    public:
      explicit FatalRendererError(const std::string& message) : std::runtime_error(message) {}
    };

    class SCISHARE DepthIndex
    {
    public:
      size_t mIndex;
      double mDepth;

      DepthIndex() :
        mIndex(0),
        mDepth(0.0)
      {}

      DepthIndex(size_t index, double depth) :
        mIndex(index),
        mDepth(depth)
      {}

      bool operator<(const DepthIndex& di) const
      {
        return this->mDepth < di.mDepth;
      }
    };

    class SCISHARE SRObject
    {
    public:
      SRObject(const std::string& name,
        const Core::Geometry::BBox& bbox, std::optional<std::string> colorMap, int port) :
        mName(name),
        mBBox(bbox),
        mColorMap(colorMap),
        mPort(port)
      {}

      // Different types of uniform transformations that are associated
      // with the object (based off of the unsatisfied uniforms detected
      // by the Spire object).
      enum class ObjectTransforms
      {
        OBJECT_TO_WORLD,
        OBJECT_TO_CAMERA,
        OBJECT_TO_CAMERA_PROJECTION,
      };

      struct SCISHARE SRPass
      {
        SRPass(const std::string& name, Graphics::Datatypes::RenderType renType) :
          passName(name),
          renderType(renType)
        {}

        std::string passName;
        std::list<ObjectTransforms> transforms;
        Graphics::Datatypes::RenderType renderType;
      };

      std::string mName;
      std::list<SRPass> mPasses;
      Core::Geometry::BBox mBBox;          // Objects bounding box (calculated from VBO).

      std::optional<std::string> mColorMap;

      int	mPort;
    };

    class SCISHARE WidgetUpdateService : public BasicRendererObjectProvider, boost::noncopyable
    {
    public:
      WidgetUpdateService(ObjectTransformer* transformer, const ScreenParams& screen);
      void setCamera(SRCamera* cam) { camera_ = cam; }

      void doInitialUpdate(int x, int y, float depth);
      void updateWidget(int x, int y);
      void setButtonPushed(MouseButton b) { buttonPushed_ = b; }

      SRCamera& camera() const override { return *camera_; }
      const ScreenParams& screen() const override { return screen_; }
      glm::mat4 getStaticCameraViewProjection() override;

      void reset();

      void setCurrentWidget(Graphics::Datatypes::WidgetHandle w);
      Graphics::Datatypes::WidgetHandle currentWidget() const { return currentWidget_; }

      glm::mat4 widgetTransform() const { return widgetTransform_; }

    private:
      void doPostSelectSetup(int x, int y, float depth);
      float getInitialW(float depth) const;

      Graphics::Datatypes::WidgetHandle currentWidget_;
      SharedPointer<WidgetTransformEvent> event_;
      MouseButton buttonPushed_;
      ObjectTransformer* transformer_ {nullptr};
      const ScreenParams& screen_;
      SRCamera* camera_ {nullptr};
      glm::mat4 widgetTransform_ {1.0f};
    };

    SCISHARE Graphics::Datatypes::WidgetInteraction yetAnotherEnumConversion(MouseButton btn);
    SCISHARE std::ostream& operator<<(std::ostream& o, const glm::mat4& m);


  }
}

#endif
