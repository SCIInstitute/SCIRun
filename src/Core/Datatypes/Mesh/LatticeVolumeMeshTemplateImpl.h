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

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#ifndef CORE_DATATYPES_LATVOLMESH_TEMPLATE_IMPL_H
#define CORE_DATATYPES_LATVOLMESH_TEMPLATE_IMPL_H 

//#include <boost/array.hpp>
//
//#include <Core/Utils/Exception.h>
//#include <Core/Basis/Locate.h>
//#include <Core/GeometryPrimitives/Point.h>
//#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/BBox.h>
////#include <Core/Math/MiscMath.h>
//
//#include <Core/Datatypes/Mesh/FieldIterator.h>
//#include <Core/Datatypes/Mesh/Mesh.h>
////#include <Core/Datatypes/VMesh.h>
////#include <Core/Datatypes/FieldRNG.h>
//#include <Core/Datatypes/Mesh/MeshFactory.h> //TODO
//#include <Core/Datatypes/Mesh/VirtualMeshFactory.h>
//
//#include <Core/Datatypes/Mesh/Share.h>

#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>

#include <Core/Datatypes/Mesh/LatticeVolumeMesh.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

template <class Basis>
LatVolMesh<Basis>::LatVolMesh() :
min_i_(0),
  min_j_(0),
  min_k_(0),
  ni_(1),
  nj_(1),
  nk_(1)
{
  //DEBUG_CONSTRUCTOR("LatVolMesh")
  compute_jacobian();

  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  vmesh_ = VirtualMeshFactory::CreateVLatVolMesh(this);   
}

template <class Basis>
LatVolMesh<Basis>::LatVolMesh(const LatVolMesh &copy) :
Mesh5(copy),
  min_i_(copy.min_i_),
  min_j_(copy.min_j_),
  min_k_(copy.min_k_),
  ni_(copy.get_ni()),
  nj_(copy.get_nj()),
  nk_(copy.get_nk()),
  transform_(copy.transform_),
  basis_(copy.basis_)
{
  //DEBUG_CONSTRUCTOR("LatVolMesh")   
  transform_.compute_imat();
  compute_jacobian();  

  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  vmesh_ = VirtualMeshFactory::CreateVLatVolMesh(this);   
}

template <class Basis>
LatVolMesh<Basis>* LatVolMesh<Basis>::clone() const { return new LatVolMesh(*this); }

template <class Basis>
LatVolMesh<Basis>::~LatVolMesh() 
{
  //DEBUG_DESTRUCTOR("LatVolMesh")   
}

template <class Basis>
VirtualMeshHandle LatVolMesh<Basis>::vmesh() const { 
  return vmesh_;
}

template <class Basis>
typename LatVolMesh<Basis>::MeshFacadeHandle LatVolMesh<Basis>::getFacade() const 
{
  return boost::make_shared<VirtualMeshFacade<VirtualMesh>>(vmesh());
}

template <class Basis>
int LatVolMesh<Basis>::basis_order() { return (basis_.polynomial_order()); }

template <class Basis>
LatVolMesh<Basis>::LatVolMesh(size_type i, size_type j, size_type k,
                              const Geometry::Point &min, const Geometry::Point &max)
  : min_i_(0), min_j_(0), min_k_(0),
    ni_(i), nj_(j), nk_(k)
{
  //DEBUG_CONSTRUCTOR("LatVolMesh")   
    
  transform_.pre_scale(Geometry::Vector(1.0 / (i-1.0), 1.0 / (j-1.0), 1.0 / (k-1.0)));
  transform_.pre_scale(max - min);
  transform_.pre_translate(Geometry::Vector(min));
  transform_.compute_imat();
  compute_jacobian();  
  
  //! Initialize the virtual interface when the mesh is created
  vmesh_ = VirtualMeshFactory::CreateVLatVolMesh(this);  
}

template <class Basis>
void LatVolMesh<Basis>::compute_jacobian()
{
  auto J1 = transform_.project(Geometry::Vector(1.0,0.0,0.0)); 
  auto J2 = transform_.project(Geometry::Vector(0.0,1.0,0.0)); 
  auto J3 = transform_.project(Geometry::Vector(0.0,0.0,1.0)); 

  jacobian_[0] = J1.x();
  jacobian_[1] = J1.y();
  jacobian_[2] = J1.z();
  jacobian_[3] = J2.x();
  jacobian_[4] = J2.y();
  jacobian_[5] = J2.z();
  jacobian_[6] = J3.x();
  jacobian_[7] = J3.y();
  jacobian_[8] = J3.z();

    using namespace SCIRun::Core::Basis;
  det_jacobian_ = DetMatrix3x3(jacobian_);
  scaled_jacobian_ = ScaledDetMatrix3x3(jacobian_);
  det_inverse_jacobian_ = InverseMatrix3x3(jacobian_,inverse_jacobian_);
}


}}}

#endif 


