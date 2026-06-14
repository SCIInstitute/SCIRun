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

/// @file VRegularMeshGradientPointImpl.h
/// @brief X-include implementation of get_gradient_weights(Point) shared by
///        VImageMesh and VLatVolMesh.  Both use a constant inverse Jacobian
///        and do NOT set eg.basis_order in the failure path; they differ only
///        in the name of the element-location call.
///
/// Usage:
///   #define CLASSNAME       VImageMesh          // or VLatVolMesh
///   #define LOCATE_ELEM_COORDS locate_elem      // or elem_locate
///   #include <Core/Datatypes/Legacy/Field/VRegularMeshGradientPointImpl.h>
///   #undef  LOCATE_ELEM_COORDS
///   #undef  CLASSNAME

// NOTE: No include guard — intentionally included multiple times.

template <class MESH>
void
CLASSNAME<MESH>::get_gradient_weights(const Point& point,
                                      VMesh::ElemGradient& eg,
                                      int basis_order) const
{
  double* inverse_jacobian = this->mesh_->inverse_jacobian_;

  VMesh::Elem::index_type elem;
  StackVector<double,3> coords;

  if(!(LOCATE_ELEM_COORDS(elem,coords,point)))
  {
    eg.elem_index = -1;
    return;
  }

  eg.basis_order = basis_order;
  eg.elem_index = elem;

  switch (basis_order)
  {
    case 0:
      return;
    case 1:
      eg.weights.resize(this->basis_->num_linear_derivate_weights());
      this->basis_->get_linear_derivate_weights(coords,&(eg.weights[0]));
      get_nodes_from_elem(eg.node_index,elem);

      eg.inverse_jacobian.resize(9);
      for(size_t i=0; i<9;i++) eg.inverse_jacobian[i] = inverse_jacobian[i];
      eg.num_derivs = this->basis_->num_derivs();
      return;
    case 2:
      eg.weights.resize(this->basis_->num_quadratic_derivate_weights());
      this->basis_->get_quadratic_derivate_weights(coords,&(eg.weights[0]));
      get_nodes_from_elem(eg.node_index,elem);
      get_edges_from_elem(eg.edge_index,elem);

      eg.inverse_jacobian.resize(9);
      for(size_t i=0; i<9;i++) eg.inverse_jacobian[i] = inverse_jacobian[i];
      eg.num_derivs = this->basis_->num_derivs();
      return;
    case 3:
      eg.weights.resize(this->basis_->num_cubic_derivate_weights());
      this->basis_->get_cubic_derivate_weights(coords,&(eg.weights[0]));
      get_nodes_from_elem(eg.node_index,elem);
      eg.num_hderivs = this->basis_->num_hderivs();

      eg.inverse_jacobian.resize(9);
      for(size_t i=0; i<9;i++) eg.inverse_jacobian[i] = inverse_jacobian[i];
      eg.num_derivs = this->basis_->num_derivs();
      return;
  }
  ASSERTFAIL("Gradient of unknown order requested");
}
