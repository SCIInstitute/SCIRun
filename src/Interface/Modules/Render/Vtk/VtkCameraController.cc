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

#include "VtkCameraController.h"

#ifdef WITH_VTK

#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkRenderer.h>

namespace SCIRun {
    namespace Render {

        void VtkCameraController::mousePress(
            float x,
            float y,
            MouseButton button)
        {
            dragging_ = true;

            activeButton_ = button;

            lastMousePos_ = glm::vec2(x, y);
        }

        void VtkCameraController::mouseRelease()
        {
            dragging_ = false;
        }

        void VtkCameraController::mouseMove(
            float x,
            float y,
            vtkRenderer* renderer)
        {
            if (!dragging_ || !renderer)
                return;

            vtkCamera* camera = renderer->GetActiveCamera();

            if (!camera)
                return;

            const float dx = x - lastMousePos_.x;
            const float dy = y - lastMousePos_.y;

            switch (activeButton_)
            {
            case MouseButton::LEFT:
                rotate(camera, renderer, dx, dy);
                break;

            case MouseButton::MIDDLE:
                pan(camera, renderer, dx, dy);
                break;

            case MouseButton::RIGHT:
                zoom(camera, renderer, dy);
                break;

            default:
                break;
            }

            lastMousePos_ = glm::vec2(x, y);
        }

        void VtkCameraController::mouseWheel(
            int32_t delta,
            vtkRenderer* renderer)
        {
            if (!renderer)
                return;

            vtkCamera* camera = renderer->GetActiveCamera();

            if (!camera)
                return;

            zoom(camera,
                renderer,
                static_cast<float>(delta));
        }

        void VtkCameraController::rotate(
            vtkCamera* camera,
            vtkRenderer* renderer,
            float dx,
            float dy)
        {
            camera->Azimuth(-dx * rotationSpeed_);
            camera->Elevation(dy * rotationSpeed_);

            camera->OrthogonalizeViewUp();

            renderer->ResetCameraClippingRange();
        }

        void VtkCameraController::pan(
            vtkCamera* camera,
            vtkRenderer* renderer,
            float dx,
            float dy)
        {
            double position[3];
            double focalPoint[3];

            camera->GetPosition(position);
            camera->GetFocalPoint(focalPoint);

            double up[3];
            camera->GetViewUp(up);

            double right[3];
            vtkMath::Cross(
                camera->GetDirectionOfProjection(),
                up,
                right);

            for (int i = 0; i < 3; ++i)
            {
                const double offset =
                    (-dx * panSpeed_) * right[i]
                    + (dy * panSpeed_) * up[i];

                position[i] += offset;
                focalPoint[i] += offset;
            }

            camera->SetPosition(position);
            camera->SetFocalPoint(focalPoint);

            renderer->ResetCameraClippingRange();
        }

        void VtkCameraController::zoom(
            vtkCamera* camera,
            vtkRenderer* renderer,
            float amount)
        {
            const double factor =
                std::pow(1.01, amount * zoomSpeed_);

            camera->Dolly(factor);

            renderer->ResetCameraClippingRange();
        }

        void VtkCameraController::resetView(
            vtkRenderer* renderer)
        {
            if (!renderer)
                return;

            renderer->ResetCamera();

            vtkCamera* camera =
                renderer->GetActiveCamera();

            if (camera)
            {
                camera->SetViewUp(0.0, 1.0, 0.0);
                camera->OrthogonalizeViewUp();
            }

            renderer->ResetCameraClippingRange();
        }

        void VtkCameraController::setRotationSpeed(float speed)
        {
            rotationSpeed_ = speed;
        }

        void VtkCameraController::setPanSpeed(float speed)
        {
            panSpeed_ = speed;
        }

        void VtkCameraController::setZoomSpeed(float speed)
        {
            zoomSpeed_ = speed;
        }

        float VtkCameraController::rotationSpeed() const
        {
            return rotationSpeed_;
        }

        float VtkCameraController::panSpeed() const
        {
            return panSpeed_;
        }

        float VtkCameraController::zoomSpeed() const
        {
            return zoomSpeed_;
        }

    }
} // namespace SCIRun::Render

#endif