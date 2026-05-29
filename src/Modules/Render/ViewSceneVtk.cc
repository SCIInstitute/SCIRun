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


#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkAxesActor.h>

#include <es-log/trace-log.h>
#include <Modules/Render/ViewSceneVtk.h>
#include <Modules/Render/ViewScene.h>
#include <Core/Datatypes/Color.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Modules::Render;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms::Render;

MODULE_INFO_DEF(ViewSceneVtk, Render, SCIRun)

//ALGORITHM_PARAMETER_DEF(Render, ShowPlane);
//ALGORITHM_PARAMETER_DEF(Render, AutoRotationRate);
//ALGORITHM_PARAMETER_DEF(Render, RendererChoice);
//ALGORITHM_PARAMETER_DEF(Render, SeparateModelPerObject);
//ALGORITHM_PARAMETER_DEF(Render, ShowShadows);
//ALGORITHM_PARAMETER_DEF(Render, ShowFrameRate);
//ALGORITHM_PARAMETER_DEF(Render, ShowRenderAnnotations);
//ALGORITHM_PARAMETER_DEF(Render, SubsampleDuringInteraction);
//ALGORITHM_PARAMETER_DEF(Render, SamplesPerPixel);
//ALGORITHM_PARAMETER_DEF(Render, AOSamples);
//ALGORITHM_PARAMETER_DEF(Render, ViewerHeight);
//ALGORITHM_PARAMETER_DEF(Render, ViewerWidth);
//ALGORITHM_PARAMETER_DEF(Render, CameraViewAtX);
//ALGORITHM_PARAMETER_DEF(Render, CameraViewAtY);
//ALGORITHM_PARAMETER_DEF(Render, CameraViewAtZ);
//ALGORITHM_PARAMETER_DEF(Render, CameraViewFromX);
//ALGORITHM_PARAMETER_DEF(Render, CameraViewFromY);
//ALGORITHM_PARAMETER_DEF(Render, CameraViewFromZ);
//ALGORITHM_PARAMETER_DEF(Render, CameraViewUpX);
//ALGORITHM_PARAMETER_DEF(Render, CameraViewUpY);
//ALGORITHM_PARAMETER_DEF(Render, CameraViewUpZ);
//ALGORITHM_PARAMETER_DEF(Render, FrameWriterFilename);
//ALGORITHM_PARAMETER_DEF(Render, BackgroundColor);
//ALGORITHM_PARAMETER_DEF(Render, ShowAmbientLight);
//ALGORITHM_PARAMETER_DEF(Render, AmbientLightColor);
//ALGORITHM_PARAMETER_DEF(Render, AmbientLightIntensity);
//ALGORITHM_PARAMETER_DEF(Render, ShowDirectionalLight);
//ALGORITHM_PARAMETER_DEF(Render, DirectionalLightColor);
//ALGORITHM_PARAMETER_DEF(Render, DirectionalLightIntensity);
//ALGORITHM_PARAMETER_DEF(Render, DirectionalLightAzimuth);
//ALGORITHM_PARAMETER_DEF(Render, DirectionalLightElevation);
//ALGORITHM_PARAMETER_DEF(Render, ShowProbe);
//ALGORITHM_PARAMETER_DEF(Render, ProbeX);
//ALGORITHM_PARAMETER_DEF(Render, ProbeY);
//ALGORITHM_PARAMETER_DEF(Render, ProbeZ);
//ALGORITHM_PARAMETER_DEF(Render, InvertZoom);
//ALGORITHM_PARAMETER_DEF(Render, ZoomSpeed);

ViewSceneVtk::ViewSceneVtk() : ModuleWithAsyncDynamicPorts(staticInfo_, true)
{
  RENDERER_LOG_FUNCTION_SCOPE;
  //INITIALIZE_PORT(GeneralGeom);
}

void ViewSceneVtk::setStateDefaults()
{
  // keep empty for now (we don't need SCIRun params for testing)
}

void ViewSceneVtk::execute()
{
  renderTestScene();
}

void ViewSceneVtk::renderTestScene()
{
  // ----------------------------
  // Renderer / Window
  // ----------------------------
  auto renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->SetBackground(0.1, 0.2, 0.3);

  auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(800, 600);

  auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  interactor->SetRenderWindow(renderWindow);

  // ----------------------------
  // Sphere
  // ----------------------------
  auto sphereSource = vtkSmartPointer<vtkSphereSource>::New();
  sphereSource->SetCenter(0.0, 0.0, 0.0);
  sphereSource->SetRadius(1.0);

  auto sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  sphereMapper->SetInputConnection(sphereSource->GetOutputPort());

  auto sphereActor = vtkSmartPointer<vtkActor>::New();
  sphereActor->SetMapper(sphereMapper);
  sphereActor->GetProperty()->SetColor(1.0, 0.0, 0.0);  // red

  // ----------------------------
  // Cube
  // ----------------------------
  auto cubeSource = vtkSmartPointer<vtkCubeSource>::New();
  cubeSource->SetCenter(2.0, 0.0, 0.0);

  auto cubeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  cubeMapper->SetInputConnection(cubeSource->GetOutputPort());

  auto cubeActor = vtkSmartPointer<vtkActor>::New();
  cubeActor->SetMapper(cubeMapper);
  cubeActor->GetProperty()->SetColor(0.0, 1.0, 0.0);  // green

  // ----------------------------
  // Axes
  // ----------------------------
  auto axes = vtkSmartPointer<vtkAxesActor>::New();
  axes->SetTotalLength(3.0, 3.0, 3.0);

  // ----------------------------
  // Add to renderer
  // ----------------------------
  renderer->AddActor(sphereActor);
  renderer->AddActor(cubeActor);
  renderer->AddActor(axes);

  // ----------------------------
  // Render
  // ----------------------------
  renderWindow->Render();
  interactor->Start();
}
