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


#include <Core/Datatypes/Legacy/Field/TetVolMesh.h>
#include <Core/Datatypes/Legacy/Field/VUnstructuredMesh.h>

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Basis;

/// Only include this class if we included TetVol Support
#if (SCIRUN_TETVOL_SUPPORT > 0)

namespace SCIRun {

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.
template<class MESH>
class VTetVolMesh : public VUnstructuredMesh<MESH> {
public:
  virtual bool is_tetvolmesh()         { return (true); }

  /// constructor and descructor
  VTetVolMesh(MESH* mesh) : VUnstructuredMesh<MESH>(mesh)
  {
    DEBUG_CONSTRUCTOR("VTetVolMesh")
  }
  virtual ~VTetVolMesh()
  {
    DEBUG_DESTRUCTOR("VTetVolMesh")
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
                          VMesh::Node::index_type i) const;
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
                         VMesh::Node::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Face::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Cell::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Elem::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::DElem::index_type i) const;

  virtual void get_faces(VMesh::Face::array_type& faces,
                         VMesh::Node::index_type i) const;
  virtual void get_faces(VMesh::Face::array_type& faces,
                         VMesh::Edge::index_type i) const;
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
                          VMesh::Node::index_type i) const;
  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Edge::index_type i) const;
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

  virtual void insert_node_into_elem(VMesh::Elem::array_type& newelems,
                                     VMesh::Node::index_type& newnode,
                                     VMesh::Elem::index_type  elem,
                                     Point& point);

  virtual VMesh::index_type* get_elems_pointer() const;

  virtual double inscribed_circumscribed_radius_metric(VMesh::Elem::index_type idx) const;
};

/// Functions for creating the virtual interface for specific mesh types
/// These are similar to compare maker and only serve to instantiate the class

/// Currently there are only 3 variations of this mesh available
/// 1) linear interpolation
/// 2) quadratic interpolation
/// 3) cubic interpolation

/// Add the LINEAR virtual interface and the meshid for creating it

/// Create virtual interface
VMesh* CreateVTetVolMesh(TetVolMesh<TetLinearLgn<Point> >* mesh)
{
  return new VTetVolMesh<TetVolMesh<TetLinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID TetVolMesh_MeshID1(TetVolMesh<TetLinearLgn<Point> >::type_name(-1),
                  TetVolMesh<TetLinearLgn<Point> >::mesh_maker);


/// Add the QUADRATIC virtual interface and the meshid for creating it
#if (SCIRUN_QUADRATIC_SUPPORT > 0)

/// Create virtual interface
VMesh* CreateVTetVolMesh(TetVolMesh<TetQuadraticLgn<Point> >* mesh)
{
  return new VTetVolMesh<TetVolMesh<TetQuadraticLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID TetVolMesh_MeshID2(TetVolMesh<TetQuadraticLgn<Point> >::type_name(-1),
                  TetVolMesh<TetQuadraticLgn<Point> >::mesh_maker);
#endif


/// Add the CUBIC virtual interface and the meshid for creating it
#if (SCIRUN_CUBIC_SUPPORT > 0)

/// Create virtual interface
VMesh* CreateVTetVolMesh(TetVolMesh<TetCubicHmt<Point> >* mesh)
{
  return new VTetVolMesh<TetVolMesh<TetCubicHmt<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID TetVolMesh_MeshID3(TetVolMesh<TetCubicHmt<Point> >::type_name(-1),
                  TetVolMesh<TetCubicHmt<Point> >::mesh_maker);
#endif

template <class MESH>
double
VTetVolMesh<MESH>::inscribed_circumscribed_radius_metric(VMesh::Elem::index_type idx) const
{
  return(this->mesh_->inscribed_circumscribed_radius_metric(idx));
}


template <class MESH>
void
VTetVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::Edge::index_type idx) const
{
  this->mesh_->get_nodes_from_edge(nodes,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_nodes_from_face(nodes,idx);
}


template <class MESH>
void
VTetVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::Cell::index_type idx) const
{
  this->mesh_->get_nodes_from_cell(nodes,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::Elem::index_type idx) const
{
  this->mesh_->get_nodes_from_cell(nodes,idx);
}


template <class MESH>
void
VTetVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_nodes_from_face(nodes,idx);
}


template <class MESH>
void
VTetVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Node::index_type idx) const
{
  this->mesh_->get_edges_from_node(enodes,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Edge::index_type idx) const
{
  enodes.resize(1); enodes[0] = VMesh::ENode::index_type(idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_edges_from_face(enodes,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_edges_from_face(enodes,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Cell::index_type idx) const
{
  this->mesh_->get_edges_from_cell(enodes,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Elem::index_type idx) const
{
  this->mesh_->get_edges_from_cell(enodes,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::Node::index_type idx) const
{
  this->mesh_->get_edges_from_node(edges,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_edges_from_face(edges,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_edges_from_face(edges,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::Cell::index_type idx) const
{
  this->mesh_->get_edges_from_cell(edges,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::Elem::index_type idx) const
{
  this->mesh_->get_edges_from_cell(edges,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_faces(VMesh::Face::array_type &faces,
                             VMesh::Node::index_type idx) const
{
  this->mesh_->get_faces_from_node(faces,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_faces(VMesh::Face::array_type &faces,
                             VMesh::Edge::index_type idx) const
{
  this->mesh_->get_faces_from_edge(faces,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_faces(VMesh::Face::array_type &faces,
                             VMesh::Cell::index_type idx) const
{
  this->mesh_->get_faces_from_cell(faces,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_faces(VMesh::Face::array_type &faces,
                             VMesh::Elem::index_type idx) const
{
  this->mesh_->get_faces_from_cell(faces,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_faces(VMesh::Face::array_type &faces,
                             VMesh::DElem::index_type idx) const
{
  faces.resize(1); faces[0] = static_cast<VMesh::Face::index_type>(idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::Node::index_type idx) const
{
  this->mesh_->get_cells_from_node(cells,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::Edge::index_type idx) const
{
  this->mesh_->get_cells_from_edge(cells,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_cells_from_face(cells,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::Elem::index_type idx) const
{
  cells.resize(1); cells[0] = static_cast<VMesh::Cell::index_type>(idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_cells_from_face(cells,idx);
}


template <class MESH>
void
VTetVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::Node::index_type idx) const
{
  this->mesh_->get_cells_from_node(elems,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::Edge::index_type idx) const
{
  this->mesh_->get_cells_from_edge(elems,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_cells_from_face(elems,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::Cell::index_type idx) const
{
  elems.resize(1); elems[0] = static_cast<VMesh::Elem::index_type>(idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_cells_from_face(elems,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_delems(VMesh::DElem::array_type &delems,
                              VMesh::Node::index_type idx) const
{
  this->mesh_->get_faces_from_node(delems,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_delems(VMesh::DElem::array_type &delems,
                              VMesh::Edge::index_type idx) const
{
  this->mesh_->get_faces_from_edge(delems,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_delems(VMesh::DElem::array_type &delems,
                              VMesh::Cell::index_type idx) const
{
  this->mesh_->get_faces_from_cell(delems,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_delems(VMesh::DElem::array_type &delems,
                              VMesh::Elem::index_type idx) const
{
  this->mesh_->get_faces_from_cell(delems,idx);
}

template <class MESH>
void
VTetVolMesh<MESH>::get_delems(VMesh::DElem::array_type &delems,
                              VMesh::Face::index_type idx) const
{
  delems.resize(1); delems[0] = static_cast<VMesh::DElem::index_type>(idx);
}

template <class MESH>
VMesh::index_type*
VTetVolMesh<MESH>::
get_elems_pointer() const
{
  if (this->mesh_->cells_.size() == 0) return (0);
   return (&(this->mesh_->cells_[0]));
}



template <class MESH>
void
VTetVolMesh<MESH>::set_nodes(VMesh::Node::array_type& nodes,
                              VMesh::Elem::index_type i)
{
  this->mesh_->set_nodes_by_elem(nodes,i);
}

template <class MESH>
void
VTetVolMesh<MESH>::set_nodes(VMesh::Node::array_type& nodes,
                              VMesh::Cell::index_type i)
{
  this->mesh_->set_nodes_by_elem(nodes,i);
}


/// @todo: Fix this function so it does not need the vector conversion
template <class MESH>
void
VTetVolMesh<MESH>::insert_node_into_elem(VMesh::Elem::array_type& newelems,
                                     VMesh::Node::index_type& newnode,
                                     VMesh::Elem::index_type  elem,
                                     Point& point)
{
  // This wrapper fixes class types
  typename MESH::Elem::array_type array;
  typename MESH::Node::index_type index;
  this->mesh_->insert_node_in_elem(array,index,typename MESH::Elem::index_type(elem),point);
  this->convert_vector(array,newelems);
  newnode = VMesh::Node::index_type(index);
}


} // namespace SCIRun

#endif
