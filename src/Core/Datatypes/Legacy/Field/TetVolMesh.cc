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
#include <Core/Datatypes/Legacy/Field/VVolMesh.h>

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Basis;

/// Only include this class if we included TetVol Support
#if (SCIRUN_TETVOL_SUPPORT > 0)

namespace SCIRun {

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.
template<class MESH>
class VTetVolMesh : public VVolMesh<MESH> {
public:
  bool is_tetvolmesh() override { return (true); }

  /// constructor and destructor
  VTetVolMesh(MESH* mesh) : VVolMesh<MESH>(mesh)
  {
    DEBUG_CONSTRUCTOR("VTetVolMesh")
  }
  virtual ~VTetVolMesh()
  {
    DEBUG_DESTRUCTOR("VTetVolMesh")
  }

  void insert_node_into_elem(VMesh::Elem::array_type& newelems,
                                     VMesh::Node::index_type& newnode,
                                     VMesh::Elem::index_type  elem,
                                     Point& point) override;

  double inscribed_circumscribed_radius_metric(VMesh::Elem::index_type idx) const override;
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
