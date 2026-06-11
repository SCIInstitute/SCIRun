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

using namespace SCIRun;
using namespace Render;
using namespace Core::Datatypes;
using namespace Core::Geometry;

#ifdef WITH_VTK
//int VtkRenderer::vtkRendererInstances = 0;
//VtkDataManager VtkRenderer::dataManager;

VtkRenderer::VtkRenderer()
{
}

VtkRenderer::~VtkRenderer()
{
}

//Rendering-----------------------------------------------------------------------------------------
void VtkRenderer::renderFrame()
{
  renderTestScene();
}



//Interaction---------------------------------------------------------------------------------------
void VtkRenderer::resize(uint32_t width, uint32_t height)
{
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
    renderWindow_ = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderWindow_->SetReadyForRendering(true);

    renderer_ = vtkSmartPointer<vtkRenderer>::New();
    renderWindow_->AddRenderer(renderer_);

    renderer_->SetBackground(1.0, 1.0, 0.0);

    auto sphere = vtkSmartPointer<vtkSphereSource>::New();

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphere->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    renderer_->AddActor(actor);

    renderer_->ResetCamera();
    renderer_->ResetCameraClippingRange();

    initialized_ = true;
  }

  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);

  int width = vp[2];
  int height = vp[3];

  if (width <= 0 || height <= 0) return;

  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  renderWindow_->SetSize(width, height);
  renderWindow_->Render();

  glClearColor(1, 0, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

#endif
