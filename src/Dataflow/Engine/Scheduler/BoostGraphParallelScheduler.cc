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

#include <Dataflow/Engine/Scheduler/BoostGraphParallelScheduler.h>
#include <Dataflow/Network/NetworkInterface.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ConnectionId.h>

#include <boost/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/foreach.hpp>
#include <boost/bimap.hpp>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

ParallelModuleExecutionOrder BoostGraphParallelScheduler::schedule(const NetworkInterface& network)
{
  boost::bimap<std::string, int> moduleIdLookup;

  for (int i = 0; i < network.nmodules(); ++i)
  {
    moduleIdLookup.left.insert(std::make_pair(network.module(i)->get_id(), i));
  }

  typedef std::pair<int,int> Edge;

  std::vector<Edge> edges;

  BOOST_FOREACH(const ConnectionDescription& cd, network.connections())
  {
    edges.push_back(std::make_pair(moduleIdLookup.left.at(cd.out_.moduleId_), moduleIdLookup.left.at(cd.in_.moduleId_)));
  }

  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> Graph;
  Graph g(edges.begin(), edges.end(), network.nmodules());

  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
  typedef std::list<Vertex> ExecutionOrder;
  ExecutionOrder order;
  try
  {
    boost::topological_sort(g, std::front_inserter(order));
  }
  catch (std::invalid_argument& e)
  {
    BOOST_THROW_EXCEPTION(NetworkHasCyclesException() << Core::ErrorMessage(e.what()));
  }


  // Parallel compilation ordering
  std::vector<int> time(network.nmodules(), 0);
  for (ExecutionOrder::iterator i = order.begin(); i != order.end(); ++i) {    
    // Walk through the in_edges an calculate the maximum time.
    if (in_degree (*i, g) > 0) {
      Graph::in_edge_iterator j, j_end;
      int maxdist=0;
      // Through the order from topological sort, we are sure that every 
      // time we are using here is already initialized.
      for (boost::tie(j, j_end) = in_edges(*i, g); j != j_end; ++j)
        maxdist=(std::max)(time[source(*j, g)], maxdist);
      time[*i]=maxdist+1;
    }
  }
  
  ParallelModuleExecutionOrder::ModulesByGroup map;
  for (ExecutionOrder::iterator i = order.begin(); i != order.end(); ++i) 
  {
    map.insert(std::make_pair(time[*i], moduleIdLookup.right.at(*i)));
  }

  return ParallelModuleExecutionOrder(map);
}
