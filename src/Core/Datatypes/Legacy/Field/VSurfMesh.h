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

#ifndef CORE_DATATYPES_VSURFMESH_H
#define CORE_DATATYPES_VSURFMESH_H 1

#include <Core/Datatypes/Legacy/Field/VUnstructuredMesh.h>

namespace SCIRun {

/// VSurfMesh — shared virtual-dispatch base for all unstructured 2-D surface
/// mesh adapters (VQuadSurfMesh, VTriSurfMesh).
///
/// Topology conventions for surface meshes:
///   Elem  == Face  (the 2-D element)
///   DElem == Edge  (the sub-element / boundary)
///   Cell  is NOT used for surface meshes.
///
/// Provides inline implementations for the topology-traversal overrides that
/// are identical between all surface mesh adapter types.

template <class MESH>
class VSurfMesh : public VUnstructuredMesh<MESH>
{
public:
  explicit VSurfMesh(MESH* mesh) : VUnstructuredMesh<MESH>(mesh) {}

  // ----- get_nodes ----------------------------------------------------------
  void get_nodes(VMesh::Node::array_type& nodes,
                 VMesh::Edge::index_type idx) const override
    { this->mesh_->get_nodes_from_edge(nodes, idx); }

  void get_nodes(VMesh::Node::array_type& nodes,
                 VMesh::Face::index_type idx) const override
    { this->mesh_->get_nodes_from_face(nodes, idx); }

  /// For surface meshes Elem == Face.
  void get_nodes(VMesh::Node::array_type& nodes,
                 VMesh::Elem::index_type idx) const override
    { this->mesh_->get_nodes_from_face(nodes, idx); }

  /// For surface meshes DElem == Edge.
  void get_nodes(VMesh::Node::array_type& nodes,
                 VMesh::DElem::index_type idx) const override
    { this->mesh_->get_nodes_from_edge(nodes, idx); }

  // ----- get_enodes ---------------------------------------------------------
  void get_enodes(VMesh::ENode::array_type& enodes,
                  VMesh::Face::index_type idx) const override
    { this->mesh_->get_edges_from_face(enodes, idx); }

  void get_enodes(VMesh::ENode::array_type& enodes,
                  VMesh::Elem::index_type idx) const override
    { this->mesh_->get_edges_from_face(enodes, idx); }

  /// DElem is the edge itself — trivial self-reference.
  void get_enodes(VMesh::ENode::array_type& enodes,
                  VMesh::DElem::index_type idx) const override
    { enodes.resize(1); enodes[0] = static_cast<VMesh::ENode::index_type>(idx); }

  // ----- get_edges ----------------------------------------------------------
  void get_edges(VMesh::Edge::array_type& edges,
                 VMesh::Node::index_type idx) const override
    { this->mesh_->get_edges_from_node(edges, idx); }

  void get_edges(VMesh::Edge::array_type& edges,
                 VMesh::Face::index_type idx) const override
    { this->mesh_->get_edges_from_face(edges, idx); }

  void get_edges(VMesh::Edge::array_type& edges,
                 VMesh::Elem::index_type idx) const override
    { this->mesh_->get_edges_from_face(edges, idx); }

  /// DElem is the edge itself — trivial self-reference.
  void get_edges(VMesh::Edge::array_type& edges,
                 VMesh::DElem::index_type idx) const override
    { edges.resize(1); edges[0] = static_cast<VMesh::Edge::index_type>(idx); }

  // ----- get_faces ----------------------------------------------------------
  void get_faces(VMesh::Face::array_type& faces,
                 VMesh::Node::index_type idx) const override
    { this->mesh_->get_faces_from_node(faces, idx); }

  void get_faces(VMesh::Face::array_type& faces,
                 VMesh::Edge::index_type idx) const override
    { this->mesh_->get_faces_from_edge(faces, idx); }

  /// Face is the element itself — trivial self-reference.
  void get_faces(VMesh::Face::array_type& faces,
                 VMesh::Elem::index_type idx) const override
    { faces.resize(1); faces[0] = static_cast<VMesh::Face::index_type>(idx); }

  void get_faces(VMesh::Face::array_type& faces,
                 VMesh::DElem::index_type idx) const override
    { this->mesh_->get_faces_from_edge(faces, idx); }

  // ----- get_elems ----------------------------------------------------------
  /// For surface meshes Elem == Face.
  void get_elems(VMesh::Elem::array_type& elems,
                 VMesh::Node::index_type idx) const override
    { this->mesh_->get_faces_from_node(elems, idx); }

  void get_elems(VMesh::Elem::array_type& elems,
                 VMesh::Edge::index_type idx) const override
    { this->mesh_->get_faces_from_edge(elems, idx); }

  void get_elems(VMesh::Elem::array_type& elems,
                 VMesh::Face::index_type idx) const override
    { elems.resize(1); elems[0] = static_cast<VMesh::Elem::index_type>(idx); }

  void get_elems(VMesh::Elem::array_type& elems,
                 VMesh::DElem::index_type idx) const override
    { this->mesh_->get_faces_from_edge(elems, idx); }

  // ----- get_delems ---------------------------------------------------------
  /// For surface meshes DElem == Edge.
  void get_delems(VMesh::DElem::array_type& delems,
                  VMesh::Node::index_type idx) const override
    { this->mesh_->get_edges_from_node(delems, idx); }

  /// Edge is the sub-element itself — trivial self-reference.
  void get_delems(VMesh::DElem::array_type& delems,
                  VMesh::Edge::index_type idx) const override
    { delems.resize(1); delems[0] = static_cast<VMesh::DElem::index_type>(idx); }

  void get_delems(VMesh::DElem::array_type& delems,
                  VMesh::Face::index_type idx) const override
    { this->mesh_->get_edges_from_face(delems, idx); }

  void get_delems(VMesh::DElem::array_type& delems,
                  VMesh::Elem::index_type idx) const override
    { this->mesh_->get_edges_from_face(delems, idx); }

  // ----- set_nodes ----------------------------------------------------------
  void set_nodes(VMesh::Node::array_type& nodes,
                 VMesh::Elem::index_type i) override
    { this->mesh_->set_nodes_by_elem(nodes, i); }

  void set_nodes(VMesh::Node::array_type& nodes,
                 VMesh::Face::index_type i) override
    { this->mesh_->set_nodes_by_elem(nodes, i); }

  // ----- get_elems_pointer --------------------------------------------------
  VMesh::index_type* get_elems_pointer() const override
  {
    if (this->mesh_->faces_.size() == 0) return nullptr;
    return &(this->mesh_->faces_[0]);
  }
};

} // namespace SCIRun

#endif // CORE_DATATYPES_VSURFMESH_H
