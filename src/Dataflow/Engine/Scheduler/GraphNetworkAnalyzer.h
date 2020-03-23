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


#ifndef ENGINE_SCHEDULER_BOOST_GRAPH_NETWORK_ANALYZER_H
#define ENGINE_SCHEDULER_BOOST_GRAPH_NETWORK_ANALYZER_H

#include <boost/noncopyable.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/bimap.hpp>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Engine/Scheduler/SchedulerInterfaces.h>
#include <Dataflow/Engine/Scheduler/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {

  namespace NetworkGraph
  {
    typedef std::pair<int,int> Edge;
    typedef std::vector<Edge> EdgeVector;
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> DirectedGraph;
    typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS> UndirectedGraph;
    typedef boost::graph_traits<DirectedGraph>::vertex_descriptor Vertex;
    typedef std::list<Vertex> ExecutionOrder;
    typedef ExecutionOrder::const_iterator ExecutionOrderIterator;
    typedef std::map<std::string, int> ComponentMap;
  }

  class SCISHARE NetworkGraphAnalyzer : boost::noncopyable
  {
  public:
    NetworkGraphAnalyzer(const Networks::NetworkInterface& network, const Networks::ModuleFilter& moduleFilter, bool precompute);

    NetworkGraph::EdgeVector constructEdgeListFromNetwork();
    void computeExecutionOrder();

    const Networks::ModuleId& moduleAt(int vertex) const;
    NetworkGraph::ExecutionOrderIterator topologicalBegin();
    NetworkGraph::ExecutionOrderIterator topologicalEnd();
    const NetworkGraph::DirectedGraph& graph();
    int moduleCount() const;
    NetworkGraph::ComponentMap connectedComponents();
    std::vector<Networks::ModuleId> downstreamModules(const Networks::ModuleId& mid) const;

  private:
    void fillDownstreamModules(const Networks::ModuleId& mid, std::vector<Networks::ModuleId>& downstream) const;
    const Networks::NetworkInterface& network_;
    Networks::ModuleFilter moduleFilter_;

    boost::bimap<Networks::ModuleId, int> moduleIdLookup_;
    NetworkGraph::ExecutionOrder order_;
    NetworkGraph::DirectedGraph graph_;
    int moduleCount_;
  };

}}}

#endif
