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


#include <Core/Datatypes/Legacy/Field/QuadSurfMesh.h>
#include <Core/Datatypes/Legacy/Field/VUnstructuredMesh.h>

/// Only include this class if we included QuadSurf Support
#if (SCIRUN_QUADSURF_SUPPORT > 0)

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Basis;

namespace SCIRun {

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.
template<class MESH>
class VQuadSurfMesh : public VUnstructuredMesh<MESH> {
public:
  virtual bool is_quadsurfmesh()       { return (true); }

  VQuadSurfMesh(MESH* mesh) : VUnstructuredMesh<MESH>(mesh)
  {
    DEBUG_CONSTRUCTOR("VQuadSurfMesh")
  }
  virtual ~VQuadSurfMesh()
  {
    DEBUG_DESTRUCTOR("VQuadSurfMesh")
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
VMesh* CreateVQuadSurfMesh(QuadSurfMesh<QuadBilinearLgn<Point> >* mesh)
{
  return new VQuadSurfMesh<QuadSurfMesh<QuadBilinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID QuadSurfMesh_MeshID1(QuadSurfMesh<QuadBilinearLgn<Point> >::type_name(-1),
                  QuadSurfMesh<QuadBilinearLgn<Point> >::mesh_maker);


/// Add the QUADRATIC virtual interface and the meshid for creating it
#if (SCIRUN_QUADRATIC_SUPPORT > 0)

/// Create virtual interface
VMesh* CreateVQuadSurfMesh(QuadSurfMesh<QuadBiquadraticLgn<Point> >* mesh)
{
  return new VQuadSurfMesh<QuadSurfMesh<QuadBiquadraticLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID QuadSurfMesh_MeshID2(QuadSurfMesh<QuadBiquadraticLgn<Point> >::type_name(-1),
                  QuadSurfMesh<QuadBiquadraticLgn<Point> >::mesh_maker);
#endif


/// Add the CUBIC virtual interface and the meshid for creating it
#if (SCIRUN_CUBIC_SUPPORT > 0)

/// Create virtual interface
VMesh* CreateVQuadSurfMesh(QuadSurfMesh<QuadBicubicHmt<Point> >* mesh)
{
  return new VQuadSurfMesh<QuadSurfMesh<QuadBicubicHmt<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID QuadSurfMesh_MeshID3(QuadSurfMesh<QuadBicubicHmt<Point> >::type_name(-1),
                  QuadSurfMesh<QuadBicubicHmt<Point> >::mesh_maker);

#endif


template <class MESH>
void
VQuadSurfMesh<MESH>::get_nodes(VMesh::Node::array_type& nodes,
                              VMesh::Edge::index_type i) const
{
  this->mesh_->get_nodes_from_edge(nodes,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_nodes(VMesh::Node::array_type& nodes,
                              VMesh::Face::index_type i) const
{
  this->mesh_->get_nodes_from_face(nodes,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_nodes(VMesh::Node::array_type& nodes,
                               VMesh::Elem::index_type i) const
{
  this->mesh_->get_nodes_from_face(nodes,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_nodes(VMesh::Node::array_type& nodes,
                               VMesh::DElem::index_type i) const
{
  this->mesh_->get_nodes_from_edge(nodes,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_enodes(VMesh::ENode::array_type& enodes,
                               VMesh::Face::index_type i) const
{
  this->mesh_->get_edges_from_face(enodes,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_enodes(VMesh::ENode::array_type& enodes,
                               VMesh::Elem::index_type i) const
{
  this->mesh_->get_edges_from_face(enodes,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_enodes(VMesh::ENode::array_type& enodes,
                               VMesh::DElem::index_type i) const
{
  enodes.resize(1); enodes[0] = static_cast<VMesh::ENode::index_type>(i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_edges(VMesh::Edge::array_type& edges,
                              VMesh::Node::index_type i) const
{
  this->mesh_->get_edges_from_node(edges,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_edges(VMesh::Edge::array_type& edges,
                              VMesh::Face::index_type i) const
{
  this->mesh_->get_edges_from_face(edges,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_edges(VMesh::Edge::array_type& edges,
                              VMesh::Elem::index_type i) const
{
  this->mesh_->get_edges_from_face(edges,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_edges(VMesh::Edge::array_type& edges,
                               VMesh::DElem::index_type i) const
{
  edges.resize(1); edges[0] = static_cast<VMesh::Edge::index_type>(i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_faces(VMesh::Face::array_type& faces,
                               VMesh::Node::index_type i) const
{
  this->mesh_->get_faces_from_node(faces,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_faces(VMesh::Face::array_type& faces,
                               VMesh::Edge::index_type i) const
{
  this->mesh_->get_faces_from_edge(faces,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_faces(VMesh::Face::array_type& faces,
                               VMesh::Elem::index_type i) const
{
  faces.resize(1); faces[0] = static_cast<VMesh::Face::index_type>(i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_faces(VMesh::Face::array_type& faces,
                               VMesh::DElem::index_type i) const
{
  this->mesh_->get_faces_from_edge(faces,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_elems(VMesh::Elem::array_type& elems,
                               VMesh::Node::index_type i) const
{
  this->mesh_->get_faces_from_node(elems,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_elems(VMesh::Elem::array_type& elems,
                               VMesh::Edge::index_type i) const
{
  this->mesh_->get_faces_from_edge(elems,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_elems(VMesh::Elem::array_type& elems,
                               VMesh::Face::index_type i) const
{
  elems.resize(1); elems[0] = static_cast<VMesh::Elem::index_type>(i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_elems(VMesh::Elem::array_type& elems,
                               VMesh::DElem::index_type i) const
{
  this->mesh_->get_faces_from_edge(elems,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_delems(VMesh::DElem::array_type& delems,
                                VMesh::Node::index_type i) const
{
  this->mesh_->get_edges_from_node(delems,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_delems(VMesh::DElem::array_type& delems,
                                VMesh::Edge::index_type i) const
{
  delems.resize(1); delems[0] = static_cast<VMesh::DElem::index_type>(i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_delems(VMesh::DElem::array_type& delems,
                                VMesh::Face::index_type i) const
{
  this->mesh_->get_edges_from_face(delems,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::get_delems(VMesh::DElem::array_type& delems,
                                VMesh::Elem::index_type i) const
{
  this->mesh_->get_edges_from_face(delems,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::set_nodes(VMesh::Node::array_type& nodes,
                              VMesh::Elem::index_type i)
{
  this->mesh_->set_nodes_by_elem(nodes,i);
}

template <class MESH>
void
VQuadSurfMesh<MESH>::set_nodes(VMesh::Node::array_type& nodes,
                              VMesh::Face::index_type i)
{
  this->mesh_->set_nodes_by_elem(nodes,i);
}


template <class MESH>
VMesh::index_type*
VQuadSurfMesh<MESH>::
get_elems_pointer() const
{
  if (this->mesh_->faces_.size() == 0) return (0);
   return (&(this->mesh_->faces_[0]));
}


} // namespace SCIRun

#endif
