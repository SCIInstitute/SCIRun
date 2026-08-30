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

#ifndef CORE_DATATYPES_GENERICVOLMESHMETHODS_H
#define CORE_DATATYPES_GENERICVOLMESHMETHODS_H 1

/// SCIRUN_VOL_MESH_TOPOLOGY_METHODS — paste this macro once in the public
/// section of any volumetric mesh class (HexVolMesh, TetVolMesh, …) to
/// generate the standard topology-traversal methods that are identical across
/// all volumetric mesh types.
///
/// The macro must be placed inside the class body, after the Node / Edge /
/// Face / Cell nested structs and the Elem / DElem typedefs are defined, and
/// after the private helper methods (get_nodes_from_edge, etc.) are declared.
/// The class must also have a public `basis_` member (or `Basis basis_`).
///
/// Methods provided:
///   get_basis(), to_index() (×4),
///   get_nodes/edges/faces/cells/elems/delems() (×4 overloads each),
///   pwl_approx_edge(), pwl_approx_face()
// clang-format off
#define SCIRUN_VOL_MESH_TOPOLOGY_METHODS(BasisType) \
  /** Return reference to the interpolation basis. */ \
  BasisType& get_basis() { return basis_; } \
  \
  /** Convert flat SCIRun::index_type to a typed mesh index. */ \
  void to_index(typename Node::index_type &index, index_type i) const \
    { index = i; } \
  void to_index(typename Edge::index_type &index, index_type i) const \
    { index = i; } \
  void to_index(typename Face::index_type &index, index_type i) const \
    { index = i; } \
  void to_index(typename Cell::index_type &index, index_type i) const \
    { index = i; } \
  \
  /** Get child topology elements. */ \
  void get_nodes(typename Node::array_type &array, \
                 typename Node::index_type idx) const \
    { array.resize(1); array[0]= idx; } \
  void get_nodes(typename Node::array_type &array, \
                 typename Edge::index_type idx) const \
    { get_nodes_from_edge(array,idx); } \
  void get_nodes(typename Node::array_type &array, \
                 typename Face::index_type idx) const \
    { get_nodes_from_face(array,idx); } \
  void get_nodes(typename Node::array_type &array, \
                 typename Cell::index_type idx) const \
    { get_nodes_from_cell(array,idx); } \
  \
  void get_edges(typename Edge::array_type &array, \
                 typename Node::index_type idx) const \
    { get_edges_from_node(array,idx); } \
  void get_edges(typename Edge::array_type &array, \
                 typename Edge::index_type idx) const \
    { array.resize(1); array[0]= idx; } \
  void get_edges(typename Edge::array_type &array, \
                 typename Face::index_type idx) const \
    { get_edges_from_face(array,idx); } \
  void get_edges(typename Edge::array_type &array, \
                 typename Cell::index_type idx) const \
    { get_edges_from_cell(array,idx); } \
  \
  void get_faces(typename Face::array_type &array, \
                 typename Node::index_type idx) const \
    { get_faces_from_node(array,idx); } \
  void get_faces(typename Face::array_type &array, \
                 typename Edge::index_type idx) const \
    { get_faces_from_edge(array,idx); } \
  void get_faces(typename Face::array_type &array, \
                 typename Face::index_type idx) const \
    { array.resize(1); array[0]= idx; } \
  void get_faces(typename Face::array_type &array, \
                 typename Cell::index_type idx) const \
    { get_faces_from_cell(array,idx); } \
  \
  void get_cells(typename Cell::array_type &array, \
                 typename Node::index_type idx) const \
    { get_cells_from_node(array,idx); } \
  void get_cells(typename Cell::array_type &array, \
                 typename Edge::index_type idx) const \
    { get_cells_from_edge(array,idx); } \
  void get_cells(typename Cell::array_type &array, \
                 typename Face::index_type idx) const \
    { get_cells_from_face(array,idx); } \
  void get_cells(typename Cell::array_type &array, \
                 typename Cell::index_type idx) const \
    { array.resize(1); array[0]= idx; } \
  \
  /** get_elems == get_cells for volumetric meshes. */ \
  void get_elems(typename Elem::array_type &array, \
                 typename Node::index_type idx) const \
    { get_cells_from_node(array,idx); } \
  void get_elems(typename Elem::array_type &array, \
                 typename Edge::index_type idx) const \
    { get_cells_from_edge(array,idx); } \
  void get_elems(typename Elem::array_type &array, \
                 typename Face::index_type idx) const \
    { get_cells_from_face(array,idx); } \
  void get_elems(typename Elem::array_type &array, \
                 typename Cell::index_type idx) const \
    { array.resize(1); array[0]= idx; } \
  \
  /** get_delems == get_faces for volumetric meshes. */ \
  void get_delems(typename DElem::array_type &array, \
                  typename Node::index_type idx) const \
    { get_faces_from_node(array,idx); } \
  void get_delems(typename DElem::array_type &array, \
                  typename Edge::index_type idx) const \
    { get_faces_from_edge(array,idx); } \
  void get_delems(typename DElem::array_type &array, \
                  typename Face::index_type idx) const \
    { array.resize(1); array[0]= idx; } \
  void get_delems(typename DElem::array_type &array, \
                  typename Cell::index_type idx) const \
    { get_faces_from_cell(array,idx); } \
  \
  /** Piecewise-linear approximation of an edge. */ \
  template<class VECTOR, class INDEX> \
  void pwl_approx_edge(std::vector<VECTOR> &coords, \
                       INDEX, \
                       unsigned which_edge, \
                       unsigned div_per_unit) const \
    { basis_.approx_edge(which_edge, div_per_unit, coords); } \
  \
  /** Piecewise-linear approximation of a face. */ \
  template<class VECTOR, class INDEX> \
  void pwl_approx_face(std::vector<std::vector<VECTOR>> &coords, \
                       INDEX, \
                       unsigned which_face, \
                       unsigned div_per_unit) const \
    { basis_.approx_face(which_face, div_per_unit, coords); }
// clang-format on

#endif // CORE_DATATYPES_GENERICVOLMESHMETHODS_H
