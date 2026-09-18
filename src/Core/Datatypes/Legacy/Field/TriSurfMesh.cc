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
#include <Core/Datatypes/Legacy/Field/VSurfMesh.h>

using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Thread;

/// Only include this class if we included TriSurf Support
#if (SCIRUN_TRISURF_SUPPORT > 0)

namespace SCIRun {

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.
template<class MESH>
class VTriSurfMesh : public VSurfMesh<MESH> {
public:

  bool is_trisurfmesh() override { return (true); }

  /// constructor and destructor
  VTriSurfMesh(MESH* mesh) : VSurfMesh<MESH>(mesh)
  {
    DEBUG_CONSTRUCTOR("VTriSurfMesh")
  }
  virtual ~VTriSurfMesh()
  {
    DEBUG_DESTRUCTOR("VTriSurfMesh")
  }

  void insert_node_into_elem(VMesh::Elem::array_type& newelems,
                                     VMesh::Node::index_type& newnode,
                                     VMesh::Elem::index_type  elem,
                                     Point& point) override;

  SharedPointer<SearchGridT<typename SCIRun::index_type> > get_elem_search_grid() override { return this->mesh_->elem_grid_; }
  SharedPointer<SearchGridT<typename SCIRun::index_type> > get_node_search_grid() override { return this->mesh_->node_grid_; }
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
