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

   Author:              Michael Callahan
   Date:                Sept 2002
*/


#include <Core/Algorithms/Legacy/Fields/MarchingCubes/EdgeMC.h>

#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 #include <sci_hash_map.h>
#endif

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;

void EdgeMC::reset( int /*n*/, bool build_field,bool build_geom,bool transparency )
{
  build_field_ = build_field;
  build_geom_ = build_geom;
  basis_order_ = field_->basis_order();

  edge_map_.clear();
  VMesh::Node::size_type nsize;
  mesh_->size(nsize);
  nnodes_ = nsize;

  cell_map_.clear();
  VMesh::Elem::size_type csize;
  mesh_->size(csize);
  ncells_ = csize;

  if (basis_order_ == 0)
  {
    mesh_->synchronize(Mesh::EDGES_E|Mesh::ELEM_NEIGHBORS_E);
    if (build_field_)
    {
      node_map_ = std::vector<index_type>(nsize, -1);
    }
  }

 #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  points_ = 0;
  if (build_geom_)
  {
    if( transparency )
      points_ = new GeomTranspPoints;
    else
      points_ = new GeomPoints;
  }
  geomHandle_ = points_;
 #endif

  pointcloud_ = 0;
  if (build_field_)
  {
    FieldInformation fi("PointCloudMesh",0,"double");
    pointcloud_handle_ = CreateField(fi);
    pointcloud_ = pointcloud_handle_->vmesh();
  }
}

void EdgeMC::extract_e( VMesh::Elem::index_type edge, double iso )
{
  double selfvalue, nbrvalue;
  field_->get_value( selfvalue, edge );
  VMesh::Node::array_type nodes;
  mesh_->get_nodes(nodes, edge);

  VMesh::Elem::index_type nbr_edge;
  Point p0;
  VMesh::Node::array_type vertices(1);

  for (size_t i=0; i<nodes.size(); i++)
  {
    if (mesh_->get_neighbor(nbr_edge, edge, VMesh::DElem::index_type(nodes[i])) &&
        field_->value(nbrvalue, nbr_edge) &&
        selfvalue <= iso && iso < nbrvalue)
    {
      mesh_->get_center(p0, nodes[i]);

     #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      if (build_geom_)
      {
        points_->add(p0);
      }
     #endif

      if (build_field_)
      {
        vertices[0] = find_or_add_nodepoint(nodes[i]);

        VMesh::Elem::index_type pcpoint = pointcloud_->add_elem(vertices);

        const double d = (selfvalue - iso) / (selfvalue - nbrvalue);

        find_or_add_parent(edge, nbr_edge, d, pcpoint);
      }
    }
  }
}

void EdgeMC::extract_n( VMesh::Elem::index_type edge, double v )
{
  VMesh::Node::array_type node;
  Point p[2];
  double value[2];

  mesh_->get_nodes( node, edge );

  static int num[4] = { 0, 1, 1, 0 };

  int code = 0;
  mesh_->get_centers(p,node);
  field_->get_values(value,node);

  for (int i=0; i<2; i++)
  {
    code |= (value[i] > v ) << i;
  }

  //  if ( show_case != -1 && (code != show_case) ) return;
  if (num[code])
  {
    const double d0 = (v-value[0])/double(value[1]-value[0]);
    const Point p0(Interpolate(p[0], p[1], d0));

   #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (build_geom_)
      points_->add( p0 );
   #endif

    if (build_field_)
    {
      VMesh::Node::array_type cnode(1);
      cnode[0] = find_or_add_edgepoint(node[0], node[1], d0, p0);

      pointcloud_->add_elem(cnode);
      cell_map_.push_back( edge );
    }
  }
}

FieldHandle EdgeMC::get_field(double value)
{
  pointcloud_handle_->vfield()->resize_values();
  pointcloud_handle_->vfield()->set_all_values(value);

  return (pointcloud_handle_);
}

VMesh::Node::index_type EdgeMC::find_or_add_edgepoint(index_type u0, index_type u1, double d0, const Point &p)
{
  if (d0 < 0.0) { u1 = -1; }
  if (d0 > 1.0) { u0 = -1; }
  edgepair_t np;

  if (u0 < u1)  { np.first = u0; np.second = u1; np.dfirst = d0; }
  else { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }

  const edge_hash_type::iterator loc = edge_map_.find(np);

  if (loc == edge_map_.end())
  {
    const VMesh::Node::index_type nodeindex = pointcloud_->add_point(p);
    edge_map_[np] = nodeindex;
    return (nodeindex);
  }
  else
  {
    return ((*loc).second);
  }
}

VMesh::Node::index_type EdgeMC::find_or_add_nodepoint(VMesh::Node::index_type &curve_node_idx)
{
  VMesh::Node::index_type point_node_idx;
  index_type i = node_map_[curve_node_idx];
  if (i != -1) point_node_idx = (VMesh::Node::index_type) i;
  else
  {
    Point p;
    mesh_->get_center(p, curve_node_idx);
    point_node_idx = pointcloud_->add_point(p);
    node_map_[curve_node_idx] = point_node_idx;
  }
  return (curve_node_idx);
}

void EdgeMC::find_or_add_parent(index_type u0, index_type u1, double d0, index_type point)
{
  if (d0 < 0.0) { u1 = -1; }
  if (d0 > 1.0) { u0 = -1; }
  edgepair_t np;

  if (u0 < u1)  { np.first = u0; np.second = u1; np.dfirst = d0; }
  else { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }
  const edge_hash_type::iterator loc = edge_map_.find(np);

  if (loc == edge_map_.end())
  {
    edge_map_[np] = point;
  }
  else
  {
    // This should never happen
  }
}

void EdgeMC::extract( VMesh::Elem::index_type edge, double v )
{
  if (basis_order_ == 0)
    extract_e(edge, v);
  else
    extract_n(edge, v);
}
