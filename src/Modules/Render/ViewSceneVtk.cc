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

ViewSceneVtk::ViewSceneVtk() : ModuleWithAsyncDynamicPorts(staticInfo_, true)
{
  RENDERER_LOG_FUNCTION_SCOPE;
  INITIALIZE_PORT(GeneralGeom);
}

void ViewSceneVtk::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::ShowPlane, true);
  state->setValue(Parameters::AutoRotationRate, 0.025);
  state->setValue(Parameters::RendererChoice, std::string("scivis"));
  state->setValue(Parameters::SeparateModelPerObject, false);
  state->setValue(Parameters::ShowShadows, true);
  state->setValue(Parameters::ShowFrameRate, false);
  state->setValue(Parameters::ShowRenderAnnotations, false);
  state->setValue(Parameters::SubsampleDuringInteraction, false);
  state->setValue(Parameters::SamplesPerPixel, 1);
  state->setValue(Parameters::AOSamples, 1);
  state->setValue(Parameters::ViewerHeight, 600);
  state->setValue(Parameters::ViewerWidth, 800);
  state->setValue(Parameters::CameraViewAtX, 0.0);
  state->setValue(Parameters::CameraViewAtY, 0.0);
  state->setValue(Parameters::CameraViewAtZ, 0.0);
  state->setValue(Parameters::CameraViewFromX, 10.0);
  state->setValue(Parameters::CameraViewFromY, 10.0);
  state->setValue(Parameters::CameraViewFromZ, 0.0);
  state->setValue(Parameters::CameraViewUpX, 0.0);
  state->setValue(Parameters::CameraViewUpY, 0.0);
  state->setValue(Parameters::CameraViewUpZ, 1.0);
  state->setValue(Parameters::FrameWriterFilename, std::string("frames.png"));
  state->setValue(Parameters::BackgroundColor, ColorRGB(0.0, 0.0, 0.0).toString());
  state->setValue(Parameters::ShowAmbientLight, true);
  state->setValue(Parameters::AmbientLightColor, ColorRGB(1.0, 1.0, 1.0).toString());
  state->setValue(Parameters::AmbientLightIntensity, 0.1);
  state->setValue(Parameters::ShowDirectionalLight, true);
  state->setValue(Parameters::DirectionalLightColor, ColorRGB(1.0, 1.0, 1.0).toString());
  state->setValue(Parameters::DirectionalLightIntensity, 1.0);
  state->setValue(Parameters::DirectionalLightAzimuth, 80);
  state->setValue(Parameters::DirectionalLightElevation, 65);
  state->setValue(Parameters::ShowProbe, false);
  state->setValue(Parameters::ProbeX, 0.0);
  state->setValue(Parameters::ProbeY, 0.0);
  state->setValue(Parameters::ProbeZ, 0.0);
  state->setValue(Parameters::InvertZoom, false);
  state->setValue(Parameters::ZoomSpeed, 1.0);
}

void ViewSceneVtk::portRemovedSlotImpl(const PortId&)
{
  sendCompositeGeometry();
}

void ViewSceneVtk::asyncExecute(const PortId&, DatatypeHandle data)
{
  if (!data) return;

  auto geom = std::dynamic_pointer_cast<VtkGeometryObject>(data);
  if (!geom)
  {
    error("Logical error: not a geometry object on ViewSceneVtk");
    return;
  }

  sendCompositeGeometry();
}

void ViewSceneVtk::execute()
{
#ifndef WITH_VTK
  error("Must compile WITH_VTK to enable this module.");
  return;
#endif
}

void ViewSceneVtk::sendCompositeGeometry()
{
  auto allGeom = getValidDynamicInputs(GeneralGeom);
  // logWarning("allGeom size {}", allGeom.size());
  if (!allGeom.empty())
  {
    // logWarning("flattened size {}", flattened.size());
    VtkGeometryObjectHandle composite(new CompositeVtkGeometryObject(allGeom));
    // logWarning("composite ptr {}", composite.get());
    get_state()->setTransientValue(Parameters::GeomData, composite, true);
  }
}
