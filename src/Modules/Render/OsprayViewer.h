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


#ifndef MODULES_RENDER_OSPRAY_VIEWER_H
#define MODULES_RENDER_OSPRAY_VIEWER_H

#include <Dataflow/Network/ModuleWithAsyncDynamicPorts.h>
#include <Core/Thread/Mutex.h>
#include <Core/Algorithms/Base/AlgorithmMacros.h>
#include <Modules/Render/share.h>

namespace SCIRun {

  namespace Core
  {
    namespace Algorithms
    {
      namespace Render
      {
        ALGORITHM_PARAMETER_DECL(ShowPlane);
        ALGORITHM_PARAMETER_DECL(AutoRotationRate);
        ALGORITHM_PARAMETER_DECL(RendererChoice);
        ALGORITHM_PARAMETER_DECL(SeparateModelPerObject);
        ALGORITHM_PARAMETER_DECL(ShowShadows);
        ALGORITHM_PARAMETER_DECL(ShowFrameRate);
        ALGORITHM_PARAMETER_DECL(ShowRenderAnnotations);
        ALGORITHM_PARAMETER_DECL(SubsampleDuringInteraction);
        ALGORITHM_PARAMETER_DECL(SamplesPerPixel);
        ALGORITHM_PARAMETER_DECL(AOSamples);
        ALGORITHM_PARAMETER_DECL(ViewerHeight);
        ALGORITHM_PARAMETER_DECL(ViewerWidth);
        ALGORITHM_PARAMETER_DECL(CameraViewAtX);
        ALGORITHM_PARAMETER_DECL(CameraViewAtY);
        ALGORITHM_PARAMETER_DECL(CameraViewAtZ);
        ALGORITHM_PARAMETER_DECL(CameraViewFromX);
        ALGORITHM_PARAMETER_DECL(CameraViewFromY);
        ALGORITHM_PARAMETER_DECL(CameraViewFromZ);
        ALGORITHM_PARAMETER_DECL(CameraViewUpX);
        ALGORITHM_PARAMETER_DECL(CameraViewUpY);
        ALGORITHM_PARAMETER_DECL(CameraViewUpZ);
        ALGORITHM_PARAMETER_DECL(FrameWriterFilename);
        ALGORITHM_PARAMETER_DECL(BackgroundColor);
        ALGORITHM_PARAMETER_DECL(ShowAmbientLight);
        ALGORITHM_PARAMETER_DECL(AmbientLightColor);
        ALGORITHM_PARAMETER_DECL(AmbientLightIntensity);
        ALGORITHM_PARAMETER_DECL(ShowDirectionalLight);
        ALGORITHM_PARAMETER_DECL(DirectionalLightColor);
        ALGORITHM_PARAMETER_DECL(DirectionalLightIntensity);
        ALGORITHM_PARAMETER_DECL(DirectionalLightAzimuth);
        ALGORITHM_PARAMETER_DECL(DirectionalLightElevation);
        ALGORITHM_PARAMETER_DECL(ShowProbe);
        ALGORITHM_PARAMETER_DECL(ProbeX);
        ALGORITHM_PARAMETER_DECL(ProbeY);
        ALGORITHM_PARAMETER_DECL(ProbeZ);
        ALGORITHM_PARAMETER_DECL(InvertZoom);
        ALGORITHM_PARAMETER_DECL(ZoomSpeed);
      }
    }
  }

namespace Modules {
namespace Render {

  class SCISHARE OsprayViewer : public Dataflow::Networks::ModuleWithAsyncDynamicPorts,
    public Has1InputPort<AsyncDynamicPortTag<OsprayGeometryPortTag>>,
    public HasNoOutputPorts
  {
  public:
    OsprayViewer();
    virtual void asyncExecute(const Dataflow::Networks::PortId& pid, Core::Datatypes::DatatypeHandle data) override;
    virtual void setStateDefaults() override;

    INPUT_PORT_DYNAMIC(0, GeneralGeom, OsprayGeometryObject);
    virtual void execute() override;

    MODULE_TRAITS_AND_INFO(ModuleHasUI)

  #ifndef WITH_OSPRAY
    DISABLED_WITHOUT_ABOVE_COMPILE_FLAG
  #endif

  protected:
    virtual void portRemovedSlotImpl(const Dataflow::Networks::PortId& pid) override;
  private:
    void sendCompositeGeometry();
  };
}}}

#endif
