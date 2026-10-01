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

#include "VtkOrientationOverlay.h"

namespace SCIRun {
namespace Render {

  void VtkOrientationOverlay::initialize(vtkRenderer* renderer)
  {
    if (!renderer) return;

    renderWindow_ = renderer->GetRenderWindow();

    overlayRenderer_ = vtkSmartPointer<vtkRenderer>::New();

    overlayRenderer_->SetLayer(1);
    overlayRenderer_->InteractiveOff();

    axesActor_ = vtkSmartPointer<vtkAxesActor>::New();

    overlayRenderer_->AddActor(axesActor_);

    if (renderWindow_)
    {
      renderWindow_->SetNumberOfLayers(2);
      renderWindow_->AddRenderer(overlayRenderer_);
    }

    updateViewport();
  }

  void VtkOrientationOverlay::cameraChanged(vtkCamera* camera)
  {
    if (!visible_ || !camera || !overlayRenderer_)
    {
      return;
    }

    vtkCamera* overlayCamera = overlayRenderer_->GetActiveCamera();

    if (!overlayCamera) return;

    double pos[3];
    double focal[3];
    double up[3];

    camera->GetPosition(pos);
    camera->GetFocalPoint(focal);
    camera->GetViewUp(up);

    double dir[3]{pos[0] - focal[0], pos[1] - focal[1], pos[2] - focal[2]};

    vtkMath::Normalize(dir);

    constexpr double distance = 5.0;

    overlayCamera->SetFocalPoint(0.0, 0.0, 0.0);

    overlayCamera->SetPosition(dir[0] * distance, dir[1] * distance, dir[2] * distance);

    overlayCamera->SetViewUp(up);

    overlayCamera->OrthogonalizeViewUp();

    overlayRenderer_->ResetCameraClippingRange();
  }

  void VtkOrientationOverlay::resize(int width, int height)
  {
    width_ = std::max(width, 1);
    height_ = std::max(height, 1);

    updateViewport();
  }

  void VtkOrientationOverlay::setVisible(bool visible)
  {
    visible_ = visible;

    if (axesActor_) axesActor_->SetVisibility(visible);

    if (overlayRenderer_) overlayRenderer_->SetDraw(visible);

    if (visible) updateViewport();
  }

  void VtkOrientationOverlay::setSize(int value)
  {
    size_ = std::max(value, 1);

    updateViewport();
  }

  void VtkOrientationOverlay::setPosX(int value)
  {
    posX_ = value;
    updateViewport();
  }

  void VtkOrientationOverlay::setPosY(int value)
  {
    posY_ = value;
    updateViewport();
  }

  void VtkOrientationOverlay::setPosition(int x, int y)
  {
    posX_ = x;
    posY_ = y;
    updateViewport();
  }

  void VtkOrientationOverlay::updateViewport()
  {
    if (!overlayRenderer_) return;

    if (width_ <= 0 || height_ <= 0) return;

    double xmin = static_cast<double>(posX_) / static_cast<double>(width_);

    double ymin = static_cast<double>(posY_) / static_cast<double>(height_);

    double xmax = static_cast<double>(posX_ + size_) / static_cast<double>(width_);

    double ymax = static_cast<double>(posY_ + size_) / static_cast<double>(height_);

    xmin = std::clamp(xmin, 0.0, 1.0);
    ymin = std::clamp(ymin, 0.0, 1.0);
    xmax = std::clamp(xmax, 0.0, 1.0);
    ymax = std::clamp(ymax, 0.0, 1.0);

    overlayRenderer_->SetViewport(xmin, ymin, xmax, ymax);
  }

}
}