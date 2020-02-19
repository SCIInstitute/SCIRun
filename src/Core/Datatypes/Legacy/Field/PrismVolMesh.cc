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


#include <Core/Datatypes/Legacy/Field/PrismVolMesh.h>
#include <Core/Datatypes/Legacy/Field/VUnstructuredMesh.h>

using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;

/// Only include this class if we included PrismVol Support
#if (SCIRUN_PRISMVOL_SUPPORT > 0)

namespace SCIRun {

/// This is the virtual interface to the curve mesh
/// This class lives besides the real mesh class for now and solely profides
/// an interface. In the future however when dynamic compilation is gone
/// this should be put into the PrismVolMesh class.
template<class MESH> class VPrismVolMesh;

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.
template<class MESH>
class VPrismVolMesh : public VUnstructuredMesh<MESH> {
public:
  virtual bool is_prismvolmesh()       { return (true); }

  /// constructor and destructor
  explicit VPrismVolMesh(MESH* mesh) : VUnstructuredMesh<MESH>(mesh)
  {
    DEBUG_CONSTRUCTOR("VPrismVolMesh")
  }
  virtual ~VPrismVolMesh()
  {
    DEBUG_DESTRUCTOR("VPrismVolMesh")
  }

  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::Edge::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::Face::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::Cell::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::Elem::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::DElem::index_type i) const;

  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::Edge::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::Face::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::Cell::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::Elem::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::DElem::index_type i) const;

  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Face::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Cell::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Elem::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::DElem::index_type i) const;

  virtual void get_faces(VMesh::Face::array_type& faces,
                         VMesh::Cell::index_type i) const;
  virtual void get_faces(VMesh::Face::array_type& faces,
                         VMesh::Elem::index_type i) const;
  virtual void get_faces(VMesh::Face::array_type& faces,
                         VMesh::DElem::index_type i) const;

  virtual void get_cells(VMesh::Cell::array_type& cells,
                         VMesh::Node::index_type i) const;
  virtual void get_cells(VMesh::Cell::array_type& cells,
                         VMesh::Edge::index_type i) const;
  virtual void get_cells(VMesh::Cell::array_type& cells,
                         VMesh::Face::index_type i) const;
  virtual void get_cells(VMesh::Cell::array_type& cells,
                         VMesh::Elem::index_type i) const;
  virtual void get_cells(VMesh::Cell::array_type& cells,
                         VMesh::DElem::index_type i) const;

  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::Node::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::Edge::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::Face::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::Cell::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::DElem::index_type i) const;

  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Face::index_type i) const;
  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Cell::index_type i) const;
  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Elem::index_type i) const;

  virtual void set_nodes(VMesh::Node::array_type&,
                         VMesh::Elem::index_type);

  virtual void set_nodes(VMesh::Node::array_type&,
                         VMesh::Cell::index_type);


  virtual VMesh::index_type* get_elems_pointer() const;
};

/// Functions for creating the virtual interface for specific mesh types
/// These are similar to compare maker and only serve to instantiate the class

/// Currently there are only 3 variations of this mesh available
/// 1) linear interpolation
/// 2) quadratic interpolation
/// 3) cubic interpolation

/// Add the LINEAR virtual interface and the meshid for creating it

/// Create virtual interface
VMesh* CreateVPrismVolMesh(PrismVolMesh<PrismLinearLgn<Point> >* mesh)
{
  return new VPrismVolMesh<PrismVolMesh<PrismLinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID PrismVolMesh_MeshID1(PrismVolMesh<PrismLinearLgn<Point> >::type_name(-1),
                  PrismVolMesh<PrismLinearLgn<Point> >::mesh_maker);


/// Add the QUADRATIC virtual interface and the meshid for creating it
#if (SCIRUN_QUADRATIC_SUPPORT > 0)

/// Create virtual interface
VMesh* CreateVPrismVolMesh(PrismVolMesh<PrismQuadraticLgn<Point> >* mesh)
{
  return new VPrismVolMesh<PrismVolMesh<PrismQuadraticLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID PrismVolMesh_MeshID2(PrismVolMesh<PrismQuadraticLgn<Point> >::type_name(-1),
                  PrismVolMesh<PrismQuadraticLgn<Point> >::mesh_maker);
#endif


/// Add the CUBIC virtual interface and the meshid for creating it
#if (SCIRUN_CUBIC_SUPPORT > 0)

/// Create virtual interface
VMesh* CreateVPrismVolMesh(PrismVolMesh<PrismCubicHmt<Point> >* mesh)
{
  return new VPrismVolMesh<PrismVolMesh<PrismCubicHmt<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID PrismVolMesh_MeshID3(PrismVolMesh<PrismCubicHmt<Point> >::type_name(-1),
                  PrismVolMesh<PrismCubicHmt<Point> >::mesh_maker);
#endif

template <class MESH>
void
VPrismVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::Edge::index_type idx) const
{
  this->mesh_->get_nodes_from_edge(nodes,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_nodes_from_face(nodes,idx);
}


template <class MESH>
void
VPrismVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::Cell::index_type idx) const
{
  this->mesh_->get_nodes_from_cell(nodes,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::Elem::index_type idx) const
{
  this->mesh_->get_nodes_from_cell(nodes,idx);
}


template <class MESH>
void
VPrismVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_nodes_from_face(nodes,idx);
}


template <class MESH>
void
VPrismVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Edge::index_type idx) const
{
  enodes.resize(1); enodes[0] = VMesh::ENode::index_type(idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_edges_from_face(enodes,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_edges_from_face(enodes,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Cell::index_type idx) const
{
  this->mesh_->get_edges_from_cell(enodes,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Elem::index_type idx) const
{
  this->mesh_->get_edges_from_cell(enodes,idx);
}



template <class MESH>
void
VPrismVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_edges_from_face(edges,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_edges_from_face(edges,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::Cell::index_type idx) const
{
  this->mesh_->get_edges_from_cell(edges,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::Elem::index_type idx) const
{
  this->mesh_->get_edges_from_cell(edges,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_faces(VMesh::Face::array_type &faces,
                             VMesh::Cell::index_type idx) const
{
  this->mesh_->get_faces_from_cell(faces,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_faces(VMesh::Face::array_type &faces,
                             VMesh::Elem::index_type idx) const
{
  this->mesh_->get_faces_from_cell(faces,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_faces(VMesh::Face::array_type &faces,
                             VMesh::DElem::index_type idx) const
{
  faces.resize(1); faces[0] = static_cast<VMesh::Face::index_type>(idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::Node::index_type idx) const
{
  this->mesh_->get_cells_from_node(cells,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::Edge::index_type idx) const
{
  this->mesh_->get_cells_from_edge(cells,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_cells_from_face(cells,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::Elem::index_type idx) const
{
  cells.resize(1); cells[0] = static_cast<VMesh::Cell::index_type>(idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_cells_from_face(cells,idx);
}


template <class MESH>
void
VPrismVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::Node::index_type idx) const
{
  this->mesh_->get_cells_from_node(elems,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::Edge::index_type idx) const
{
  this->mesh_->get_cells_from_edge(elems,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_cells_from_face(elems,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::Cell::index_type idx) const
{
  elems.resize(1); elems[0] = static_cast<VMesh::Elem::index_type>(idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_cells_from_face(elems,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_delems(VMesh::DElem::array_type &delems,
                              VMesh::Cell::index_type idx) const
{
  this->mesh_->get_faces_from_cell(delems,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_delems(VMesh::DElem::array_type &delems,
                              VMesh::Elem::index_type idx) const
{
  this->mesh_->get_faces_from_cell(delems,idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::get_delems(VMesh::DElem::array_type &delems,
                              VMesh::Face::index_type idx) const
{
  delems.resize(1); delems[0] = static_cast<VMesh::DElem::index_type>(idx);
}

template <class MESH>
void
VPrismVolMesh<MESH>::set_nodes(VMesh::Node::array_type& nodes,
                              VMesh::Elem::index_type i)
{
  this->mesh_->set_nodes_by_elem(nodes,i);
}

template <class MESH>
void
VPrismVolMesh<MESH>::set_nodes(VMesh::Node::array_type& nodes,
                              VMesh::Cell::index_type i)
{
  this->mesh_->set_nodes_by_elem(nodes,i);
}

template <class MESH>
VMesh::index_type*
VPrismVolMesh<MESH>::
get_elems_pointer() const
{
  if (this->mesh_->cells_.size() == 0) return (0);
   return (&(this->mesh_->cells_[0]));
}

}

#endif
