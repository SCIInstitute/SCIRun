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

//#include <vtkAxesActor.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkDataObject.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>

#include "VtkRenderer.h"
#include <Core/GeometryPrimitives/BBox.h>

#include <iostream>

using namespace SCIRun;
using namespace Render;
using namespace Core::Datatypes;
using namespace Core::Geometry;

#ifdef WITH_VTK

VtkRenderer::VtkRenderer()
{
}

VtkRenderer::~VtkRenderer()
{
}

//Rendering-----------------------------------------------------------------------------------------
void VtkRenderer::renderFrame()
{
  if (!initialized_) return;

  renderWindow_->SetSize(width_, height_);

  renderWindow_->Render();

  w2i_->Modified();
  w2i_->Update();

  vtkImageData* image = w2i_->GetOutput();

  int dims[3];
  image->GetDimensions(dims);

  int numComponents = image->GetNumberOfScalarComponents();

  imagePixels_ = static_cast<unsigned char*>(image->GetScalarPointer());

  if (numComponents == 3)
  {
    int bytesPerLine = dims[0] * 3;

    image_ = QImage(imagePixels_, dims[0], dims[1], bytesPerLine, QImage::Format_RGB888).copy();
  }
  else if (numComponents == 4)
  {
    int bytesPerLine = dims[0] * 4;

    image_ = QImage(imagePixels_, dims[0], dims[1], bytesPerLine, QImage::Format_RGBA8888).copy();
  }

  // VTK is usually vertically flipped relative to Qt
  image_ = image_.flipped(Qt::Vertical);

  //std::cout << "Rendered image size: " << image_.width() << "x" << image_.height() << std::endl;
}



//Interaction---------------------------------------------------------------------------------------
void VtkRenderer::resize(uint32_t width, uint32_t height)
{
  width_ = width;
  height_ = height;

  if (renderWindow_)
  {
    renderWindow_->SetSize(width, height);
  }

  if (renderer_)
  {
    renderer_->SetViewport(0.0, 0.0, 1.0, 1.0);
  }
}

void VtkRenderer::mousePress(float x, float y, MouseButton btn)
{
  cameraController_.mousePress(x, y, btn);
}

void VtkRenderer::mouseMove(float x, float y, MouseButton btn)
{
  cameraController_.mouseMove(x, y, renderer_);

  renderFrame();
}

void VtkRenderer::mouseRelease()
{
  cameraController_.mouseRelease();
}

void VtkRenderer::mouseWheel(int32_t delta)
{
  cameraController_.mouseWheel(delta, renderer_);

  renderFrame();
}

void VtkRenderer::autoView()
{
  cameraController_.resetView(renderer_);

  renderFrame();
}

//Data----------------------------------------------------------------------------------------------
void VtkRenderer::updateGeometries(const std::vector<VtkGeometryObjectHandle>& geometries)
{
  if (!initialized_) initialize();

  renderer_->RemoveAllViewProps();
  actors_.clear();

  for (const auto& geo : geometries)
  {
    addGeometry(geo);
  }

  renderer_->ResetCamera();
}

void VtkRenderer::initialize()
{
  renderWindow_ = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow_->SetOffScreenRendering(1);

  renderer_ = vtkSmartPointer<vtkRenderer>::New();

  renderWindow_->AddRenderer(renderer_);

  renderer_->SetBackground(0.1, 0.2, 0.4);

  w2i_ = vtkSmartPointer<vtkWindowToImageFilter>::New();
  w2i_->SetInput(renderWindow_);

  initialized_ = true;
}

void VtkRenderer::addGeometry(const VtkGeometryObjectHandle& geo)
{
  if (!geo || !geo->dataObject) return;

  if (auto image = vtkImageData::SafeDownCast(geo->dataObject))
  {
    renderImageData(image, geo);
    return;
  }

  if (auto grid = vtkUnstructuredGrid::SafeDownCast(geo->dataObject))
  {
    renderUnstructuredGrid(grid, geo);
    return;
  }

  if (auto poly = vtkPolyData::SafeDownCast(geo->dataObject))
  {
    renderPolyData(poly, geo);
    return;
  }

  std::cout << "Unsupported VTK dataset" << std::endl;
}

void VtkRenderer::renderPolyData(vtkPolyData* poly, const VtkGeometryObjectHandle& geo)
{
  if (!poly) return;

  auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

  mapper->SetInputData(poly);

  auto actor = vtkSmartPointer<vtkActor>::New();

  actor->SetMapper(mapper);

  applyMaterial(actor, geo->material);

  renderer_->AddActor(actor);

  actors_.push_back(actor);
}

void VtkRenderer::renderUnstructuredGrid(vtkUnstructuredGrid* ugrid, const VtkGeometryObjectHandle& geo)
{
  if (!ugrid) return;

  double range[2];
  ugrid->GetScalarRange(range);

  auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();

  mapper->SetInputData(ugrid);

  mapper->ScalarVisibilityOn();
  mapper->SetColorModeToMapScalars();
  mapper->SetScalarModeToUsePointData();
  mapper->SetScalarRange(range);

  auto lut = createLookupTable(geo->tfn, range);

  mapper->SetLookupTable(lut);

  auto actor = vtkSmartPointer<vtkActor>::New();

  actor->SetMapper(mapper);

  applyMaterial(actor, geo->material);

  renderer_->AddActor(actor);

  actors_.push_back(actor);
}

void VtkRenderer::renderImageData(vtkImageData* image, const VtkGeometryObjectHandle& geo)
{
  auto mapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
  mapper->SetInputData(image);

  auto volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
  volumeProperty->ShadeOff();
  volumeProperty->SetInterpolationTypeToLinear();

  //----------------------------------
  // Opacity transfer function
  //----------------------------------

  auto opacity = vtkSmartPointer<vtkPiecewiseFunction>::New();

  //----------------------------------
  // Color transfer function
  //----------------------------------

  auto color = vtkSmartPointer<vtkColorTransferFunction>::New();

  if (!geo->tfn.range.empty())
  {
    // TODO:
    // Build from SCIRun transfer function
  }
  else
  {
    double range[2];
    image->GetScalarRange(range);

    opacity->AddPoint(range[0], 0.0);
    opacity->AddPoint(range[1], 1.0);

    color->AddRGBPoint(range[0], 0.0, 0.0, 1.0);
    color->AddRGBPoint(range[1], 1.0, 0.0, 0.0);
  }

  volumeProperty->SetScalarOpacity(opacity);
  volumeProperty->SetColor(color);

  auto volume = vtkSmartPointer<vtkVolume>::New();
  volume->SetMapper(mapper);
  volume->SetProperty(volumeProperty);

  renderer_->AddVolume(volume);
}

void VtkRenderer::applyMaterial(vtkActor* actor, const VtkGeometryObject::Material& mat)
{
  if (!actor) return;

  auto prop = actor->GetProperty();

  prop->SetColor(mat.color[0], mat.color[1], mat.color[2]);

  prop->SetOpacity(mat.opacity);
}

vtkSmartPointer<vtkLookupTable> VtkRenderer::createLookupTable(const VtkGeometryObject::TransferFunc& tfn, double range[2])
{
  auto lut = vtkSmartPointer<vtkLookupTable>::New();

  lut->SetRange(range);

  if (tfn.colors.empty())
  {
    lut->SetHueRange(0.667, 0.0);
  }

  lut->Build();

  return lut;
}

void VtkRenderer::addDirectionalLight(glm::vec3 color, glm::vec3 direction)
{

}

void VtkRenderer::addAmbientLight(glm::vec3 color, float intensity)
{

}

#endif
