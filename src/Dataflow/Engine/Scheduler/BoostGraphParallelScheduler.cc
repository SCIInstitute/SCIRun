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


#include <Dataflow/Engine/Scheduler/GraphNetworkAnalyzer.h>
#include <Dataflow/Engine/Scheduler/BoostGraphParallelScheduler.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Engine::NetworkGraph;
using namespace SCIRun::Dataflow::Networks;

BoostGraphParallelScheduler::BoostGraphParallelScheduler(const ModuleFilter& filter) : filter_(filter) {}

ParallelModuleExecutionOrder BoostGraphParallelScheduler::schedule(const NetworkInterface& network) const
{
  NetworkGraphAnalyzer graphAnalyzer(network, filter_, true);
  const DirectedGraph& g = graphAnalyzer.graph();

  // Parallel compilation ordering
  std::vector<int> time(graphAnalyzer.moduleCount(), 0);
  for (auto i = graphAnalyzer.topologicalBegin(); i != graphAnalyzer.topologicalEnd(); ++i)
  {
    // Walk through the in_edges an calculate the maximum time.
    if (in_degree (*i, g) > 0)
    {
      DirectedGraph::in_edge_iterator j, j_end;
      int maxdist = 0;
      // Through the order from topological sort, we are sure that every
      // time we are using here is already initialized.
      for (boost::tie(j, j_end) = in_edges(*i, g); j != j_end; ++j)
        maxdist = std::max(time[source(*j, g)], maxdist);

      time[*i] = maxdist + 1;
    }
  }

  ParallelModuleExecutionOrder::ModulesByGroup map;

  std::transform(
    graphAnalyzer.topologicalBegin(), graphAnalyzer.topologicalEnd(),
    std::inserter(map, map.begin()),
    [&](int vertex){ return std::make_pair(time[vertex], graphAnalyzer.moduleAt(vertex)); }
  );

  return ParallelModuleExecutionOrder(map);
}
