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

#ifdef WITH_VTK
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkHexahedron.h>
#include <vtkCellArray.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkFloatArray.h>
#include <vtkTriangle.h>
#include <vtkQuad.h>
#include <vtkLine.h>
#include <vtkTetra.h>
#endif

#include <Core/Algorithms/Visualization/VtkGeometryBuilder.h>
#include <Core/Algorithms/Visualization/VtkDataAlgorithm.h>
#include <Core/Datatypes/Geometry.h>
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
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/connected_components.hpp>

using namespace SCIRun;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Visualization;
using namespace VtkVisualization;
using namespace Core::Datatypes;

struct detect_loops : public boost::dfs_visitor<>
{
  detect_loops(std::vector<Vertex_u>& _source_vertex) : source_vertex(_source_vertex) {}

  bool LoopDetected() const { return !source_vertex.empty(); }

  template <class Edge, class Graph>
  void back_edge(Edge e, const Graph& g)
  {
    source_vertex.push_back(source(e, g));
  }
  std::vector<Vertex_u>& source_vertex;
};

VtkGeometryBuilder::VtkGeometryBuilder(
	const VtkDataAlgorithm& algorithm)
	: algorithm_(algorithm)
{
}

VtkGeometryBuilder&
VtkGeometryBuilder::add(
	VtkGeometryObjectHandle obj)
{
	if (obj)
		objects_.push_back(obj);

	return *this;
}

VtkGeometryObjectHandle
VtkGeometryBuilder::finalize()
{
	if (objects_.empty())
		return nullptr;

	if (objects_.size() == 1)
		return objects_.front();

	return std::make_shared<CompositeVtkGeometryObject>(
		objects_);
}

VtkGeometryObjectHandle
VtkGeometryBuilder::buildGeometryObject(
    FieldHandle field,
    ColorMapHandle colorMap)
{
    FieldInformation info(field);

    bool showEdges = algorithm_.get(VtkVisualization::Parameters::ShowEdges).toBool();
    bool showNodes = algorithm_.get(VtkVisualization::Parameters::ShowNodes).toBool();

    if (info.is_trisurfmesh())
    {
        add(addTriSurface(field, colorMap));

        if (showEdges)
        {
            add(addCylinder(field, colorMap));
        }
    }
    else if (info.is_quadsurfmesh())
    {
        add(addQuadSurface(field, colorMap));

        if (showEdges)
        {
            add(addCylinder(field, colorMap));
        }
    }
    else if (info.is_volume())
    {
        if (info.is_latvol())
        {
            add(addStructVol(field, colorMap));
        }
        else if (info.is_hexvol() ||
            info.is_tetvol())
        {
            add(addUnstructVol(field, colorMap));
        }

        if (showEdges)
        {
            add(addCylinder(field, colorMap));
        }

        if (showNodes)
        {
            add(addSphere(field, colorMap));
        }
    }
    else if (info.is_pointcloudmesh())
    {
        add(addSphere(field, colorMap));
    }
    else if (info.is_curvemesh())
    {
        add(addStreamline(field, colorMap));
    }

    return finalize();
}

VtkGeometryObjectHandle VtkGeometryBuilder::addStreamline(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = makeObject(field);

#ifdef WITH_VTK
  obj->type = GeometryType::STREAMLINE;
  obj->radius = static_cast<float>(algorithm_.get(Parameters::Radius).toDouble());

  //----------------------------------
  // VTK polydata
  //----------------------------------

  auto poly = vtkSmartPointer<vtkPolyData>::New();
  auto points = vtkSmartPointer<vtkPoints>::New();
  auto lines = vtkSmartPointer<vtkCellArray>::New();

  auto facade = field->mesh()->getFacade();
  auto vfield = field->vfield();

  //----------------------------------
  // Create VTK points
  //----------------------------------

  std::unordered_map<uint32_t, vtkIdType> pointMap;

  for (const auto& node : facade->nodes())
  {
    auto p = node.point();

    vtkIdType pid = points->InsertNextPoint(p.x(), p.y(), p.z());

    pointMap[node.index()] = pid;
  }

  poly->SetPoints(points);

  //----------------------------------
  // Point scalars
  //----------------------------------

  auto scalars = vtkSmartPointer<vtkDoubleArray>::New();
  scalars->SetName("Values");

  double value = 0.0;

  for (const auto& node : facade->nodes())
  {
    if (vfield->num_values() > 0)
    {
      vfield->get_value(value, node.index());
    }
    else
    {
      value = 0.0;
    }

    scalars->InsertNextValue(value);
  }

  poly->GetPointData()->SetScalars(scalars);

  //----------------------------------
  // Collect mesh edges
  //----------------------------------

  EdgeVector all_edges;

  for (const auto& edge : facade->edges())
  {
    auto nodes = edge.nodeIndices();

    all_edges.push_back(std::make_pair(static_cast<Vertex_u>(nodes[0]), static_cast<Vertex_u>(nodes[1])));
  }

  //----------------------------------
  // Split into connected components
  //----------------------------------

  std::vector<EdgeVector> subsets;
  std::vector<int> size_regions;

  connected_component_edges(all_edges, subsets, size_regions);

  //----------------------------------
  // Build one vtkPolyLine per component
  //----------------------------------

  for (const auto& subset : subsets)
  {
    if (subset.empty()) continue;

    auto orderedNodes = sort_cc(subset);

    if (orderedNodes.size() < 2) continue;

    auto polyLine = vtkSmartPointer<vtkPolyLine>::New();

    polyLine->GetPointIds()->SetNumberOfIds(static_cast<vtkIdType>(orderedNodes.size()));

    vtkIdType idx = 0;

    for (auto nodeId : orderedNodes)
    {
      auto it = pointMap.find(nodeId);

      if (it == pointMap.end()) continue;

      polyLine->GetPointIds()->SetId(idx++, it->second);
    }

    lines->InsertNextCell(polyLine);
  }

  poly->SetLines(lines);

  //----------------------------------
  // Material
  //----------------------------------

  obj->material.color[0] = static_cast<float>(algorithm_.get(Parameters::DefaultColorR).toDouble());

  obj->material.color[1] = static_cast<float>(algorithm_.get(Parameters::DefaultColorG).toDouble());

  obj->material.color[2] = static_cast<float>(algorithm_.get(Parameters::DefaultColorB).toDouble());

  obj->material.opacity = static_cast<float>(algorithm_.get(Parameters::DefaultColorA).toDouble());

  //----------------------------------
  // Transfer function
  //----------------------------------

  double range[2];
  scalars->GetRange(range);

  obj->tfn.range = {static_cast<float>(range[0]), static_cast<float>(range[1])};

  if (colorMap)
  {
    ColorMap_OSP_helper cmp(colorMap);

    obj->tfn.colors = cmp.colorList_;
    obj->tfn.opacities = cmp.opacityList_;
  }
  else
  {
    obj->tfn.colors = {obj->material.color[0], obj->material.color[1], obj->material.color[2]};

    obj->tfn.opacities = {obj->material.opacity};
  }

  obj->dataObject = poly;
#endif

  return obj;
}

void VtkGeometryBuilder::connected_component_edges(EdgeVector all_edges, std::vector<EdgeVector>& subsets, std::vector<int>& size_regions) const
{
  UndirectedGraph graph = UndirectedGraph(all_edges.begin(), all_edges.end(), all_edges.size());
  std::vector<int> component(boost::num_vertices(graph));
  boost::connected_components(graph, &component[0]);

  int max_comp = 0;
  for (size_t i = 0; i < component.size(); ++i)
    if (component[i] > max_comp) max_comp = component[i];
  size_regions.clear();
  size_regions.resize(max_comp + 1, 0);
  for (size_t i = 0; i < component.size(); ++i)
    size_regions[component[i]]++;
  subsets.clear();
  subsets.resize(max_comp + 1);
  boost::graph_traits<UndirectedGraph>::edge_iterator ei, ei_end;
  for (tie(ei, ei_end) = edges(graph); ei != ei_end; ++ei)
  {
    subsets[component[source(*ei, graph)]].push_back(std::make_pair(source(*ei, graph), target(*ei, graph)));
  }
}

std::list<Vertex_u> VtkGeometryBuilder::sort_cc(EdgeVector sub_edges) const
{
  UndirectedGraph graph = UndirectedGraph(sub_edges.begin(), sub_edges.end(), sub_edges.size());

  std::vector<Vertex_u> source_vertex;

  detect_loops vis(source_vertex);
  std::vector<boost::default_color_type> vertex_color(boost::num_vertices(graph));
  auto idmap = boost::get(boost::vertex_index, graph);
  auto vcmap = make_iterator_property_map(vertex_color.begin(), idmap);
  std::map<UndirectedGraph::edge_descriptor, boost::default_color_type> edge_color;
  auto ecmap = boost::make_assoc_property_map(edge_color);
  boost::undirected_dfs(graph, vis, vcmap, ecmap);

  std::list<Vertex_u> v_path;
  Vertex_u v1 = sub_edges[0].first;
  v_path.push_back(v1);

  FindPath(graph, v1, v_path, false);

  if (vis.LoopDetected())
  {
    v_path.push_back(v_path.front());
  }

  return v_path;
}

bool VtkGeometryBuilder::FindPath(UndirectedGraph& graph, Vertex_u& curr_v, std::list<Vertex_u>& v_path, bool front) const
{
  bool no_branch = true;
  boost::graph_traits<UndirectedGraph>::out_edge_iterator ei, ei_end;
  size_t edge_idx = 0;
  int cnt = 0;
  int neigh_cnt = 0;
  for (tie(ei, ei_end) = out_edges(curr_v, graph); ei != ei_end; ++ei, ++edge_idx)
  {
    cnt++;
    source(*ei, graph);
    Vertex_u v2b = target(*ei, graph);

    if (cnt == 2)
    {
    }
    else if (cnt > 2)
    {
      //remark("branch detected");
      // TODO: deal with branching streamlines
      no_branch = false;
      continue;
    }

    if (std::find(v_path.cbegin(), v_path.cend(), v2b) == v_path.cend())
    {
      neigh_cnt++;
      if (neigh_cnt == 2) front = true;

      if (front)
        v_path.push_front(v2b);
      else
        v_path.push_back(v2b);

      FindPath(graph, v2b, v_path, front);
      front = false;
    }
  }
  return no_branch;
}

VtkGeometryObjectHandle VtkGeometryBuilder::addTriSurface(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = makeObject(field);

#ifdef WITH_VTK
  obj->type = GeometryType::TRI_SURFACE;

  auto poly = vtkSmartPointer<vtkPolyData>::New();
  auto points = vtkSmartPointer<vtkPoints>::New();
  auto polys = vtkSmartPointer<vtkCellArray>::New();

  auto facade = field->mesh()->getFacade();
  auto vfield = field->vfield();
  auto vmesh = field->vmesh();

  //----------------------------------
  // Points
  //----------------------------------

  for (const auto& node : facade->nodes())
  {
    auto p = node.point();

    points->InsertNextPoint(p.x(), p.y(), p.z());
  }

  poly->SetPoints(points);

  //----------------------------------
  // Point scalars
  //----------------------------------

  auto scalars = vtkSmartPointer<vtkDoubleArray>::New();
  scalars->SetName("Values");

  double value = 0.0;

  for (const auto& node : facade->nodes())
  {
    if (vfield->num_values() > 0)
    {
      vfield->get_value(value, node.index());
    }
    else
    {
      value = 0.0;
    }

    scalars->InsertNextValue(value);
  }

  poly->GetPointData()->SetScalars(scalars);

  //----------------------------------
  // Normals
  //----------------------------------

  if (algorithm_.get(Parameters::UseNormals).toBool())
  {
    vmesh->synchronize(Mesh::NORMALS_E);

    auto normals = vtkSmartPointer<vtkFloatArray>::New();
    normals->SetName("Normals");
    normals->SetNumberOfComponents(3);

    Vector n;

    for (const auto& node : facade->nodes())
    {
      vmesh->get_normal(n, node.index());

      normals->InsertNextTuple3(static_cast<float>(n.x()), static_cast<float>(n.y()), static_cast<float>(n.z()));
    }

    poly->GetPointData()->SetNormals(normals);
  }

  //----------------------------------
  // Triangle faces
  //----------------------------------

  for (const auto& face : facade->faces())
  {
    auto nodes = face.nodeIndices();

    if (nodes.size() != 3) continue;

    vtkNew<vtkTriangle> tri;

    tri->GetPointIds()->SetId(0, static_cast<vtkIdType>(nodes[0]));
    tri->GetPointIds()->SetId(1, static_cast<vtkIdType>(nodes[1]));
    tri->GetPointIds()->SetId(2, static_cast<vtkIdType>(nodes[2]));

    polys->InsertNextCell(tri);
  }

  poly->SetPolys(polys);

  //----------------------------------
  // Material
  //----------------------------------

  obj->material.color[0] = static_cast<float>(algorithm_.get(Parameters::DefaultColorR).toDouble());

  obj->material.color[1] = static_cast<float>(algorithm_.get(Parameters::DefaultColorG).toDouble());

  obj->material.color[2] = static_cast<float>(algorithm_.get(Parameters::DefaultColorB).toDouble());

  obj->material.opacity = static_cast<float>(algorithm_.get(Parameters::DefaultColorA).toDouble());

  //----------------------------------
  // Transfer function
  //----------------------------------

  double range[2];
  scalars->GetRange(range);

  obj->tfn.range = {static_cast<float>(range[0]), static_cast<float>(range[1])};

  if (colorMap)
  {
    ColorMap_OSP_helper cmp(colorMap);

    obj->tfn.colors = cmp.colorList_;
    obj->tfn.opacities = cmp.opacityList_;
  }
  else
  {
    obj->tfn.colors = {obj->material.color[0], obj->material.color[1], obj->material.color[2]};

    obj->tfn.opacities = {obj->material.opacity};
  }

  obj->dataObject = poly;
#endif

  return obj;
}

VtkGeometryObjectHandle VtkGeometryBuilder::addQuadSurface(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = makeObject(field);

#ifdef WITH_VTK
  obj->type = GeometryType::QUAD_SURFACE;

  auto poly = vtkSmartPointer<vtkPolyData>::New();
  auto points = vtkSmartPointer<vtkPoints>::New();
  auto polys = vtkSmartPointer<vtkCellArray>::New();

  auto facade = field->mesh()->getFacade();
  auto vfield = field->vfield();
  auto vmesh = field->vmesh();

  //----------------------------------
  // Points
  //----------------------------------

  for (const auto& node : facade->nodes())
  {
    auto p = node.point();

    points->InsertNextPoint(p.x(), p.y(), p.z());
  }

  poly->SetPoints(points);

  //----------------------------------
  // Point scalars
  //----------------------------------

  auto scalars = vtkSmartPointer<vtkDoubleArray>::New();
  scalars->SetName("Values");

  double value = 0.0;

  for (const auto& node : facade->nodes())
  {
    if (vfield->num_values() > 0)
    {
      vfield->get_value(value, node.index());
    }
    else
    {
      value = 0.0;
    }

    scalars->InsertNextValue(value);
  }

  poly->GetPointData()->SetScalars(scalars);

  //----------------------------------
  // Normals
  //----------------------------------

  if (algorithm_.get(Parameters::UseNormals).toBool())
  {
    vmesh->synchronize(Mesh::NORMALS_E);

    auto normals = vtkSmartPointer<vtkFloatArray>::New();
    normals->SetName("Normals");
    normals->SetNumberOfComponents(3);

    Vector n;

    for (const auto& node : facade->nodes())
    {
      vmesh->get_normal(n, node.index());

      normals->InsertNextTuple3(static_cast<float>(n.x()), static_cast<float>(n.y()), static_cast<float>(n.z()));
    }

    poly->GetPointData()->SetNormals(normals);
  }

  //----------------------------------
  // Quad faces
  //----------------------------------

  for (const auto& face : facade->faces())
  {
    auto nodes = face.nodeIndices();

    if (nodes.size() != 4) continue;

    vtkNew<vtkQuad> quad;

    // Preserve the winding correction used by
    // the original implementation.

    quad->GetPointIds()->SetId(0, static_cast<vtkIdType>(nodes[3]));
    quad->GetPointIds()->SetId(1, static_cast<vtkIdType>(nodes[2]));
    quad->GetPointIds()->SetId(2, static_cast<vtkIdType>(nodes[1]));
    quad->GetPointIds()->SetId(3, static_cast<vtkIdType>(nodes[0]));

    polys->InsertNextCell(quad);
  }

  poly->SetPolys(polys);

  //----------------------------------
  // Material
  //----------------------------------

  obj->material.color[0] = static_cast<float>(algorithm_.get(Parameters::DefaultColorR).toDouble());

  obj->material.color[1] = static_cast<float>(algorithm_.get(Parameters::DefaultColorG).toDouble());

  obj->material.color[2] = static_cast<float>(algorithm_.get(Parameters::DefaultColorB).toDouble());

  obj->material.opacity = static_cast<float>(algorithm_.get(Parameters::DefaultColorA).toDouble());

  //----------------------------------
  // Transfer function
  //----------------------------------

  double range[2];
  scalars->GetRange(range);

  obj->tfn.range = {static_cast<float>(range[0]), static_cast<float>(range[1])};

  if (colorMap)
  {
    ColorMap_OSP_helper cmp(colorMap);

    obj->tfn.colors = cmp.colorList_;
    obj->tfn.opacities = cmp.opacityList_;
  }
  else
  {
    obj->tfn.colors = {obj->material.color[0], obj->material.color[1], obj->material.color[2]};

    obj->tfn.opacities = {obj->material.opacity};
  }

  obj->dataObject = poly;
#endif

  return obj;
}

VtkGeometryObjectHandle VtkGeometryBuilder::addStructVol(FieldHandle field, ColorMapHandle colorMap) const
{
  bool showFaces = algorithm_.get(Parameters::ShowFaces).toBool();
  bool showVolume = algorithm_.get(Parameters::ShowVolume).toBool();

  auto volumeObj = showVolume ? makeObject(field) : nullptr;
  auto meshObj = showFaces ? makeObject(field) : nullptr;

#ifdef WITH_VTK
  auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();

  auto facade = field->mesh()->getFacade();
  auto vfield = field->vfield();
  auto vmesh = field->vmesh();

  // volume
  if (showVolume)
  {
    auto image = vtkSmartPointer<vtkImageData>::New();

    const auto ni = vmesh->get_ni();
    const auto nj = vmesh->get_nj();
    const auto nk = vmesh->get_nk();
    auto bbox = vmesh->get_bounding_box();
    const auto originX = bbox.get_min().x();
    const auto originY = bbox.get_min().y();
    const auto originZ = bbox.get_min().z();
    const auto sizeX = bbox.get_max().x() - originX;
    const auto sizeY = bbox.get_max().y() - originY;
    const auto sizeZ = bbox.get_max().z() - originZ;
    const auto dx = sizeX / (ni > 1 ? ni - 1 : 1);
    const auto dy = sizeY / (nj > 1 ? nj - 1 : 1);
    const auto dz = sizeZ / (nk > 1 ? nk - 1 : 1);

    image->SetDimensions(ni, nj, nk);
    image->SetOrigin(originX, originY, originZ);
    image->SetSpacing(dx, dy, dz);

    auto imageScalars = vtkSmartPointer<vtkDoubleArray>::New();
    imageScalars->SetName("Values");
    imageScalars->SetNumberOfComponents(1);
    imageScalars->SetNumberOfTuples(ni * nj * nk);

    double value = 0.0;
    bool value_num = vfield->num_values() > 0;

    for (const auto& node : facade->nodes())
    {
      if (value_num)
        vfield->get_value(value, node.index());
      imageScalars->SetValue(node.index(), value);
    }

    image->GetPointData()->SetScalars(imageScalars);

    volumeObj->dataObject = image;
    volumeObj->material.color[0] = static_cast<float>(algorithm_.get(Parameters::DefaultColorR).toDouble());
    volumeObj->material.color[1] = static_cast<float>(algorithm_.get(Parameters::DefaultColorG).toDouble());
    volumeObj->material.color[2] = static_cast<float>(algorithm_.get(Parameters::DefaultColorB).toDouble());
    volumeObj->material.opacity = static_cast<float>(algorithm_.get(Parameters::DefaultColorA).toDouble());
    volumeObj->tfn.range = {static_cast<float>(imageScalars->GetRange()[0]), static_cast<float>(imageScalars->GetRange()[1])};
    if (colorMap)
    {
      ColorMap_OSP_helper cmp(colorMap);

      volumeObj->tfn.colors = cmp.colorList_;
      volumeObj->tfn.opacities = cmp.opacityList_;
    }
    else
    {
      volumeObj->tfn.colors = {volumeObj->material.color[0], volumeObj->material.color[1], volumeObj->material.color[2]};
      volumeObj->tfn.opacities = {volumeObj->material.opacity};
    }
    volumeObj->type = GeometryType::STRUCTURED_VOLUME;
  }

  if (showFaces)
  {
    //----------------------------------
    // Points
    //----------------------------------

    auto points = vtkSmartPointer<vtkPoints>::New();

    for (const auto& node : facade->nodes())
    {
      auto p = node.point();

      points->InsertNextPoint(p.x(), p.y(), p.z());
    }

    grid->SetPoints(points);

    //----------------------------------
    // Point scalars
    //----------------------------------

    auto scalars = vtkSmartPointer<vtkDoubleArray>::New();
    scalars->SetName("Values");

    double value = 0.0;
    bool value_num = vfield->num_values() > 0;
    std::vector<double> valueRange(2);
    valueRange[0] = std::numeric_limits<double>::max();
    valueRange[1] = std::numeric_limits<double>::lowest();

    for (const auto& node : facade->nodes())
    {
      if (value_num)
        vfield->get_value(value, node.index());
      scalars->InsertNextValue(value);
      valueRange[0] = std::min(valueRange[0], value);
      valueRange[1] = std::max(valueRange[1], value);
    }

    grid->GetPointData()->SetScalars(scalars);

    //----------------------------------
    // Hex cells
    //----------------------------------

    for (const auto& cell : facade->cells())
    {
      VMesh::Node::array_type nodes;
      vmesh->get_nodes(nodes, cell.index());

      if (nodes.size() != 8) continue;

      vtkNew<vtkHexahedron> hex;

      for (size_t i = 0; i < 8; ++i)
      {
        hex->GetPointIds()->SetId(static_cast<vtkIdType>(i), static_cast<vtkIdType>(nodes[i]));
      }

      grid->InsertNextCell(hex->GetCellType(), hex->GetPointIds());
    }

    meshObj->dataObject = grid;
    meshObj->material.color[0] = static_cast<float>(algorithm_.get(Parameters::DefaultColorR).toDouble());
    meshObj->material.color[1] = static_cast<float>(algorithm_.get(Parameters::DefaultColorG).toDouble());
    meshObj->material.color[2] = static_cast<float>(algorithm_.get(Parameters::DefaultColorB).toDouble());
    meshObj->material.opacity = static_cast<float>(algorithm_.get(Parameters::DefaultColorA).toDouble());
    meshObj->tfn.range = {static_cast<float>(valueRange[0]), static_cast<float>(valueRange[1])};
    if (colorMap)
    {
      ColorMap_OSP_helper cmp(colorMap);

      meshObj->tfn.colors = cmp.colorList_;
      meshObj->tfn.opacities = cmp.opacityList_;
    }
    else
    {
      meshObj->tfn.colors = {meshObj->material.color[0], meshObj->material.color[1], meshObj->material.color[2]};
      meshObj->tfn.opacities = {meshObj->material.opacity};
    }
    meshObj->type = GeometryType::STRUCTURED_VOLUME;
  }

  if (showVolume && !showFaces)
  {
    return volumeObj;
  }
  else if (!showVolume && showFaces)
  {
    return meshObj;
  }
  else if (showVolume && showFaces)
  {
    return std::make_shared<CompositeVtkGeometryObject>(std::vector<VtkGeometryObjectHandle>{volumeObj, meshObj});
  }
#endif

  return nullptr;
}

VtkGeometryObjectHandle VtkGeometryBuilder::addUnstructVol(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = makeObject(field);

#ifdef WITH_VTK
  obj->type = GeometryType::UNSTRUCTURED_VOLUME;

  auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();

  auto facade = field->mesh()->getFacade();
  auto vfield = field->vfield();
  auto vmesh = field->vmesh();

  //----------------------------------
  // Points
  //----------------------------------

  auto points = vtkSmartPointer<vtkPoints>::New();

  for (const auto& node : facade->nodes())
  {
    auto p = node.point();

    points->InsertNextPoint(p.x(), p.y(), p.z());
  }

  grid->SetPoints(points);

  //----------------------------------
  // Point scalars
  //----------------------------------

  auto scalars = vtkSmartPointer<vtkDoubleArray>::New();
  scalars->SetName("Values");

  double value = 0.0;

  for (const auto& node : facade->nodes())
  {
    if (vfield->num_values() > 0)
    {
      vfield->get_value(value, node.index());
    }
    else
    {
      value = 0.0;
    }

    scalars->InsertNextValue(value);
  }

  grid->GetPointData()->SetScalars(scalars);

  //----------------------------------
  // Cells
  //----------------------------------

  FieldInformation info(field);

  for (const auto& cell : facade->cells())
  {
    VMesh::Node::array_type nodes;
    vmesh->get_nodes(nodes, cell.index());

    if (info.is_tetvol())
    {
      if (nodes.size() < 4) continue;

      vtkNew<vtkTetra> tet;

      for (size_t i = 0; i < 4; ++i)
      {
        tet->GetPointIds()->SetId(static_cast<vtkIdType>(i), static_cast<vtkIdType>(nodes[i]));
      }

      grid->InsertNextCell(tet->GetCellType(), tet->GetPointIds());
    }
    else if (info.is_hexvol())
    {
      if (nodes.size() < 8) continue;

      vtkNew<vtkHexahedron> hex;

      for (size_t i = 0; i < 8; ++i)
      {
        hex->GetPointIds()->SetId(static_cast<vtkIdType>(i), static_cast<vtkIdType>(nodes[i]));
      }

      grid->InsertNextCell(hex->GetCellType(), hex->GetPointIds());
    }
  }

  //----------------------------------
  // Material
  //----------------------------------

  obj->material.color[0] = static_cast<float>(algorithm_.get(Parameters::DefaultColorR).toDouble());

  obj->material.color[1] = static_cast<float>(algorithm_.get(Parameters::DefaultColorG).toDouble());

  obj->material.color[2] = static_cast<float>(algorithm_.get(Parameters::DefaultColorB).toDouble());

  obj->material.opacity = static_cast<float>(algorithm_.get(Parameters::DefaultColorA).toDouble());

  //----------------------------------
  // Transfer function
  //----------------------------------

  double range[2];
  scalars->GetRange(range);

  obj->tfn.range = {static_cast<float>(range[0]), static_cast<float>(range[1])};

  if (colorMap)
  {
    ColorMap_OSP_helper cmp(colorMap);

    obj->tfn.colors = cmp.colorList_;
    obj->tfn.opacities = cmp.opacityList_;

    if (obj->tfn.opacities.empty())
    {
      obj->tfn.opacities.push_back(obj->material.opacity);
    }
  }
  else
  {
    obj->tfn.colors = {obj->material.color[0], obj->material.color[1], obj->material.color[2]};

    obj->tfn.opacities = {obj->material.opacity};
  }

  obj->dataObject = grid;
#endif

  return obj;
}

VtkGeometryObjectHandle VtkGeometryBuilder::addCylinder(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = makeObject(field);

#ifdef WITH_VTK
  obj->type = GeometryType::CYLINDER;
  obj->radius = static_cast<float>(algorithm_.get(Parameters::Radius).toDouble());

  auto poly = vtkSmartPointer<vtkPolyData>::New();
  auto points = vtkSmartPointer<vtkPoints>::New();
  auto lines = vtkSmartPointer<vtkCellArray>::New();

  auto facade = field->mesh()->getFacade();
  auto vfield = field->vfield();

  //----------------------------------
  // Points
  //----------------------------------

  for (const auto& node : facade->nodes())
  {
    auto p = node.point();

    points->InsertNextPoint(p.x(), p.y(), p.z());
  }

  poly->SetPoints(points);

  //----------------------------------
  // Point scalars
  //----------------------------------

  auto scalars = vtkSmartPointer<vtkDoubleArray>::New();
  scalars->SetName("Values");

  double value = 0.0;

  for (const auto& node : facade->nodes())
  {
    if (vfield->num_values() > 0)
    {
      vfield->get_value(value, node.index());
    }
    else
    {
      value = 0.0;
    }

    scalars->InsertNextValue(value);
  }

  poly->GetPointData()->SetScalars(scalars);

  //----------------------------------
  // Line cells from mesh edges
  //----------------------------------

  for (const auto& edge : facade->edges())
  {
    auto nodes = edge.nodeIndices();

    if (nodes.size() != 2) continue;

    vtkNew<vtkLine> line;

    line->GetPointIds()->SetId(0, static_cast<vtkIdType>(nodes[0]));

    line->GetPointIds()->SetId(1, static_cast<vtkIdType>(nodes[1]));

    lines->InsertNextCell(line);
  }

  poly->SetLines(lines);

  //----------------------------------
  // Material
  //----------------------------------

  obj->material.color[0] = static_cast<float>(algorithm_.get(Parameters::DefaultColorR).toDouble());

  obj->material.color[1] = static_cast<float>(algorithm_.get(Parameters::DefaultColorG).toDouble());

  obj->material.color[2] = static_cast<float>(algorithm_.get(Parameters::DefaultColorB).toDouble());

  obj->material.opacity = static_cast<float>(algorithm_.get(Parameters::DefaultColorA).toDouble());

  //----------------------------------
  // Transfer function
  //----------------------------------

  double range[2];
  scalars->GetRange(range);

  obj->tfn.range = {static_cast<float>(range[0]), static_cast<float>(range[1])};

  if (colorMap)
  {
    ColorMap_OSP_helper cmp(colorMap);

    obj->tfn.colors = cmp.colorList_;
    obj->tfn.opacities = cmp.opacityList_;
  }
  else
  {
    obj->tfn.colors = {obj->material.color[0], obj->material.color[1], obj->material.color[2]};

    obj->tfn.opacities = {obj->material.opacity};
  }

  obj->dataObject = poly;
#endif

  return obj;
}

VtkGeometryObjectHandle VtkGeometryBuilder::addSphere(FieldHandle field, ColorMapHandle colorMap) const
{
  auto obj = makeObject(field);

#ifdef WITH_VTK
  obj->type = GeometryType::SPHERE;
  obj->radius = static_cast<float>(algorithm_.get(Parameters::Radius).toDouble());

  auto poly = vtkSmartPointer<vtkPolyData>::New();
  auto points = vtkSmartPointer<vtkPoints>::New();

  auto facade = field->mesh()->getFacade();
  auto vfield = field->vfield();

  //----------------------------------
  // Points (sphere centers)
  //----------------------------------

  for (const auto& node : facade->nodes())
  {
    auto p = node.point();

    points->InsertNextPoint(p.x(), p.y(), p.z());
  }

  poly->SetPoints(points);

  //----------------------------------
  // Point scalars
  //----------------------------------

  auto scalars = vtkSmartPointer<vtkDoubleArray>::New();
  scalars->SetName("Values");

  double value = 0.0;

  for (const auto& node : facade->nodes())
  {
    if (vfield->num_values() > 0)
    {
      vfield->get_value(value, node.index());
    }
    else
    {
      value = 0.0;
    }

    scalars->InsertNextValue(value);
  }

  poly->GetPointData()->SetScalars(scalars);

  //----------------------------------
  // Material
  //----------------------------------

  obj->material.color[0] = static_cast<float>(algorithm_.get(Parameters::DefaultColorR).toDouble());

  obj->material.color[1] = static_cast<float>(algorithm_.get(Parameters::DefaultColorG).toDouble());

  obj->material.color[2] = static_cast<float>(algorithm_.get(Parameters::DefaultColorB).toDouble());

  obj->material.opacity = static_cast<float>(algorithm_.get(Parameters::DefaultColorA).toDouble());

  //----------------------------------
  // Transfer function
  //----------------------------------

  double range[2];
  scalars->GetRange(range);

  obj->tfn.range = {static_cast<float>(range[0]), static_cast<float>(range[1])};

  if (colorMap)
  {
    ColorMap_OSP_helper cmp(colorMap);

    obj->tfn.colors = cmp.colorList_;
    obj->tfn.opacities = cmp.opacityList_;
  }
  else
  {
    obj->tfn.colors = {obj->material.color[0], obj->material.color[1], obj->material.color[2]};

    obj->tfn.opacities = {obj->material.opacity};
  }

  obj->dataObject = poly;
#endif

  return obj;
}

VtkGeometryObjectHandle VtkGeometryBuilder::makeObject(FieldHandle field) const
{
  VtkGeometryObjectHandle obj(new VtkGeometryObject);
  auto vmesh = field->vmesh();
  auto bbox = vmesh->get_bounding_box();
  obj->box = bbox;
  return obj;
}
