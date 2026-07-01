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

#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkAxesActor.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

#include "VtkRenderer.h"
#include <Core/GeometryPrimitives/BBox.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/glu.h>
#else
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX  // prevent Windows macros from breaking std::min/max
#endif

#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif
#include <cstdio>
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
  clearViewportTest();
  //renderTestScene();
  testOffscreen();
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

void VtkRenderer::renderTestScene()
{
  if (!initialized_)
  {
    renderWindow_ = vtkSmartPointer<vtkRenderWindow>::New();
    renderer_ = vtkSmartPointer<vtkRenderer>::New();
    renderWindow_->AddRenderer(renderer_);

    auto sphere = vtkSmartPointer<vtkSphereSource>::New();
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphere->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    renderer_->AddActor(actor);
    renderer_->SetBackground(0.1, 0.2, 0.4);
    renderer_->ResetCamera();

    renderWindow_->SetWindowName("VTK Test Window");
    renderWindow_->SetSize(800, 600);

    interactor_ = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor_->SetRenderWindow(renderWindow_);
    renderWindow_->Render();
    interactor_->Initialize();

    initialized_ = true;
  }
}

void VtkRenderer::testOffscreen()
{
  static bool initialized = false;

  static vtkSmartPointer<vtkRenderWindow> renWin;
  static vtkSmartPointer<vtkRenderer> ren;
  static vtkSmartPointer<vtkWindowToImageFilter> w2i;

  if (!initialized)
  {
    // ----- VTK Setup -----

    renWin = vtkSmartPointer<vtkRenderWindow>::New();
    renWin->SetOffScreenRendering(1);

    ren = vtkSmartPointer<vtkRenderer>::New();
    renWin->AddRenderer(ren);

    auto sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetThetaResolution(64);
    sphere->SetPhiResolution(64);

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphere->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    ren->AddActor(actor);
    ren->SetBackground(0.1, 0.2, 0.4);
    ren->ResetCamera();

    w2i = vtkSmartPointer<vtkWindowToImageFilter>::New();
    w2i->SetInput(renWin);

    // ----- OpenGL Texture -----

    glGenTextures(1, &vtkTexture_);

    glBindTexture(GL_TEXTURE_2D, vtkTexture_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    initialized = true;
  }

  renWin->SetSize(width_, height_);

  // Render VTK offscreen
  renWin->Render();

  // Capture image
  w2i->Modified();
  w2i->Update();

  vtkImageData* image = w2i->GetOutput();

  int dims[3];
  image->GetDimensions(dims);

  int numComponents = image->GetNumberOfScalarComponents();

  unsigned char* pixels = static_cast<unsigned char*>(image->GetScalarPointer());

  GLenum format = (numComponents == 4) ? GL_RGBA : GL_RGB;

  // Upload image into texture

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glBindTexture(GL_TEXTURE_2D, vtkTexture_);

  glTexImage2D(GL_TEXTURE_2D, 0, (format == GL_RGBA) ? GL_RGBA8 : GL_RGB8, dims[0], dims[1], 0, format, GL_UNSIGNED_BYTE, pixels);

  glBindTexture(GL_TEXTURE_2D, 0);

  //
  // At this point vtkTexture_ contains the rendered image.
  //
  // Next step:
  //   bind vtkTexture_
  //   draw fullscreen quad
  //
}

void VtkRenderer::clearViewportTest()
{
  // Make sure we are operating on the correct framebuffer
  glViewport(0, 0, width_, height_);

  // Set a very obvious color so you know it's working (bright red)
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

  // Clear color + depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Optional: force execution (mostly for debugging)
  glFlush();
}

#endif
