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

#include <Engine/Scheduler/BoostGraphSerialScheduler.h>
#include <Core/Dataflow/Network/NetworkInterface.h>
#include <Core/Dataflow/Network/ModuleInterface.h>
#include <Core/Dataflow/Network/ConnectionId.h>

#include <boost/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
//#include <boost/graph/depth_first_search.hpp>
//#include <boost/graph/dijkstra_shortest_paths.hpp>
//#include <boost/graph/visitors.hpp>
#include <boost/foreach.hpp>
#include <boost/bimap.hpp>

using namespace SCIRun::Engine;
using namespace SCIRun::Domain::Networks;

ModuleExecutionOrder BoostGraphSerialScheduler::schedule(const NetworkInterface& network)
{
  //std::map<int, std::string> idModuleLookup;
  boost::bimap<std::string, int> moduleIdLookup;

  for (int i = 0; i < network.nmodules(); ++i)
  {
    moduleIdLookup.left.insert(std::make_pair(network.module(i)->get_id(), i));
  }

  typedef std::pair<int,int> Edge;

  std::vector<Edge> edges;

  BOOST_FOREACH(const ConnectionDescription& cd, network.connections())
  {
    edges.push_back(std::make_pair(moduleIdLookup.left.at(cd.moduleId1_), moduleIdLookup.left.at(cd.moduleId2_)));
  }

  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> Graph;
  Graph g(edges.begin(), edges.end(), network.nmodules());

  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
  typedef std::list<Vertex> ExecutionOrder;
  ExecutionOrder order;
  boost::topological_sort(g, std::front_inserter(order));

  ModuleExecutionOrder::ModuleIdList list;
  for (ExecutionOrder::iterator i = order.begin(); i != order.end(); ++i) 
  {
    list.push_back(moduleIdLookup.right.at(*i));
  }

  return ModuleExecutionOrder(list);
}
