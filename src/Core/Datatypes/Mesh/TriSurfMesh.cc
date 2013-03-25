/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Core/Datatypes/Mesh/TriSurfMesh.h>
#include <Core/Datatypes/Mesh/TriSurfMeshRegister.h>
#include <Core/Datatypes/Mesh/VUnstructuredMesh.h>
#include <Core/Datatypes/Mesh/MeshFactory.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Basis;

void SCIRun::Core::Datatypes::registerTriSurfMeshes()
{
  //! Register class maker, so we can instantiate it
  static MeshRegistry::MeshTypeID TriSurfMesh_MeshID1("TriSurfMesh<TriLinearLgn<Point>>", 
    TriSurfMesh<TriLinearLgn<Point> >::mesh_maker);
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 static MeshRegistry::MeshTypeID TriSurfMesh_MeshID2("TriSurfMesh<TriQuadraticLgn<Point>>",
    TriSurfMesh<TriQuadraticLgn<Point> >::mesh_maker);
  static MeshRegistry::MeshTypeID TriSurfMesh_MeshID3("TriSurfMesh<TriCubicHmt<Point>>", 
    TriSurfMesh<TriCubicHmt<Point> >::mesh_maker);
#endif
}

namespace SCIRun {
  namespace Core {
    namespace Datatypes {
//! This class is not exposed to the general interface and only the VirtualMesh class
//! is accessed by other classes.
template<class MESH>
class VTriSurfMesh : public VUnstructuredMesh<MESH> {
public:

  virtual bool is_trisurfmesh()        { return (true); }

  VTriSurfMesh(MESH* mesh) : VUnstructuredMesh<MESH>(mesh) 
  {
    //DEBUG_CONSTRUCTOR("VTriSurfMesh")
  }
  virtual ~VTriSurfMesh() 
  {
    //DEBUG_DESTRUCTOR("VTriSurfMesh")
  }
  
  virtual void get_nodes(VirtualMesh::Node::array_type& nodes, 
                         VirtualMesh::Edge::index_type i) const;
  virtual void get_nodes(VirtualMesh::Node::array_type& nodes,
                         VirtualMesh::Face::index_type i) const;
  virtual void get_nodes(VirtualMesh::Node::array_type& nodes, 
                         VirtualMesh::Elem::index_type i) const;
  virtual void get_nodes(VirtualMesh::Node::array_type& nodes,
                         VirtualMesh::DElem::index_type i) const;

  virtual void get_enodes(VirtualMesh::ENode::array_type& edges,
                          VirtualMesh::Face::index_type i) const;
  virtual void get_enodes(VirtualMesh::ENode::array_type& edges, 
                          VirtualMesh::Elem::index_type i) const;
  virtual void get_enodes(VirtualMesh::ENode::array_type& edges,
                          VirtualMesh::DElem::index_type i) const;

  virtual void get_edges(VirtualMesh::Edge::array_type& edges, 
                         VirtualMesh::Node::index_type i) const;                                            
  virtual void get_edges(VirtualMesh::Edge::array_type& edges, 
                         VirtualMesh::Face::index_type i) const;
  virtual void get_edges(VirtualMesh::Edge::array_type& edges,
                         VirtualMesh::Elem::index_type i) const;
  virtual void get_edges(VirtualMesh::Edge::array_type& edges,
                         VirtualMesh::DElem::index_type i) const;

  virtual void get_faces(VirtualMesh::Face::array_type& faces, 
                         VirtualMesh::Node::index_type i) const;
  virtual void get_faces(VirtualMesh::Face::array_type& faces, 
                         VirtualMesh::Edge::index_type i) const;
  virtual void get_faces(VirtualMesh::Face::array_type& faces,
                         VirtualMesh::Elem::index_type i) const;
  virtual void get_faces(VirtualMesh::Face::array_type& faces,
                         VirtualMesh::DElem::index_type i) const;
  
  virtual void get_elems(VirtualMesh::Elem::array_type& elems, 
                         VirtualMesh::Node::index_type i) const;
  virtual void get_elems(VirtualMesh::Elem::array_type& elems, 
                         VirtualMesh::Edge::index_type i) const;
  virtual void get_elems(VirtualMesh::Elem::array_type& elems, 
                         VirtualMesh::Face::index_type i) const;
  virtual void get_elems(VirtualMesh::Elem::array_type& elems,
                         VirtualMesh::DElem::index_type i) const;

  virtual void get_delems(VirtualMesh::DElem::array_type& delems,
                          VirtualMesh::Node::index_type i) const;
  virtual void get_delems(VirtualMesh::DElem::array_type& delems,
                          VirtualMesh::Edge::index_type i) const;
  virtual void get_delems(VirtualMesh::DElem::array_type& delems, 
                          VirtualMesh::Face::index_type i) const;
  virtual void get_delems(VirtualMesh::DElem::array_type& delems,
                          VirtualMesh::Elem::index_type i) const;
                          
  virtual void set_nodes(VirtualMesh::Node::array_type&,
                         VirtualMesh::Elem::index_type);
  
  virtual void set_nodes(VirtualMesh::Node::array_type&,
                         VirtualMesh::Face::index_type);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  virtual void insert_node_into_elem(VirtualMesh::Elem::array_type& newelems, 
                                     VirtualMesh::Node::index_type& newnode,
                                     VirtualMesh::Elem::index_type  elem,
                                     Point& point);

  virtual VirtualMesh::index_type* get_elems_pointer() const;

  virtual LockingHandle<SearchGridT<typename SCIRun::index_type> > get_elem_search_grid() { return this->mesh_->elem_grid_; }
  virtual LockingHandle<SearchGridT<typename SCIRun::index_type> > get_node_search_grid() { return this->mesh_->node_grid_; }
#endif
};
}}}


//! Functions for creating the virtual interface for specific mesh types
//! These are similar to compare maker and only serve to instantiate the class

//! Currently there are only 3 variations of this mesh available
//! 1) linear interpolation
//! 2) quadratic interpolation
//! 3) cubic interpolation

//! Add the LINEAR virtual interface and the meshid for creating it 

//! Create virtual interface

VirtualMeshHandle SCIRun::Core::Datatypes::CreateVTriSurfMesh(TriSurfMesh<TriLinearLgn<Point> >* mesh)
{
  return VirtualMeshHandle(new VTriSurfMesh<TriSurfMesh<TriLinearLgn<Point> > >(mesh));
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

// TODO: SCIRUN_QUADRATIC_SUPPORT and SCIRUN_CUBIC_SUPPORT actually supported in SCIRun 4?
// Couldn't definitions (check SCIRun 3)
#if (SCIRUN_QUADRATIC_SUPPORT > 0)
VirtualMeshHandle SCIRun::Core::Datatypes::CreateVTriSurfMesh(TriSurfMesh<TriQuadraticLgn<Point> >* mesh)
{
  return new VTriSurfMesh<TriSurfMesh<TriQuadraticLgn<Point> > >(mesh);
}
#endif

#if (SCIRUN_CUBIC_SUPPORT > 0)
VirtualMeshHandle SCIRun::Core::Datatypes::CreateVTriSurfMesh(TriSurfMesh<TriCubicHmt<Point> >* mesh)
{
  return new VTriSurfMesh<TriSurfMesh<TriCubicHmt<Point> > >(mesh);
}
#endif

#endif

#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
//! Register class maker, so we can instantiate it
static MeshTypeID TriSurfMesh_MeshID1(
                  TriSurfMesh<TriLinearLgn<Point> >::type_name(-1),
                  TriSurfMesh<TriLinearLgn<Point> >::mesh_maker);

//! Register class maker, so we can instantiate it
static MeshTypeID TriSurfMesh_MeshID2(TriSurfMesh<TriQuadraticLgn<Point> >::type_name(-1),
                  TriSurfMesh<TriQuadraticLgn<Point> >::mesh_maker);

//! Register class maker, so we can instantiate it
static MeshTypeID TriSurfMesh_MeshID3(TriSurfMesh<TriCubicHmt<Point> >::type_name(-1),
                  TriSurfMesh<TriCubicHmt<Point> >::mesh_maker);

#endif

template <class MESH>
void 
VTriSurfMesh<MESH>::get_nodes(VirtualMesh::Node::array_type& nodes, 
                              VirtualMesh::Edge::index_type i) const
{
  this->mesh_->get_nodes_from_edge(nodes,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_nodes(VirtualMesh::Node::array_type& nodes, 
                              VirtualMesh::Face::index_type i) const
{
  this->mesh_->get_nodes_from_face(nodes,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_nodes(VirtualMesh::Node::array_type& nodes, 
                               VirtualMesh::Elem::index_type i) const
{
  this->mesh_->get_nodes_from_face(nodes,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_nodes(VirtualMesh::Node::array_type& nodes, 
                               VirtualMesh::DElem::index_type i) const
{
  this->mesh_->get_nodes_from_edge(nodes,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_enodes(VirtualMesh::ENode::array_type& enodes, 
                               VirtualMesh::Face::index_type i) const
{
  this->mesh_->get_edges_from_face(enodes,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_enodes(VirtualMesh::ENode::array_type& enodes, 
                               VirtualMesh::Elem::index_type i) const
{
  this->mesh_->get_edges_from_face(enodes,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_enodes(VirtualMesh::ENode::array_type& enodes, 
                               VirtualMesh::DElem::index_type i) const
{
  enodes.resize(1); enodes[0] = static_cast<VirtualMesh::ENode::index_type>(i);
}



template <class MESH>
void 
VTriSurfMesh<MESH>::get_edges(VirtualMesh::Edge::array_type& edges, 
                              VirtualMesh::Node::index_type i) const
{
  this->mesh_->get_edges_from_node(edges,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_edges(VirtualMesh::Edge::array_type& edges, 
                              VirtualMesh::Face::index_type i) const
{
  this->mesh_->get_edges_from_face(edges,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_edges(VirtualMesh::Edge::array_type& edges, 
                              VirtualMesh::Elem::index_type i) const
{
  this->mesh_->get_edges_from_face(edges,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_edges(VirtualMesh::Edge::array_type& edges, 
                               VirtualMesh::DElem::index_type i) const
{
  edges.resize(1); edges[0] = static_cast<VirtualMesh::Edge::index_type>(i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_faces(VirtualMesh::Face::array_type& faces, 
                               VirtualMesh::Node::index_type i) const
{
  this->mesh_->get_faces_from_node(faces,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_faces(VirtualMesh::Face::array_type& faces, 
                               VirtualMesh::Edge::index_type i) const
{
  this->mesh_->get_faces_from_edge(faces,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_faces(VirtualMesh::Face::array_type& faces, 
                               VirtualMesh::Elem::index_type i) const
{
  faces.resize(1); faces[0] = static_cast<VirtualMesh::Face::index_type>(i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_faces(VirtualMesh::Face::array_type& faces, 
                               VirtualMesh::DElem::index_type i) const
{
  this->mesh_->get_faces_from_edge(faces,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_elems(VirtualMesh::Elem::array_type& elems, 
                               VirtualMesh::Node::index_type i) const
{
  this->mesh_->get_faces_from_node(elems,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_elems(VirtualMesh::Elem::array_type& elems, 
                               VirtualMesh::Edge::index_type i) const
{
  this->mesh_->get_faces_from_edge(elems,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_elems(VirtualMesh::Elem::array_type& elems, 
                               VirtualMesh::Face::index_type i) const
{
  elems.resize(1); elems[0] = static_cast<VirtualMesh::Elem::index_type>(i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_elems(VirtualMesh::Elem::array_type& elems, 
                               VirtualMesh::DElem::index_type i) const
{
  this->mesh_->get_faces_from_edge(elems,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_delems(VirtualMesh::DElem::array_type& delems, 
                                VirtualMesh::Node::index_type i) const
{
  this->mesh_->get_edges_from_node(delems,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_delems(VirtualMesh::DElem::array_type& delems, 
                               VirtualMesh::Edge::index_type i) const
{
  delems.resize(1); delems[0] = static_cast<VirtualMesh::DElem::index_type>(i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_delems(VirtualMesh::DElem::array_type& delems, 
                                VirtualMesh::Face::index_type i) const
{
  this->mesh_->get_edges_from_face(delems,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::get_delems(VirtualMesh::DElem::array_type& delems, 
                                VirtualMesh::Elem::index_type i) const
{
  this->mesh_->get_edges_from_face(delems,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::set_nodes(VirtualMesh::Node::array_type& nodes, 
                              VirtualMesh::Elem::index_type i)
{
  this->mesh_->set_nodes_by_elem(nodes,i);
}

template <class MESH>
void 
VTriSurfMesh<MESH>::set_nodes(VirtualMesh::Node::array_type& nodes, 
                              VirtualMesh::Face::index_type i)
{
  this->mesh_->set_nodes_by_elem(nodes,i);
}


#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
template <class MESH>
VirtualMesh::index_type*
VTriSurfMesh<MESH>::
get_elems_pointer() const
{
  if (this->mesh_->faces_.size() == 0) return (0);
   return (&(this->mesh_->faces_[0]));
}

// TODO: Fix this function so it does not need the vector conversion
template <class MESH>
void
VTriSurfMesh<MESH>::insert_node_into_elem(VirtualMesh::Elem::array_type& newelems, 
                                     VirtualMesh::Node::index_type& newnode,
                                     VirtualMesh::Elem::index_type  elem,
                                     Point& point)
{
  // This wrapper fixes class types
  typename MESH::Elem::array_type array;
  typename MESH::Node::index_type index;
  this->mesh_->insert_node_in_face(array,index,typename MESH::Elem::index_type(elem),point);
  this->convert_vector(array,newelems);
  newnode = VirtualMesh::Node::index_type(index);
}
#endif