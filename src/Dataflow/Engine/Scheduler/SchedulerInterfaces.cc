/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Dataflow/Engine/Scheduler/SchedulerInterfaces.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <boost/thread.hpp>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

ScopedExecutionBoundsSignaller::ScopedExecutionBoundsSignaller(const ExecutionBounds* bounds, boost::function<int()> errorCodeRetriever) : bounds_(bounds), errorCodeRetriever_(errorCodeRetriever)
{
  bounds_->executeStarts_();
}

ScopedExecutionBoundsSignaller::~ScopedExecutionBoundsSignaller()
{
  bounds_->executeFinishes_(errorCodeRetriever_());
}

const ExecuteAllModules& ExecuteAllModules::Instance()
{
  static ExecuteAllModules instance_;
  return instance_;
}

ExecutionContext::ExecutionContext(const NetworkInterface& net) : network(net), lookup(net) {}

const ExecutionBounds& ExecutionContext::bounds() const
{
  return executionBounds_;
}

bool WaitsForStartupInitialization::waitedAlready_(false);

void WaitsForStartupInitialization::waitForStartupInit(const ExecutableLookup& lookup) const
{
  if (!waitedAlready_ && lookup.containsViewScene())
  {
    std::cout << "Waiting for rendering system initialization...." << std::endl;
    boost::this_thread::sleep(boost::posix_time::milliseconds(600));
    std::cout << "Done waiting." << std::endl;
    waitedAlready_ = true;
  }
}
