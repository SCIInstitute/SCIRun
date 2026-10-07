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

namespace SCIRun {
namespace Render {

    void VtkScaleBarOverlay::initialize(vtkRenderer* renderer)
    {
        if (!renderer)
            return;

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

    void VtkScaleBarOverlay::cameraChanged(vtkCamera*)
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

    void VtkScaleBarOverlay::updateScale()
    {
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
            static_cast<double>(posX_) / 100.0 * width_;

        double y =
            static_cast<double>(posY_) / 100.0 * height_;

        //------------------------------------------------------------------
        // Placeholder scale.
        //
        // Later:
        // worldUnitsPerPixel -> nice value -> barPixels_
        //------------------------------------------------------------------

        double lengthPixels = static_cast<double>(barPixels_);

        //------------------------------------------------------------------
        // Main horizontal bar.
        //------------------------------------------------------------------

        vtkIdType p0 = points->InsertNextPoint(x, y, 0.0);
        vtkIdType p1 = points->InsertNextPoint(x + lengthPixels, y, 0.0);

        lines->InsertNextCell(2);
        lines->InsertCellPoint(p0);
        lines->InsertCellPoint(p1);

        //------------------------------------------------------------------
        // Left tick.
        //------------------------------------------------------------------

        vtkIdType p2 = points->InsertNextPoint(x, y - 5.0, 0.0);
        vtkIdType p3 = points->InsertNextPoint(x, y + 5.0, 0.0);

        lines->InsertNextCell(2);
        lines->InsertCellPoint(p2);
        lines->InsertCellPoint(p3);

        //------------------------------------------------------------------
        // Right tick.
        //------------------------------------------------------------------

        vtkIdType p4 = points->InsertNextPoint(
            x + lengthPixels, y - 5.0, 0.0);

        vtkIdType p5 = points->InsertNextPoint(
            x + lengthPixels, y + 5.0, 0.0);

        lines->InsertNextCell(2);
        lines->InsertCellPoint(p4);
        lines->InsertCellPoint(p5);

        polyData->SetPoints(points);
        polyData->SetLines(lines);

        polyData->Modified();

        //------------------------------------------------------------------
        // Placeholder label.
        //------------------------------------------------------------------

        textActor_->SetInput("10 mm");

        textActor_->SetDisplayPosition(
            static_cast<int>(x + lengthPixels * 0.5 - 20.0),
            static_cast<int>(y + 10.0));
    }

}
}