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

#include <Core/Datatypes/VTK/VtkIncludes.h>

namespace SCIRun {
namespace Render {

  void VtkCameraController::setCamera(vtkCamera* camera)
  {
    camera_ = camera;
  }

  void VtkCameraController::mousePress(float x, float y, MouseButton button)
  {
    dragging_ = true;

    activeButton_ = button;

    lastMousePos_ = glm::vec2(x, y);
  }

  void VtkCameraController::mouseRelease()
  {
    dragging_ = false;
  }

  void VtkCameraController::mouseMove(float x, float y)
  {
    if (!dragging_ || !camera_) return;

    float dx = x - lastMousePos_.x;
    float dy = y - lastMousePos_.y;

    switch (activeButton_)
    {
    case MouseButton::LEFT: rotate(dx, dy); break;

    case MouseButton::MIDDLE: pan(dx, dy); break;

    case MouseButton::RIGHT: zoom(dy); break;

    default: break;
    }

    lastMousePos_ = {x, y};
  }

  void VtkCameraController::mouseWheel(int32_t delta)
  {
    if (!camera_) return;

    zoom(static_cast<float>(delta));
  }

  void VtkCameraController::rotate(float dx, float dy)
  {
    if (!camera_) return;

    camera_->Azimuth(-dx * rotationSpeed_);

    camera_->Elevation(dy * rotationSpeed_);

    camera_->OrthogonalizeViewUp();

    camera_->Modified();
  }

  void VtkCameraController::pan(float dx, float dy)
  {
    if (!camera_) return;

    double position[3];
    double focalPoint[3];

    camera_->GetPosition(position);
    camera_->GetFocalPoint(focalPoint);
    double up[3];
    camera_->GetViewUp(up);

    double right[3];
    vtkMath::Cross(camera_->GetDirectionOfProjection(), up, right);

    for (int i = 0; i < 3; ++i)
    {
      const double offset = (-dx * panSpeed_) * right[i] + (dy * panSpeed_) * up[i];

      position[i] += offset;
      focalPoint[i] += offset;
    }

    camera_->SetPosition(position);
    camera_->SetFocalPoint(focalPoint);

  }

  void VtkCameraController::zoom(float amount)
  {
    if (!camera_) return;

    if (camera_->GetParallelProjection())
    {
      double scale = camera_->GetParallelScale();
      scale /= std::pow(1.0 + zoomSpeed_, amount);
      camera_->SetParallelScale(scale);
      camera_->Modified();
      return;
    }

    double factor = std::pow(1.0 + zoomSpeed_, amount);
    double distance = camera_->GetDistance();
    const double minDistance = 1.0; // Minimum distance to maintain to focal point

    // If zooming in would put us too close to the focal point, shift the focal point forward
    if (factor > 1.0 && (distance / factor) < minDistance)
    {
      double pos[3], fp[3], dop[3];
      camera_->GetPosition(pos);
      camera_->GetFocalPoint(fp);
      camera_->GetDirectionOfProjection(dop);

      // Move camera forward by a step proportional to minDistance
      double step = minDistance * (factor - 1.0);
      for (int i = 0; i < 3; ++i)
      {
        pos[i] += dop[i] * step;
        fp[i] = pos[i] + dop[i] * minDistance;
      }

      camera_->SetPosition(pos);
      camera_->SetFocalPoint(fp);
    }
    else
    {
      camera_->Dolly(factor);
    }

    camera_->Modified();
  }

  void VtkCameraController::resetView()
  {
    if (!camera_) return;

    camera_->SetViewUp(0.0, 1.0, 0.0);
    camera_->OrthogonalizeViewUp();

  }

}
}  // namespace SCIRun::Render

#endif