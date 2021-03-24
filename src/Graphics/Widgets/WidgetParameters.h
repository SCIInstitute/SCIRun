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

#include <Core/Datatypes/Feedback.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Tensor.h>
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

      struct SCISHARE SuperquadricParameters
      {
        CommonWidgetParameters common;
        Core::Geometry::Point point;
        Core::Geometry::Tensor tensor;
        double A;
        double B;
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
        Core::Geometry::Point center_;
        std::vector<Core::Geometry::Vector> scaledEigvecs_;

        // void setPosition(const Core::Geometry::Point &center,
                         // const std::vector<Core::Geometry::Vector>& scaledEigvecs);
        // Core::Geometry::Point getCenter() const;
        // const std::vector<Core::Geometry::Vector>& getScaledEigvecs() const;
      };

      struct SCISHARE BasicBoundingBoxParameters
      {
        CommonWidgetParameters common;
        BoxPosition pos;
      };

      struct SCISHARE BoundingBoxParameters
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

      // Whether a widget's movement can be propagated using a shared transform calculator.
      enum class WidgetMovementSharing
      {
        SHARED,
        UNIQUE
      };

      using WidgetMovementFamilyMap = std::map<Core::Datatypes::WidgetMovement, WidgetMovementSharing>;

      struct SCISHARE WidgetMovementFamily
      {
        Core::Datatypes::WidgetMovement base;
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
        explicit WidgetMovementFamilyBuilder(Core::Datatypes::WidgetMovement base) : base_(base) {}
        WidgetMovementFamilyBuilder& sharedMovements(std::initializer_list<Core::Datatypes::WidgetMovement> moves);
        WidgetMovementFamilyBuilder& uniqueMovements(std::initializer_list<Core::Datatypes::WidgetMovement> moves);
        WidgetMovementFamily build() const { return { base_, wmf_ }; }
      private:
        Core::Datatypes::WidgetMovement base_;
        WidgetMovementFamilyMap wmf_;
      };

      SCISHARE WidgetMovementFamily singleMovementWidget(Core::Datatypes::WidgetMovement base);

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
        virtual Core::Datatypes::WidgetMovement baseMovement() const = 0;
        virtual void move(WidgetBase* widget, Core::Datatypes::WidgetMovement moveType) const = 0;
      };

      using WidgetEventPtr = SharedPointer<WidgetEvent>;

      class SCISHARE WidgetMovementMediator
      {
      public:
        WidgetMovementMediator() {}

        void registerObserver(Core::Datatypes::WidgetMovement clickedMovement, WidgetBase* observer, Core::Datatypes::WidgetMovement observerMovement)
        {
          observers_[clickedMovement][observerMovement].push_back(observer);
        }

        void mediate(WidgetBase* sender, WidgetEventPtr event) const;

        glm::mat4 latestTransform() const;

      private:
        using SubwidgetMovementMap = std::map<Core::Datatypes::WidgetMovement, std::vector<WidgetBase*>>;
        std::map<Core::Datatypes::WidgetMovement, SubwidgetMovementMap> observers_;
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

      struct SCISHARE Scaling : Rotation //TODO change inheritance
      {
        explicit Scaling(const Core::Geometry::Point& p, const Core::Geometry::Vector& v) : Rotation(p), flip(v) {}
        const Core::Geometry::Vector flip;
      };

      struct SCISHARE AxisTransformParameters : TransformParameters
      {
        explicit AxisTransformParameters(const Core::Geometry::Vector& sa)
          : scaleAxis(sa) {}
        const Core::Geometry::Vector scaleAxis;
      };

      struct SCISHARE AxisScaling : AxisTransformParameters
      {
        explicit AxisScaling(const Core::Geometry::Point& p, const Core::Geometry::Vector& sa,
                             const size_t scaleAxisIndex)
          : AxisTransformParameters(sa), origin(p), scaleAxisIndex(scaleAxisIndex) {}
        const Core::Geometry::Point origin;
        const size_t scaleAxisIndex;
      };

     struct SCISHARE AxisTranslation : AxisTransformParameters
     {
        explicit AxisTranslation(const Core::Geometry::Vector& sa)
          : AxisTransformParameters(sa) {}
     };

      SCISHARE Core::Geometry::Point getRotationOrigin(const MultiTransformParameters& t);
      SCISHARE Core::Geometry::Vector getScaleFlipVector(const MultiTransformParameters& t);
      SCISHARE Core::Geometry::Vector getAxisVector(const MultiTransformParameters& t);
      SCISHARE size_t getAxisIndex(const MultiTransformParameters& t);
      SCISHARE glm::mat4 getScaleAxisTrans(const MultiTransformParameters& t);

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
