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

#include <iostream>
#include <Dataflow/Engine/Scheduler/BasicMultithreadedNetworkExecutor.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Core/Thread/Parallel.h>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;

void BasicMultithreadedNetworkExecutor::executeAll(const ExecutableLookup& lookup, const ParallelModuleExecutionOrder& order, const ExecutionBounds& bounds)
{
  bounds.executeStarts_();
  for (int group = order.minGroup(); group <= order.maxGroup(); ++group)
  {
    auto groupIter = order.getGroup(group);

    std::vector<boost::function<void()>> tasks;

    std::transform(groupIter.first, groupIter.second, std::back_inserter(tasks), 
      [&](const ParallelModuleExecutionOrder::ModulesByGroup::value_type& mod) -> boost::function<void()>
    {
      return [&]() { lookup.lookupExecutable(mod.second)->execute(); };
    });

    std::cout << "Running group " << group << " of size " << tasks.size() << std::endl;
    Parallel::RunTasks([&](int i) { tasks[i](); }, tasks.size());
  }
  bounds.executeFinishes_(lookup.errorCode());
}
