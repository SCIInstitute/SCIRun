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

  class SCISHARE NetworkGraphAnalyzer : boost::noncopyable
  {
  public:
    typedef std::pair<int,int> Edge;
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> Graph;
    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef std::list<Vertex> ExecutionOrder;

    explicit NetworkGraphAnalyzer(const SCIRun::Dataflow::Networks::NetworkInterface& network, 
		const SCIRun::Dataflow::Networks::ModuleFilter& moduleFilter = [](SCIRun::Dataflow::Networks::ModuleHandle) {return true; });

    const Networks::ModuleId& moduleAt(int vertex) const;
    ExecutionOrder::iterator topologicalBegin();
    ExecutionOrder::iterator topologicalEnd();
    Graph& graph();
    int moduleCount() const;

  private:
    boost::bimap<Networks::ModuleId, int> moduleIdLookup_;
    ExecutionOrder order_;
    Graph graph_;
    int moduleCount_;
  };

}}}

#endif