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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/utility.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/cstdlib.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/lambda/lambda.hpp>

#include <Core/Logging/Log.h>
#include <spdlog/fmt/ostr.h>


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
  addParameter(Parameters::UseNormals, true);
}

struct detect_loops : public boost::dfs_visitor<>
{
  detect_loops(std::vector<Vertex_u>& _source_vertex) : source_vertex(_source_vertex) { }
  
  bool LoopDetected() const {return !source_vertex.empty();}
  
  template <class Edge, class Graph>
  void back_edge(Edge e, const Graph& g)
  {
    source_vertex.push_back( source(e, g) );
  }
  std::vector<Vertex_u>& source_vertex;
};


OsprayGeometryObjectHandle OsprayDataAlgorithm::addStreamline(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = fillDataBuffers(field, colorMap);

  obj->isStreamline = true;
  obj->GeomType="Streamlines";
  auto& fieldData = obj->data;
  std::vector<float> vertex_orig, color_orig;
  auto& vertex = fieldData.vertex;
  auto& color = fieldData.color;
  auto& index = fieldData.index;
  obj->radius = static_cast<float>(get(Parameters::Radius).toDouble());
  
  EdgeVector all_edges;
  std::list<Vertex> order, order_test;
  
  std::vector<float> vertex_new, color_new;
  std::vector<int32_t> index_new;
  

  std::vector<int32_t> index_orig;
  {
    auto facade(field->mesh()->getFacade());
    for (const auto& edge : facade->edges())
    {
      auto nodesFromEdge = edge.nodeIndices();
      index_orig.push_back(nodesFromEdge[0]);
      auto nodePoints = edge.nodePoints();
      all_edges.push_back(std::make_pair(nodesFromEdge[0],nodesFromEdge[1]));
    }
  }
  
  std::vector<int32_t> cc_index;
  std::vector<int32_t> index_sort = sort_points(all_edges,cc_index);
  
  ReorderNodes(index_sort, cc_index, vertex, color, index_new, vertex_new, color_new);
  
  
  index = index_new;
  vertex = vertex_new;
  color = color_new;
  
  return obj;
}

void OsprayDataAlgorithm::ReorderNodes(std::vector<int32_t> index, std::vector<int32_t> cc_index, std::vector<float> vertex, std::vector<float> color, std::vector<int32_t>& index_new, std::vector<float>& vertex_new,std::vector<float>& color_new) const
{
  
  int cc_cnt = 0;
  for (size_t k=0;k<index.size();k++)
  {
    if (k!=cc_index[cc_cnt])
    {
      index_new.push_back(k);
    }
    else
    {
      cc_cnt++;
    }

    
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

void OsprayDataAlgorithm::connected_component_edges(EdgeVector all_edges, std::vector<EdgeVector>& subsets, std::vector<int>& size_regions) const
{
  UndirectedGraph graph = UndirectedGraph(all_edges.begin(), all_edges.end(), all_edges.size());
  std::vector<int> component(boost::num_vertices(graph));
  boost::connected_components(graph, &component[0]);
  
  int max_comp=0;
  for (size_t i = 0; i < component.size(); ++i) if (component[i]>max_comp) max_comp = component[i];
  size_regions.clear();
  size_regions.resize(max_comp+1,0);
  for (size_t i = 0; i < component.size(); ++i) size_regions[component[i]]++;
  subsets.clear();
  subsets.resize(max_comp+1);
  boost::graph_traits<UndirectedGraph>::edge_iterator ei, ei_end;
  for (tie(ei,ei_end)= edges(graph); ei != ei_end; ++ei)
  {
    subsets[component[source(*ei, graph)]].push_back(std::make_pair(source(*ei, graph),target(*ei, graph)));
    
  }
  
  
}

std::list<Vertex_u> OsprayDataAlgorithm::sort_cc(EdgeVector sub_edges) const
{
  UndirectedGraph graph = UndirectedGraph(sub_edges.begin(), sub_edges.end(), sub_edges.size());
  
  std::vector<Vertex_u> source_vertex;
  
  detect_loops vis(source_vertex);
  std::vector<boost::default_color_type> vertex_color( boost::num_vertices(graph) );
  auto idmap = boost::get( boost::vertex_index, graph );
  auto vcmap = make_iterator_property_map( vertex_color.begin(), idmap );
  std::map<typename UndirectedGraph::edge_descriptor, boost::default_color_type> edge_color;
  auto ecmap = boost::make_assoc_property_map( edge_color );
  boost::undirected_dfs(graph,vis,vcmap,ecmap);
  
  std::list<Vertex_u> v_path;
  Vertex_u v1 = sub_edges[0].first;
  v_path.push_back(v1);
  

  FindPath(graph,v1,v_path,false);
  
  if (vis.LoopDetected())
  {
    v_path.push_back(v_path.front());
  }
  
  return v_path;
  
}

bool OsprayDataAlgorithm::FindPath(UndirectedGraph& graph, Vertex_u& curr_v, std::list<Vertex_u>& v_path, bool front) const
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

    if (cnt ==2)
    {
      
    }
    else if (cnt>2)
    {
      remark("branch detected");
      //TODO: deal with branching streamlines
      no_branch = false;
      continue;
    }
    
    if ( std::find( v_path.cbegin(), v_path.cend(), v2b ) == v_path.cend() )
    {
      neigh_cnt++;
      if (neigh_cnt ==2) front = true;
      
      if (front) v_path.push_front(v2b);
      else v_path.push_back(v2b);
      
      FindPath( graph, v2b, v_path,front);
      front = false;
    }
  }
  return no_branch;
}


std::vector<int32_t> OsprayDataAlgorithm::sort_points(EdgeVector edges, std::vector<int32_t>& cc_index) const
{
  std::vector<EdgeVector> subsets;
  std::vector<int> size_regions;
  connected_component_edges(edges, subsets, size_regions);
  std::list<Vertex> order;
  
  int cnt=-1;
  for (auto edges_subset : subsets)
  {
    
    cnt++;
    LOG_DEBUG("subset size = {}",edges_subset.size());
    std::ostringstream ostr;
    ostr << "edge_subset["<<cnt<<"] = [ ";
    for (auto e : edges_subset)
    {
      ostr<<" ["<<e.first<<","<<e.second<<"]";
    }
    ostr<<" ]";
    LOG_DEBUG(ostr.str());
    
    int sum_regions = 0;
    for (int it=0; it<=cnt; it++) { sum_regions+=size_regions[it];}
    
    
    std::list<Vertex_u> order_subset = sort_cc(edges_subset);
    
    LOG_DEBUG("order size = {}",order_subset.size());
    std::ostringstream ostr_2;
    ostr_2 << "order_subset["<<cnt<<"] = [ ";
    for (auto o : order_subset)
    {
      ostr_2<<" "<<o;
    }
    ostr_2<<" ]";
    LOG_DEBUG(ostr_2.str());
    
    order_subset.reverse();
    if (cnt ==0) cc_index.push_back(order_subset.size()-1);
    else cc_index.push_back(cc_index.back()+order_subset.size());
    order.splice(order.end(), order_subset);
  }
  
  
  std::vector<int32_t> index{ std::make_move_iterator(std::begin(order)),
    std::make_move_iterator(std::end(order)) };
  
  return index;
  
  
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addSurface(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = fillDataBuffers(field, colorMap);
  return obj;
  obj->isSurface = true;
  obj->GeomType="Surface";
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addSphere(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = fillDataBuffers(field, colorMap);
  obj->isSphere = true;
  obj->GeomType="Spheres";
std::cout<<"sphere radius = "<<static_cast<float> (get(Parameters::Radius).toDouble())<<std::endl;
  obj->radius = static_cast<float>(get(Parameters::Radius).toDouble());
  
  return obj;
}


OsprayGeometryObjectHandle OsprayDataAlgorithm::fillDataBuffers(FieldHandle field, ColorMapHandle colorMap) const
{
  auto facade(field->mesh()->getFacade());

  auto obj = makeObject(field);
  auto& fieldData = obj->data;
  auto& vertex = fieldData.vertex;
  auto& color = fieldData.color;
  auto& vertex_normal = fieldData.vertex_normal;
  
  

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
  
  FieldInformation info(field);
  std::cout<<"useNormals = "<<get(Parameters::UseNormals).toBool()<<std::endl;
  std::cout<<"is trisurf ="<<info.is_trisurfmesh()<<std::endl;
  
  if (get(Parameters::UseNormals).toBool() && info.is_trisurfmesh())
  {
    std::cout<<"using normals"<<std::endl;
    {
      Vector norm;
      
      for (const auto& node : facade->nodes())
      {
        field->vmesh()->get_normal(norm,node.index());
        
        vertex_normal.push_back(static_cast<float>(norm.x()));
        vertex_normal.push_back(static_cast<float>(norm.y()));
        vertex_normal.push_back(static_cast<float>(norm.z()));
        vertex_normal.push_back(0);
      }
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
      THROW_ALGORITHM_INPUT_ERROR("field type not supported.");
    }

    
  }

  auto geom = boost::make_shared<CompositeOsprayGeometryObject>(renderables);
  AlgorithmOutput output;
  output[Name("SceneGraph")] = geom;
  return output;
}
