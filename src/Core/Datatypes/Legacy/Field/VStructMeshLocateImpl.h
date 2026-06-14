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

/// @file VStructMeshLocateImpl.h
/// @brief X-include implementation of Point-based interpolation/gradient
///        methods shared by VStructQuadSurfMesh and VStructHexVolMesh.
///
/// Usage: define CLASSNAME to the unqualified class name, then #include
/// this file, then #undef CLASSNAME.
///
/// These implementations call the virtual locate(elem, point),
/// get_coords(coords, point, elem), and inv_jacobian(coords, elem, Ji)
/// methods (all resolved through virtual dispatch on `this`).
///
/// Methods provided (all take a Point argument):
///   get_minterpolate_weights(point, ei, basis_order)
///   get_gradient_weights(point, eg, basis_order)
///   get_mgradient_weights(point, eg, basis_order)

// NOTE: No include guard — this file is intentionally included multiple times
// with different values of CLASSNAME.

template <class MESH>
void
CLASSNAME<MESH>::get_minterpolate_weights(const std::vector<Point>& point,
                                          VMesh::MultiElemInterpolate& ei,
                                          int basis_order) const
{
  ei.resize(point.size());

  switch (basis_order)
  {
    case 0:
      {
        for (size_t i=0; i<ei.size();i++)
        {
          VMesh::Elem::index_type elem;
          if(locate(elem,point[i]))
          {
            ei[i].basis_order = basis_order;
            ei[i].elem_index = elem;
          }
          else
          {
            ei[i].basis_order = basis_order;
            ei[i].elem_index  = -1;
          }
        }
      }
      return;
    case 1:
      {
        StackVector<double,3> coords;
        VMesh::Elem::index_type elem;

        for (size_t i=0; i<ei.size();i++)
        {
          ei[i].basis_order = basis_order;
          if(locate(elem,point[i]))
          {
            get_coords(coords,point[i],elem);
            ei[i].elem_index = elem;
            ei[i].weights.resize(this->basis_->num_linear_weights());
            this->basis_->get_linear_weights(coords,&(ei[i].weights[0]));
            this->get_nodes_from_elem(ei[i].node_index,elem);
          }
          else
          {
            ei[i].elem_index  = -1;
          }
        }
      }
      return;
    case 2:
      {
        StackVector<double,3> coords;
        VMesh::Elem::index_type elem;

        for (size_t i=0; i<ei.size();i++)
        {
          ei[i].basis_order = basis_order;
          if(locate(elem,point[i]))
          {
            get_coords(coords,point[i],elem);
            ei[i].elem_index = elem;
            ei[i].weights.resize(this->basis_->num_quadratic_weights());
            this->basis_->get_quadratic_weights(coords,&(ei[i].weights[0]));
            this->get_nodes_from_elem(ei[i].node_index,elem);
            this->get_edges_from_elem(ei[i].edge_index,elem);
          }
          else
          {
            ei[i].elem_index  = -1;
          }
        }
      }
      return;
    case 3:
      {
        StackVector<double,3> coords;
        VMesh::Elem::index_type elem;

        for (size_t i=0; i<ei.size();i++)
        {
          ei[i].basis_order = basis_order;
          if(locate(elem,point[i]))
          {
            get_coords(coords,point[i],elem);
            ei[i].elem_index = elem;
            ei[i].weights.resize(this->basis_->num_cubic_weights());
            this->basis_->get_cubic_weights(coords,&(ei[i].weights[0]));
            this->get_nodes_from_elem(ei[i].node_index,elem);
            ei[i].num_hderivs = this->basis_->num_hderivs();
          }
          else
          {
            ei[i].elem_index  = -1;
          }
        }
      }
      return;
  }
  ASSERTFAIL("Interpolation of unknown order requested");
}


template <class MESH>
void
CLASSNAME<MESH>::get_gradient_weights(const Point& point,
                                      VMesh::ElemGradient& eg,
                                      int basis_order) const
{
  VMesh::Elem::index_type elem;
  StackVector<double,3> coords;

  if(!(locate(elem,point)))
  {
    eg.basis_order = basis_order;
    eg.elem_index = -1;
    return;
  }

  get_coords(coords,point,elem);
  eg.basis_order = basis_order;
  eg.elem_index = elem;

  switch (basis_order)
  {
    case 0:
      return;
    case 1:
      eg.weights.resize(this->basis_->num_linear_derivate_weights());
      this->basis_->get_linear_derivate_weights(coords,&(eg.weights[0]));
      this->get_nodes_from_elem(eg.node_index,elem);

      eg.inverse_jacobian.resize(9);
      inv_jacobian(coords,elem,&(eg.inverse_jacobian[0]));
      eg.num_derivs = this->basis_->num_derivs();
      return;
    case 2:
      eg.weights.resize(this->basis_->num_quadratic_derivate_weights());
      this->basis_->get_quadratic_derivate_weights(coords,&(eg.weights[0]));
      this->get_nodes_from_elem(eg.node_index,elem);
      this->get_edges_from_elem(eg.edge_index,elem);

      eg.inverse_jacobian.resize(9);
      inv_jacobian(coords,elem,&(eg.inverse_jacobian[0]));
      eg.num_derivs = this->basis_->num_derivs();
      return;
    case 3:
      eg.weights.resize(this->basis_->num_cubic_derivate_weights());
      this->basis_->get_cubic_derivate_weights(coords,&(eg.weights[0]));
      this->get_nodes_from_elem(eg.node_index,elem);
      eg.num_hderivs = this->basis_->num_hderivs();

      eg.inverse_jacobian.resize(9);
      inv_jacobian(coords,elem,&(eg.inverse_jacobian[0]));
      eg.num_derivs = this->basis_->num_derivs();
      return;
  }
  ASSERTFAIL("Gradient of unknown order requested");
}


template <class MESH>
void
CLASSNAME<MESH>::get_mgradient_weights(const std::vector<Point>& point,
                                       VMesh::MultiElemGradient& eg,
                                       int basis_order) const
{
  eg.resize(point.size());

  switch (basis_order)
  {
    case 0:
      {
        VMesh::Elem::index_type elem;
        for (size_t i=0; i< point.size(); i++)
        {
          if(locate(elem,point[i]))
          {
            eg[i].basis_order = basis_order;
            eg[i].elem_index = elem;
          }
          else
          {
            eg[i].basis_order = basis_order;
            eg[i].elem_index = -1;
          }
        }
      }
      return;
    case 1:
      {
        VMesh::Elem::index_type elem;
        StackVector<double,3> coords;
        for (size_t i=0; i< point.size(); i++)
        {
          eg[i].basis_order = basis_order;
          if(locate(elem,point[i]))
          {
            eg[i].elem_index = elem;
            get_coords(coords,point[i],elem);
            eg[i].weights.resize(this->basis_->num_linear_derivate_weights());
            this->basis_->get_linear_derivate_weights(coords,&(eg[i].weights[0]));
            this->get_nodes_from_elem(eg[i].node_index,elem);

            eg[i].inverse_jacobian.resize(9);
            inv_jacobian(coords,elem,&(eg[i].inverse_jacobian[0]));
            eg[i].num_derivs = this->basis_->num_derivs();
          }
          else
          {
            eg[i].elem_index = -1;
          }
        }
      }
      return;
    case 2:
      {
        VMesh::Elem::index_type elem;
        StackVector<double,3> coords;
        for (size_t i=0; i< point.size(); i++)
        {
          eg[i].basis_order = basis_order;
          if(locate(elem,point[i]))
          {
            eg[i].elem_index = elem;
            get_coords(coords,point[i],elem);
            eg[i].weights.resize(this->basis_->num_quadratic_derivate_weights());
            this->basis_->get_quadratic_derivate_weights(coords,&(eg[i].weights[0]));
            this->get_nodes_from_elem(eg[i].node_index,elem);
            this->get_edges_from_elem(eg[i].edge_index,elem);

            eg[i].inverse_jacobian.resize(9);
            inv_jacobian(coords,elem,&(eg[i].inverse_jacobian[0]));
            eg[i].num_derivs = this->basis_->num_derivs();
          }
          else
          {
            eg[i].elem_index = -1;
          }

        }
      }
      return;
    case 3:
      {
        VMesh::Elem::index_type elem;
        StackVector<double,3> coords;
        for (size_t i=0; i< point.size(); i++)
        {
          eg[i].basis_order = basis_order;
          if(locate(elem,point[i]))
          {
            eg[i].elem_index = elem;
            get_coords(coords,point[i],elem);
            eg[i].weights.resize(this->basis_->num_cubic_derivate_weights());
            this->basis_->get_cubic_derivate_weights(coords,&(eg[i].weights[0]));
            this->get_nodes_from_elem(eg[i].node_index,elem);
            eg[i].num_hderivs = this->basis_->num_hderivs();

            eg[i].inverse_jacobian.resize(9);
            inv_jacobian(coords,elem,&(eg[i].inverse_jacobian[0]));
            eg[i].num_derivs = this->basis_->num_derivs();
          }
          else
          {
            eg[i].elem_index = -1;
          }
        }
      }
      return;
  }
  ASSERTFAIL("Gradient of unknown order requested");
}
