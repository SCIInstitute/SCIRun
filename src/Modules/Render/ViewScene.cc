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


#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Logging/Log.h>
#include <Modules/Render/ViewScene.h>
#include <boost/thread.hpp>
#include <es-log/trace-log.h>

// Needed to fix conflict between define in X11 header
// and eigen enum member.
#ifdef Success
#  undef Success
#endif

using namespace SCIRun::Modules::Render;
using namespace SCIRun::Core::Algorithms;
using namespace Render;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Logging;

MODULE_INFO_DEF(ViewScene, Render, SCIRun)

Mutex ViewScene::mutex_("ViewScene");

ViewScene::ScopedExecutionReporter::ScopedExecutionReporter(ModuleStateHandle state)
  : state_(state)
{
  state_->setValue(IsExecuting, true);
}

ViewScene::ScopedExecutionReporter::~ScopedExecutionReporter()
{
  state_->setValue(IsExecuting, false);
}

ALGORITHM_PARAMETER_DEF(Render, GeomData);
ALGORITHM_PARAMETER_DEF(Render, VSMutex);
ALGORITHM_PARAMETER_DEF(Render, GeometryFeedbackInfo);
ALGORITHM_PARAMETER_DEF(Render, ScreenshotData);
ALGORITHM_PARAMETER_DEF(Render, MeshComponentSelection);
ALGORITHM_PARAMETER_DEF(Render, ShowFieldStates);

ViewScene::ViewScene() : ModuleWithAsyncDynamicPorts(staticInfo_, true)
{
  RENDERER_LOG_FUNCTION_SCOPE;
  INITIALIZE_PORT(GeneralGeom);
  INITIALIZE_PORT(ScreenshotDataRed);
  INITIALIZE_PORT(ScreenshotDataGreen);
  INITIALIZE_PORT(ScreenshotDataBlue);

  get_state()->setTransientValue(Parameters::VSMutex, &screenShotMutex_, true);
}

ViewScene::~ViewScene()
{
}

void ViewScene::setStateDefaults()
{
  auto state = get_state();
  state->setValue(BackgroundColor, ColorRGB(0.0, 0.0, 0.0).toString());
  state->setValue(Ambient, 0.2);
  state->setValue(Diffuse, 1.0);
  state->setValue(Specular, 0.3);
  state->setValue(Shine, 0.5);
  state->setValue(Emission, 0.0);
  state->setValue(FogOn, false);
  state->setValue(ObjectsOnly, true);
  state->setValue(UseBGColor, true);
  state->setValue(FogStart, 0.0);
  state->setValue(FogEnd, 0.71);
  state->setValue(FogColor, ColorRGB(0.0, 0.0, 1.0).toString());
  state->setValue(ShowScaleBar, false);
  state->setValue(ScaleBarUnitValue, std::string("mm"));
  state->setValue(ScaleBarLength, 1.0);
  state->setValue(ScaleBarHeight, 1.0);
  state->setValue(ScaleBarMultiplier, 1.0);
  state->setValue(ScaleBarNumTicks, 11);
  state->setValue(ScaleBarLineWidth, 1.0);
  state->setValue(ScaleBarFontSize, 8);
  state->setValue(Lighting, true);
  state->setValue(ShowBBox, false);
  state->setValue(UseClip, true);
  state->setValue(BackCull, false);
  state->setValue(DisplayList, false);
  state->setValue(Stereo, false);
  state->setValue(StereoFusion, 0.4);
  state->setValue(PolygonOffset, 0.0);
  state->setValue(TextOffset, 0.0);
  state->setValue(FieldOfView, 20);
  state->setValue(HeadLightOn, true);
  state->setValue(Light1On, false);
  state->setValue(Light2On, false);
  state->setValue(Light3On, false);
  state->setValue(HeadLightColor, ColorRGB(0.0, 0.0, 0.0).toString());
  state->setValue(Light1Color, ColorRGB(0.0, 0.0, 0.0).toString());
  state->setValue(Light2Color, ColorRGB(0.0, 0.0, 0.0).toString());
  state->setValue(Light3Color, ColorRGB(0.0, 0.0, 0.0).toString());
  state->setValue(HeadLightAzimuth, 180);
  state->setValue(Light1Azimuth, 180);
  state->setValue(Light2Azimuth, 180);
  state->setValue(Light3Azimuth, 180);
  state->setValue(HeadLightInclination, 90);
  state->setValue(Light1Inclination, 90);
  state->setValue(Light2Inclination, 90);
  state->setValue(Light3Inclination, 90);
  state->setValue(ShowViewer, false);
  state->setValue(CameraDistance, 3.0);
  state->setValue(IsExecuting, false);
  state->setValue(TimeExecutionFinished, 0);
  state->setValue(CameraDistanceMinimum, 1e-10);
  state->setValue(CameraLookAt, Point(0.0, 0.0, 0.0).get_string());
  state->setValue(CameraRotation, std::string("Quaternion(1.0,0.0,0.0,0.0)"));
  state->setValue(HasNewGeometry, false);

  get_state()->connectSpecificStateChanged(Parameters::GeometryFeedbackInfo, [this]() { processViewSceneObjectFeedback(); });
  get_state()->connectSpecificStateChanged(Parameters::MeshComponentSelection, [this]() { processMeshComponentSelection(); });
}

void ViewScene::fireTransientStateChangeSignalForGeomData()
{
  //this is gross but I dont see any other way to fire the signal associated with geom data
  auto transient = get_state()->getTransientValue(Parameters::GeomData);
  auto geoms = transient_value_cast<GeomListPtr>(transient);
  get_state()->setTransientValue(Parameters::GeomData, geoms, true);
}

void ViewScene::portRemovedSlotImpl(const PortId& pid)
{
  //lock for state modification
  {
    Guard lock(mutex_.get());
    auto loc = activeGeoms_.find(pid);
    if (loc != activeGeoms_.end())
      activeGeoms_.erase(loc);
    updateTransientList();
  }

  fireTransientStateChangeSignalForGeomData();
}

void ViewScene::updateTransientList()
{
  auto transient = get_state()->getTransientValue(Parameters::GeomData);

  auto geoms = transient_value_cast<GeomListPtr>(transient);
  if (!geoms)
  {
    geoms.reset(new GeomList());
  }

  geoms->clear();

  for (const auto& geomPair : activeGeoms_)
  {
    auto geom = geomPair.second;
    geom->addToList(geom, *geoms);
    LOG_DEBUG("updateTransientList added geom to state list: {}", geomPair.first.toString());
  }

  // Grab geometry inputs and pass them along in a transient value to the GUI
  // thread where they will be transported to Spire.
  // NOTE: I'm not implementing mutex locks for this now. But for production
  // purposes, they NEED to be in there!

  // Pass geometry object up through transient... really need to be concerned
  // about the lifetimes of the buffers we have in GeometryObject. Need to
  // switch to std::shared_ptr on an std::array when in production.

  // todo Need to make this data transfer mechanism thread safe!
  // I thought about dynamic casting geometry object to a weak_ptr, but I don't
  // know where it will be destroyed. For now, it will have have stale pointer
  // data lying around in it... yuck.
  get_state()->setTransientValue(Parameters::GeomData, geoms, false);
}

void ViewScene::asyncExecute(const PortId& pid, DatatypeHandle data)
{
  if (!data) return;
  //lock for state modification
  {
    LOG_DEBUG("ViewScene::asyncExecute {} before locking", id().id_);
    Guard lock(mutex_.get());

    get_state()->setTransientValue(Parameters::ScreenshotData, boost::any(), false);

    LOG_DEBUG("ViewScene::asyncExecute {} after locking", id().id_);

    auto geom = boost::dynamic_pointer_cast<GeometryObject>(data);
    if (!geom)
    {
      error("Logical error: not a geometry object on ViewScene");
      return;
    }

    {
      auto iport = getInputPort(pid);
      auto connectedModuleId = iport->connectedModuleId();
      if (connectedModuleId->find("ShowField") != std::string::npos)
      {
        auto state = iport->stateFromConnectedModule();
        syncMeshComponentFlags(*connectedModuleId, state);
      }
    }

    activeGeoms_[pid] = geom;
    LOG_DEBUG("asyncExecute added active geom to map: {}", pid.toString());
    updateTransientList();
  }
}

void ViewScene::syncMeshComponentFlags(const std::string& connectedModuleId, ModuleStateHandle state)
{
  if (connectedModuleId.find("ShowField:") != std::string::npos)
  {
    auto map = transient_value_cast<ShowFieldStatesMap>(get_state()->getTransientValue(Parameters::ShowFieldStates));
    map[connectedModuleId] = state;
    get_state()->setTransientValue(Parameters::ShowFieldStates, map, false);
  }
}

void ViewScene::execute()
{
  auto state = get_state();
  auto executionReporter = ScopedExecutionReporter(state);

  fireTransientStateChangeSignalForGeomData();
#ifdef BUILD_HEADLESS
  sendOutput(ScreenshotDataRed, boost::make_shared<DenseMatrix>(0, 0));
  sendOutput(ScreenshotDataGreen, boost::make_shared<DenseMatrix>(0, 0));
  sendOutput(ScreenshotDataBlue, boost::make_shared<DenseMatrix>(0, 0));
#else
  Guard lock(screenShotMutex_.get());
  if (needToExecute() && inputPorts().size() >= 1) // only send screenshot if input is present
  {
    ModuleStateInterface::TransientValueOption screenshotDataOption;
    screenshotDataOption = state->getTransientValue(Parameters::ScreenshotData);
    {
      auto screenshotData = transient_value_cast<RGBMatrices>(screenshotDataOption);
      if (screenshotData.red) sendOutput(ScreenshotDataRed, screenshotData.red);
      if (screenshotData.green) sendOutput(ScreenshotDataGreen, screenshotData.green);
      if (screenshotData.blue) sendOutput(ScreenshotDataBlue, screenshotData.blue);
    }
  }
#endif
  state->setValue(HasNewGeometry, true);
  state->setValue(TimeExecutionFinished, int(getCurrentTimeSinceEpoch()));
}

long ViewScene::getCurrentTimeSinceEpoch()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
}

void ViewScene::processViewSceneObjectFeedback()
{
  //TODO: match ID of touched geom object with port id, and send that info back too.
  auto state = get_state();
  auto newInfo = state->getTransientValue(Parameters::GeometryFeedbackInfo);
  //TODO: lost equality test here due to change to boost::any. Would be nice to form a data class with equality to avoid repetitive signalling.
  if (newInfo)
  {
    auto vsInfo = transient_value_cast<ViewSceneFeedback>(newInfo);
    sendFeedbackUpstreamAlongIncomingConnections(vsInfo);
  }
}

void ViewScene::processMeshComponentSelection()
{
  auto state = get_state();
  auto newInfo = state->getTransientValue(Parameters::MeshComponentSelection);
  if (newInfo)
  {
    auto vsInfo = transient_value_cast<MeshComponentSelectionFeedback>(newInfo);
    sendFeedbackUpstreamAlongIncomingConnections(vsInfo);
  }
}

const AlgorithmParameterName ViewScene::BackgroundColor("BackgroundColor");
const AlgorithmParameterName ViewScene::Ambient("Ambient");
const AlgorithmParameterName ViewScene::Diffuse("Diffuse");
const AlgorithmParameterName ViewScene::Specular("Specular");
const AlgorithmParameterName ViewScene::Shine("Shine");
const AlgorithmParameterName ViewScene::Emission("Emission");
const AlgorithmParameterName ViewScene::FogOn("FogOn");
const AlgorithmParameterName ViewScene::ObjectsOnly("ObjectsOnly");
const AlgorithmParameterName ViewScene::UseBGColor("UseBGColor");
const AlgorithmParameterName ViewScene::FogStart("FogStart");
const AlgorithmParameterName ViewScene::FogEnd("FogEnd");
const AlgorithmParameterName ViewScene::FogColor("FogColor");
const AlgorithmParameterName ViewScene::ShowScaleBar("ShowScaleBar");
const AlgorithmParameterName ViewScene::ScaleBarUnitValue("ScaleBarUnitValue");
const AlgorithmParameterName ViewScene::ScaleBarLength("ScaleBarLength");
const AlgorithmParameterName ViewScene::ScaleBarHeight("ScaleBarHeight");
const AlgorithmParameterName ViewScene::ScaleBarMultiplier("ScaleBarMultiplier");
const AlgorithmParameterName ViewScene::ScaleBarNumTicks("ScaleBarNumTicks");
const AlgorithmParameterName ViewScene::ScaleBarLineWidth("ScaleBarLineWidth");
const AlgorithmParameterName ViewScene::ScaleBarFontSize("ScaleBarFontSize");
const AlgorithmParameterName ViewScene::Lighting("Lighting");
const AlgorithmParameterName ViewScene::ShowBBox("ShowBBox");
const AlgorithmParameterName ViewScene::UseClip("UseClip");
const AlgorithmParameterName ViewScene::Stereo("Stereo");
const AlgorithmParameterName ViewScene::BackCull("BackCull");
const AlgorithmParameterName ViewScene::DisplayList("DisplayList");
const AlgorithmParameterName ViewScene::StereoFusion("StereoFusion");
const AlgorithmParameterName ViewScene::PolygonOffset("PolygonOffset");
const AlgorithmParameterName ViewScene::TextOffset("TextOffset");
const AlgorithmParameterName ViewScene::FieldOfView("FieldOfView");
const AlgorithmParameterName ViewScene::HeadLightOn("HeadLightOn");
const AlgorithmParameterName ViewScene::Light1On("Light1On");
const AlgorithmParameterName ViewScene::Light2On("Light2On");
const AlgorithmParameterName ViewScene::Light3On("Light3On");
const AlgorithmParameterName ViewScene::HeadLightColor("HeadLightColor");
const AlgorithmParameterName ViewScene::Light1Color("Light1Color");
const AlgorithmParameterName ViewScene::Light2Color("Light2Color");
const AlgorithmParameterName ViewScene::Light3Color("Light3Color");
const AlgorithmParameterName ViewScene::HeadLightAzimuth("HeadLightAzimuth");
const AlgorithmParameterName ViewScene::Light1Azimuth("Light1Azimuth");
const AlgorithmParameterName ViewScene::Light2Azimuth("Light2Azimuth");
const AlgorithmParameterName ViewScene::Light3Azimuth("Light3Azimuth");
const AlgorithmParameterName ViewScene::HeadLightInclination("HeadLightInclination");
const AlgorithmParameterName ViewScene::Light1Inclination("Light1Inclination");
const AlgorithmParameterName ViewScene::Light2Inclination("Light2Inclination");
const AlgorithmParameterName ViewScene::Light3Inclination("Light3Inclination");
const AlgorithmParameterName ViewScene::ShowViewer("ShowViewer");
const AlgorithmParameterName ViewScene::CameraDistance("CameraDistance");
const AlgorithmParameterName ViewScene::CameraDistanceMinimum("CameraDistanceMinimum");
const AlgorithmParameterName ViewScene::CameraLookAt("CameraLookAt");
const AlgorithmParameterName ViewScene::CameraRotation("CameraRotation");
const AlgorithmParameterName ViewScene::IsExecuting("IsExecuting");
const AlgorithmParameterName ViewScene::TimeExecutionFinished("TimeExecutionFinished");
const AlgorithmParameterName ViewScene::HasNewGeometry("HasNewGeometry");
