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

#include "VtkScaleBarOverlay.h"
#include <sstream>

namespace SCIRun {
namespace Render {

    void VtkScaleBarOverlay::initialize(vtkRenderer* renderer)
    {
        if (!renderer)
            return;

        sceneRenderer_ = renderer;

        auto renderWindow = renderer->GetRenderWindow();

        overlayRenderer_ = vtkSmartPointer<vtkRenderer>::New();

        overlayRenderer_->SetLayer(1);
        overlayRenderer_->InteractiveOff();

        // Create empty geometry.
        auto points = vtkSmartPointer<vtkPoints>::New();
        auto lines = vtkSmartPointer<vtkCellArray>::New();
        auto polyData = vtkSmartPointer<vtkPolyData>::New();

        polyData->SetPoints(points);
        polyData->SetLines(lines);

        auto mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
        mapper->SetInputData(polyData);

        barActor_ = vtkSmartPointer<vtkActor2D>::New();
        barActor_->SetMapper(mapper);
        barActor_->GetProperty()->SetColor(1.0, 1.0, 1.0);
        barActor_->GetProperty()->SetLineWidth(2.0);

        textActor_ = vtkSmartPointer<vtkTextActor>::New();

        textActor_->GetTextProperty()->SetColor(1.0, 1.0, 1.0);
        textActor_->GetTextProperty()->SetFontSize(16);
        textActor_->GetTextProperty()->BoldOn();

        overlayRenderer_->AddActor(barActor_);
        overlayRenderer_->AddActor2D(textActor_);

        if (renderWindow)
        {
            renderWindow->SetNumberOfLayers(2);
            renderWindow->AddRenderer(overlayRenderer_);
        }

        setVisible(visible_);
        if (visible_)
            updateScale();
    }

    void VtkScaleBarOverlay::cameraChanged(vtkCamera* camera)
    {
        if (!overlayRenderer_)
            return;

        updateScale();
    }

    void VtkScaleBarOverlay::resize(int width, int height)
    {
        width_ = std::max(width, 1);
        height_ = std::max(height, 1);

        updateScale();
    }

    void VtkScaleBarOverlay::setVisible(bool visible)
    {
      visible_ = visible;

      if (barActor_) barActor_->SetVisibility(visible);
      if (textActor_) textActor_->SetVisibility(visible);

      if (overlayRenderer_) overlayRenderer_->SetDraw(visible);

      if (visible) updateScale();
    }

    void VtkScaleBarOverlay::setFontSize(int size)
    {
      sbFontSize_ = size;
      updateScale();
    }

    void VtkScaleBarOverlay::setLength(double length)
    {
      sbLength_ = length;
      updateScale();
    }

    void VtkScaleBarOverlay::setHeight(double height)
    {
      sbHeight_ = height;
      updateScale();
    }

    void VtkScaleBarOverlay::setMultiplier(double mul)
    {
      sbMultiplier_ = mul;
      updateScale();
    }

    void VtkScaleBarOverlay::setNumTicks(double num)
    {
      sbNumTicks_ = num;
      updateScale();
    }

    void VtkScaleBarOverlay::setLineWidth(double width)
    {
      sbLineWidth_ = width;
      updateScale();
    }

    void VtkScaleBarOverlay::setLineColor(double color)
    {
      sbLineColor_ = color;
      updateScale();
    }

    void VtkScaleBarOverlay::setUnit(const std::string& unit)
    {
      sbUnit_ = unit;
      updateScale();
    }

    void VtkScaleBarOverlay::setProjLength(double length)
    {
      sbProjLength_ = length;
      updateScale();
    }

    void VtkScaleBarOverlay::updateProjectedLength()
    {
      if (!sceneRenderer_) return;

      vtkCamera* camera = sceneRenderer_->GetActiveCamera();
      if (!camera) return;

      int* size = sceneRenderer_->GetSize();
      if (!size || size[1] <= 0) return;

      // Orthographic camera
      if (camera->GetParallelProjection())
      {
        // World height visible in viewport
        const double worldHeight = 2.0 * camera->GetParallelScale();

        const double pixelsPerWorldUnit = static_cast<double>(size[1]) / worldHeight;

        sbProjLength_ = sbLength_ * pixelsPerWorldUnit;
        return;
      }

      // Perspective camera

      double fp[3];
      camera->GetFocalPoint(fp);

      double dop[3];
      camera->GetDirectionOfProjection(dop);
      vtkMath::Normalize(dop);

      double up[3];
      camera->GetViewUp(up);
      vtkMath::Normalize(up);

      double right[3];
      vtkMath::Cross(dop, up, right);
      vtkMath::Normalize(right);

      double p1[4];
      double p2[4];

      for (int i = 0; i < 3; ++i)
      {
        p1[i] = fp[i] - 0.5 * sbLength_ * right[i];
        p2[i] = fp[i] + 0.5 * sbLength_ * right[i];
      }

      p1[3] = 1.0;
      p2[3] = 1.0;

      sceneRenderer_->SetWorldPoint(p1);
      sceneRenderer_->WorldToDisplay();

      double d1[3];
      sceneRenderer_->GetDisplayPoint(d1);

      sceneRenderer_->SetWorldPoint(p2);
      sceneRenderer_->WorldToDisplay();

      double d2[3];
      sceneRenderer_->GetDisplayPoint(d2);

      const double dx = d2[0] - d1[0];
      const double dy = d2[1] - d1[1];

      sbProjLength_ = std::sqrt(dx * dx + dy * dy);
    }

    void VtkScaleBarOverlay::updateScale()
    {
      updateProjectedLength();

      if (!barActor_) return;

      auto mapper = vtkPolyDataMapper2D::SafeDownCast(barActor_->GetMapper());

      if (!mapper) return;

      auto polyData = mapper->GetInput();

      if (!polyData) return;

      auto points = vtkSmartPointer<vtkPoints>::New();
      auto lines = vtkSmartPointer<vtkCellArray>::New();

      //------------------------------------------------------------------
      // Screen position.
      //------------------------------------------------------------------

      const double x = static_cast<double>(90) / 100.0 * width_;

      const double y = static_cast<double>(10) / 100.0 * height_;

      //------------------------------------------------------------------
      // Build label.
      //------------------------------------------------------------------

      std::ostringstream ss;
      ss << sbLength_ * sbMultiplier_;

      if (!sbUnit_.empty()) ss << " " << sbUnit_;

      const std::string label = ss.str();

      constexpr double gap = 5.0;

      const double lengthPixels = sbProjLength_;

      // Bar occupies [x0, x1]
      const double x0 = x - lengthPixels;
      const double x1 = x;

      //------------------------------------------------------------------
      // Main horizontal bar.
      //------------------------------------------------------------------

      vtkIdType p0 = points->InsertNextPoint(x0, y, 0.0);
      vtkIdType p1 = points->InsertNextPoint(x1, y, 0.0);

      lines->InsertNextCell(2);
      lines->InsertCellPoint(p0);
      lines->InsertCellPoint(p1);

      //------------------------------------------------------------------
      // Ticks.
      //------------------------------------------------------------------

      const int numTicks = std::max(2, static_cast<int>(std::round(sbNumTicks_)));

      for (int i = 0; i < numTicks; ++i)
      {
        const double tx = x0 + i * lengthPixels / static_cast<double>(numTicks - 1);

        vtkIdType a = points->InsertNextPoint(tx, y, 0.0);
        vtkIdType b = points->InsertNextPoint(tx, y + sbHeight_, 0.0);

        lines->InsertNextCell(2);
        lines->InsertCellPoint(a);
        lines->InsertCellPoint(b);
      }

      //------------------------------------------------------------------
      // Update polydata.
      //------------------------------------------------------------------

      polyData->SetPoints(points);
      polyData->SetLines(lines);
      polyData->Modified();

      //------------------------------------------------------------------
      // Line appearance.
      //------------------------------------------------------------------

      if (auto* prop = barActor_->GetProperty())
      {
        prop->SetColor(sbLineColor_, sbLineColor_, sbLineColor_);

        prop->SetLineWidth(sbLineWidth_);
      }

      //------------------------------------------------------------------
      // Text.
      //------------------------------------------------------------------

      if (textActor_)
      {
        textActor_->SetInput(label.c_str());

        auto* textProp = textActor_->GetTextProperty();

        textProp->SetFontSize(sbFontSize_);

        textProp->SetColor(sbLineColor_, sbLineColor_, sbLineColor_);

        textProp->SetJustificationToLeft();
        textProp->SetVerticalJustificationToCentered();

        // Center of the entire scale bar extent.
        const double centerY = y + 0.5 * sbHeight_;

        textActor_->SetPosition(x1 + gap, centerY);
      }
    }

}
}