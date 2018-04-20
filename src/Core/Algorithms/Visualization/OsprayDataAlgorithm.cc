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

#include <Core/Algorithms/Visualization/OsprayDataAlgorithm.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Visualization;
using namespace Core::Datatypes;

ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorR);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorG);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorB);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorA);
ALGORITHM_PARAMETER_DEF(Visualization, Radius);
ALGORITHM_PARAMETER_DEF(Visualization, UseNormals);

OsprayDataAlgorithm::OsprayDataAlgorithm()
{
  addParameter(Parameters::DefaultColorR, 0.5);
  addParameter(Parameters::DefaultColorG, 0.5);
  addParameter(Parameters::DefaultColorB, 0.5);
  addParameter(Parameters::DefaultColorA, 1.0);
  addParameter(Parameters::Radius, 0.1);
  addParameter(Parameters::UseNormals, false);
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addStreamline(FieldHandle field) const
{
  auto obj = fillDataBuffers(field, nullptr);

  obj->isStreamline = true;
  auto& fieldData = obj->data;
  const auto& vertex = fieldData.vertex;
  const auto& color = fieldData.color;
  obj->radius = get(Parameters::Radius).toDouble();
  
  EdgeVector all_edges;
  std::list<Vertex> order, order_test;
  

  auto& index = fieldData.index;
  {
    auto facade(field->mesh()->getFacade());
    for (const auto& edge : facade->edges())
    {
      auto nodesFromEdge = edge.nodeIndices();
      index.push_back(nodesFromEdge[0]);
      auto nodePoints = edge.nodePoints();
      all_edges.push_back(std::make_pair(nodesFromEdge[0],nodesFromEdge[1]));
    }
  }
  
  std::vector<int32_t> cc_index;
  std::vector<int32_t> index = sort_points(all_edges,cc_index);
  //      std::cout<<"cc_index = "<<cc_index<<std::endl;
  
  std::vector<float> vertex_new, color_new;
  std::vector<int32_t> index_new;
  
  ReorderNodes(index, cc_index, vertex, color, index_new, vertex_new, color_new);
  
  vertex = vertex_new;
  index = index_new;
  color = color_new;
  
  return obj;
}

void ReorderNodes(std::vector<int32_t> index, std::vector<int32_t> cc_index, std::vector<float> vertex, std::vector<float> color, std::vector<int32_t>& index_new, std::vector<float>& vertex_new,std::vector<float>& color_new)
{
  //      std::cout<<"Reordering nodes"<<std::endl;
  
  int cc_cnt = 0;
  for (size_t k=0;k<index.size();k++)
  {
    if (k!=cc_index[cc_cnt])
    {
      index_new.push_back(k);
    }
    else
    {
      //          std::cout<<"end point"<<std::endl;
      cc_cnt++;
    }
    
    //        std::cout<<k<<std::endl;
    //        std::cout<<"vertex = [ "<<vertex[k*4]<<" , "<<vertex[k*4+1]<<" , "<<vertex[k*4+2]<<" ]"<<std::endl;
    //        std::cout<<"color = [ "<<color[k*4]<<" , "<<color[k*4+1]<<" , "<<color[k*4+2]<<" , "<<color[k*4+3]<<" ]"<<std::endl;
    
    vertex_new.push_back(vertex[index[k]*4]);
    vertex_new.push_back(vertex[index[k]*4+1]);
    vertex_new.push_back(vertex[index[k]*4+2]);
    vertex_new.push_back(0);
    color_new.push_back(color[index[k]*4]);
    color_new.push_back(color[index[k]*4+1]);
    color_new.push_back(color[index[k]*4+2]);
    color_new.push_back(color[index[k]*4+3]);
  }
}

void connected_component_edges(EdgeVector all_edges, std::vector<EdgeVector>& subsets, std::vector<int>& size_regions)
{
  UndirectedGraph graph = UndirectedGraph(all_edges.begin(), all_edges.end(), all_edges.size());
  std::vector<int> component(boost::num_vertices(graph));
  boost::connected_components(graph, &component[0]);
  //      std::cout<<"conn comp ="<<component<<std::endl;
  
  int max_comp=0;
  for (size_t i = 0; i < component.size(); ++i) if (component[i]>max_comp) max_comp = component[i];
  size_regions.clear();
  size_regions.resize(max_comp+1,0);
  for (size_t i = 0; i < component.size(); ++i) size_regions[component[i]]++;
  
  //      std::cout<<"num of cc = "<<max_comp+1<<std::endl;
  //      std::cout<<"size of ccs = "<<size_regions<<std::endl;
  
  subsets.clear();
  subsets.resize(max_comp+1);
  boost::graph_traits<UndirectedGraph>::edge_iterator ei, ei_end;
  for (tie(ei,ei_end)= edges(graph); ei != ei_end; ++ei)
  {
    //                std::cout <<"edge["<<*ei<< "]=(" << source(*ei, graph)
    //                << "," << target(*ei, graph) << ") ";
    subsets[component[source(*ei, graph)]].push_back(std::make_pair(source(*ei, graph),target(*ei, graph)));
    
  }
  //      std::cout<<"Subsets created.  Size = "<<subsets.size()<<std::endl;
  //      std::cout << std::endl;
  
  
}

std::list<Vertex_u> sort_cc(EdgeVector sub_edges)
{
  UndirectedGraph graph = UndirectedGraph(sub_edges.begin(), sub_edges.end(), sub_edges.size());
  //      std::cout << "back edges:\n";
  std::vector<Vertex_u> source_vertex;
  
  detect_loops vis(source_vertex);
  std::vector<boost::default_color_type> vertex_color( boost::num_vertices(graph) );
  auto idmap = boost::get( boost::vertex_index, graph );
  auto vcmap = make_iterator_property_map( vertex_color.begin(), idmap );
  std::map<typename UndirectedGraph::edge_descriptor, boost::default_color_type> edge_color;
  auto ecmap = boost::make_assoc_property_map( edge_color );
  boost::undirected_dfs(graph,vis,vcmap,ecmap);
  //      std::cout << std::endl;
  //
  //      std::cout<<"loop detected? "<<vis.LoopDetected()<<std::endl;
  //      std::cout<<"source_vertex = " << source_vertex<<std::endl;
  
  
  std::list<Vertex_u> v_path;
  Vertex_u v1 = sub_edges[0].first;
  v_path.push_back(v1);
  
  //      std::cout<<"starting point  = "<<v1<<std::endl;
  
  FindPath(graph,v1,v_path,false);
  
  if (vis.LoopDetected())
  {
    v_path.push_back(v_path.front());
  }
  
  
  return v_path;
  
}

bool FindPath(UndirectedGraph& graph, Vertex_u& curr_v, std::list<Vertex_u>& v_path,bool front)
{
  bool no_branch = true;
  typename boost::graph_traits<UndirectedGraph>::out_edge_iterator ei, ei_end;
  size_t edge_idx = 0;
  int cnt = 0;
  int neigh_cnt=0;
  for ( tie(ei, ei_end)=out_edges( curr_v, graph ); ei != ei_end; ++ei, ++edge_idx )
  {
    cnt++;
    Vertex_u v2a = source(*ei, graph);
    Vertex_u v2b = target(*ei, graph);
    //        std::cout<<"edge = (" << v2a<<","<<v2b<<")" <<std::endl;
    
    if (cnt ==2)
    {
      
    }
    else if (cnt>2)
    {
      std::cout<<"branch detected"<<std::endl;
      no_branch = false;
      continue;
    }
    
    if ( std::find( v_path.cbegin(), v_path.cend(), v2b ) == v_path.cend() )
    {
      neigh_cnt++;
      if (neigh_cnt ==2) front = true;
      
      if (front) v_path.push_front(v2b);
      else v_path.push_back(v2b);
      
      FindPath(graph, v2b, v_path,front);
      front = false;
    }
  }
  //      std::cout<<std::endl;
  return no_branch;
}


std::vector<int32_t> sort_points(EdgeVector edges, std::vector<int32_t>& cc_index)
{
  std::vector<EdgeVector> subsets;
  std::vector<int> size_regions;
  connected_component_edges(edges, subsets, size_regions);
  std::list<Vertex> order;
  
  int cnt=-1;
  for (auto edges_subset : subsets)
  {
    
    cnt++;
    
    //        std::cout<<"subset size ="<<edges_subset.size()<<std::endl;
    //        std::cout<<"edge_subset["<<cnt<<"] = [ ";
    //        for (auto e : edges_subset)
    //        {
    //          std::cout<<" ["<<e.first<<","<<e.second<<"]";
    //        }
    //        std::cout<<" ]"<<std::endl;
    
    int sum_regions = 0;
    for (int it=0; it<=cnt; it++) { sum_regions+=size_regions[it];}
    
    
    std::list<Vertex_u> order_subset = sort_cc(edges_subset);
    
    
    
    //        std::cout<<"order size ="<<order_subset.size()<<std::endl;
    //        std::cout<<"order_subset["<<cnt<<"] = [ ";
    //        for (auto o : order_subset)
    //        {
    //          std::cout<<" "<<o;
    //        }
    //
    //        std::cout<<" ]"<<std::endl;
    //        std::cout<<"splicing lists"<<std::endl;
    
    order_subset.reverse();
    if (cnt ==0) cc_index.push_back(order_subset.size()-1);
    else cc_index.push_back(cc_index.back()+order_subset.size());
    //        std::cout<<"cc_index = "<<cc_index.back()<<std::endl;
    //        order_subset.pop_back();
    order.splice(order.end(), order_subset);
  }
  
  
  std::vector<int32_t> index{ std::make_move_iterator(std::begin(order)),
    std::make_move_iterator(std::end(order)) };
  
  return index;
  
  
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addSurface(FieldHandle field) const
{
  auto obj = fillDataBuffers(field, nullptr);
  return obj;
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addSpheres(FieldHandle field) const
{
  auto obj = fillDataBuffers(field, nullptr);
  obj->isSphere = true;
  obj->radius = get(Parameters::Radius).toDouble();
  
  return obj;
}


OsprayGeometryObjectHandle OsprayDataAlgorithm::fillDataBuffers(FieldHandle field, ColorMapHandle colorMap) const
{
  auto facade(field->mesh()->getFacade());

  auto obj = makeObject(field);
  auto& fieldData = obj->data;
  auto& vertex = fieldData.vertex;
  auto& color = fieldData.color;

  auto vfield = field->vfield();

  {
    double value;
    ColorRGB nodeColor(get(Parameters::DefaultColorR).toDouble(),
      get(Parameters::DefaultColorG).toDouble(),
      get(Parameters::DefaultColorB).toDouble());
    auto alpha = static_cast<float>(get(Parameters::DefaultColorA).toDouble());

    for (const auto& node : facade->nodes())
    {
      auto point = node.point();
      vertex.push_back(static_cast<float>(point.x()));
      vertex.push_back(static_cast<float>(point.y()));
      vertex.push_back(static_cast<float>(point.z()));
      vertex.push_back(0);

      vfield->get_value(value, node.index());
      if (colorMap)
      {
        nodeColor = colorMap->valueToColor(value);
      }
      color.push_back(static_cast<float>(nodeColor.r()));
      color.push_back(static_cast<float>(nodeColor.g()));
      color.push_back(static_cast<float>(nodeColor.b()));
      color.push_back(alpha);
    }
  }

  auto& index = fieldData.index;
  {
    for (const auto& face : facade->faces())
    {
      auto nodes = face.nodeIndices();
      index.push_back(static_cast<int32_t>(nodes[0]));
      index.push_back(static_cast<int32_t>(nodes[1]));
      index.push_back(static_cast<int32_t>(nodes[2]));
    }
  }
  return obj;
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::makeObject(FieldHandle field) const
{
  OsprayGeometryObjectHandle obj(new OsprayGeometryObject);
  auto vmesh = field->vmesh();
  auto bbox = vmesh->get_bounding_box();
  obj->box = bbox;
  return obj;
}

AlgorithmOutput OsprayDataAlgorithm::run(const AlgorithmInput& input) const
{ 
  auto fields = input.getList<Field>(Name("Field"));
  auto colorMaps = input.getList<ColorMap>(Name("ColorMapObject"));
  auto streamlines = input.getList<Field>(Name("Streamlines"));

  std::vector<OsprayGeometryObjectHandle> renderables;

  for (auto&& fieldColor : zip(fields, colorMaps))
  {
    FieldHandle field;
    ColorMapHandle colorMap;
    boost::tie(field, colorMap) = fieldColor;

    FieldInformation info(field);

    if (info.is_trisurfmesh())
    {
      renderables.push_back(addSurface(field, colorMap));
    }
    else if (info.is_pointcloudmesh())
    {
      renderables.push_back(addSphere(field, colorMap));
    }
    else if (info.is_curvemesh())
    {
      renderables.push_back(addStreamline(field, colorMap));
    }
    else
    {
      THROW_ALGORITHM_INPUT_ERROR("Ospray rendering currently only works with trisurfs.");
    }

    
  }

  for (auto& streamline : streamlines)
  {
    FieldInformation info(streamline);

    if (!info.is_curvemesh())
      THROW_ALGORITHM_INPUT_ERROR("Ospray rendering currently only works with curvemesh streamlines.");

    renderables.push_back(addStreamline(streamline));
  }

  auto geom = boost::make_shared<CompositeOsprayGeometryObject>(renderables);
  AlgorithmOutput output;
  output[Name("SceneGraph")] = geom;
  return output;
}
