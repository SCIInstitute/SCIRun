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


#include <Core/Datatypes/Legacy/Field/TriSurfMesh.h>
#include <Core/Datatypes/Legacy/Field/VUnstructuredMesh.h>

using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Thread;

/// Only include this class if we included TriSurf Support
#if (SCIRUN_TRISURF_SUPPORT > 0)

namespace SCIRun {

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.
template<class MESH>
class VTriSurfMesh : public VUnstructuredMesh<MESH> {
public:

  virtual bool is_trisurfmesh()        { return (true); }

  /// constructor and destructor
  VTriSurfMesh(MESH* mesh) : VUnstructuredMesh<MESH>(mesh)
  {
    DEBUG_CONSTRUCTOR("VTriSurfMesh")
  }
  virtual ~VTriSurfMesh()
  {
    DEBUG_DESTRUCTOR("VTriSurfMesh")
  }

  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::Edge::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::Face::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::Elem::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::DElem::index_type i) const;

  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::Face::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::Elem::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::DElem::index_type i) const;

  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Node::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Face::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Elem::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::DElem::index_type i) const;

  virtual void get_faces(VMesh::Face::array_type& faces,
                         VMesh::Node::index_type i) const;
  virtual void get_faces(VMesh::Face::array_type& faces,
                         VMesh::Edge::index_type i) const;
  virtual void get_faces(VMesh::Face::array_type& faces,
                         VMesh::Elem::index_type i) const;
  virtual void get_faces(VMesh::Face::array_type& faces,
                         VMesh::DElem::index_type i) const;

  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::Node::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::Edge::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::Face::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::DElem::index_type i) const;

  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Node::index_type i) const;
  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Edge::index_type i) const;
  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Face::index_type i) const;
  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Elem::index_type i) const;

  virtual void set_nodes(VMesh::Node::array_type&,
                         VMesh::Elem::index_type);

  virtual void set_nodes(VMesh::Node::array_type&,
                         VMesh::Face::index_type);


  virtual void insert_node_into_elem(VMesh::Elem::array_type& newelems,
                                     VMesh::Node::index_type& newnode,
                                     VMesh::Elem::index_type  elem,
                                     Point& point);

  virtual VMesh::index_type* get_elems_pointer() const;
  virtual boost::shared_ptr<SearchGridT<typename SCIRun::index_type> > get_elem_search_grid() { return this->mesh_->elem_grid_; }
  virtual boost::shared_ptr<SearchGridT<typename SCIRun::index_type> > get_node_search_grid() { return this->mesh_->node_grid_; }

};


/// Functions for creating the virtual interface for specific mesh types
/// These are similar to compare maker and only serve to instantiate the class

/// Currently there are only 3 variations of this mesh available
/// 1) linear interpolation
/// 2) quadratic interpolation
/// 3) cubic interpolation

/// Add the LINEAR virtual interface and the meshid for creating it

/// Create virtual interface
VMesh* CreateVTriSurfMesh(TriSurfMesh<TriLinearLgn<Point> >* mesh)
{
  return new VTriSurfMesh<TriSurfMesh<TriLinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID TriSurfMesh_MeshID1(
                  TriSurfMesh<TriLinearLgn<Point> >::type_name(-1),
                  TriSurfMesh<TriLinearLgn<Point> >::mesh_maker);


/// Add the QUADRATIC virtual interface and the meshid for creating it
#if (SCIRUN_QUADRATIC_SUPPORT > 0)

/// Create virtual interface
VMesh* CreateVTriSurfMesh(TriSurfMesh<TriQuadraticLgn<Point> >* mesh)
{
  return new VTriSurfMesh<TriSurfMesh<TriQuadraticLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID TriSurfMesh_MeshID2(TriSurfMesh<TriQuadraticLgn<Point> >::type_name(-1),
                  TriSurfMesh<TriQuadraticLgn<Point> >::mesh_maker);
#endif


/// Add the CUBIC virtual interface and the meshid for creating it
#if (SCIRUN_CUBIC_SUPPORT > 0)

/// Create virtual interface
VMesh* CreateVTriSurfMesh(TriSurfMesh<TriCubicHmt<Point> >* mesh)
{
  return new VTriSurfMesh<TriSurfMesh<TriCubicHmt<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID TriSurfMesh_MeshID3(TriSurfMesh<TriCubicHmt<Point> >::type_name(-1),
                  TriSurfMesh<TriCubicHmt<Point> >::mesh_maker);

#endif

template <class MESH>
void
VTriSurfMesh<MESH>::get_nodes(VMesh::Node::array_type& nodes,
                              VMesh::Edge::index_type i) const
{
  this->mesh_->get_nodes_from_edge(nodes,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_nodes(VMesh::Node::array_type& nodes,
                              VMesh::Face::index_type i) const
{
  this->mesh_->get_nodes_from_face(nodes,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_nodes(VMesh::Node::array_type& nodes,
                               VMesh::Elem::index_type i) const
{
  this->mesh_->get_nodes_from_face(nodes,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_nodes(VMesh::Node::array_type& nodes,
                               VMesh::DElem::index_type i) const
{
  this->mesh_->get_nodes_from_edge(nodes,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_enodes(VMesh::ENode::array_type& enodes,
                               VMesh::Face::index_type i) const
{
  this->mesh_->get_edges_from_face(enodes,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_enodes(VMesh::ENode::array_type& enodes,
                               VMesh::Elem::index_type i) const
{
  this->mesh_->get_edges_from_face(enodes,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_enodes(VMesh::ENode::array_type& enodes,
                               VMesh::DElem::index_type i) const
{
  enodes.resize(1); enodes[0] = static_cast<VMesh::ENode::index_type>(i);
}



template <class MESH>
void
VTriSurfMesh<MESH>::get_edges(VMesh::Edge::array_type& edges,
                              VMesh::Node::index_type i) const
{
  this->mesh_->get_edges_from_node(edges,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_edges(VMesh::Edge::array_type& edges,
                              VMesh::Face::index_type i) const
{
  this->mesh_->get_edges_from_face(edges,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_edges(VMesh::Edge::array_type& edges,
                              VMesh::Elem::index_type i) const
{
  this->mesh_->get_edges_from_face(edges,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_edges(VMesh::Edge::array_type& edges,
                               VMesh::DElem::index_type i) const
{
  edges.resize(1); edges[0] = static_cast<VMesh::Edge::index_type>(i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_faces(VMesh::Face::array_type& faces,
                               VMesh::Node::index_type i) const
{
  this->mesh_->get_faces_from_node(faces,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_faces(VMesh::Face::array_type& faces,
                               VMesh::Edge::index_type i) const
{
  this->mesh_->get_faces_from_edge(faces,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_faces(VMesh::Face::array_type& faces,
                               VMesh::Elem::index_type i) const
{
  faces.resize(1); faces[0] = static_cast<VMesh::Face::index_type>(i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_faces(VMesh::Face::array_type& faces,
                               VMesh::DElem::index_type i) const
{
  this->mesh_->get_faces_from_edge(faces,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_elems(VMesh::Elem::array_type& elems,
                               VMesh::Node::index_type i) const
{
  this->mesh_->get_faces_from_node(elems,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_elems(VMesh::Elem::array_type& elems,
                               VMesh::Edge::index_type i) const
{
  this->mesh_->get_faces_from_edge(elems,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_elems(VMesh::Elem::array_type& elems,
                               VMesh::Face::index_type i) const
{
  elems.resize(1); elems[0] = static_cast<VMesh::Elem::index_type>(i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_elems(VMesh::Elem::array_type& elems,
                               VMesh::DElem::index_type i) const
{
  this->mesh_->get_faces_from_edge(elems,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_delems(VMesh::DElem::array_type& delems,
                                VMesh::Node::index_type i) const
{
  this->mesh_->get_edges_from_node(delems,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_delems(VMesh::DElem::array_type& delems,
                               VMesh::Edge::index_type i) const
{
  delems.resize(1); delems[0] = static_cast<VMesh::DElem::index_type>(i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_delems(VMesh::DElem::array_type& delems,
                                VMesh::Face::index_type i) const
{
  this->mesh_->get_edges_from_face(delems,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::get_delems(VMesh::DElem::array_type& delems,
                                VMesh::Elem::index_type i) const
{
  this->mesh_->get_edges_from_face(delems,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::set_nodes(VMesh::Node::array_type& nodes,
                              VMesh::Elem::index_type i)
{
  this->mesh_->set_nodes_by_elem(nodes,i);
}

template <class MESH>
void
VTriSurfMesh<MESH>::set_nodes(VMesh::Node::array_type& nodes,
                              VMesh::Face::index_type i)
{
  this->mesh_->set_nodes_by_elem(nodes,i);
}



template <class MESH>
VMesh::index_type*
VTriSurfMesh<MESH>::
get_elems_pointer() const
{
  if (this->mesh_->faces_.size() == 0) return (0);
   return (&(this->mesh_->faces_[0]));
}

/// @todo: Fix this function so it does not need the vector conversion
template <class MESH>
void
VTriSurfMesh<MESH>::insert_node_into_elem(VMesh::Elem::array_type& newelems,
                                     VMesh::Node::index_type& newnode,
                                     VMesh::Elem::index_type  elem,
                                     Point& point)
{
  // This wrapper fixes class types
  typename MESH::Elem::array_type array;
  typename MESH::Node::index_type index;
  this->mesh_->insert_node_in_face(array,index,typename MESH::Elem::index_type(elem),point);
  this->convert_vector(array,newelems);
  newnode = VMesh::Node::index_type(index);
}


}// namespace SCIRun

#endif
