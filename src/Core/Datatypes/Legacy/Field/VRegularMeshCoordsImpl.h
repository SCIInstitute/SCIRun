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

/// @file VRegularMeshCoordsImpl.h
/// @brief X-include implementation of coords-based interpolation/gradient
///        methods shared by VImageMesh, VLatVolMesh, and VScanlineMesh
///        (regular structured meshes with a constant inverse Jacobian).
///
/// Usage: define CLASSNAME to the unqualified class name, then #include
/// this file, then #undef CLASSNAME.
///
/// These implementations access this->mesh_->inverse_jacobian_ directly
/// (the Jacobian is constant for regular/axis-aligned meshes).
///
/// Methods provided (all take coords/elem arguments rather than a Point):
///   get_minterpolate_weights(coords, elem, ei, basis_order)
///   get_gradient_weights(coords, elem, eg, basis_order)
///   get_mgradient_weights(coords, elem, eg, basis_order)

// NOTE: No include guard — this file is intentionally included multiple times
// with different values of CLASSNAME.

template <class MESH>
void
CLASSNAME<MESH>::get_minterpolate_weights(const std::vector<VMesh::coords_type>& coords,
                                          VMesh::Elem::index_type elem,
                                          VMesh::MultiElemInterpolate& ei,
                                          int basis_order) const
{
  ei.resize(coords.size());

  for (size_t i=0; i<coords.size(); i++)
  {
    ei[i].basis_order = basis_order;
    ei[i].elem_index = elem;
  }

  switch (basis_order)
  {
    case 0:
      return;
    case 1:
      for (size_t i=0; i<coords.size(); i++)
      {
        ei[i].weights.resize(this->basis_->num_linear_weights());
        this->basis_->get_linear_weights(coords[i],&(ei[i].weights[0]));
        get_nodes_from_elem(ei[i].node_index,elem);
      }
      return;
    case 2:
      for (size_t i=0; i<coords.size(); i++)
      {
        ei[i].weights.resize(this->basis_->num_quadratic_weights());
        this->basis_->get_quadratic_weights(coords[i],&(ei[i].weights[0]));
        get_nodes_from_elem(ei[i].node_index,elem);
        get_edges_from_elem(ei[i].edge_index,elem);
      }
      return;
    case 3:
      for (size_t i=0; i<coords.size(); i++)
      {
        ei[i].weights.resize(this->basis_->num_cubic_weights());
        this->basis_->get_cubic_weights(coords[i],&(ei[i].weights[0]));
        get_nodes_from_elem(ei[i].node_index,elem);
        ei[i].num_hderivs = this->basis_->num_hderivs();
      }
      return;
  }
  ASSERTFAIL("Interpolation of unknown order requested");
}


template <class MESH>
void
CLASSNAME<MESH>::get_gradient_weights(const VMesh::coords_type& coords,
                                      VMesh::Elem::index_type elem,
                                      VMesh::ElemGradient& eg,
                                      int basis_order) const
{
  double* inverse_jacobian = this->mesh_->inverse_jacobian_;

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


template <class MESH>
void
CLASSNAME<MESH>::get_mgradient_weights(const std::vector<VMesh::coords_type>& coords,
                                       VMesh::Elem::index_type elem,
                                       VMesh::MultiElemGradient& eg,
                                       int basis_order) const
{
  double* inverse_jacobian = this->mesh_->inverse_jacobian_;

  eg.resize(coords.size());
  for (size_t i=0; i< coords.size(); i++)
  {
    eg[i].basis_order = basis_order;
    eg[i].elem_index = elem;
  }

  switch (basis_order)
  {
    case 0:
      return;
    case 1:
      for (size_t i=0; i< coords.size(); i++)
      {
        eg[i].weights.resize(this->basis_->num_linear_derivate_weights());
        this->basis_->get_linear_derivate_weights(coords[i],&(eg[i].weights[0]));
        get_nodes_from_elem(eg[i].node_index,elem);

        eg[i].inverse_jacobian.resize(9);
        for(size_t j=0; j<9;j++) eg[i].inverse_jacobian[j] = inverse_jacobian[j];
        eg[i].num_derivs = this->basis_->num_derivs();
      }
      return;
    case 2:
      for (size_t i=0; i< coords.size(); i++)
      {
        eg[i].weights.resize(this->basis_->num_quadratic_derivate_weights());
        this->basis_->get_quadratic_derivate_weights(coords[i],&(eg[i].weights[0]));
        get_nodes_from_elem(eg[i].node_index,elem);
        get_edges_from_elem(eg[i].edge_index,elem);

        eg[i].inverse_jacobian.resize(9);
        for(size_t j=0; j<9;j++) eg[i].inverse_jacobian[j] = inverse_jacobian[j];
        eg[i].num_derivs = this->basis_->num_derivs();
      }
      return;
    case 3:
      for (size_t i=0; i< coords.size(); i++)
      {
        eg[i].weights.resize(this->basis_->num_cubic_derivate_weights());
        this->basis_->get_cubic_derivate_weights(coords[i],&(eg[i].weights[0]));
        get_nodes_from_elem(eg[i].node_index,elem);
        eg[i].num_hderivs = this->basis_->num_hderivs();

        eg[i].inverse_jacobian.resize(9);
        for(size_t j=0; j<9;j++) eg[i].inverse_jacobian[j] = inverse_jacobian[j];
        eg[i].num_derivs = this->basis_->num_derivs();
      }
      return;
  }
  ASSERTFAIL("Gradient of unknown order requested");
}
