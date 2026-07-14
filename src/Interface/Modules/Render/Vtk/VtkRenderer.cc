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
#include <vtkUnstructuredGrid.h>
#include <vtkDataObject.h>
#include <vtkLookupTable.h>

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
  image_ = image_.mirrored(false, true);

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
}

void VtkRenderer::mouseMove(float x, float y, MouseButton btn)
{
}

void VtkRenderer::mouseRelease()
{
}

void VtkRenderer::mouseWheel(int delta)
{
}

void VtkRenderer::autoView()
{
}

//Data----------------------------------------------------------------------------------------------
void VtkRenderer::updateGeometries(const std::vector<VtkGeometryObjectHandle>& geometries)
{
  if (!initialized_) initialize();

  renderer_->RemoveAllViewProps();
  actors_.clear();

  for (const auto& geo : geometries)
  {
    if (!geo || !geo->dataObject) continue;

    auto ugrid = vtkUnstructuredGrid::SafeDownCast(geo->dataObject);

    if (!ugrid) continue;

    auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();

    mapper->SetInputData(ugrid);

    double range[2];
    ugrid->GetScalarRange(range);

    mapper->SetScalarRange(range);

    auto actor = vtkSmartPointer<vtkActor>::New();

    actor->SetMapper(mapper);

    renderer_->AddActor(actor);

    actors_.push_back(actor);
  }

  renderer_->ResetCamera();
}

void VtkRenderer::addInstaceOfGroup()
{
}

void VtkRenderer::addGroup()
{
}

void VtkRenderer::addDirectionalLight(glm::vec3 col, glm::vec3 dir)
{
}

void VtkRenderer::addAmbientLight(glm::vec3 col, float intensity)
{
}

void VtkRenderer::addSphereLight(glm::vec3 col, glm::vec3 position, float radius, float intensity)
{
}

void VtkRenderer::addQuadLight(glm::vec3 col, glm::vec3 position, glm::vec3 edge1, glm::vec3 edge2, float intensity)
{
}

void VtkRenderer::setLightsAsObject()
{
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

//void VtkRenderer::renderTestScene()
//{
//  if (!initialized_)
//  {
//    renderWindow_ = vtkSmartPointer<vtkRenderWindow>::New();
//    renderer_ = vtkSmartPointer<vtkRenderer>::New();
//    renderWindow_->AddRenderer(renderer_);
//
//    auto sphere = vtkSmartPointer<vtkSphereSource>::New();
//    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//    mapper->SetInputConnection(sphere->GetOutputPort());
//
//    auto actor = vtkSmartPointer<vtkActor>::New();
//    actor->SetMapper(mapper);
//
//    renderer_->AddActor(actor);
//    renderer_->SetBackground(0.1, 0.2, 0.4);
//    renderer_->ResetCamera();
//
//    renderWindow_->SetWindowName("VTK Test Window");
//    renderWindow_->SetSize(800, 600);
//
//    interactor_ = vtkSmartPointer<vtkRenderWindowInteractor>::New();
//    interactor_->SetRenderWindow(renderWindow_);
//    renderWindow_->Render();
//    interactor_->Initialize();
//
//    initialized_ = true;
//  }
//}

//void VtkRenderer::testOffscreen()
//{
//  static bool initialized = false;
//
//  static vtkSmartPointer<vtkRenderWindow> renWin;
//  static vtkSmartPointer<vtkRenderer> ren;
//  static vtkSmartPointer<vtkWindowToImageFilter> w2i;
//
//  if (!initialized)
//  {
//    renWin = vtkSmartPointer<vtkRenderWindow>::New();
//    renWin->SetOffScreenRendering(1);
//
//    ren = vtkSmartPointer<vtkRenderer>::New();
//    renWin->AddRenderer(ren);
//
//    // green sphere in lower-left
//
//    auto sphere = vtkSmartPointer<vtkSphereSource>::New();
//
//    auto sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//    sphereMapper->SetInputConnection(sphere->GetOutputPort());
//
//    auto sphereActor = vtkSmartPointer<vtkActor>::New();
//    sphereActor->SetMapper(sphereMapper);
//    sphereActor->GetProperty()->SetColor(0.0, 1.0, 0.0);
//    sphereActor->SetPosition(-2.0, -2.0, 0.0);
//
//    ren->AddActor(sphereActor);
//
//    // red cube in upper-right
//
//    auto cube = vtkSmartPointer<vtkCubeSource>::New();
//
//    auto cubeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//    cubeMapper->SetInputConnection(cube->GetOutputPort());
//
//    auto cubeActor = vtkSmartPointer<vtkActor>::New();
//    cubeActor->SetMapper(cubeMapper);
//    cubeActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
//    cubeActor->SetPosition(2.0, 2.0, 0.0);
//
//    ren->AddActor(cubeActor);
//
//    // axes
//
//    auto axes = vtkSmartPointer<vtkAxesActor>::New();
//    axes->SetTotalLength(2.0, 2.0, 2.0);
//
//    ren->AddActor(axes);
//
//    ren->SetBackground(0.1, 0.2, 0.4);
//    ren->ResetCamera();
//
//    w2i = vtkSmartPointer<vtkWindowToImageFilter>::New();
//    w2i->SetInput(renWin);
//
//    initialized = true;
//  }
//
//  renWin->SetSize(width_, height_);
//
//  // Render VTK scene
//  renWin->Render();
//
//  // Capture framebuffer
//  w2i->Modified();
//  w2i->Update();
//
//  vtkImageData* image = w2i->GetOutput();
//
//  int dims[3];
//  image->GetDimensions(dims);
//
//  int numComponents = image->GetNumberOfScalarComponents();
//
//  imagePixels_ = static_cast<unsigned char*>(image->GetScalarPointer());
//
//  if (numComponents == 3)
//  {
//    int bytesPerLine = dims[0] * 3;
//
//    image_ = QImage(imagePixels_, dims[0], dims[1], bytesPerLine, QImage::Format_RGB888).copy();
//  }
//  else if (numComponents == 4)
//  {
//    int bytesPerLine = dims[0] * 4;
//
//    image_ = QImage(imagePixels_, dims[0], dims[1], bytesPerLine, QImage::Format_RGBA8888).copy();
//  }
//
//  // VTK is usually vertically flipped relative to Qt
//  image_ = image_.mirrored(false, true);
//
//  //std::cout << "Rendered image size: " << image_.width() << "x" << image_.height() << std::endl;
//}

#endif
