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
      enum WidgetMovement
      {
        NONE,
        TRANSLATE,
        ROTATE,
        SCALE,
        TRANSLATE_AXIS,
        TRANSLATE_AXIS_HALF,
        TRANSLATE_AXIS_REVERSE,
        SCALE_UNIDIRECTIONAL,
        SCALE_AXIS,
        SCALE_AXIS_HALF,
        SCALE_AXIS_UNIDIRECTIONAL
      };

      // Whether a widget's movement can be propagated using a shared transform calculator.
      enum class WidgetMovementSharing
      {
        SHARED,
        UNIQUE
      };

      using WidgetMovementFamilyMap = std::map<WidgetMovement, WidgetMovementSharing>;

      struct SCISHARE WidgetMovementFamily
      {
        WidgetMovement base;
        WidgetMovementFamilyMap propagated;
      };

      enum class WidgetInteraction
      {
        CLICK,
        RIGHT_CLICK,
        OTHER_TYPE_OF_CLICK //TODO
      };

      using TransformMapping = std::map<WidgetInteraction, WidgetMovementFamily>;

      class SCISHARE WidgetMovementFamilyBuilder
      {
      public:
        explicit WidgetMovementFamilyBuilder(WidgetMovement base) : base_(base) {}
        WidgetMovementFamilyBuilder& sharedMovements(std::initializer_list<WidgetMovement> moves);
        WidgetMovementFamilyBuilder& uniqueMovements(std::initializer_list<WidgetMovement> moves);
        WidgetMovementFamily build() const { return { base_, wmf_ }; }
      private:
        WidgetMovement base_;
        WidgetMovementFamilyMap wmf_;
      };

      SCISHARE WidgetMovementFamily singleMovementWidget(WidgetMovement base);

      struct SCISHARE WidgetBaseParameters
      {
        const Core::GeometryIDGenerator& idGenerator;
        std::string tag;
        TransformMapping mapping;
      };

      class AbstractGlyphFactory;
      using AbstractGlyphFactoryPtr = SharedPointer<AbstractGlyphFactory>;

      struct SCISHARE GeneralWidgetParameters
      {
        WidgetBaseParameters base;
        AbstractGlyphFactoryPtr glyphMaker;
      };

      class WidgetBase;

      class SCISHARE WidgetEvent
      {
      public:
        virtual ~WidgetEvent() {}
        virtual WidgetMovement baseMovement() const = 0;
        virtual void move(WidgetBase* widget, WidgetMovement moveType) const = 0;
      };

      using WidgetEventPtr = SharedPointer<WidgetEvent>;

      class SCISHARE WidgetMovementMediator
      {
      public:
        WidgetMovementMediator() {}

        void registerObserver(WidgetMovement clickedMovement, WidgetBase* observer, WidgetMovement observerMovement)
        {
          observers_[clickedMovement][observerMovement].push_back(observer);
        }

        void mediate(WidgetBase* sender, WidgetEventPtr event) const;

        glm::mat4 latestTransform() const;

      private:
        using SubwidgetMovementMap = std::map<WidgetMovement, std::vector<WidgetBase*>>;
        std::map<WidgetMovement, SubwidgetMovementMap> observers_;
      };

      class SCISHARE InputTransformMapper
      {
      public:
        explicit InputTransformMapper(const TransformMapping& tm);
        WidgetMovementFamily movementType(WidgetInteraction interaction) const;
      private:
        TransformMapping interactionMap_;
      };

      struct SCISHARE TransformParameters
      {
        virtual ~TransformParameters() {}
      };

      using TransformParametersPtr = std::shared_ptr<TransformParameters>;
      using MultiTransformParameters = std::vector<TransformParametersPtr>;

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

      SCISHARE Core::Geometry::Point getRotationOrigin(const MultiTransformParameters& t);
      SCISHARE Core::Geometry::Vector getScaleFlipVector(const MultiTransformParameters& t);

      class SCISHARE Transformable
      {
      public:
        const MultiTransformParameters& transformParameters() const { return transformParameters_; }
        template <class TransformType, class ... Params>
        void addTransformParameters(Params&&... t) { transformParameters_.push_back(std::make_shared<TransformType>(t...)); }
      private:
        MultiTransformParameters transformParameters_;
      };
    }
  }
}

#endif
