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

   Author:          Martin Cole
   Date:            Jun 15 2001
*/


#include <Core/Algorithms/Legacy/Fields/MarchingCubes/TetMC.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 #include <sci_hash_map.h>
#endif

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;

void
TetMC::reset( int /*n*/, bool build_field, bool build_geom, bool transparency )
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
      node_map_ = std::vector<SCIRun::index_type>(nnodes_, -1);
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
  if (build_field_)
  {
    FieldInformation fi("TriSurfMesh",basis_order_,"double");
    trisurf_handle_ = CreateField(fi);
    trisurf_ = trisurf_handle_->vmesh();
  }
}


VMesh::Node::index_type
TetMC::find_or_add_edgepoint(index_type u0,
                             index_type u1,
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
    const VMesh::Node::index_type nodeindex = trisurf_->add_point(p);
    edge_map_[np] = nodeindex;
    return nodeindex;
  }
  else
  {
    return ((*loc).second);
  }
}


VMesh::Node::index_type
TetMC::find_or_add_nodepoint(VMesh::Node::index_type &tet_node_idx)
{
  VMesh::Node::index_type surf_node_idx;
  index_type i = node_map_[tet_node_idx];
  if (i != -1) surf_node_idx = (VMesh::Node::index_type) i;
  else
  {
    Point p;
    mesh_->get_point(p, tet_node_idx);
    surf_node_idx = trisurf_->add_point(p);
    node_map_[tet_node_idx] = surf_node_idx;
  }
  return (surf_node_idx);
}


void
TetMC::find_or_add_parent(index_type u0, index_type u1,
                          double d0, index_type face)
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
    ASSERT(loc == edge_map_.end())
  }
}


void
TetMC::extract( VMesh::Elem::index_type cell, double v )
{
  if (basis_order_ == 0)
    extract_c(cell, v);
  else
    extract_n(cell, v);
}


void
TetMC::extract_c( VMesh::Elem::index_type cell, double iso )
{
  double selfvalue, nbrvalue;
  field_->value( selfvalue, cell );
  VMesh::DElem::array_type faces;
  mesh_->get_delems(faces, cell);

  VMesh::Elem::index_type nbr_cell;
  Point p[3];
  VMesh::Node::array_type nodes;
  VMesh::Node::array_type vertices(3);

  for (size_t i=0; i<faces.size(); i++)
  {
    if (mesh_->get_neighbor(nbr_cell, cell, faces[i]) &&
        field_->value(nbrvalue, nbr_cell) &&
        selfvalue <= iso && iso < nbrvalue)
    {
      mesh_->get_nodes(nodes, faces[i]);
      mesh_->get_centers(p,nodes);

     #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      if (build_geom_)
      {
        triangles_->add(p[0], p[1], p[2]);
      }
     #endif

      if (build_field_)
      {
        for (int j=0; j<3; j++)
        {
          vertices[j] = find_or_add_nodepoint(nodes[j]);
        }

        VMesh::Elem::index_type tface = trisurf_->add_elem(vertices);

        const double d = (selfvalue - iso) / (selfvalue - nbrvalue);

        find_or_add_parent(cell, nbr_cell, d, tface);
      }
    }
  }
}

void TetMC::extract_n( VMesh::Elem::index_type cell, double v )
{
  static int num[16] = { 0, 1, 1, 2, 1, 2, 2, 1, 1, 2, 2, 1, 2, 1, 1, 0 };
  static int order[16][4] = {
    {0, 0, 0, 0},   /* none - ignore */
    {3, 2, 0, 1},   /* 3 */
    {2, 1, 0, 3},   /* 2 */
    {2, 1, 0, 3},   /* 2, 3 */
    {1, 3, 0, 2},   /* 1 */
    {1, 0, 2, 3},   /* 1, 3 */
    {1, 3, 0, 2},   /* 1, 2 */
    {0, 2, 3, 1},   /* 1, 2, 3 */
    {0, 2, 1, 3},   /* 0 */
    {2, 3, 0, 1},   /* 0, 3 - reverse of 1, 2 */
    {3, 0, 2, 1},   /* 0, 2 - reverse of 1, 3 */
    {1, 0, 3, 2},   /* 0, 2, 3 - reverse of 1 */
    {3, 1, 0, 2},   /* 0, 1 - reverse of 2, 3 */
    {2, 3, 0, 1},   /* 0, 1, 3 - reverse of 2 */
    {3, 2, 1, 0},   /* 0, 1, 2 - reverse of 3 */
    {0, 0, 0, 0}    /* all - ignore */
  };

  VMesh::Node::array_type nodes(3);
  VMesh::Node::array_type node;
  Point p[4];
  double value[4];

  mesh_->get_nodes( node, cell );
  mesh_->get_centers(p,node);
  field_->get_values(value,node);

  int code = 0;
  for (int i=0; i<4; i++)
  {
    code = code*2+(value[i] > v );
  }

  switch ( num[code] )
  {
  case 1:
    {
      // make a single triangle
      int o = order[code][0];
      int i = order[code][1];
      int j = order[code][2];
      int k = order[code][3];

      const double v1 = (v-value[o])/double(value[i]-value[o]);
      const double v2 = (v-value[o])/double(value[j]-value[o]);
      const double v3 = (v-value[o])/double(value[k]-value[o]);
      const Point p1(Interpolate( p[o],p[i], v1));
      const Point p2(Interpolate( p[o],p[j], v2));
      const Point p3(Interpolate( p[o],p[k], v3));

     #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      if (build_geom_)
      {
        triangles_->add( p1, p2, p3 );
      }
     #endif

      if (build_field_)
      {
        VMesh::Node::index_type i1, i2, i3;
        nodes[0] = find_or_add_edgepoint(node[o], node[i], v1, p1);
        nodes[1] = find_or_add_edgepoint(node[o], node[j], v2, p2);
        nodes[2] = find_or_add_edgepoint(node[o], node[k], v3, p3);
        if (nodes[0] != nodes[1] && nodes[1] != nodes[2] && nodes[2] != nodes[0])
        {
          trisurf_->add_elem(nodes);
          cell_map_.push_back( cell );
        }
      }
    }
    break;
  case 2:
    {
      // make order triangles
      const int o = order[code][0];
      const int i = order[code][1];
      const int j = order[code][2];
      const int k = order[code][3];
      const double v1 = (v-value[o])/double(value[i]-value[o]);
      const double v2 = (v-value[o])/double(value[j]-value[o]);
      const double v3 = (v-value[k])/double(value[j]-value[k]);
      const double v4 = (v-value[k])/double(value[i]-value[k]);
      const Point p1(Interpolate( p[o],p[i], v1));
      const Point p2(Interpolate( p[o],p[j], v2));
      const Point p3(Interpolate( p[k],p[j], v3));
      const Point p4(Interpolate( p[k],p[i], v4));

     #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      if (build_geom_)
      {
        triangles_->add( p1, p2, p3 );
        triangles_->add( p1, p3, p4 );
      }
     #endif

      if (build_field_)
      {
        VMesh::Node::index_type i1, i2, i3, i4;
        i1 = find_or_add_edgepoint(node[o], node[i], v1, p1);
        i2 = find_or_add_edgepoint(node[o], node[j], v2, p2);
        i3 = find_or_add_edgepoint(node[k], node[j], v3, p3);
        i4 = find_or_add_edgepoint(node[k], node[i], v4, p4);
        if (i1 != i2 && i2 != i3 && i3 != i1)
        {
          nodes[0] = i1; nodes[1] = i2; nodes[2] = i3;
          trisurf_->add_elem(nodes);
          cell_map_.push_back( cell );
        }
        if (i1 != i3 && i3 != i4 && i4 != i1)
        {
          nodes[0] = i1; nodes[1] = i3; nodes[2] = i4;
          trisurf_->add_elem(nodes);
          cell_map_.push_back( cell );
        }
      }
    }
    break;
  default:
    // do nothing.
    // MarchingCubes calls extract on each and every cell. i.e., this is
    // not an error
    break;
  }
}

FieldHandle
TetMC::get_field(double value)
{
  trisurf_handle_->vfield()->resize_values();
  trisurf_handle_->vfield()->set_all_values(value);

  return (trisurf_handle_);
}
