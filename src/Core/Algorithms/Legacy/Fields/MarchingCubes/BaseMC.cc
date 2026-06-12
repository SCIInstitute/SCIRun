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


#include <Core/Algorithms/Legacy/Fields/MarchingCubes/BaseMC.h>
#include <Core/Algorithms/Legacy/Fields/MarchingCubes/mcube2.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

VMesh::Node::index_type BaseMC::find_or_add_edgepoint(
    index_type u0, index_type u1, double d0, const Point& p)
{
  if (d0 < 0.0) { u1 = -1; }
  if (d0 > 1.0) { u0 = -1; }
  edgepair_t np;
  if (u0 < u1) { np.first = u0; np.second = u1; np.dfirst = d0; }
  else         { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }
  const edge_hash_type::iterator loc = edge_map_.find(np);
  if (loc == edge_map_.end())
  {
    const VMesh::Node::index_type nodeindex = trisurf_->add_point(p);
    edge_map_[np] = nodeindex;
    return nodeindex;
  }
  else
  {
    return (*loc).second;
  }
}

void BaseMC::interpolateAndBuildTriangles(
    int* vertex, const int (*edge_table)[2],
    const Point* p, const double* value,
    const VMesh::Node::index_type* node, double iso,
    VMesh::Elem::index_type cell)
{
  Point q[12];
  VMesh::Node::index_type surf_node[12];
  index_type v = 0;
  bool visited[12] = {};

  while (vertex[v] != -1)
  {
    const index_type i = vertex[v++];
    if (visited[i]) continue;
    visited[i] = true;
    const index_type v1 = edge_table[i][0];
    const index_type v2 = edge_table[i][1];
    const double d = (value[v1] - iso) / double(value[v1] - value[v2]);
    q[i] = Interpolate(p[v1], p[v2], d);
    if (build_field_)
    {
      surf_node[i] = find_or_add_edgepoint(node[v1], node[v2], d, q[i]);
    }
  }

  v = 0;
  while (vertex[v] != -1)
  {
    const index_type v0 = vertex[v++];
    const index_type v1 = vertex[v++];
    const index_type v2 = vertex[v++];
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
        cell_map_.push_back(cell);
      }
    }
  }
}

MatrixHandle BaseMC::get_interpolant()
{
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   if( !build_field_ ) return (0);

    // The columns represent the source nodes while the rows
    // represent the destination nodes
    const size_type nrows = static_cast<size_type>(edge_map_.size());
    const size_type ncols = nnodes_;

    //  Yale Sparse Row Matrix format.
    SparseRowMatrix::Data sparseData(nrows+1, nrows*2);
    const SparseRowMatrix::Rows& rr = sparseData.rows();
    const SparseRowMatrix::Columns& cc = sparseData.columns();
    const SparseRowMatrix::Storage& dd = sparseData.data();

    edge_hash_type::iterator eiter = edge_map_.begin();
    while (eiter != edge_map_.end())
    {
      const SCIRun::index_type ei = (*eiter).second;

      cc[ei * 2 + 0] = (*eiter).first.first;
      cc[ei * 2 + 1] = (*eiter).first.second;
      dd[ei * 2 + 0] = 1.0 - (*eiter).first.dfirst;
      dd[ei * 2 + 1] = (*eiter).first.dfirst;

      ++eiter;
    }

    size_type nnz = 0;
    index_type i;
    for (i = 0; i < nrows; i++)
    {
      rr[i] = nnz;
      if (cc[i * 2 + 0] >= 0)
      {
        cc[nnz] = cc[i * 2 + 0];
        dd[nnz] = dd[i * 2 + 0];
        nnz++;
      }
      if (cc[i * 2 + 1] >= 0)
      {
        cc[nnz] = cc[i * 2 + 1];
        dd[nnz] = dd[i * 2 + 1];
        nnz++;
      }
    }

    rr[i] = nnz;

    return new SparseRowMatrix(nrows, ncols, sparseData, nnz);
   #endif
    return MatrixHandle();
}


MatrixHandle BaseMC::get_parent_cells()
{
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
   if( !build_field_ ) return (0);


    // The columns represent the source cells while the rows
    // represent the destination cells
    const size_type nrows = cell_map_.size();
    const size_type ncols = ncells_;

    // Yale Sparse Row Matrix format.
    SparseRowMatrix::Data sparseData(nrows+1, nrows);
    const SparseRowMatrix::Rows& rr = sparseData.rows();
    const SparseRowMatrix::Columns& cc = sparseData.columns();
    const SparseRowMatrix::Storage& dd = sparseData.data();

    size_type nnz = nrows;
    index_type i;

    for (i = 0; i < nrows; i++)
    {
      rr[i] = i;

      cc[i] = cell_map_[i];
      dd[i] = 1;
    }

    rr[i] = nrows;

    return new SparseRowMatrix(nrows, ncols, sparseData, nnz);
  #endif
    return MatrixHandle();
}
