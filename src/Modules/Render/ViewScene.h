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
        ALGORITHM_PARAMETER_DECL(GeometryFeedbackInfo);
        ALGORITHM_PARAMETER_DECL(ScreenshotData);

        ALGORITHM_PARAMETER_DECL(IsExecuting);
        ALGORITHM_PARAMETER_DECL(TimeExecutionFinished);
        ALGORITHM_PARAMETER_DECL(HasNewGeometry);

        // these should move from transient to saved
        ALGORITHM_PARAMETER_DECL(MeshComponentSelection);
        ALGORITHM_PARAMETER_DECL(ShowFieldStates);
        // new state variable to save the whole list. Won't break the delicate transient state behavior
        ALGORITHM_PARAMETER_DECL(VisibleItemListState);

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
        ALGORITHM_PARAMETER_DECL(ScaleBarLineColor);
        ALGORITHM_PARAMETER_DECL(ClippingPlaneEnabled);
        ALGORITHM_PARAMETER_DECL(ClippingPlaneNormalReversed);
        ALGORITHM_PARAMETER_DECL(ClippingPlaneX);
        ALGORITHM_PARAMETER_DECL(ClippingPlaneY);
        ALGORITHM_PARAMETER_DECL(ClippingPlaneZ);
        ALGORITHM_PARAMETER_DECL(ClippingPlaneD);
        ALGORITHM_PARAMETER_DECL(AxesVisible);
        ALGORITHM_PARAMETER_DECL(AxesSize);
        ALGORITHM_PARAMETER_DECL(AxesX);
        ALGORITHM_PARAMETER_DECL(AxesY);


        // save/load confirmed, uses standard widget managers.

        // save/load confirmed, uses pullSpecial.
        ALGORITHM_PARAMETER_DECL(ShowViewer);
        ALGORITHM_PARAMETER_DECL(WindowSizeX);
        ALGORITHM_PARAMETER_DECL(WindowSizeY);
        ALGORITHM_PARAMETER_DECL(WindowPositionX);
        ALGORITHM_PARAMETER_DECL(WindowPositionY);
        ALGORITHM_PARAMETER_DECL(IsFloating);
        ALGORITHM_PARAMETER_DECL(CameraDistance);
        ALGORITHM_PARAMETER_DECL(CameraDistanceMinimum);
        ALGORITHM_PARAMETER_DECL(CameraLookAt);
        ALGORITHM_PARAMETER_DECL(CameraRotation);
        ALGORITHM_PARAMETER_DECL(ToolBarMainPosition);
        ALGORITHM_PARAMETER_DECL(ToolBarRenderPosition);
        ALGORITHM_PARAMETER_DECL(ToolBarAdvancedPosition);

        // save/load has issues.
        ALGORITHM_PARAMETER_DECL(HeadLightOn);
        ALGORITHM_PARAMETER_DECL(Light1On);
        ALGORITHM_PARAMETER_DECL(Light2On);
        ALGORITHM_PARAMETER_DECL(Light3On);
        ALGORITHM_PARAMETER_DECL(HeadLightColor);
        ALGORITHM_PARAMETER_DECL(Light1Color);
        ALGORITHM_PARAMETER_DECL(Light2Color);
        ALGORITHM_PARAMETER_DECL(Light3Color);
        ALGORITHM_PARAMETER_DECL(HeadLightAzimuth);
        ALGORITHM_PARAMETER_DECL(Light1Azimuth);
        ALGORITHM_PARAMETER_DECL(Light2Azimuth);
        ALGORITHM_PARAMETER_DECL(Light3Azimuth);
        ALGORITHM_PARAMETER_DECL(HeadLightInclination);
        ALGORITHM_PARAMETER_DECL(Light1Inclination);
        ALGORITHM_PARAMETER_DECL(Light2Inclination);
        ALGORITHM_PARAMETER_DECL(Light3Inclination);

        ALGORITHM_PARAMETER_DECL(ScreenshotDirectory);

        // not used--GUI hidden/never implemented
        //ALGORITHM_PARAMETER_DECL(Lighting);
        //ALGORITHM_PARAMETER_DECL(ShowBBox);
        // ALGORITHM_PARAMETER_DECL(UseClip);
        // ALGORITHM_PARAMETER_DECL(Stereo);
        // ALGORITHM_PARAMETER_DECL(BackCull);
        // ALGORITHM_PARAMETER_DECL(DisplayList);
        // ALGORITHM_PARAMETER_DECL(StereoFusion);
        // ALGORITHM_PARAMETER_DECL(PolygonOffset);
        // ALGORITHM_PARAMETER_DECL(TextOffset);
        // ALGORITHM_PARAMETER_DECL(FieldOfView);

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

  class SCISHARE ViewSceneLocks
  {
  public:
    ~ViewSceneLocks();
    Core::Thread::Mutex& stateMutex() { return mutex_; }
    Core::Thread::Mutex& screenShotMutex() { return screenShotMutex_; }
  private:
    Core::Thread::Mutex mutex_ {"generalVSMutex"};
    Core::Thread::Mutex screenShotMutex_ {"ViewSceneScreenShotMutex"};
  };

  using ViewSceneLocksPtr = std::shared_ptr<ViewSceneLocks>;
  using ViewSceneLockKey = const SCIRun::Dataflow::Networks::ModuleStateInterface*;
  using ViewSceneLockManagerMap = std::map<ViewSceneLockKey, ViewSceneLocksPtr>;

  class SCISHARE ViewSceneLockManager
  {
  public:
    static ViewSceneLocksPtr get(ViewSceneLockKey id);
    static void remove(ViewSceneLockKey id);
  private:
    static ViewSceneLockManagerMap lockMap_;
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
    ~ViewScene() override;
    void asyncExecute(const Dataflow::Networks::PortId& pid, Core::Datatypes::DatatypeHandle data) override;
    void setStateDefaults() override;

    INPUT_PORT_DYNAMIC(0, GeneralGeom, GeometryObject)
    OUTPUT_PORT(0, ScreenshotDataRed, DenseMatrix)
    OUTPUT_PORT(1, ScreenshotDataGreen, DenseMatrix)
    OUTPUT_PORT(2, ScreenshotDataBlue, DenseMatrix)
    void execute() override;

    MODULE_TRAITS_AND_INFO(ModuleFlags::ModuleHasUI)

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
