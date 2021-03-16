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

#ifndef INTERFACE_MODULES_RENDER_SPIRESCIRUN_OBJECTTRANSFORMCALCULATORS_H
#define INTERFACE_MODULES_RENDER_SPIRESCIRUN_OBJECTTRANSFORMCALCULATORS_H

#include <Core/Datatypes/Feedback.h>
#include <es-general/comp/Transform.hpp>
#include <Externals/spire/arc-ball/ArcBall.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <Interface/Modules/Render/ES/RendererInterfaceFwd.h>
#include <Interface/Modules/Render/ES/RendererInterfaceCollaborators.h>
#include <Interface/Modules/Render/share.h>

namespace SCIRun
{
  namespace Render
  {
    class SCISHARE ObjectTransformCalculator
    {
    public:
      virtual ~ObjectTransformCalculator() {}
      virtual Core::Datatypes::WidgetMovement movementType() const = 0;
      virtual gen::Transform computeTransform(int x, int y) const = 0;
    };

    using ObjectTransformCalculatorPtr = SharedPointer<ObjectTransformCalculator>;

    struct SCISHARE ScreenParams
    {
      size_t width {640}, height {480};
      glm::vec2 positionFromClick(int x, int y) const;
    };

    class SCISHARE BasicRendererObjectProvider
    {
    public:
      virtual ~BasicRendererObjectProvider() {}
      virtual SRCamera& camera() const = 0;
      virtual const ScreenParams& screen() const = 0;
      virtual glm::mat4 getStaticCameraViewProjection() = 0;
    };

    class SCISHARE ObjectTransformCalculatorFactory : public boost::enable_shared_from_this<ObjectTransformCalculatorFactory>
    {
    public:
      ObjectTransformCalculatorFactory(BasicRendererObjectProvider* brop, const glm::vec2& initPos, float initW);
      ObjectTransformCalculatorPtr create(Core::Datatypes::WidgetMovement movement, Graphics::Datatypes::WidgetBase* baseWidget) const;
    private:
      BasicRendererObjectProvider* brop_;
      glm::vec2 initPos_;
      float initW_;
    };

    using ObjectTransformCalculatorFactoryPtr = SharedPointer<ObjectTransformCalculatorFactory>;
    using CalcMap = std::map<Graphics::Datatypes::WidgetBase*, ObjectTransformCalculatorPtr>;

    class SCISHARE TransformCalculatorFamily
    {
    public:
      explicit TransformCalculatorFamily(const Graphics::Datatypes::WidgetMovementFamily& movements, ObjectTransformCalculatorFactoryPtr factory);
      Core::Datatypes::WidgetMovement baseMovement() const { return movements_.base; }
      ObjectTransformCalculatorPtr calcFor(Graphics::Datatypes::WidgetBase* widget, Core::Datatypes::WidgetMovement movement);
    private:
      Graphics::Datatypes::WidgetMovementFamily movements_;
      ObjectTransformCalculatorFactoryPtr factory_;
      CalcMap calcs_;
    };

    class SCISHARE WidgetTransformEvent : public Graphics::Datatypes::WidgetEvent
    {
    public:
      WidgetTransformEvent(ObjectTransformer* transformer, std::shared_ptr<TransformCalculatorFamily> calcFamily);
      ~WidgetTransformEvent();
      Core::Datatypes::WidgetMovement baseMovement() const override;
      void move(Graphics::Datatypes::WidgetBase* widget, Core::Datatypes::WidgetMovement moveType) const override;
      void transformAt(int x, int y);
      glm::mat4 getTransformFor(Graphics::Datatypes::WidgetHandle w) const;
    private:
      class WidgetTransformEventImpl;
      std::unique_ptr<WidgetTransformEventImpl> impl_;
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
      struct SCISHARE Params
      {
        glm::vec2 initialPosition_;
        float w_;
        glm::mat4 viewProj;
      };
      ObjectTranslationCalculator(const BasicRendererObjectProvider* s, const Params& t);
      gen::Transform computeTransform(int x, int y) const override;
      Core::Datatypes::WidgetMovement movementType() const override { return Core::Datatypes::TRANSLATE; }
    private:
      glm::vec2 initialPosition_;
      float w_;
      glm::mat4 invViewProj_ {1.0};
    };

    class SCISHARE ObjectScaleCalculator : public ObjectTransformCalculatorBase
    {
    public:
      struct SCISHARE Params
      {
        glm::vec2 initialPosition_;
        float w_;
        glm::vec3 flipAxisWorld_;
        glm::vec3 originWorld_;
      };
      explicit ObjectScaleCalculator(const BasicRendererObjectProvider* s, const Params& p);
      gen::Transform computeTransform(int x, int y) const override;
      Core::Datatypes::WidgetMovement movementType() const override { return Core::Datatypes::SCALE; }
    private:
      glm::vec3 originView_;
      float projectedW_;
      glm::vec3 flipAxisWorld_;
      glm::vec3 originToInitialSpos_;
      float originToInitialSposLength_;
      glm::vec3 originWorld_;
    };

    class SCISHARE ObjectRotationCalculator : public ObjectTransformCalculatorBase
    {
    public:
      struct SCISHARE Params
      {
        glm::vec2 initialPosition_;
        float w_;
        glm::vec3 originWorld_;
      };
      explicit ObjectRotationCalculator(const BasicRendererObjectProvider* s, const Params& p);
      gen::Transform computeTransform(int x, int y) const override;
      Core::Datatypes::WidgetMovement movementType() const override { return Core::Datatypes::ROTATE; }
    private:
      glm::vec3 originWorld_;
      float initialW_;
      std::shared_ptr<spire::ArcBall>	widgetBall_;
    };

    class SCISHARE ObjectScaleAxisCalculator : public ObjectTransformCalculatorBase
    {
    public:
      struct SCISHARE Params
      {
        glm::vec2 initialPosition_;
        float w_;
        glm::vec3 flipAxisWorld_;
        glm::vec3 originWorld_;
        glm::vec3 axis_;
        size_t scaleAxisIndex_;
        glm::mat4 scaleTrans_;
      };
      explicit ObjectScaleAxisCalculator(const BasicRendererObjectProvider* s, const Params &p);
      gen::Transform computeTransform(int x, int y) const override;
      Core::Datatypes::WidgetMovement movementType() const override { return Core::Datatypes::ROTATE; }
      void setMultiplier(double multiplier);
    private:
      glm::mat4 scaleTrans_;
      glm::vec3 axis_;
      glm::vec3 originView_;
      glm::vec3 originWorld_;
      glm::vec3 flipAxisWorld_;
      glm::vec3 originToInitialSpos_;
      double multiplier_;
      float projectedW_;
      float originToInitialSposLength_;
      size_t scaleAxisIndex_;
    };

class SCISHARE ObjectAxisTranslationCalculator : public ObjectTransformCalculatorBase
    {
    public:
      struct SCISHARE Params
      {
        glm::mat4 viewProj;
        glm::vec2 initialPosition_;
        glm::vec3 axis_;
        float w_;
      };
      ObjectAxisTranslationCalculator(const BasicRendererObjectProvider* s, const Params& t);
      gen::Transform computeTransform(int x, int y) const override;
      Core::Datatypes::WidgetMovement movementType() const override { return Core::Datatypes::TRANSLATE; }
    private:
      glm::mat4 invViewProj_ {1.0};
      glm::vec2 initialPosition_;
      glm::vec3 axis_;
      float w_;
    };
  }
}

#endif
