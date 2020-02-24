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


#include <Core/Datatypes/Legacy/Field/CurveMesh.h>
#include <Core/Datatypes/Legacy/Field/VUnstructuredMesh.h>

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Basis;

/// Only include this class if we included Curve Support
#if (SCIRUN_CURVE_SUPPORT > 0)

namespace SCIRun {

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.
template<class MESH>
class VCurveMesh : public VUnstructuredMesh<MESH> {
public:
  virtual bool is_curvemesh()          { return (true); }

  VCurveMesh(MESH* mesh) : VUnstructuredMesh<MESH>(mesh)
  {
    DEBUG_CONSTRUCTOR("VCurveMesh")
  }

  virtual ~VCurveMesh()
  {
    DEBUG_DESTRUCTOR("VCurveMesh")
  }


  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::Edge::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::Elem::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::DElem::index_type i) const;

  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::Node::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::Elem::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::DElem::index_type i) const;

  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Node::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Elem::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::DElem::index_type i) const;

  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::Node::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::Edge::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::DElem::index_type i) const;

  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Node::index_type i) const;
  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Edge::index_type i) const;
  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Elem::index_type i) const;

  virtual void set_nodes(VMesh::Node::array_type&,
                         VMesh::Elem::index_type);

  virtual void set_nodes(VMesh::Node::array_type&,
                         VMesh::Edge::index_type);


  virtual VMesh::index_type* get_elems_pointer() const;
};


/// Functions for creating the virtual interface for specific mesh types
/// These are similar to compare maker and only serve to instantiate the class

/// Currently there are only 3 variations of this mesh available
/// 1) linear interpolation
/// 2) quadratic interpolation
/// 3) cubic interpolation
VMesh* CreateVCurveMesh(CurveMesh<CrvLinearLgn<Point> >* mesh)
{
  return new VCurveMesh<CurveMesh<CrvLinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID CurveMesh_MeshID1(CurveMesh<CrvLinearLgn<Point> >::type_name(-1),
                  CurveMesh<CrvLinearLgn<Point> >::mesh_maker);

#if (SCIRUN_QUADRATIC_SUPPORT > 0)
VMesh* CreateVCurveMesh(CurveMesh<CrvQuadraticLgn<Point> >* mesh)
{
  return new VCurveMesh<CurveMesh<CrvQuadraticLgn<Point> > >(mesh);
}
/// Register class maker, so we can instantiate it
static MeshTypeID CurveMesh_MeshID2(CurveMesh<CrvQuadraticLgn<Point> >::type_name(-1),
                  CurveMesh<CrvQuadraticLgn<Point> >::mesh_maker);
#endif


/// Add the CUBIC virtual interface and the meshid for creating it
#if (SCIRUN_CUBIC_SUPPORT > 0)
VMesh* CreateVCurveMesh(CurveMesh<CrvCubicHmt<Point> >* mesh)
{
  return new VCurveMesh<CurveMesh<CrvCubicHmt<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID CurveMesh_MeshID3(CurveMesh<CrvCubicHmt<Point> >::type_name(-1),
                  CurveMesh<CrvCubicHmt<Point> >::mesh_maker);
#endif

template <class MESH>
void
VCurveMesh<MESH>::get_nodes(VMesh::Node::array_type& nodes,
                            VMesh::Edge::index_type i) const
{
  this->mesh_->get_nodes_from_edge(nodes,i);
}

template <class MESH>
void
VCurveMesh<MESH>::get_nodes(VMesh::Node::array_type& nodes,
                            VMesh::Elem::index_type i) const
{
  this->mesh_->get_nodes_from_edge(nodes,i);
}

template <class MESH>
void
VCurveMesh<MESH>::get_nodes(VMesh::Node::array_type& nodes,
                            VMesh::DElem::index_type i) const
{
  nodes.resize(1); nodes[0] = static_cast<VMesh::Node::index_type>(i);
}

template <class MESH>
void
VCurveMesh<MESH>::get_enodes(VMesh::ENode::array_type& enodes,
                             VMesh::Node::index_type i) const
{
  this->mesh_->get_edges_from_node(enodes,i);
}

template <class MESH>
void
VCurveMesh<MESH>::get_enodes(VMesh::ENode::array_type& enodes,
                             VMesh::Elem::index_type i) const
{
  enodes.resize(1); enodes[0] = static_cast<VMesh::ENode::index_type>(i);
}

template <class MESH>
void
VCurveMesh<MESH>::get_enodes(VMesh::ENode::array_type& enodes,
                             VMesh::DElem::index_type i) const
{
  this->mesh_->get_edges_from_node(enodes,i);
}


template <class MESH>
void
VCurveMesh<MESH>::get_edges(VMesh::Edge::array_type& edges,
                            VMesh::Node::index_type i) const
{
  this->mesh_->get_edges_from_node(edges,i);
}

template <class MESH>
void
VCurveMesh<MESH>::get_edges(VMesh::Edge::array_type& edges,
                            VMesh::Elem::index_type i) const
{
  edges.resize(1); edges[0] = static_cast<VMesh::Edge::index_type>(i);
}

template <class MESH>
void
VCurveMesh<MESH>::get_edges(VMesh::Edge::array_type& edges,
                            VMesh::DElem::index_type i) const
{
  this->mesh_->get_edges_from_node(edges,i);
}

template <class MESH>
void
VCurveMesh<MESH>::get_elems(VMesh::Elem::array_type& elems,
                            VMesh::Node::index_type i) const
{
  this->mesh_->get_edges_from_node(elems,i);
}

template <class MESH>
void
VCurveMesh<MESH>::get_elems(VMesh::Elem::array_type& elems,
                            VMesh::Edge::index_type i) const
{
  elems.resize(1); elems[0] = static_cast<VMesh::Elem::index_type>(i);
}

template <class MESH>
void
VCurveMesh<MESH>::get_elems(VMesh::Elem::array_type& elems,
                            VMesh::DElem::index_type i) const
{
  this->mesh_->get_edges_from_node(elems,i);
}

template <class MESH>
void
VCurveMesh<MESH>::get_delems(VMesh::DElem::array_type& delems,
                             VMesh::Node::index_type i) const
{
  delems.resize(1); delems[0] = static_cast<VMesh::DElem::index_type>(i);
}


template <class MESH>
void
VCurveMesh<MESH>::get_delems(VMesh::DElem::array_type& delems,
                             VMesh::Elem::index_type i) const
{
  this->mesh_->get_nodes_from_edge(delems,i);
}

template <class MESH>
void
VCurveMesh<MESH>::get_delems(VMesh::DElem::array_type& delems,
                             VMesh::Edge::index_type i) const
{
  this->mesh_->get_nodes_from_edge(delems,i);
}

template <class MESH>
void
VCurveMesh<MESH>::set_nodes(VMesh::Node::array_type& nodes,
                            VMesh::Elem::index_type i)
{
  this->mesh_->set_nodes_by_elem(nodes,i);
}

template <class MESH>
void
VCurveMesh<MESH>::set_nodes(VMesh::Node::array_type& nodes,
                            VMesh::Edge::index_type i)
{
  this->mesh_->set_nodes_by_elem(nodes,i);
}

template <class MESH>
VMesh::index_type*
VCurveMesh<MESH>::
get_elems_pointer() const
{
  if (this->mesh_->edges_.size() == 0) return (0);
   return (&(this->mesh_->edges_[0]));
}

} // end namespace


#endif
