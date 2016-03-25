/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <Modules/Render/ViewScene.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Logging/Log.h>
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/DenseMatrix.h>

// Needed to fix conflict between define in X11 header
// and eigen enum member.
#ifdef Success
#  undef Success
#endif

using namespace SCIRun::Modules::Render;
using namespace SCIRun::Core::Algorithms;
using namespace Render;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;

const ModuleLookupInfo ViewScene::staticInfo_("ViewScene", "Render", "SCIRun");
Mutex ViewScene::mutex_("ViewScene");

ALGORITHM_PARAMETER_DEF(Render, GeomData);
ALGORITHM_PARAMETER_DEF(Render, GeometryFeedbackInfo);
ALGORITHM_PARAMETER_DEF(Render, ScreenshotData);

ViewScene::ViewScene() : ModuleWithAsyncDynamicPorts(staticInfo_, true), asyncUpdates_(0)
{
  INITIALIZE_PORT(GeneralGeom);
#ifdef BUILD_TESTING
  INITIALIZE_PORT(ScreenshotDataRed);
  INITIALIZE_PORT(ScreenshotDataGreen);
  INITIALIZE_PORT(ScreenshotDataBlue);
#endif
}

void ViewScene::setStateDefaults()
{
  auto state = get_state();
  state->setValue(BackgroundColor, ColorRGB(0.0, 0.0, 0.0).toString());
  state->setValue(Ambient, 0.2);
  state->setValue(Diffuse, 1.0);
  state->setValue(Specular, 0.4);
  state->setValue(Shine, 1.0);
  state->setValue(Emission, 1.0);
  state->setValue(FogOn, false);
  state->setValue(ObjectsOnly, true);
  state->setValue(UseBGColor, true);
  state->setValue(FogStart, 0.0);
  state->setValue(FogEnd, 0.71);
  state->setValue(FogColor, ColorRGB(0.0, 0.0, 1.0).toString());
  state->setValue(ShowScaleBar, false);
  state->setValue(ScaleBarUnitValue, "mm");
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
  postStateChangeInternalSignalHookup();
}

void ViewScene::postStateChangeInternalSignalHookup()
{
  get_state()->connect_state_changed([this]() { processViewSceneObjectFeedback(); });
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
  get_state()->fireTransientStateChangeSignal();
}

void ViewScene::updateTransientList()
{
  auto transient = get_state()->getTransientValue(Parameters::GeomData);

  auto geoms = transient_value_cast<GeomListPtr>(transient);
  if (!geoms)
  {
    geoms.reset(new GeomList());
  }
  auto activeHandles = activeGeoms_ | boost::adaptors::map_values;
  geoms->clear();
  geoms->insert(activeHandles.begin(), activeHandles.end());

  // Grab geometry inputs and pass them along in a transient value to the GUI
  // thread where they will be transported to Spire.
  // NOTE: I'm not implementing mutex locks for this now. But for production
  // purposes, they NEED to be in there!

  // Pass geometry object up through transient... really need to be concerned
  // about the lifetimes of the buffers we have in GeometryObject. Need to
  // switch to std::shared_ptr on an std::array when in production.

  /// \todo Need to make this data transfer mechanism thread safe!
  // I thought about dynamic casting geometry object to a weak_ptr, but I don't
  // know where it will be destroyed. For now, it will have have stale pointer
  // data lying around in it... yuck.
  get_state()->setTransientValue(Parameters::GeomData, geoms, false);
}

void ViewScene::asyncExecute(const PortId& pid, DatatypeHandle data)
{
  //lock for state modification
  {
    LOG_DEBUG("ViewScene::asyncExecute before locking");
    Guard lock(mutex_.get());
    get_state()->setTransientValue(Parameters::ScreenshotData, boost::any(), false);

    LOG_DEBUG("ViewScene::asyncExecute after locking");

    auto geom = boost::dynamic_pointer_cast<GeometryObject>(data);
    if (!geom)
    {
      error("Logical error: not a geometry object on ViewScene");
      return;
    }

    activeGeoms_[pid] = geom;
    updateTransientList();
  }
  get_state()->fireTransientStateChangeSignal();
  asyncUpdates_.fetch_add(1);
  //std::cout << "asyncExecute " << asyncUpdates_ << std::endl;
}

#ifdef BUILD_TESTING
void ViewScene::execute()
{
  if (needToExecute())
  {
    //std::cout << "1execute " << asyncUpdates_ << std::endl;
    const int maxAsyncWaitTries = 100; //TODO: make configurable for longer-running networks
    auto asyncWaitTries = 0;
    if (inputPorts().size() > 1) // only send screenshot if input is present
    {
      while (asyncUpdates_ < inputPorts().size() - 1)
      {
        //std::cout << "2execute " << asyncUpdates_ << std::endl;
        asyncWaitTries++;
        if (asyncWaitTries == maxAsyncWaitTries)
          return; // nothing coming down the ports
        //wait until all asyncExecutes are done.
      }

      ModuleStateInterface::TransientValueOption screenshotDataOption;
      auto state = get_state();
      do
      {
        //std::cout << "3execute " << asyncUpdates_ << std::endl;
        screenshotDataOption = state->getTransientValue(Parameters::ScreenshotData);
        if (screenshotDataOption)
        {
          //std::cout << "4execute found a non-empty" << asyncUpdates_ << std::endl;
          auto screenshotData = transient_value_cast<RGBMatrices>(screenshotDataOption);
          if (screenshotData.red)
          {
            sendOutput(ScreenshotDataRed, screenshotData.red);
          }
          if (screenshotData.green)
          {
            sendOutput(ScreenshotDataGreen, screenshotData.green);
          }
          if (screenshotData.blue)
          {
            sendOutput(ScreenshotDataBlue, screenshotData.blue);
          }
        }
      } while (!screenshotDataOption);
    }
    asyncUpdates_ = 0;

    //std::cout << "999execute " << asyncUpdates_ << std::endl;
    //std::cout << "execute setting none " << asyncUpdates_ << std::endl;
    get_state()->setTransientValue(Parameters::ScreenshotData, boost::any(), false);
  }
}
#endif

void ViewScene::processViewSceneObjectFeedback()
{
  //TODO: match ID of touched geom object with port id, and send that info back too.
  auto state = get_state();
  auto newInfo = state->getTransientValue(Parameters::GeometryFeedbackInfo);
  //TODO: lost equality test here due to change to boost::any. Would be nice to form a data class with equality to avoid repetitive signalling.
  if (newInfo)
  {
    sendFeedbackUpstreamAlongIncomingConnections(*newInfo);
    (*newInfo).clear();
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
