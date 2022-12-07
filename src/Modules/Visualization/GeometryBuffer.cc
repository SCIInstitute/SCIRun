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


#include <Modules/Visualization/GeometryBuffer.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Dataflow/Network/Connection.h>
#include <chrono>
#include <numeric>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Core::Datatypes;
using namespace Core::Thread;
using namespace Dataflow::Networks;
using namespace Core::Algorithms;
using namespace Core::Algorithms::Visualization;
using namespace Core::Geometry;
using namespace Graphics::Datatypes;

ALGORITHM_PARAMETER_DEF(Visualization, BufferSize);
ALGORITHM_PARAMETER_DEF(Visualization, GeometryIndex);
ALGORITHM_PARAMETER_DEF(Visualization, PlayModeActive);
ALGORITHM_PARAMETER_DEF(Visualization, SingleStep);
ALGORITHM_PARAMETER_DEF(Visualization, LoopForever);
ALGORITHM_PARAMETER_DEF(Visualization, GeometryIncrement);
ALGORITHM_PARAMETER_DEF(Visualization, PlayModeDelay);
ALGORITHM_PARAMETER_DEF(Visualization, ClearFlag);

MODULE_INFO_DEF(GeometryBuffer, Visualization, SCIRun)

using IncomingBuffer = std::map<std::string, std::vector<GeometryBaseHandle>>;
using OutgoingBuffer = std::map<int, std::vector<GeometryBaseHandle>>;

namespace SCIRun::Modules::Visualization
{
  class GeometryBufferImpl
  {
  public:
    explicit GeometryBufferImpl(GeometryBuffer* module) : module_(module) {}
    IncomingBuffer buffer_;
    Mutex lock_;
    OutgoingBuffer makeOutgoing() const
    {
      OutgoingBuffer out;
      for (const auto& p : buffer_)
      {
        const auto& seq = p.second;
        int i = 0;
        for (const auto& g : seq)
        {
          out[i++].push_back(g);
        }
      }
      return out;
    }
    void indexIncremented();
    void sendOneSetOfGeometries(const std::vector<GeometryBaseHandle>& geomList);
    void sendAllGeometries();
    void updateBufferSize();
  private:
    GeometryBuffer* module_;
  };
}

GeometryBuffer::GeometryBuffer() : ModuleWithAsyncDynamicPorts(staticInfo_, true),
  impl_(new GeometryBufferImpl(this))
{
  INITIALIZE_PORT(GeometryInput);
  INITIALIZE_PORT(GeometryOutputSeries0);
  INITIALIZE_PORT(GeometryOutputSeries1);
  INITIALIZE_PORT(GeometryOutputSeries2);
  INITIALIZE_PORT(GeometryOutputSeries3);
  INITIALIZE_PORT(GeometryOutputSeries4);
  INITIALIZE_PORT(GeometryOutputSeries5);
  INITIALIZE_PORT(GeometryOutputSeries6);
  INITIALIZE_PORT(GeometryOutputSeries7);
}

GeometryBuffer::~GeometryBuffer() = default;

void GeometryBuffer::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::BufferSize, 50);
  state->setValue(Parameters::GeometryIndex, 0);
  state->setValue(Parameters::PlayModeActive, false);
  state->setValue(Parameters::LoopForever, false);
  state->setValue(Parameters::GeometryIncrement, 1);
  state->setValue(Parameters::PlayModeDelay, 100);
  state->setValue(Parameters::SingleStep, false);
  state->connectSpecificStateChanged(Parameters::PlayModeActive, [this]()
    {
      Core::Thread::Util::launchAsyncThread([this]() { impl_->sendAllGeometries(); });
    });
  state->connectSpecificStateChanged(Parameters::SingleStep, [this]()
    {
      Core::Thread::Util::launchAsyncThread([this]() { impl_->indexIncremented(); });
    });
  state->connectSpecificStateChanged(Parameters::ClearFlag, [this]()
    {
      impl_->buffer_.clear();
      impl_->updateBufferSize();
      get_state()->setTransientValue(Parameters::ClearFlag, false, false);
    });
}

void GeometryBuffer::execute()
{

}

void GeometryBufferImpl::sendAllGeometries()
{
  auto state = module_->get_state();
  if (!state->getValue(Parameters::PlayModeActive).toBool())
    return;

  Guard g(lock_);

  auto outgoingBuffer = makeOutgoing();
  const auto frameTime = state->getValue(Parameters::PlayModeDelay).toInt();
  const auto startingFrame = state->getValue(Parameters::GeometryIndex).toInt();
  bool firstLoop = true;

  using namespace std::chrono_literals;

  while (state->getValue(Parameters::PlayModeActive).toBool())
  {
    for (const auto& geomPack : outgoingBuffer)
    {
      const int geomIndex = geomPack.first;
      if (firstLoop && geomIndex < startingFrame)
        continue;

      state->setValue(Parameters::GeometryIndex, geomIndex);

      const auto& geomList = geomPack.second;
      sendOneSetOfGeometries(geomList);

      std::this_thread::sleep_for(frameTime * 1ms);

      if (!state->getValue(Parameters::PlayModeActive).toBool())
        break;
    }

    firstLoop = false;

    if (!state->getValue(Parameters::LoopForever).toBool())
      state->setValue(Parameters::PlayModeActive, false);
  }
}

void GeometryBufferImpl::indexIncremented()
{
  Guard g(lock_);

  auto state = module_->get_state();
  auto outgoingBuffer = makeOutgoing();
  sendOneSetOfGeometries(outgoingBuffer[state->getValue(Parameters::GeometryIndex).toInt()]);

  state->setValue(Parameters::SingleStep, false);
}

void GeometryBufferImpl::sendOneSetOfGeometries(const std::vector<GeometryBaseHandle>& geomList)
{
  for (size_t portIndex = 0; portIndex < geomList.size(); ++portIndex)
  {
    if (portIndex >= module_->numOutputPorts())
      break;

    const auto outputPort = module_->outputPorts()[portIndex];
    if (outputPort->nconnections() == 0)
      break;

    for (int j = 0; j < outputPort->nconnections(); ++j)
    {
      auto viewScene = outputPort->connection(j)->iport_->underlyingModule();
      if (viewScene->id().id_.find("ViewScene") == std::string::npos)
        break;

      //logCritical("Outputting geom number {} on port {} to module {}", geomIndex, outputPort->id().toString(), viewScene->id().id_);
      module_->send_output_handle(outputPort->internalId(), geomList[portIndex]);
      viewScene->execute();
    }
  }
}

void GeometryBufferImpl::updateBufferSize()
{
  size_t size = 0;
  if (!buffer_.empty())
  {
    size = std::accumulate(buffer_.begin(), buffer_.end(), std::numeric_limits<size_t>::max(),
      [](auto acc, auto p) { return std::min(acc, p.second.size()); });
  }
  module_->get_state()->setValue(Parameters::BufferSize, static_cast<int>(size));
}

void GeometryBuffer::asyncExecute(const PortId& pid, DatatypeHandle data)
{
  const auto geom = std::dynamic_pointer_cast<GeometryObject>(data);
  impl_->buffer_[pid.toString()].push_back(geom);
  impl_->updateBufferSize();
}

void GeometryBuffer::portRemovedSlotImpl(const PortId& pid)
{
  if (!impl_->buffer_.empty())
  {
    Guard g(impl_->lock_);
    impl_->buffer_[pid.toString()].clear();
  }
}
