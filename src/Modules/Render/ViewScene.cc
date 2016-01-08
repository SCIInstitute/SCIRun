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

ModuleLookupInfo ViewScene::staticInfo_("ViewScene", "Render", "SCIRun");
Mutex ViewScene::mutex_("ViewScene");

ALGORITHM_PARAMETER_DEF(Render, GeomData);
ALGORITHM_PARAMETER_DEF(Render, GeometryFeedbackInfo);
ALGORITHM_PARAMETER_DEF(Render, ScreenshotData);

ViewScene::ViewScene() : ModuleWithAsyncDynamicPorts(staticInfo_, true)
{
  INITIALIZE_PORT(GeneralGeom);
#ifdef BUILD_TESTING
  INITIALIZE_PORT(ScreenshotData);
#endif
}

void ViewScene::setStateDefaults()
{
  auto state = get_state();
  state->setValue(BackgroundColor, ColorRGB(0.0, 0.0, 0.0).toString());
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
    get_state()->setTransientValue(Parameters::ScreenshotData, nullptr, false);

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
}

#ifdef BUILD_TESTING
void ViewScene::execute()
{
  if (inputPorts().size() > 1) // only send screenshot if input is present
  {
    DenseMatrixHandle screenshotData;
    auto state = get_state();
    do
    {
      auto transient = state->getTransientValue(Parameters::ScreenshotData);
      screenshotData = transient_value_cast<DenseMatrixHandle>(transient);
      if (screenshotData)
      {
        sendOutput(ScreenshotData, screenshotData);
      }
    }
    while (!screenshotData);
  }
}
#endif

void ViewScene::processViewSceneObjectFeedback()
{
  //TODO: match ID of touched geom object with port id, and send that info back too.
  //std::cout << "slot for state change in VS module" << std::endl;
  auto state = get_state();
  auto newInfo = state->getValue(Parameters::GeometryFeedbackInfo).toVector();
  //std::cout << "feedback info: " << newInfo << std::endl;
  if (feedbackInfo_ != newInfo)
  {
    //std::cout << "new feedback info: " << newInfo << std::endl;
    feedbackInfo_ = newInfo;

    sendFeedbackUpstreamAlongIncomingConnections(feedbackInfo_);
  }
}

AlgorithmParameterName ViewScene::BackgroundColor("BackgroundColor");
