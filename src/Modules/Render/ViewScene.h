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
#include <Modules/Render/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Render
      {
        ALGORITHM_PARAMETER_DECL(GeomData);
        ALGORITHM_PARAMETER_DECL(VSMutex);
        ALGORITHM_PARAMETER_DECL(GeometryFeedbackInfo);
        ALGORITHM_PARAMETER_DECL(ScreenshotData);
        ALGORITHM_PARAMETER_DECL(MeshComponentSelection);
        ALGORITHM_PARAMETER_DECL(ShowFieldStates);
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
    virtual void asyncExecute(const Dataflow::Networks::PortId& pid, Core::Datatypes::DatatypeHandle data) override;
    virtual void setStateDefaults() override;

    static const Core::Algorithms::AlgorithmParameterName BackgroundColor;
    static const Core::Algorithms::AlgorithmParameterName Ambient;
    static const Core::Algorithms::AlgorithmParameterName Diffuse;
    static const Core::Algorithms::AlgorithmParameterName Specular;
    static const Core::Algorithms::AlgorithmParameterName Shine;
    static const Core::Algorithms::AlgorithmParameterName Emission;
    static const Core::Algorithms::AlgorithmParameterName FogOn;
    static const Core::Algorithms::AlgorithmParameterName ObjectsOnly;
    static const Core::Algorithms::AlgorithmParameterName UseBGColor;
    static const Core::Algorithms::AlgorithmParameterName FogStart;
    static const Core::Algorithms::AlgorithmParameterName FogEnd;
    static const Core::Algorithms::AlgorithmParameterName FogColor;
    static const Core::Algorithms::AlgorithmParameterName ShowScaleBar;
    static const Core::Algorithms::AlgorithmParameterName ScaleBarUnitValue;
    static const Core::Algorithms::AlgorithmParameterName ScaleBarLength;
    static const Core::Algorithms::AlgorithmParameterName ScaleBarHeight;
    static const Core::Algorithms::AlgorithmParameterName ScaleBarMultiplier;
    static const Core::Algorithms::AlgorithmParameterName ScaleBarNumTicks;
    static const Core::Algorithms::AlgorithmParameterName ScaleBarLineWidth;
    static const Core::Algorithms::AlgorithmParameterName ScaleBarFontSize;
    static const Core::Algorithms::AlgorithmParameterName Lighting;
    static const Core::Algorithms::AlgorithmParameterName ShowBBox;
    static const Core::Algorithms::AlgorithmParameterName UseClip;
    static const Core::Algorithms::AlgorithmParameterName Stereo;
    static const Core::Algorithms::AlgorithmParameterName BackCull;
    static const Core::Algorithms::AlgorithmParameterName DisplayList;
    static const Core::Algorithms::AlgorithmParameterName StereoFusion;
    static const Core::Algorithms::AlgorithmParameterName PolygonOffset;
    static const Core::Algorithms::AlgorithmParameterName TextOffset;
    static const Core::Algorithms::AlgorithmParameterName FieldOfView;
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

    INPUT_PORT_DYNAMIC(0, GeneralGeom, GeometryObject);
    OUTPUT_PORT(0, ScreenshotDataRed, DenseMatrix);
    OUTPUT_PORT(1, ScreenshotDataGreen, DenseMatrix);
    OUTPUT_PORT(2, ScreenshotDataBlue, DenseMatrix);
    virtual void execute() override;

    MODULE_TRAITS_AND_INFO(ModuleHasUI)

    static Core::Thread::Mutex mutex_;
    Core::Thread::Mutex screenShotMutex_ {"ViewSceneScreenShotMutex"};

    typedef SharedPointer<Core::Datatypes::GeomList> GeomListPtr;
    typedef std::map<Dataflow::Networks::PortId, Core::Datatypes::GeometryBaseHandle> ActiveGeometryMap;
  protected:
    virtual void portRemovedSlotImpl(const Dataflow::Networks::PortId& pid) override;
  private:
    void processViewSceneObjectFeedback();
    void processMeshComponentSelection();
    void fireTransientStateChangeSignalForGeomData();
    void updateTransientList();
    void syncMeshComponentFlags(const std::string& connectedModuleId, Dataflow::Networks::ModuleStateHandle state);
    long getCurrentTimeSinceEpoch();

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
