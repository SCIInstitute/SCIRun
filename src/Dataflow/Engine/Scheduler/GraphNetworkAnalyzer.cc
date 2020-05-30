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
#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Network/PortInterface.h>
#include <Dataflow/Network/Connection.h>
#include <Dataflow/Engine/Scheduler/BoostGraphParallelScheduler.h>
#include <Core/Logging/Log.h>

#include <boost/utility.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/lambda/lambda.hpp>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Engine::NetworkGraph;
using namespace SCIRun::Dataflow::Networks;

NetworkGraphAnalyzer::NetworkGraphAnalyzer(const NetworkInterface& network, const ModuleFilter& moduleFilter, bool precompute)
  : network_(network), moduleFilter_(moduleFilter), moduleCount_(0)
{
  if (precompute)
  {
    computeExecutionOrder();
  }
}

const ModuleId& NetworkGraphAnalyzer::moduleAt(int vertex) const
{
  return moduleIdLookup_.right.at(vertex);
}

ExecutionOrderIterator NetworkGraphAnalyzer::topologicalBegin()
{
  return order_.begin();
}

ExecutionOrderIterator NetworkGraphAnalyzer::topologicalEnd()
{
  return order_.end();
}

const DirectedGraph& NetworkGraphAnalyzer::graph()
{
  return graph_;
}

int NetworkGraphAnalyzer::moduleCount() const
{
  return moduleCount_;
}

EdgeVector NetworkGraphAnalyzer::constructEdgeListFromNetwork()
{
  moduleCount_ = 0;
  moduleIdLookup_.clear();

  for (int i = 0; i < network_.nmodules(); ++i)
  {
    auto module = network_.module(i);
    if (moduleFilter_(module))
    {
      moduleIdLookup_.left.insert(std::make_pair(module->id(), moduleCount_));
      moduleCount_++;
    }
  }

  std::vector<Edge> edges;

  for (const ConnectionDescription& cd : network_.connections(false))
  {
    if (moduleIdLookup_.left.find(cd.out_.moduleId_) != moduleIdLookup_.left.end()
      && moduleIdLookup_.left.find(cd.in_.moduleId_) != moduleIdLookup_.left.end())
    {
      edges.push_back(std::make_pair(moduleIdLookup_.left.at(cd.out_.moduleId_), moduleIdLookup_.left.at(cd.in_.moduleId_)));
    }
  }

  return edges;
}

void NetworkGraphAnalyzer::computeExecutionOrder()
{
  auto edges = constructEdgeListFromNetwork();

  graph_ = DirectedGraph(edges.begin(), edges.end(), moduleCount_);

  try
  {
    boost::topological_sort(graph_, std::front_inserter(order_));
  }
  catch (std::invalid_argument& e)
  {
    BOOST_THROW_EXCEPTION(NetworkHasCyclesException() << SCIRun::Core::ErrorMessage(e.what()));
  }
}

ComponentMap NetworkGraphAnalyzer::connectedComponents()
{
  auto edges = constructEdgeListFromNetwork();
  UndirectedGraph undirected(edges.begin(), edges.end(), moduleCount_);

  std::vector<int> component(boost::num_vertices(undirected));
  boost::connected_components(undirected, &component[0]);

  ComponentMap componentMap;
  for (size_t i = 0; i < component.size(); ++i)
  {
    componentMap[moduleAt(i)] = component[i];
  }
  return componentMap;
}

std::vector<ModuleId> NetworkGraphAnalyzer::downstreamModules(const ModuleId& mid) const
{
  std::vector<ModuleId> downstream {mid};
  fillDownstreamModules(mid, downstream);
  return downstream;
}

void NetworkGraphAnalyzer::fillDownstreamModules(const ModuleId& mid, std::vector<ModuleId>& downstream) const
{
  auto module = network_.lookupModule(mid);
  if (!module)
    return;

  for (const auto& output : module->outputPorts())
  {
    for (size_t i = 0; i < output->nconnections(); ++i)
    {
      auto c = output->connection(i);
      if (!c->disabled() && !c->isVirtual())
      {
        auto down = c->iport_->getUnderlyingModuleId();
        downstream.push_back(down);
        fillDownstreamModules(down, downstream);
      }
    }
  }
}

namespace SCIRun
{
  namespace Dataflow
  {
    namespace Engine
    {
      class ExecuteSingleModuleImpl
      {
      public:
        std::vector<ModuleId> downstream_;
        bool isDownstreamFromRoot(const ModuleId& toCheckId) const
        {
          return std::find(downstream_.begin(), downstream_.end(), toCheckId) != downstream_.end();
        }
      };
    }
  }
}

ExecuteSingleModule::ExecuteSingleModule(ModuleHandle mod,
  const NetworkInterface& network,
  bool executeUpstream) : module_(mod), //network_(network),
  executeUpstream_(executeUpstream)
{
  //TODO: composite with which filter?
  NetworkGraphAnalyzer analyze(network, ExecuteAllModules::Instance(), false);
  components_ = analyze.connectedComponents();

  if (!executeUpstream_)
  {
    orderImpl_.reset(new ExecuteSingleModuleImpl);
    analyze.computeExecutionOrder();
    orderImpl_->downstream_ = analyze.downstreamModules(module_->id());
  }
}

bool ExecuteSingleModule::operator()(ModuleHandle mod) const
{
  auto toCheckId = mod->id();
  auto modIdIter = components_.find(toCheckId);
  if (modIdIter == components_.end())
    THROW_INVALID_ARGUMENT("Module not found in component map");

  auto rootId = module_->id();
  if (rootId.name_ == "Subnet")
    return false;
  auto rootIdIter = components_.find(rootId);
  if (rootIdIter == components_.end())
    THROW_INVALID_ARGUMENT("Current module not found in component map");

  if (executeUpstream_)
  {
    // should execute if in same connected component
    return modIdIter->second == rootIdIter->second;
  }
  else
  {
    // should execute if in same connected component, and downstream only
    return modIdIter->second == rootIdIter->second
      && orderImpl_->isDownstreamFromRoot(toCheckId);
  }
}
