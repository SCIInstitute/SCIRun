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
    }

    void VtkScaleBarOverlay::setLength(double length)
    {
      sbLength_ = length;
    }

    void VtkScaleBarOverlay::setHeight(double height)
    {
      sbHeight_ = height;
    }

    void VtkScaleBarOverlay::setMultiplier(double mul)
    {
      sbMultiplier_ = mul;
    }

    void VtkScaleBarOverlay::setNumTicks(double num)
    {
      sbNumTicks_ = num;
    }

    void VtkScaleBarOverlay::setLineWidth(double width)
    {
      sbLineWidth_ = width;
    }

    void VtkScaleBarOverlay::setLineColor(double color)
    {
      sbLineColor_ = color;
    }

    void VtkScaleBarOverlay::setUnit(const std::string& unit)
    {
      sbUnit_ = unit;
    }

    void VtkScaleBarOverlay::setProjLength(double length)
    {
      sbProjLength_ = length;
    }

    void VtkScaleBarOverlay::updateProjectedLength()
    {
      if (!sceneRenderer_) return;

      double p1[4] = {-sbLength_ * 0.5, 0.0, 0.0, 1.0};
      double p2[4] = {sbLength_ * 0.5, 0.0, 0.0, 1.0};

      sceneRenderer_->SetWorldPoint(p1);
      sceneRenderer_->WorldToDisplay();

      double d1[3];
      sceneRenderer_->GetDisplayPoint(d1);

      sceneRenderer_->SetWorldPoint(p2);
      sceneRenderer_->WorldToDisplay();

      double d2[3];
      sceneRenderer_->GetDisplayPoint(d2);

      double dx = d2[0] - d1[0];
      double dy = d2[1] - d1[1];

      sbProjLength_ = std::sqrt(dx * dx + dy * dy);
    }

    void VtkScaleBarOverlay::updateScale()
    {
        updateProjectedLength();

        if (!barActor_)
            return;

        auto mapper =
            vtkPolyDataMapper2D::SafeDownCast(barActor_->GetMapper());

        if (!mapper)
            return;

        auto polyData = mapper->GetInput();

        if (!polyData)
            return;

        auto points = vtkSmartPointer<vtkPoints>::New();
        auto lines = vtkSmartPointer<vtkCellArray>::New();

        //------------------------------------------------------------------
        // Screen position.
        //------------------------------------------------------------------

        double x =
            static_cast<double>(10) / 100.0 * width_;

        double y =
            static_cast<double>(90) / 100.0 * height_;

        //------------------------------------------------------------------
        // Main horizontal bar.
        //------------------------------------------------------------------

        std::ostringstream ss;
        ss << sbLength_ * sbMultiplier_;

        if (!sbUnit_.empty()) ss << " " << sbUnit_;

        std::string label = ss.str();

        double textWidth = sbFontSize_ * label.length() * 0.6;

        double gap = 5.0;

        double lengthPixels = sbProjLength_;

        double x0 = x - lengthPixels - textWidth - gap;
        double x1 = x - textWidth - gap;

        // Main bar.
        vtkIdType p0 = points->InsertNextPoint(x0, y, 0.0);
        vtkIdType p1 = points->InsertNextPoint(x1, y, 0.0);

        lines->InsertNextCell(2);
        lines->InsertCellPoint(p0);
        lines->InsertCellPoint(p1);

        // Ticks.
        int numTicks = std::max(2, static_cast<int>(std::round(sbNumTicks_)));

        for (int i = 0; i < numTicks; ++i)
        {
          double tx = x0 + i * lengthPixels / static_cast<double>(numTicks - 1);

          vtkIdType a = points->InsertNextPoint(tx, y, 0.0);

          vtkIdType b = points->InsertNextPoint(tx, y + sbHeight_, 0.0);

          lines->InsertNextCell(2);
          lines->InsertCellPoint(a);
          lines->InsertCellPoint(b);
        }

        polyData->SetPoints(points);
        polyData->SetLines(lines);

        polyData->Modified();
    }

}
}