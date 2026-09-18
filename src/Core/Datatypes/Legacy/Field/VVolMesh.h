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

#ifndef CORE_DATATYPES_VVOLMESH_H
#define CORE_DATATYPES_VVOLMESH_H 1

#include <Core/Datatypes/Legacy/Field/VUnstructuredMesh.h>

namespace SCIRun {

/// VVolMesh — shared virtual-dispatch base for all unstructured 3-D volumetric
/// mesh adapters (VHexVolMesh, VTetVolMesh, …).
///
/// It sits between VUnstructuredMesh<MESH> and the concrete V*Mesh adapters
/// and provides inline implementations for all topology-traversal overrides
/// that are identical across the 3-D volume mesh types:
///   get_nodes / get_enodes / get_edges / get_faces /
///   get_cells / get_elems / get_delems / set_nodes / get_elems_pointer
///
/// Each concrete adapter only needs to declare (and implement) the small set of
/// methods that differ per mesh type (e.g. get_elem/get_delem for HexVol, or
/// insert_node_into_elem for TetVol).

template <class MESH>
class VVolMesh : public VUnstructuredMesh<MESH>
{
public:
  explicit VVolMesh(MESH* mesh) : VUnstructuredMesh<MESH>(mesh) {}

  // ----- get_nodes ----------------------------------------------------------
  void get_nodes(VMesh::Node::array_type& nodes,
                 VMesh::Edge::index_type idx) const override
    { this->mesh_->get_nodes_from_edge(nodes, idx); }

  void get_nodes(VMesh::Node::array_type& nodes,
                 VMesh::Face::index_type idx) const override
    { this->mesh_->get_nodes_from_face(nodes, idx); }

  void get_nodes(VMesh::Node::array_type& nodes,
                 VMesh::Cell::index_type idx) const override
    { this->mesh_->get_nodes_from_cell(nodes, idx); }

  void get_nodes(VMesh::Node::array_type& nodes,
                 VMesh::Elem::index_type idx) const override
    { this->mesh_->get_nodes_from_cell(nodes, idx); }

  void get_nodes(VMesh::Node::array_type& nodes,
                 VMesh::DElem::index_type idx) const override
    { this->mesh_->get_nodes_from_face(nodes, idx); }

  // ----- get_enodes ---------------------------------------------------------
  void get_enodes(VMesh::ENode::array_type& enodes,
                  VMesh::Node::index_type idx) const override
    { this->mesh_->get_edges_from_node(enodes, idx); }

  void get_enodes(VMesh::ENode::array_type& enodes,
                  VMesh::Edge::index_type idx) const override
    { enodes.resize(1); enodes[0] = VMesh::ENode::index_type(idx); }

  void get_enodes(VMesh::ENode::array_type& enodes,
                  VMesh::Face::index_type idx) const override
    { this->mesh_->get_edges_from_face(enodes, idx); }

  void get_enodes(VMesh::ENode::array_type& enodes,
                  VMesh::DElem::index_type idx) const override
    { this->mesh_->get_edges_from_face(enodes, idx); }

  void get_enodes(VMesh::ENode::array_type& enodes,
                  VMesh::Cell::index_type idx) const override
    { this->mesh_->get_edges_from_cell(enodes, idx); }

  void get_enodes(VMesh::ENode::array_type& enodes,
                  VMesh::Elem::index_type idx) const override
    { this->mesh_->get_edges_from_cell(enodes, idx); }

  // ----- get_edges ----------------------------------------------------------
  void get_edges(VMesh::Edge::array_type& edges,
                 VMesh::Node::index_type idx) const override
    { this->mesh_->get_edges_from_node(edges, idx); }

  void get_edges(VMesh::Edge::array_type& edges,
                 VMesh::Face::index_type idx) const override
    { this->mesh_->get_edges_from_face(edges, idx); }

  void get_edges(VMesh::Edge::array_type& edges,
                 VMesh::DElem::index_type idx) const override
    { this->mesh_->get_edges_from_face(edges, idx); }

  void get_edges(VMesh::Edge::array_type& edges,
                 VMesh::Cell::index_type idx) const override
    { this->mesh_->get_edges_from_cell(edges, idx); }

  void get_edges(VMesh::Edge::array_type& edges,
                 VMesh::Elem::index_type idx) const override
    { this->mesh_->get_edges_from_cell(edges, idx); }

  // ----- get_faces ----------------------------------------------------------
  void get_faces(VMesh::Face::array_type& faces,
                 VMesh::Node::index_type idx) const override
    { this->mesh_->get_faces_from_node(faces, idx); }

  void get_faces(VMesh::Face::array_type& faces,
                 VMesh::Edge::index_type idx) const override
    { this->mesh_->get_faces_from_edge(faces, idx); }

  void get_faces(VMesh::Face::array_type& faces,
                 VMesh::Cell::index_type idx) const override
    { this->mesh_->get_faces_from_cell(faces, idx); }

  void get_faces(VMesh::Face::array_type& faces,
                 VMesh::Elem::index_type idx) const override
    { this->mesh_->get_faces_from_cell(faces, idx); }

  void get_faces(VMesh::Face::array_type& faces,
                 VMesh::DElem::index_type idx) const override
    { faces.resize(1); faces[0] = static_cast<VMesh::Face::index_type>(idx); }

  // ----- get_cells ----------------------------------------------------------
  void get_cells(VMesh::Cell::array_type& cells,
                 VMesh::Node::index_type idx) const override
    { this->mesh_->get_cells_from_node(cells, idx); }

  void get_cells(VMesh::Cell::array_type& cells,
                 VMesh::Edge::index_type idx) const override
    { this->mesh_->get_cells_from_edge(cells, idx); }

  void get_cells(VMesh::Cell::array_type& cells,
                 VMesh::Face::index_type idx) const override
    { this->mesh_->get_cells_from_face(cells, idx); }

  void get_cells(VMesh::Cell::array_type& cells,
                 VMesh::Elem::index_type idx) const override
    { cells.resize(1); cells[0] = static_cast<VMesh::Cell::index_type>(idx); }

  void get_cells(VMesh::Cell::array_type& cells,
                 VMesh::DElem::index_type idx) const override
    { this->mesh_->get_cells_from_face(cells, idx); }

  // ----- get_elems ----------------------------------------------------------
  void get_elems(VMesh::Elem::array_type& elems,
                 VMesh::Node::index_type idx) const override
    { this->mesh_->get_cells_from_node(elems, idx); }

  void get_elems(VMesh::Elem::array_type& elems,
                 VMesh::Edge::index_type idx) const override
    { this->mesh_->get_cells_from_edge(elems, idx); }

  void get_elems(VMesh::Elem::array_type& elems,
                 VMesh::Face::index_type idx) const override
    { this->mesh_->get_cells_from_face(elems, idx); }

  void get_elems(VMesh::Elem::array_type& elems,
                 VMesh::Cell::index_type idx) const override
    { elems.resize(1); elems[0] = static_cast<VMesh::Elem::index_type>(idx); }

  void get_elems(VMesh::Elem::array_type& elems,
                 VMesh::DElem::index_type idx) const override
    { this->mesh_->get_cells_from_face(elems, idx); }

  // ----- get_delems ---------------------------------------------------------
  void get_delems(VMesh::DElem::array_type& delems,
                  VMesh::Node::index_type idx) const override
    { this->mesh_->get_faces_from_node(delems, idx); }

  void get_delems(VMesh::DElem::array_type& delems,
                  VMesh::Edge::index_type idx) const override
    { this->mesh_->get_faces_from_edge(delems, idx); }

  void get_delems(VMesh::DElem::array_type& delems,
                  VMesh::Cell::index_type idx) const override
    { this->mesh_->get_faces_from_cell(delems, idx); }

  void get_delems(VMesh::DElem::array_type& delems,
                  VMesh::Elem::index_type idx) const override
    { this->mesh_->get_faces_from_cell(delems, idx); }

  void get_delems(VMesh::DElem::array_type& delems,
                  VMesh::Face::index_type idx) const override
    { delems.resize(1); delems[0] = static_cast<VMesh::DElem::index_type>(idx); }

  // ----- set_nodes ----------------------------------------------------------
  void set_nodes(VMesh::Node::array_type& nodes,
                 VMesh::Elem::index_type i) override
    { this->mesh_->set_nodes_by_elem(nodes, i); }

  void set_nodes(VMesh::Node::array_type& nodes,
                 VMesh::Cell::index_type i) override
    { this->mesh_->set_nodes_by_elem(nodes, i); }

  // ----- get_elems_pointer --------------------------------------------------
  VMesh::index_type* get_elems_pointer() const override
  {
    if (this->mesh_->cells_.size() == 0) return nullptr;
    return &(this->mesh_->cells_[0]);
  }
};

} // namespace SCIRun

#endif // CORE_DATATYPES_VVOLMESH_H
