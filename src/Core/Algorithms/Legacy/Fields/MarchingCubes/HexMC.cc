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

   Author:              Martin Cole
   Date:                June 15 2001
*/


#include <Core/Algorithms/Legacy/Fields/MarchingCubes/HexMC.h>

#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Legacy/Fields/MarchingCubes/mcube2.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 #include <sci_hash_map.h>

 #include <teem/air.h>
#endif

#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;

void  HexMC::reset( int /*n*/, bool build_field, bool build_geom, bool transparency )
{
  build_field_ = build_field;
  build_geom_  = build_geom;
  basis_order_ = field_->basis_order();

  edge_map_.clear();
  VMesh::Node::size_type nsize;
  mesh_->size(nsize);
  nnodes_ = nsize;

  cell_map_.clear();
  VMesh::Cell::size_type csize;
  mesh_->size(csize);
  ncells_ = csize;

  if (basis_order_ == 0)
  {
    mesh_->synchronize(Mesh::FACES_E|Mesh::ELEM_NEIGHBORS_E);
    if (build_field_)
    {
      node_map_ = std::vector<index_type>(nnodes_, -1);
    }
  }

 #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  triangles_ = 0;
  if (build_geom_)
  {
    if( transparency )
      triangles_ = new GeomTranspTriangles;
    else
      triangles_ = new GeomFastTriangles;
  }
  geomHandle_ = triangles_;
 #endif

  trisurf_ = 0;
  quadsurf_ = 0;
  if (build_field_)
  {
    if (basis_order_ == 0)
    {
      FieldInformation fi("QuadSurfMesh",basis_order_,"double");
      quadsurf_handle_ = CreateField(fi);
      quadsurf_ = quadsurf_handle_->vmesh();
    }
    else
    {
      FieldInformation fi("TriSurfMesh",basis_order_,"double");
      trisurf_handle_ = CreateField(fi);
      trisurf_ = trisurf_handle_->vmesh();
    }
  }
}

VMesh::Node::index_type HexMC::find_or_add_nodepoint(VMesh::Node::index_type& tet_node_idx)
{
  VMesh::Node::index_type surf_node_idx;
  index_type i = node_map_[tet_node_idx];

  if (i != -1) surf_node_idx = VMesh::Node::index_type(i);
  else
  {
    Point p;
    mesh_->get_point(p, tet_node_idx);
    surf_node_idx = quadsurf_->add_point(p);
    node_map_[tet_node_idx] = surf_node_idx;
  }

  return (surf_node_idx);
}

void HexMC::find_or_add_parent(index_type u0, index_type u1, double d0, index_type face)
{
  if (d0 < 0.0) { u1 = -1; }
  if (d0 > 1.0) { u0 = -1; }
  edgepair_t np;

  if (u0 < u1)  { np.first = u0; np.second = u1; np.dfirst = d0; }
  else { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }
  const edge_hash_type::iterator loc = edge_map_.find(np);

  if (loc == edge_map_.end())
  {
    edge_map_[np] = face;
  }
  else
  {
    // This should never happen
  }
}

void HexMC::extract( VMesh::Elem::index_type cell, double iso )
{
  if (basis_order_ == 0)
    extract_c(cell, iso);
  else
    extract_n(cell, iso);
}

void HexMC::extract_c( VMesh::Elem::index_type cell, double iso )
{
  double selfvalue, nbrvalue;
  field_->value( selfvalue, cell );

 #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (!(airExists(selfvalue))) return;
 #endif
  if (IsNan(selfvalue)) return;

  VMesh::DElem::array_type faces;
  mesh_->get_delems(faces, cell);

  VMesh::Elem::index_type nbr_cell;
  Point p[4];
  VMesh::Node::array_type face_nodes;
  VMesh::Node::array_type vertices(4);

  for (size_t i=0; i<faces.size(); i++)
  {
    if (mesh_->get_neighbor(nbr_cell, cell, faces[i]) &&
        field_->value(nbrvalue, nbr_cell) &&
	#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
         airExists(nbrvalue) &&
	#endif
	!IsNan(selfvalue) &&
        selfvalue <= iso && iso < nbrvalue)
    {
      mesh_->get_nodes(face_nodes, faces[i]);

      mesh_->get_centers(p,face_nodes);
     #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      if (build_geom_)
      {
        triangles_->add(p[0], p[1], p[2]);
        triangles_->add(p[2], p[3], p[0]);
      }
     #endif
      if (build_field_)
      {
        for (int j=0; j<4; j++)
        {
          vertices[j] = find_or_add_nodepoint(face_nodes[j]);
        }

        VMesh::Elem::index_type qface = quadsurf_->add_elem(vertices);
        const double d = (selfvalue - iso) / (selfvalue - nbrvalue);
        find_or_add_parent(cell, nbr_cell, d, qface);
      }
    }
  }
}

void  HexMC::extract_n( VMesh::Elem::index_type cell, double iso )
{
  VMesh::Node::array_type node(8);
  Point p[8];
  double value[8];
  int code = 0;

  mesh_->get_nodes( node, cell );
  mesh_->get_centers(p,node);
  field_->get_values(value,node);

  for (int i=7; i>=0; i--)
  {
    // skip anything with a NaN
    #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
     if (!airExists(value[i])) return;
    #endif
    if (IsNan(value[i])) return;
    code = code*2+(value[i] < iso );
  }

  if ( code == 0 || code == 255 )
    return;

  TRIANGLE_CASES *tcase= &triCases[code];
  int *vertex = tcase->edges;

  Point q[12];
  VMesh::Node::index_type surf_node[12];

  // interpolate and project vertices
  index_type v = 0;
  bool visited[12];
  for (int i=0;i<12;i++) visited[i] = false;

  while (vertex[v] != -1)
  {
    index_type i = vertex[v++];
    if (visited[i]) continue;
    visited[i]=true;
    index_type v1 = edge_tab[i][0];
    index_type v2 = edge_tab[i][1];
    const double d = (value[v1]-iso) / double(value[v1]-value[v2]);
    q[i] = Interpolate(p[v1], p[v2], d);
    if (build_field_)
    {
      surf_node[i] = find_or_add_edgepoint(node[v1], node[v2], d, q[i]);
    }
  }

  v = 0;
  while(vertex[v] != -1)
  {
    index_type v0 = vertex[v++];
    index_type v1 = vertex[v++];
    index_type v2 = vertex[v++];

   #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (build_geom_)
    {
      triangles_->add(q[v0], q[v1], q[v2]);
    }
   #endif

    if (build_field_)
    {
      if (surf_node[v0] != surf_node[v1] &&
          surf_node[v1] != surf_node[v2] &&
          surf_node[v2] != surf_node[v0])
      {
        VMesh::Node::array_type nodes(3);
        nodes[0] = surf_node[v0];
        nodes[1] = surf_node[v1];
        nodes[2] = surf_node[v2];
        trisurf_->add_elem(nodes);
        cell_map_.push_back( cell );
      }
    }
  }
}

FieldHandle HexMC::get_field(double value)
{
  if (basis_order_ == 0)
  {
    quadsurf_handle_->vfield()->resize_values();
    quadsurf_handle_->vfield()->set_all_values(value);
    return (quadsurf_handle_);
  }
  else
  {
    trisurf_handle_->vfield()->resize_values();
    trisurf_handle_->vfield()->set_all_values(value);
    return (trisurf_handle_);
  }
}

VMesh::Node::index_type HexMC::find_or_add_edgepoint(index_type u0, index_type u1, double d0, const Point &p)
{
  if (d0 < 0.0) { u1 = -1; }
  if (d0 > 1.0) { u0 = -1; }
  edgepair_t np;

  if (u0 < u1)  { np.first = u0; np.second = u1; np.dfirst = d0; }
  else { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }
  const edge_hash_type::iterator loc = edge_map_.find(np);

  if (loc == edge_map_.end())
  {
    const VMesh::Node::index_type nodeindex = trisurf_->add_point(p);
    edge_map_[np] = nodeindex;
    return (nodeindex);
  }
  else
  {
    return ((*loc).second);
  }
}
