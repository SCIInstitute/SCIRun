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


#ifndef MODULES_RENDER_VIEWSCENE_H
#define MODULES_RENDER_VIEWSCENE_H

#include <Dataflow/Network/ModuleWithAsyncDynamicPorts.h>
#include <Core/Thread/Mutex.h>
#include <Core/Algorithms/Base/AlgorithmMacros.h>
//TODO: split out header with shared state keys.
#include <Modules/Render/OsprayViewer.h>
#include <Modules/Render/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Render
      {
        // these are transient state keys for right now
        ALGORITHM_PARAMETER_DECL(GeomData);
        ALGORITHM_PARAMETER_DECL(VSMutex);
        ALGORITHM_PARAMETER_DECL(GeometryFeedbackInfo);
        ALGORITHM_PARAMETER_DECL(ScreenshotData);
        // these should move from transient to saved
        ALGORITHM_PARAMETER_DECL(MeshComponentSelection);
        ALGORITHM_PARAMETER_DECL(ShowFieldStates);

        // save/load confirmed. Need refactoring to standard push/pull model.
        //ALGORITHM_PARAMETER_DECL(BackgroundColor); -->in OsprayViewer.h
        ALGORITHM_PARAMETER_DECL(Ambient);
        ALGORITHM_PARAMETER_DECL(Diffuse);
        ALGORITHM_PARAMETER_DECL(Specular);
        ALGORITHM_PARAMETER_DECL(Shine);
        ALGORITHM_PARAMETER_DECL(Emission); // not connected
        ALGORITHM_PARAMETER_DECL(FogOn);
        ALGORITHM_PARAMETER_DECL(ObjectsOnly); // not connected
        ALGORITHM_PARAMETER_DECL(UseBGColor);
        ALGORITHM_PARAMETER_DECL(FogStart);
        ALGORITHM_PARAMETER_DECL(FogEnd);
        ALGORITHM_PARAMETER_DECL(FogColor);
        ALGORITHM_PARAMETER_DECL(ShowScaleBar);  // issues with how/when it is shown
        ALGORITHM_PARAMETER_DECL(ScaleBarUnitValue);
        ALGORITHM_PARAMETER_DECL(ScaleBarLength);
        ALGORITHM_PARAMETER_DECL(ScaleBarHeight);
        ALGORITHM_PARAMETER_DECL(ScaleBarMultiplier);
        ALGORITHM_PARAMETER_DECL(ScaleBarNumTicks);
        ALGORITHM_PARAMETER_DECL(ScaleBarLineWidth);
        ALGORITHM_PARAMETER_DECL(ScaleBarFontSize);


        // save/load confirmed, uses standard widget managers.


        // save/load has issues.


        // not used--GUI hidden/never implemented
        ALGORITHM_PARAMETER_DECL(Lighting);
        ALGORITHM_PARAMETER_DECL(ShowBBox);
        ALGORITHM_PARAMETER_DECL(UseClip);
        ALGORITHM_PARAMETER_DECL(Stereo);
        ALGORITHM_PARAMETER_DECL(BackCull);
        ALGORITHM_PARAMETER_DECL(DisplayList);
        ALGORITHM_PARAMETER_DECL(StereoFusion);
        ALGORITHM_PARAMETER_DECL(PolygonOffset);
        ALGORITHM_PARAMETER_DECL(TextOffset);
        ALGORITHM_PARAMETER_DECL(FieldOfView);
      }
    }
  }

namespace Modules {
namespace Render {

  struct SCISHARE RGBMatrices
  {
    Core::Datatypes::DenseMatrixHandle red;
    Core::Datatypes::DenseMatrixHandle green;
    Core::Datatypes::DenseMatrixHandle blue;
  };

  using ShowFieldStatesMap = std::map<std::string, Dataflow::Networks::ModuleStateHandle>;

/// @class ViewScene
/// @brief The ViewScene displays interactive graphical output to the computer screen.
///
/// Use the ViewScene to see a geometry, or spatial data. The ViewScene
/// provides access to many simulation parameters and controls, thus,
/// indirectly initiates new iterations of the simulation steps important to
/// computational steering.

  class SCISHARE ViewScene : public Dataflow::Networks::ModuleWithAsyncDynamicPorts,
    public Has1InputPort<AsyncDynamicPortTag<GeometryPortTag>>,
    public Has3OutputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag>
  {
  public:
    ViewScene();
    ~ViewScene();
    void asyncExecute(const Dataflow::Networks::PortId& pid, Core::Datatypes::DatatypeHandle data) override;
    void setStateDefaults() override;

    static const Core::Algorithms::AlgorithmParameterName HeadLightOn;
    static const Core::Algorithms::AlgorithmParameterName Light1On;
    static const Core::Algorithms::AlgorithmParameterName Light2On;
    static const Core::Algorithms::AlgorithmParameterName Light3On;
    static const Core::Algorithms::AlgorithmParameterName HeadLightColor;
    static const Core::Algorithms::AlgorithmParameterName Light1Color;
    static const Core::Algorithms::AlgorithmParameterName Light2Color;
    static const Core::Algorithms::AlgorithmParameterName Light3Color;
    static const Core::Algorithms::AlgorithmParameterName HeadLightAzimuth;
    static const Core::Algorithms::AlgorithmParameterName Light1Azimuth;
    static const Core::Algorithms::AlgorithmParameterName Light2Azimuth;
    static const Core::Algorithms::AlgorithmParameterName Light3Azimuth;
    static const Core::Algorithms::AlgorithmParameterName HeadLightInclination;
    static const Core::Algorithms::AlgorithmParameterName Light1Inclination;
    static const Core::Algorithms::AlgorithmParameterName Light2Inclination;
    static const Core::Algorithms::AlgorithmParameterName Light3Inclination;
    static const Core::Algorithms::AlgorithmParameterName ShowViewer;
    static const Core::Algorithms::AlgorithmParameterName CameraDistance;
    static const Core::Algorithms::AlgorithmParameterName CameraDistanceMinimum;
    static const Core::Algorithms::AlgorithmParameterName CameraLookAt;
    static const Core::Algorithms::AlgorithmParameterName CameraRotation;
    static const Core::Algorithms::AlgorithmParameterName IsExecuting;
    static const Core::Algorithms::AlgorithmParameterName TimeExecutionFinished;
    static const Core::Algorithms::AlgorithmParameterName HasNewGeometry;

    INPUT_PORT_DYNAMIC(0, GeneralGeom, GeometryObject)
    OUTPUT_PORT(0, ScreenshotDataRed, DenseMatrix)
    OUTPUT_PORT(1, ScreenshotDataGreen, DenseMatrix)
    OUTPUT_PORT(2, ScreenshotDataBlue, DenseMatrix)
    void execute() override;

    MODULE_TRAITS_AND_INFO(ModuleHasUI)

    static Core::Thread::Mutex mutex_;
    Core::Thread::Mutex screenShotMutex_ {"ViewSceneScreenShotMutex"};

    typedef SharedPointer<Core::Datatypes::GeomList> GeomListPtr;
    typedef std::map<Dataflow::Networks::PortId, Core::Datatypes::GeometryBaseHandle> ActiveGeometryMap;
  protected:
    void portRemovedSlotImpl(const Dataflow::Networks::PortId& pid) override;
  private:
    void processViewSceneObjectFeedback();
    void processMeshComponentSelection();
    void fireTransientStateChangeSignalForGeomData();
    void updateTransientList();
    void syncMeshComponentFlags(const std::string& connectedModuleId, Dataflow::Networks::ModuleStateHandle state);
    unsigned long getCurrentTimeSinceEpoch();

    ActiveGeometryMap activeGeoms_;

    class SCISHARE ScopedExecutionReporter
    {
      Dataflow::Networks::ModuleStateHandle state_;
    public:
      explicit ScopedExecutionReporter(Dataflow::Networks::ModuleStateHandle state);
      ~ScopedExecutionReporter();
    };
  };
}}}

#endif
