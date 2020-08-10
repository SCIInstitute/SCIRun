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


#include <Core/Algorithms/Visualization/OsprayDataAlgorithm.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/LatVolMesh.h>
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
using namespace OsprayVisualization;
using namespace Core::Datatypes;

ALGORITHM_PARAMETER_DEF(OsprayVisualization, DefaultColorR);
ALGORITHM_PARAMETER_DEF(OsprayVisualization, DefaultColorG);
ALGORITHM_PARAMETER_DEF(OsprayVisualization, DefaultColorB);
ALGORITHM_PARAMETER_DEF(OsprayVisualization, DefaultColorA);
ALGORITHM_PARAMETER_DEF(OsprayVisualization, Radius);
ALGORITHM_PARAMETER_DEF(OsprayVisualization, UseNormals);
ALGORITHM_PARAMETER_DEF(OsprayVisualization, ShowEdges);
ALGORITHM_PARAMETER_DEF(OsprayVisualization, ModuleID);

OsprayDataAlgorithm::OsprayDataAlgorithm()
{
  addParameter(Parameters::DefaultColorR, 0.5);
  addParameter(Parameters::DefaultColorG, 0.5);
  addParameter(Parameters::DefaultColorB, 0.5);
  addParameter(Parameters::DefaultColorA, 1.0);
  addParameter(Parameters::Radius, 0.1);
  addParameter(Parameters::UseNormals, true);
  addParameter(Parameters::ShowEdges, false);
  addParameter(Parameters::ModuleID, 0);
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

static uint32_t getNewVersionNumber()
{
  static uint32_t versionNumber = 0;
  return ++versionNumber;
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addStreamline(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = fillDataBuffers(field, colorMap);
  obj->type = GeometryType::STREAMLINE;
  auto& fieldData = obj->data;
  std::vector<float> vertex_orig, color_orig;
  auto& vertex = fieldData.vertex;
  auto& color = fieldData.color;
  auto& index = fieldData.index;
  obj->radius = static_cast<float>(get(Parameters::Radius).toDouble());

  EdgeVector all_edges;
  std::list<Vertex> order, order_test;

  std::vector<float> vertex_new, color_new;
  std::vector<uint32_t> index_new;


  std::vector<uint32_t> index_orig;
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

  std::vector<uint32_t> cc_index;
  std::vector<uint32_t> index_sort = sort_points(all_edges,cc_index);

  ReorderNodes(index_sort, cc_index, vertex, color, index_new, vertex_new, color_new);


  index = index_new;
  vertex = vertex_new;
  color = color_new;

  return obj;
}

void OsprayDataAlgorithm::ReorderNodes(std::vector<uint32_t> index, std::vector<uint32_t> cc_index, std::vector<float> vertex, std::vector<float> color, std::vector<uint32_t>& index_new, std::vector<float>& vertex_new,std::vector<float>& color_new) const
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
  std::map<UndirectedGraph::edge_descriptor, boost::default_color_type> edge_color;
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
  boost::graph_traits<UndirectedGraph>::out_edge_iterator ei, ei_end;
  size_t edge_idx = 0;
  int cnt = 0;
  int neigh_cnt=0;
  for ( tie(ei, ei_end)=out_edges( curr_v, graph ); ei != ei_end; ++ei, ++edge_idx )
  {
    cnt++;
    source(*ei, graph);
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


std::vector<uint32_t> OsprayDataAlgorithm::sort_points(EdgeVector edges, std::vector<uint32_t>& cc_index) const
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


  std::vector<uint32_t> index{ std::make_move_iterator(std::begin(order)),
    std::make_move_iterator(std::end(order)) };

  return index;
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addTriSurface(FieldHandle field, ColorMapHandle colorMap) const
{
  printf("add tri-surface\n");
  auto obj = fillDataBuffers(field, colorMap);
  obj->type = GeometryType::TRI_SURFACE;
  return obj;
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addQuadSurface(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = fillDataBuffers(field, colorMap);
  obj->type = GeometryType::QUAD_SURFACE;
  return obj;
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addStructVol(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = makeObject(field);
  obj->type = GeometryType::STRUCTURED_VOLUME;

  auto& fieldData = obj->data;

  std::vector<float> voxels;
  std::vector<float> vertex_new;

  auto facade(field->mesh()->getFacade());
  auto vfield = field->vfield();
  auto vmesh = field->vmesh();

  const BBox bbox = vmesh->get_bounding_box();
  Vector size = bbox.diagonal();
  Point center = bbox.center();
  VMesh::dimension_type dim;
  vmesh->get_dimensions(dim);
  Vector dimensions_ = Vector(1.0,1.0,1.0);
  for (size_t p=0;p<dim.size();p++) dimensions_[p] = static_cast<double>(dim[p]);

  fieldData.dim[0] = dimensions_[0];
  fieldData.dim[1] = dimensions_[1];
  fieldData.dim[2] = dimensions_[2];
  fieldData.origin[0] = center.x() - size.x()/2.0;
  fieldData.origin[2] = center.y() - size.y()/2.0;
  fieldData.origin[2] = center.z() - size.z()/2.0;
  fieldData.spacing[0] = size.x()/dimensions_[0];
  fieldData.spacing[1] = size.y()/dimensions_[1];
  fieldData.spacing[2] = size.z()/dimensions_[2];

  double value;
  //std::cout << "mname:" << field->mesh()->type_name << std::endl;

  for (const auto& node : facade->nodes())
  {
    auto point = node.point();
    if (vfield->num_values() > 0)
    {
      vfield->get_value(value, node.index());
      voxels.push_back(value);
    }
    vertex_new.push_back(static_cast<float>(point.x()));
    vertex_new.push_back(static_cast<float>(point.y()));
    vertex_new.push_back(static_cast<float>(point.z()));

  }
  //auto alpha = static_cast<float>(get(Parameters::DefaultColorA).toDouble());
  if (colorMap)
  {
    ColorMap_OSP_helper cmp(colorMap->getColorMapName());
    obj->tfn.colors = cmp.colorList;

    // set default opacity for now
    // alpha pushed twice for both upper and lower values
    auto alpha = static_cast<float>(get(Parameters::DefaultColorA).toDouble());
    obj->tfn.opacities.push_back(alpha);
    obj->tfn.opacities.push_back(alpha);
  }
  else
  {
    auto red = static_cast<float>(get(Parameters::DefaultColorR).toDouble());
    auto green = static_cast<float>(get(Parameters::DefaultColorG).toDouble());
    auto blue = static_cast<float>(get(Parameters::DefaultColorB).toDouble());

    obj->tfn.colors.push_back(red);
    obj->tfn.colors.push_back(green);
    obj->tfn.colors.push_back(blue);
    obj->tfn.colors.push_back(red);
    obj->tfn.colors.push_back(green);
    obj->tfn.colors.push_back(blue);

    auto alpha = static_cast<float>(get(Parameters::DefaultColorA).toDouble());
    obj->tfn.opacities.push_back(alpha);
    obj->tfn.opacities.push_back(alpha);
  }

  fieldData.color = voxels;
  fieldData.vertex = vertex_new;
  return obj;
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addUnstructVol(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = makeObject(field);
  obj->type = GeometryType::UNSTRUCTURED_VOLUME;

  auto& fieldData = obj->data;

  std::vector<float> voxels;
  std::vector<float> vertex_new;
  std::vector<uint32_t> index_new;

  auto facade(field->mesh()->getFacade());
  auto vfield = field->vfield();
  auto vmesh = field->vmesh();


  double value;
  //std::cout << "mname:" << field->mesh()->type_name << std::endl;
  for (const auto& node : facade->nodes())
  {
    auto point = node.point();
    if (vfield->num_values() > 0)
    {
      vfield->get_value(value, node.index());
      voxels.push_back(value);
    }
    vertex_new.push_back(static_cast<float>(point.x()));
    vertex_new.push_back(static_cast<float>(point.y()));
    vertex_new.push_back(static_cast<float>(point.z()));
  }

  VMesh::Cell::iterator meshCellIter;
  VMesh::Cell::iterator meshCellEnd;
  vmesh->end(meshCellEnd);

  int numVPerCell = -1;
  FieldInformation info(field);

  if(info.is_tetvol()) numVPerCell =4;
  else if(info.is_hexvol()) numVPerCell =8;
  else THROW_ALGORITHM_INPUT_ERROR("hex or tet only for unstructured volume!");

  for (vmesh->begin(meshCellIter); meshCellIter != meshCellEnd; ++meshCellIter)
  {
    // OSPRay require an index array of size 8
    // for each unstructured mesh cell
    // a tetrahedral cell's first 4 vertices are set to -1
    // a wedge cell's first 2 vertices are set to -2
    VMesh::Cell::index_type elemID = *meshCellIter;
    VMesh::Node::array_type nodesFromCell(8);
    vmesh->get_nodes(nodesFromCell, elemID);
    for(int i=numVPerCell;i<8;i++)
      nodesFromCell[i] = -4/(8-numVPerCell);
    for(int i=numVPerCell;i<8+numVPerCell;i++){
      index_new.push_back(nodesFromCell[i%8]);
    }
  }

  if (colorMap)
  {
    ColorMap_OSP_helper cmp(colorMap->getColorMapName());
    obj->tfn.colors = cmp.colorList;

    // set default opacity for now
    // alpha pushed twice for both upper and lower values
    auto alpha = static_cast<float>(get(Parameters::DefaultColorA).toDouble());
    obj->tfn.opacities.push_back(alpha);
    obj->tfn.opacities.push_back(alpha);
  }
  else
  {
    auto red = static_cast<float>(get(Parameters::DefaultColorR).toDouble());
    auto green = static_cast<float>(get(Parameters::DefaultColorG).toDouble());
    auto blue = static_cast<float>(get(Parameters::DefaultColorB).toDouble());

    obj->tfn.colors.push_back(red);
    obj->tfn.colors.push_back(green);
    obj->tfn.colors.push_back(blue);
    obj->tfn.colors.push_back(red);
    obj->tfn.colors.push_back(green);
    obj->tfn.colors.push_back(blue);

    auto alpha = static_cast<float>(get(Parameters::DefaultColorA).toDouble());
    obj->tfn.opacities.push_back(alpha);
    obj->tfn.opacities.push_back(alpha);
  }

  fieldData.color = voxels;
  fieldData.vertex = vertex_new;
  fieldData.index = index_new;
  return obj;
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addCylinder(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = fillDataBuffers(field, colorMap);
  obj->type = GeometryType::CYLINDER;
  obj->radius = static_cast<float>(get(Parameters::Radius).toDouble());


  auto& fieldData = obj->data;

  auto& vertex = fieldData.vertex;
  auto& color = fieldData.color;
  auto& index = fieldData.index;

  std::vector<float> vertex_new, color_new;
  std::vector<uint32_t> index_new;

  ColorRGB nodeColor(get(Parameters::DefaultColorR).toDouble(),
                     get(Parameters::DefaultColorG).toDouble(),
                     get(Parameters::DefaultColorB).toDouble());
  auto alpha = static_cast<float>(get(Parameters::DefaultColorA).toDouble());

  std::vector<float> vertex_orig;
  {
    auto facade(field->mesh()->getFacade());
    for (const auto& edge : facade->edges())
    {
      auto nodePoints = edge.nodePoints();
//      std::cout<<"points ="<<nodePoints[0]<<", "<<nodePoints[1]<<std::endl;

      vertex_new.push_back(static_cast<float>(nodePoints[0].x()));
      vertex_new.push_back(static_cast<float>(nodePoints[0].y()));
      vertex_new.push_back(static_cast<float>(nodePoints[0].z()));
      vertex.push_back(1);
      vertex_new.push_back(static_cast<float>(nodePoints[1].x()));
      vertex_new.push_back(static_cast<float>(nodePoints[1].y()));
      vertex_new.push_back(static_cast<float>(nodePoints[1].z()));
      vertex.push_back(1);

      // hard coded to default value for now
      color_new.push_back(static_cast<float>(nodeColor.r()));
      color_new.push_back(static_cast<float>(nodeColor.g()));
      color_new.push_back(static_cast<float>(nodeColor.b()));
      color_new.push_back(alpha);
    }
  }

  index = index_new;
  vertex = vertex_new;
  color = color_new;

  return obj;
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addSphere(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = fillDataBuffers(field, colorMap);
  obj->type = GeometryType::SPHERE;
  obj->radius = static_cast<float>(get(Parameters::Radius).toDouble());
  return obj;
}


OsprayGeometryObjectHandle OsprayDataAlgorithm::fillDataBuffers(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = makeObject(field);
  auto& fieldData = obj->data;
  auto& vertex = fieldData.vertex;
  auto& color = fieldData.color;
  auto& normal = fieldData.normal;

  FieldInformation info(field);
  auto facade(field->mesh()->getFacade());
  auto vfield = field->vfield();

  for (const auto& node : facade->nodes())
  {
    auto point = node.point();
    vertex.push_back(static_cast<float>(point.x()));
    vertex.push_back(static_cast<float>(point.y()));
    vertex.push_back(static_cast<float>(point.z()));
  }

  {
    double value;
    ColorRGB nodeColor(get(Parameters::DefaultColorR).toDouble(),
                       get(Parameters::DefaultColorG).toDouble(),
                       get(Parameters::DefaultColorB).toDouble());
    float alpha = static_cast<float>(get(Parameters::DefaultColorA).toDouble());
    for (const auto& node : facade->nodes())
    {
      if (vfield->num_values() > 0)
      {
        vfield->get_value(value, node.index());
        if (colorMap) nodeColor = colorMap->valueToColor(value);
      }

      color.push_back(static_cast<float>(nodeColor.r()));
      color.push_back(static_cast<float>(nodeColor.g()));
      color.push_back(static_cast<float>(nodeColor.b()));
      color.push_back(alpha);
    }
  }

  if (get(Parameters::UseNormals).toBool() && info.is_surface())
  {
    auto mesh = field->vmesh();
    mesh->synchronize(Mesh::NORMALS_E);
    {
      Vector norm;
      for (const auto& node : facade->nodes())
      {
        mesh->get_normal(norm, node.index());
        normal.push_back(static_cast<float>(norm.x()));
        normal.push_back(static_cast<float>(norm.y()));
        normal.push_back(static_cast<float>(norm.z()));
      }
    }
  }

  auto& index = fieldData.index;
  for (const auto& face : facade->faces())
  {
    auto nodes = face.nodeIndices();
    if(info.is_quadsurfmesh())
    {
      // quad face added in reverse order for correct normal in OSPRay viewer
      index.push_back(static_cast<uint32_t>(nodes[3]));
      index.push_back(static_cast<uint32_t>(nodes[2]));
      index.push_back(static_cast<uint32_t>(nodes[1]));
      index.push_back(static_cast<uint32_t>(nodes[0]));
    }
    else
    {
      index.push_back(static_cast<uint32_t>(nodes[0]));
      index.push_back(static_cast<uint32_t>(nodes[1]));
      index.push_back(static_cast<uint32_t>(nodes[2]));
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
  auto field = input.get<Field>(Name("Field"));
  auto colorMap = input.get<ColorMap>(Name("ColorMapObject"));

  OsprayGeometryObjectHandle renderable;

  FieldInformation info(field);
  if (info.is_trisurfmesh())
  {
    // currently only supports one output, so no point in doing both
    if (get(Parameters::ShowEdges).toBool())
    {
      renderable = addCylinder(field, colorMap);
    }
    else
    {
      renderable = addTriSurface(field, colorMap);
    }
  }
  else if (info.is_quadsurfmesh()){
      renderable = addQuadSurface(field, colorMap);
      //THROW_ALGORITHM_INPUT_ERROR("field type quad.");
  }
  else if (info.is_volume())
  {
    if(info.is_hexvol()){
      renderable = addUnstructVol(field, colorMap);
      //THROW_ALGORITHM_INPUT_ERROR("Hex vol not supported. LatVol only at this point");
    }else if(info.is_latvol()){
      renderable = addStructVol(field, colorMap);
      //renderable = addStructVol(field, colorMap);
    }else if(info.is_tetvol()){
      renderable = addUnstructVol(field, colorMap);
      //THROW_ALGORITHM_INPUT_ERROR("Tet vol not supported. LatVol only at this point");
    }else {
      THROW_ALGORITHM_INPUT_ERROR("Unknown vol type. LatVol only at this point");
    }
  }
  else if (info.is_pointcloudmesh())
  {
    renderable = addSphere(field, colorMap);
  }
  else if (info.is_curvemesh())
  {
    renderable = addStreamline(field, colorMap);
  }
  else
  {
    if (get(Parameters::ShowEdges).toBool())
    {
      renderable = addCylinder(field, colorMap);
    }
    else
    {
      THROW_ALGORITHM_INPUT_ERROR("field type not supported.");
    }
  }

  renderable->version = getNewVersionNumber();
  renderable->id = get(Parameters::ModuleID).toInt();

  AlgorithmOutput output;
  output[Name("SceneGraph")] = renderable;
  return output;
}
