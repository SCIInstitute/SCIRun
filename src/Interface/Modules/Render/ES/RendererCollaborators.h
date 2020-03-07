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

#include <es-general/comp/Transform.hpp>
#include <Externals/spire/arc-ball/ArcBall.hpp>
#include <Interface/Modules/Render/ES/RendererInterfaceFwd.h>
#include <Interface/Modules/Render/ES/RendererInterfaceCollaborators.h>
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
      SRObject(const std::string& name, const glm::mat4& objToWorld,
        const Core::Geometry::BBox& bbox, boost::optional<std::string> colorMap, int port) :
        mName(name),
        mObjectToWorld(objToWorld),
        mBBox(bbox),
        mColorMap(colorMap),
        mPort(port)
      {}

      // Different types of uniform transformations that are associated
      // with the object (based off of the unsatisfied uniforms detected
      // by the Spire object).
      enum ObjectTransforms
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
      glm::mat4 mObjectToWorld;
      std::list<SRPass> mPasses;
      Core::Geometry::BBox mBBox;          // Objects bounding box (calculated from VBO).

      boost::optional<std::string> mColorMap;

      int	mPort;
    };

    struct SCISHARE TranslateParameters
    {
      glm::vec2 initialPosition_;
      float w_;
      glm::mat4 viewProj;
    };

    struct SCISHARE RotateParameters
    {
      glm::vec2 initialPosition_;
      float w_;
      glm::vec3 originWorld_;
    };

    struct SCISHARE ScaleParameters
    {
      glm::vec2 initialPosition_;
      float w_;
      glm::vec3 flipAxisWorld_;
      glm::vec3 originWorld_;
    };

    struct SCISHARE ScreenParams
    {
      size_t width {640}, height {480};
      glm::vec2 positionFromClick(int x, int y) const;
    };

    class SCISHARE ObjectTransformCalculator
    {
    public:
      virtual ~ObjectTransformCalculator() {}
      virtual gen::Transform computeTransform(int x, int y) const = 0;
    };

    using ObjectTransformCalculatorPtr = SharedPointer<ObjectTransformCalculator>;

    class SCISHARE WidgetEventBase
    {
    public:
      explicit WidgetEventBase(const gen::Transform& t) : transform(t) {}
      gen::Transform transform;
    };

    using WidgetEventPtr = SharedPointer<WidgetEventBase>;

    // class SCISHARE WidgetTranslateEvent : public WidgetEventBase
    // {
    // public:
    //   using WidgetEventBase::WidgetEventBase;
    // };
    //
    // class SCISHARE WidgetRotateEvent : public WidgetEventBase
    // {
    // public:
    //   using WidgetEventBase::WidgetEventBase;
    // };
    //
    // class SCISHARE WidgetScaleEvent : public WidgetEventBase
    // {
    // public:
    //   using WidgetEventBase::WidgetEventBase;
    // };

    class SCISHARE BasicRendererObjectProvider
    {
    public:
      virtual ~BasicRendererObjectProvider() {}
      virtual SRCamera& camera() const = 0;
      virtual const ScreenParams& screen() const = 0;
    };

    class ObjectTransformCalculatorFactory
    {
    public:
      explicit ObjectTransformCalculatorFactory(const BasicRendererObjectProvider* brop) : brop_(brop) {}
      template <class Params>
      ObjectTransformCalculatorPtr create(const Params& p);
    private:
      const BasicRendererObjectProvider* brop_;
    };

    using TransformCalcMaker = std::function<ObjectTransformCalculatorPtr(const glm::vec2&, float)>;
    using TransformCalcMakerMapping = std::map<Graphics::Datatypes::WidgetMovement, TransformCalcMaker>;

    class SCISHARE WidgetUpdateService : public BasicRendererObjectProvider, boost::noncopyable
    {
    public:
      WidgetUpdateService(ObjectTransformer* transformer, const ScreenParams& screen);
      void setCamera(SRCamera* cam) { camera_ = cam; }

      void modifyWidget(WidgetEventPtr event);
      void doInitialUpdate(int x, int y, float depth);
      void updateWidget(int x, int y);

      SRCamera& camera() const override { return *camera_; }
      const ScreenParams& screen() const override { return screen_; };

      void reset();

      void setCurrentWidget(Graphics::Datatypes::WidgetHandle w);
      Graphics::Datatypes::WidgetHandle currentWidget() const { return widget_; }

      glm::mat4 widgetTransform() const { return widgetTransform_; }

    private:
      void doPostSelectSetup(int x, int y, float depth);
      float getInitialW(float depth) const;
      TranslateParameters buildTranslation(const glm::vec2& initPos, float initW);
      ScaleParameters buildScale(const glm::vec2& initPos, float initW);
      RotateParameters buildRotation(const glm::vec2& initPos, float initW);

      Graphics::Datatypes::WidgetHandle widget_;
      Graphics::Datatypes::WidgetMovement movement_ {Graphics::Datatypes::WidgetMovement::NONE};
      ObjectTransformer* transformer_ {nullptr};
      const ScreenParams& screen_;
      ObjectTransformCalculatorPtr objectTransformCalculator_;
      ObjectTransformCalculatorFactory transformFactory_;
      TransformCalcMakerMapping transformCalcMakerMapping_;
      SRCamera* camera_ {nullptr};
      glm::mat4 widgetTransform_ {};
    };

    class SCISHARE ObjectTransformCalculatorBase : public ObjectTransformCalculator, boost::noncopyable
    {
    public:
      explicit ObjectTransformCalculatorBase(const BasicRendererObjectProvider* s) :
        service_(s) {}
    protected:
      const BasicRendererObjectProvider* service_;
    };

    class SCISHARE ObjectTranslationCalculator : public ObjectTransformCalculatorBase
    {
    public:
      ObjectTranslationCalculator(const BasicRendererObjectProvider* s, const TranslateParameters& t);
      gen::Transform computeTransform(int x, int y) const override;
    private:
      glm::vec2 initialPosition_;
      float w_;
      glm::mat4 invViewProj_;
    };

    class SCISHARE ObjectScaleCalculator : public ObjectTransformCalculatorBase
    {
    public:
      explicit ObjectScaleCalculator(const BasicRendererObjectProvider* s, const ScaleParameters& p);
      gen::Transform computeTransform(int x, int y) const override;
    private:
      glm::vec3 originView_;
      float projectedW_;
      glm::vec3 flipAxisWorld_;
      glm::vec3 originToSpos_;
      glm::vec3 originWorld_;
    };

    class SCISHARE ObjectRotationCalculator : public ObjectTransformCalculatorBase
    {
    public:
      explicit ObjectRotationCalculator(const BasicRendererObjectProvider* s, const RotateParameters& p);
      gen::Transform computeTransform(int x, int y) const override;
    private:
      glm::vec3 originWorld_;
      float initialW_;
      std::shared_ptr<spire::ArcBall>	widgetBall_;
    };

    SCISHARE std::ostream& operator<<(std::ostream& o, const glm::mat4& m);
  }
}

#endif
