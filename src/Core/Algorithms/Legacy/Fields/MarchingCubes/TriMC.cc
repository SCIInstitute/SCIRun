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

   Author:            Michael Callahan
   Date:              Sept 2002
*/


#include <Core/Algorithms/Legacy/Fields/MarchingCubes/TriMC.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 #include <sci_hash_map.h>
#endif

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;

void  TriMC::reset( int /*n*/, bool build_field, bool build_geom, bool transparency )
{
  build_field_ = build_field;
  build_geom_  = build_geom;
  basis_order_ = field_->basis_order();

  edge_map_.clear();
  VMesh::Node::size_type nsize;
  mesh_->size(nsize);
  nnodes_ = nsize;

  cell_map_.clear();
  VMesh::Face::size_type csize;
  mesh_->size(csize);
  ncells_ = csize;

  if (basis_order_ == 0)
  {
    mesh_->synchronize(Mesh::EDGES_E|Mesh::ELEM_NEIGHBORS_E);
    if (build_field_)
    {
      node_map_ = std::vector<SCIRun::index_type>(nsize, -1);
    }
  }

  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  lines_ = 0;
  if (build_geom_)
  {
    if( transparency )
      lines_ = new GeomTranspLines;
    else
      lines_ = new GeomLines;
  }
  geomHandle_ = lines_;
  #endif

  curve_ = 0;
  if (build_field_)
  {
    FieldInformation fi("CurveMesh",basis_order_,"double");
    curve_handle_ = CreateField(fi);
    curve_ = curve_handle_->vmesh();
  }
}

VMesh::Node::index_type TriMC::find_or_add_nodepoint(VMesh::Node::index_type &tri_node_idx)
{
  VMesh::Node::index_type curve_node_idx;
  SCIRun::index_type i = node_map_[tri_node_idx];
  if (i != -1) curve_node_idx = VMesh::Node::index_type(i);
  else
  {
    Point p;
    mesh_->get_point(p, tri_node_idx);
    curve_node_idx = curve_->add_point(p);
    node_map_[tri_node_idx] = curve_node_idx;
  }
  return (curve_node_idx);
}

void TriMC::extract( VMesh::Elem::index_type cell, double v )
{
  if (basis_order_ == 0)
    extract_f(cell, v);
  else
    extract_n(cell, v);
}

void TriMC::find_or_add_parent(index_type u0, index_type u1, double d0, index_type edge)
{
  if (d0 < 0.0) { u1 = -1; }
  if (d0 > 1.0) { u0 = -1; }

  edgepair_t np;

  if (u0 < u1)  { np.first = u0; np.second = u1; np.dfirst = d0; }
  else { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }
  const edge_hash_type::iterator loc = edge_map_.find(np);
  if (loc == edge_map_.end())
  {
    edge_map_[np] = edge;
  }
  else
  {
    // This should never happen
  }
}

void TriMC::extract_f( VMesh::Elem::index_type cell, double iso)
{
  double selfvalue, nbrvalue;
  field_->get_value( selfvalue, cell );

  VMesh::DElem::array_type edges;
  mesh_->get_delems(edges, cell);

  VMesh::Elem::index_type nbr_cell;
  Point p[2];

  VMesh::Node::array_type nodes;
  VMesh::Node::array_type vertices(2);

  for (size_t i=0; i<edges.size(); i++)
  {
    if (mesh_->get_neighbor(nbr_cell, cell, edges[i]) &&
        field_->value(nbrvalue, nbr_cell) &&
        selfvalue <= iso && iso < nbrvalue)
    {
      mesh_->get_nodes(nodes, edges[i]);

      for (int j=0; j<2; j++)
      {
        mesh_->get_center(p[j], nodes[j]);
      }

      #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      if (build_geom_)
      {
        lines_->add(p[0], p[1]);
      }
      #endif

      if (build_field_)
      {
        for (int j=0; j<2; j ++)
        {
          vertices[j] = find_or_add_nodepoint(nodes[j]);
        }

        VMesh::Elem::index_type cedge = curve_->add_elem(vertices);

        const double d = (selfvalue - iso) / (selfvalue - nbrvalue);

        find_or_add_parent(cell, nbr_cell, d, cedge);
      }
    }
  }
}

VMesh::Node::index_type
TriMC::find_or_add_edgepoint(index_type u0, index_type u1,
                             double d0, const Point &p)
{
  if (d0 < 0.0) { u1 = -1; }
  if (d0 > 1.0) { u0 = -1; }

  edgepair_t np;

  if (u0 < u1)  { np.first = u0; np.second = u1; np.dfirst = d0; }
  else { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }

  const edge_hash_type::iterator loc = edge_map_.find(np);

  if (loc == edge_map_.end())
  {
    const VMesh::Node::index_type nodeindex = curve_->add_point(p);
    edge_map_[np] = nodeindex;
    return nodeindex;
  }
  else
  {
    return ((*loc).second);
  }
}


void TriMC::extract_n( VMesh::Elem::index_type cell, double v )
{
  VMesh::Node::array_type node;
  Point p[3];
  double value[3];

  mesh_->get_nodes( node, cell );

  static int num[8] = { 0, 1, 1, 1, 1, 1, 1, 0 };
  static int clip[8] = { 0, 0, 1, 2, 2, 1, 0, 0 };

  int code = 0;
  mesh_->get_centers(p,node);
  field_->get_values(value,node);

  for (int i=0; i<3; i++)
  {
    code |= (value[i] > v ) << i;
  }

  if (num[code])
  {
    const int a = clip[code];
    const int b = (a + 1) % 3;
    const int c = (a + 2) % 3;

    const double d0 = (v-value[a])/(value[b]-value[a]);
    const double d1 = (v-value[a])/(value[c]-value[a]);

    const Point p0(Interpolate(p[a], p[b], d0));
    const Point p1(Interpolate(p[a], p[c], d1));

    #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (build_geom_)
    {
      lines_->add( p0, p1 );
    }
    #endif

    if (build_field_)
    {
      VMesh::Node::array_type cnode(2);
      cnode[0] = find_or_add_edgepoint(node[a], node[b], d0, p0);
      cnode[1] = find_or_add_edgepoint(node[a], node[c], d1, p1);
      if (cnode[0] != cnode[1])
        curve_->add_elem(cnode);
    }
  }
}

FieldHandle TriMC::get_field(double value)
{
  curve_handle_->vfield()->resize_values();
  curve_handle_->vfield()->set_all_values(value);

  return (curve_handle_);
}
