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

#ifndef Graphics_Graphics_Widgets_WidgetParameters_H
#define Graphics_Graphics_Widgets_WidgetParameters_H

#include <Core/GeometryPrimitives/Point.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun
{
  namespace Graphics
  {
    namespace Datatypes
    {
      struct SCISHARE CommonWidgetParameters
      {
        double scale;
        std::string defaultColor;
        Core::Geometry::Point origin;
        Core::Geometry::BBox bbox;
        int resolution;
      };

      struct SCISHARE SphereParameters
      {
        CommonWidgetParameters common;
        Core::Geometry::Point point;
      };

      struct SCISHARE CylinderParameters
      {
        CommonWidgetParameters common;
        Core::Geometry::Point p1, p2;
      };

      using DiskParameters = CylinderParameters;

      struct SCISHARE ConeParameters
      {
        CylinderParameters cylinder;
        bool renderBase;
      };

      struct SCISHARE BoxPosition
      {
        Core::Geometry::Point center_, right_, down_, in_;

        void setPosition(const Core::Geometry::Point &center,
                         const Core::Geometry::Point &right,
                         const Core::Geometry::Point &down,
                         const Core::Geometry::Point &in);
        void getPosition(Core::Geometry::Point &center,
                         Core::Geometry::Point &right,
                         Core::Geometry::Point &down,
                         Core::Geometry::Point &in) const;
      };

      struct SCISHARE BasicBoundingBoxParameters
      {
        CommonWidgetParameters common;
        BoxPosition pos;
      };

      struct SCISHARE ArrowParameters
      {
        CommonWidgetParameters common;
        Core::Geometry::Point pos;
        Core::Geometry::Vector dir;
        bool show_as_vector;
        size_t widget_num, widget_iter;
      };

      // These will give different types of widget movement through ViewScene.
      // To use rotation and scaling, an origin point must be given.
      enum class WidgetMovement
      {
        NONE,
        TRANSLATE,
        ROTATE,
        SCALE
      };

      enum class WidgetInteraction
      {
        CLICK,
        OTHER_TYPE_OF_CLICK //TODO
      };

      using TransformMapping = std::map<WidgetInteraction, WidgetMovement>;
      using TransformMappingParams = std::initializer_list<TransformMapping::value_type>;

      struct SCISHARE WidgetBaseParameters
      {
        const Core::GeometryIDGenerator& idGenerator;
        std::string tag;
        TransformMappingParams mapping;
      };

      class AbstractGlyphFactory;
      using AbstractGlyphFactoryPtr = SharedPointer<AbstractGlyphFactory>;

      struct SCISHARE GeneralWidgetParameters
      {
        WidgetBaseParameters base;
        AbstractGlyphFactoryPtr glyphMaker;
      };

      using SimpleWidgetEventFunc = std::function<void(const std::string&)>;
      struct SimpleWidgetEvent
      {
        WidgetMovement moveType;
        SimpleWidgetEventFunc func;
      };

      //TODO: generify
      struct SimpleWidgetEventKey
      {
        WidgetMovement operator()(const SimpleWidgetEvent& e) const { return e.moveType; }
      };

      struct SimpleWidgetEventValue
      {
        SimpleWidgetEventFunc operator()(const SimpleWidgetEvent& e) const { return e.func; }
      };

      template <class Observer, class EventKey, class Event, class KeyFunc, class ObserveFunc, class IdFunc>
      class Observable
      {
      public:
        Observable() {}
        void registerObserver(const EventKey& event, const Observer& observer)
        {
          observers_[event].push_back(observer);
        }

        void notify(const Event& event) const
        {
          auto eventObservers = observers_.find(keyFunc_(event));
          if (eventObservers != observers_.cend())
          {
            for (const auto& obs : eventObservers->second)
              observeFunc_(event)(idFunc_(obs));
          }
        }

      private:
        KeyFunc keyFunc_;
        ObserveFunc observeFunc_;
        IdFunc idFunc_;
        std::map<EventKey, std::vector<Observer>> observers_;
      };

      class SCISHARE InputTransformMapper
      {
      public:
        explicit InputTransformMapper(TransformMappingParams pairs);
        WidgetMovement movementType(WidgetInteraction interaction) const;
      private:
        TransformMapping interactionMap_;
      };

      struct SCISHARE TransformParameters
      {
        virtual ~TransformParameters() {}
      };

      using TransformParametersPtr = std::shared_ptr<TransformParameters>;

      struct SCISHARE Rotation : TransformParameters
      {
        explicit Rotation(const Core::Geometry::Point& p) : origin(p) {}
        const Core::Geometry::Point origin;
      };

      struct SCISHARE Scaling : Rotation
      {
        explicit Scaling(const Core::Geometry::Point& p, const Core::Geometry::Vector& v) : Rotation(p), flip(v) {}
        const Core::Geometry::Vector flip;
      };

      SCISHARE Core::Geometry::Point getRotationOrigin(TransformParametersPtr t);
      SCISHARE Core::Geometry::Vector getScaleFlipVector(TransformParametersPtr t);

      class SCISHARE Transformable
      {
      public:
        TransformParametersPtr transformParameters() const { return transformParameters_; }
        template <class TransformType, class ... Params>
        void setTransformParameters(Params&&... t) { transformParameters_ = std::make_shared<TransformType>(t...); }
      private:
        TransformParametersPtr transformParameters_;
      };
    }
  }
}

#endif
