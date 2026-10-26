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

#pragma once

#ifdef WITH_VTK
#include <Core/Algorithms/Visualization/VtkIncludes.h>
#include <Core/Datatypes/feedback.h>
#include <Interface/Modules/Render/Vtk/share.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace SCIRun {
    namespace Core {
        namespace Datatypes {
        enum WidgetMovement;
      }
    }
    namespace Graphics {
        namespace Datatypes {

            class WidgetBase;
            using WidgetHandle = SharedPointer<WidgetBase>;
            using WidgetList = std::vector<WidgetHandle>;
            using WidgetListIterator = WidgetList::const_iterator;

        }
    }
}

namespace SCIRun
{
    namespace Render
    {

        struct SCISHARE VtkInteractionContext
        {
        glm::mat4 worldToProjection;

        glm::mat4 projectionToWorld;

        glm::vec3 cameraPosition;

        glm::vec3 focalPoint;

        glm::vec3 viewUp;

        int viewportWidth;

        int viewportHeight;
        };

        class VtkTransformCalculator
        {
         public:
          virtual ~VtkTransformCalculator() = default;

          virtual glm::mat4 computeTransform(const glm::vec3& currentWorldPos) = 0;
        };

        class VtkTranslationCalculator : public VtkTransformCalculator
        {
         public:
          explicit VtkTranslationCalculator(const glm::vec3& initialWorld) : initialWorld_(initialWorld) {}

          glm::mat4 computeTransform(const glm::vec3& currentWorldPos) override
          {
            const glm::vec3 delta = currentWorldPos - initialWorld_;

            return glm::translate(glm::mat4(1.f), delta);
          }

         private:
          glm::vec3 initialWorld_;
        };

        class VtkAxisTranslationCalculator : public VtkTransformCalculator
        {
         public:
          VtkAxisTranslationCalculator(const glm::vec3& initialWorld, const glm::vec3& axis) : initialWorld_(initialWorld), axis_(glm::normalize(axis)) {}

          glm::mat4 computeTransform(const glm::vec3& currentWorldPos) override
          {
            const glm::vec3 delta = currentWorldPos - initialWorld_;

            const float d = glm::dot(delta, axis_);

            const glm::vec3 movement = d * axis_;

            return glm::translate(glm::mat4(1.f), movement);
          }

         private:
          glm::vec3 initialWorld_;
          glm::vec3 axis_;
        };

        class VtkScaleCalculator : public VtkTransformCalculator
        {
         public:
          VtkScaleCalculator(const glm::vec3& origin, const glm::vec3& initialWorld) : origin_(origin), initialWorld_(initialWorld)
          {
            startDistance_ = glm::length(initialWorld_ - origin_);

            if (startDistance_ < 1e-6f) startDistance_ = 1.f;
          }

          glm::mat4 computeTransform(const glm::vec3& currentWorldPos) override
          {
            const float currentDistance = glm::length(currentWorldPos - origin_);

            float scale = currentDistance / startDistance_;

            scale = std::max(scale, 0.001f);

            auto T1 = glm::translate(glm::mat4(1.f), origin_);

            auto S = glm::scale(glm::mat4(1.f), glm::vec3(scale));

            auto T2 = glm::translate(glm::mat4(1.f), -origin_);

            return T1 * S * T2;
          }

         private:
          glm::vec3 origin_;
          glm::vec3 initialWorld_;
          float startDistance_;
        };

        class VtkScaleAxisCalculator : public VtkTransformCalculator
        {
         public:
          VtkScaleAxisCalculator(const glm::vec3& origin, const glm::vec3& initialWorld, const glm::vec3& axis) : origin_(origin), initialWorld_(initialWorld), axis_(glm::normalize(axis))
          {
            startProjection_ = glm::dot(initialWorld_ - origin_, axis_);

            if (std::abs(startProjection_) < 1e-6f) startProjection_ = 1.f;
          }

          glm::mat4 computeTransform(const glm::vec3& currentWorldPos) override
          {
            float currentProjection = glm::dot(currentWorldPos - origin_, axis_);

            float scale = currentProjection / startProjection_;

            scale = std::max(scale, 0.001f);

            glm::vec3 scaleVector(1.f);

            if (std::abs(axis_.x) > 0.5f) scaleVector.x = scale;

            if (std::abs(axis_.y) > 0.5f) scaleVector.y = scale;

            if (std::abs(axis_.z) > 0.5f) scaleVector.z = scale;

            auto T1 = glm::translate(glm::mat4(1.f), origin_);

            auto S = glm::scale(glm::mat4(1.f), scaleVector);

            auto T2 = glm::translate(glm::mat4(1.f), -origin_);

            return T1 * S * T2;
          }

         private:
          glm::vec3 origin_;
          glm::vec3 initialWorld_;
          glm::vec3 axis_;

          float startProjection_;
        };

        class VtkRotationCalculator : public VtkTransformCalculator
        {
         public:
          VtkRotationCalculator(const glm::vec3& origin, const glm::vec3& axis, const glm::vec3& initialWorld) : origin_(origin), axis_(glm::normalize(axis))
          {
            startVector_ = glm::normalize(initialWorld - origin_);
          }

          glm::mat4 computeTransform(const glm::vec3& currentWorldPos) override
          {
            glm::vec3 currentVector = glm::normalize(currentWorldPos - origin_);

            float angle = acos(glm::clamp(glm::dot(startVector_, currentVector), -1.f, 1.f));

            auto T1 = glm::translate(glm::mat4(1.f), origin_);

            auto R = glm::rotate(glm::mat4(1.f), angle, axis_);

            auto T2 = glm::translate(glm::mat4(1.f), -origin_);

            return T1 * R * T2;
          }

         private:
          glm::vec3 origin_;
          glm::vec3 axis_;
          glm::vec3 startVector_;
        };

        class SCISHARE VtkTransformCalculatorFactory
        {
         public:
          static std::unique_ptr<VtkTransformCalculator> Create(
              Core::Datatypes::WidgetMovement movement,
              Graphics::Datatypes::WidgetHandle widget,
              const VtkInteractionContext& context,
              const glm::vec3& initialPickWorld);
        };

        class VtkRenderer;
        class SCISHARE VtkWidgetUpdateService
        {
        public:
            VtkWidgetUpdateService(VtkRenderer* renderer);

            void setCurrentWidget(Graphics::Datatypes::WidgetHandle widget);

            void setButtonPushed(MouseButton button);

            void beginInteraction(
                int x,
                int y,
                vtkCellPicker* picker);

            void updateWidget(int x, int y);

            void reset();

            Graphics::Datatypes::WidgetHandle currentWidget() const;

            glm::mat4 widgetTransform() const;

        private:
            VtkRenderer* renderer_{nullptr};
            VtkInteractionContext context_;

            Graphics::Datatypes::WidgetHandle currentWidget_;

            MouseButton buttonPushed_{MouseButton::NONE};

            glm::vec3 initialPickWorld_;

            glm::vec3 lastPickWorld_;

            glm::mat4 widgetTransform_{1.0f};

            std::unique_ptr<VtkTransformCalculator> calculator_;
        };

    }
}
#endif