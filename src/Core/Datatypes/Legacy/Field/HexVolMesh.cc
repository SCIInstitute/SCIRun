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


#include <Core/Datatypes/Legacy/Field/HexVolMesh.h>
#include <Core/Datatypes/Legacy/Field/VVolMesh.h>

/// Only include this class if we included HexVol Support
#if (SCIRUN_HEXVOL_SUPPORT > 0)

using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;

namespace SCIRun {

/// This is the virtual interface to the curve mesh
/// This class lives besides the real mesh class for now and solely provides
/// an interface. In the future however when dynamic compilation is gone
/// this should be put into the HexVolMesh class.
template<class MESH> class VHexVolMesh;

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.
template<class MESH>
class VHexVolMesh : public VVolMesh<MESH> {
public:
  bool is_hexvolmesh() override { return (true); }

  VHexVolMesh(MESH* mesh) : VVolMesh<MESH>(mesh)
  {
    DEBUG_CONSTRUCTOR("VHexVolMesh")
  }
  virtual ~VHexVolMesh()
  {
    DEBUG_DESTRUCTOR("VHexVolMesh")
  }

  bool get_elem(VMesh::Elem::index_type& elem,
                        VMesh::Node::array_type& nodes) const override;
  bool get_delem(VMesh::DElem::index_type& delem,
                         VMesh::Node::array_type& nodes) const override;
  bool get_cell(VMesh::Cell::index_type& cell,
                        VMesh::Node::array_type& nodes) const override;
  bool get_face(VMesh::Face::index_type& face,
                        VMesh::Node::array_type& nodes) const override;
  bool get_edge(VMesh::Edge::index_type& edge,
                        VMesh::Node::array_type& nodes) const override;
};

/// Functions for creating the virtual interface for specific mesh types
/// These are similar to compare maker and only serve to instantiate the class

/// Currently there are only 3 variations of this mesh available
/// 1) linear interpolation
/// 2) quadratic interpolation
/// 3) cubic interpolation

/// Add the LINEAR virtual interface and the meshid for creating it

/// Create virtual interface
VMesh* CreateVHexVolMesh(HexVolMesh<HexTrilinearLgn<Point> >* mesh)
{
  return new VHexVolMesh<HexVolMesh<HexTrilinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID HexVolMesh_MeshID1(HexVolMesh<HexTrilinearLgn<Point> >::type_name(-1),
                  HexVolMesh<HexTrilinearLgn<Point> >::mesh_maker);


/// Add the QUADRATIC virtual interface and the meshid for creating it
#if (SCIRUN_QUADRATIC_SUPPORT > 0)

/// Create virtual interface
VMesh* CreateVHexVolMesh(HexVolMesh<HexTriquadraticLgn<Point> >* mesh)
{
  return new VHexVolMesh<HexVolMesh<HexTriquadraticLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID HexVolMesh_MeshID2(HexVolMesh<HexTriquadraticLgn<Point> >::type_name(-1),
                  HexVolMesh<HexTriquadraticLgn<Point> >::mesh_maker);
#endif


/// Add the CUBIC virtual interface and the meshid for creating it
#if (SCIRUN_CUBIC_SUPPORT > 0)

/// Create virtual interface
VMesh* CreateVHexVolMesh(HexVolMesh<HexTricubicHmt<Point> >* mesh)
{
  return new VHexVolMesh<HexVolMesh<HexTricubicHmt<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID HexVolMesh_MeshID3(HexVolMesh<HexTricubicHmt<Point> >::type_name(-1),
                  HexVolMesh<HexTricubicHmt<Point> >::mesh_maker);
#endif


template <class MESH>
bool
VHexVolMesh<MESH>::get_elem(VMesh::Elem::index_type& elem,
                        VMesh::Node::array_type& nodes) const
{
  return(this->mesh_->get_cell_from_nodes(elem,nodes));
}

template <class MESH>
bool
VHexVolMesh<MESH>::get_delem(VMesh::DElem::index_type& delem,
                         VMesh::Node::array_type& nodes) const
{
  return(this->mesh_->get_face_from_nodes(delem,nodes));
}


template <class MESH>
bool
VHexVolMesh<MESH>::get_cell(VMesh::Cell::index_type& cell,
                        VMesh::Node::array_type& nodes) const
{
  return(this->mesh_->get_cell_from_nodes(cell,nodes));
}

template <class MESH>
bool
VHexVolMesh<MESH>::get_face(VMesh::Face::index_type& face,
                        VMesh::Node::array_type& nodes) const
{
  return(this->mesh_->get_face_from_nodes(face,nodes));
}

template <class MESH>
bool
VHexVolMesh<MESH>::get_edge(VMesh::Edge::index_type& edge,
                        VMesh::Node::array_type& nodes) const
{
  return(this->mesh_->get_edge_from_nodes(edge,nodes));
}




} // namespace SCIRun

#endif
