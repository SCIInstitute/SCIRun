/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/Datatypes/Legacy/Field/PointCloudMesh.h>
#include <Core/Datatypes/Legacy/Field/VMeshShared.h>

/// Only include this class if we included PointCloud Support
#if (SCIRUN_POINTCLOUD_SUPPORT > 0)

using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;

namespace SCIRun {

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.

template<class MESH>
class VPointCloudMesh : public VMeshShared<MESH> {
public:

  virtual bool is_pointcloudmesh()     { return (true); }

  /// constructor and descructor
  VPointCloudMesh(MESH* mesh) : VMeshShared<MESH>(mesh)
  {
    DEBUG_CONSTRUCTOR("VPointCloudMesh")
  }

  virtual ~VPointCloudMesh()
  {
    DEBUG_DESTRUCTOR("VPointCloudMesh")
  }

  virtual void size(VMesh::Node::size_type& size) const;
  virtual void size(VMesh::ENode::size_type& size) const;
  virtual void size(VMesh::Elem::size_type& size) const;

  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::Elem::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::Node::index_type i) const;

  virtual void get_center(Point &point, VMesh::Node::index_type i) const;
  virtual void get_center(Point &point, VMesh::Elem::index_type i) const;

  virtual void get_centers(Point* points, const VMesh::Node::array_type& array) const;
  virtual void get_centers(Point* points, const VMesh::Elem::array_type& array) const;

  virtual double get_size(VMesh::Node::index_type i) const;
  virtual double get_size(VMesh::Elem::index_type i) const;

  virtual bool locate(VMesh::Node::index_type &i, const Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, const Point &point) const;

  virtual bool get_coords(VMesh::coords_type &coords, const Point &point,
                          VMesh::Elem::index_type i) const;
  virtual void interpolate(Point &p, const VMesh::coords_type &coords,
                           VMesh::Elem::index_type i) const;
  virtual void derivate(VMesh::dpoints_type &p, const
                        VMesh::coords_type &coords,
                        VMesh::Elem::index_type i) const;

  virtual void get_random_point(Point &p, VMesh::Elem::index_type i,
                                FieldRNG &rng) const;

  virtual void set_point(const Point &point, VMesh::Node::index_type i);

  virtual Point* get_points_pointer() const;

  virtual void add_node(const Point &point,VMesh::Node::index_type &i);
  virtual void add_elem(const VMesh::Node::array_type &nodes,
                        VMesh::Elem::index_type &i);


  virtual bool get_neighbor(VMesh::Elem::index_type &neighbor,
                            VMesh::Elem::index_type elem,
                            VMesh::DElem::index_type delem) const;
  virtual bool get_neighbors(VMesh::Elem::array_type &elems,
                             VMesh::Elem::index_type elem,
                             VMesh::DElem::index_type delem) const;
  virtual void get_neighbors(VMesh::Elem::array_type &elems,
                             VMesh::Elem::index_type elem) const;
  virtual void get_neighbors(VMesh::Node::array_type &nodes,
                             VMesh::Node::index_type node) const;


  virtual double det_jacobian(const VMesh::coords_type& coords,
                              VMesh::Elem::index_type idx) const;

  virtual void   jacobian(const VMesh::coords_type& coords,
                          VMesh::Elem::index_type idx,
                          double* J) const;

  virtual double inverse_jacobian(const VMesh::coords_type& coords,
                                  VMesh::Elem::index_type idx,
                                  double* Ji) const;

  virtual void node_reserve(size_t size);
  virtual void elem_reserve(size_t size);
  virtual void resize_nodes(size_t size);
  virtual void resize_elems(size_t size);

  virtual bool find_closest_node(double& pdist, Point& result,
                                 VMesh::Node::index_type &i,
                                 const Point &point) const;

  virtual bool find_closest_node(double& pdist, Point& result,
                                 VMesh::Node::index_type &i,
                                 const Point &point,
                                 double maxdist) const;

  virtual bool find_closest_elem(double& pdist, Point& result,
                                 VMesh::coords_type& coords,
                                 VMesh::Elem::index_type &i,
                                 const Point &point) const;

  virtual bool find_closest_elem(double& pdist, Point& result,
                                 VMesh::coords_type& coords,
                                 VMesh::Elem::index_type &i,
                                 const Point &point,
                                 double maxdist) const;

  virtual bool find_closest_elems(double& pdist, Point& result,
                                  VMesh::Elem::array_type &i,
                                  const Point &point) const;

  virtual void get_interpolate_weights(const Point& point,
                                       VMesh::ElemInterpolate& ei,
                                       int basis_order) const;

  virtual void get_interpolate_weights(const VMesh::coords_type& coords,
                                       VMesh::Elem::index_type elem,
                                       VMesh::ElemInterpolate& ei,
                                       int basis_order) const;

  virtual void get_minterpolate_weights(const std::vector<Point>& point,
                                        VMesh::MultiElemInterpolate& ei,
                                        int basis_order) const;

  virtual void get_minterpolate_weights(const std::vector<VMesh::coords_type>& coords,
                                        VMesh::Elem::index_type elem,
                                        VMesh::MultiElemInterpolate& ei,
                                        int basis_order) const;

  virtual void get_gradient_weights(const Point& point,
                                    VMesh::ElemGradient& eg,
                                    int basis_order) const;

  virtual void get_gradient_weights(const VMesh::coords_type& coords,
                                    VMesh::Elem::index_type elem,
                                    VMesh::ElemGradient& eg,
                                    int basis_order) const;

  virtual void get_mgradient_weights(const std::vector<Point>& point,
                                     VMesh::MultiElemGradient& eg,
                                     int basis_order) const;

  virtual void get_mgradient_weights(const std::vector<VMesh::coords_type>& coords,
                                     VMesh::Elem::index_type elem,
                                     VMesh::MultiElemGradient& eg,
                                     int basis_order) const;

  virtual VMesh::index_type* get_elems_pointer() const;

};

/// Functions for creating the virtual interface for specific mesh types
/// These are similar to compare maker and only serve to instantiate the class

/// Currently there are only 3 variations of this mesh available
/// 1) constant interpolation

VMesh* CreateVPointCloudMesh(PointCloudMesh<ConstantBasis<Point> >* mesh)
{
  return new VPointCloudMesh<PointCloudMesh<ConstantBasis<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID PointCloudMesh_MeshID1(
                  PointCloudMesh<ConstantBasis<Point> >::type_name(-1),
                  PointCloudMesh<ConstantBasis<Point> >::mesh_maker);

template <class MESH>
void
VPointCloudMesh<MESH>::size(VMesh::Node::size_type& sz) const
{
  typename MESH::Node::index_type s;
  this->mesh_->size(s); sz = VMesh::Node::index_type(s);
}

template <class MESH>
void
VPointCloudMesh<MESH>::size(VMesh::ENode::size_type& sz) const
{
  sz = 0;
}

template <class MESH>
void
VPointCloudMesh<MESH>::size(VMesh::Elem::size_type& sz) const
{
  typename MESH::Elem::index_type s;
  this->mesh_->size(s); sz = VMesh::Elem::index_type(s);
}

template <class MESH>
void
VPointCloudMesh<MESH>::get_nodes(VMesh::Node::array_type& nodes,
                                 VMesh::Elem::index_type i) const
{
  nodes.resize(1); nodes[0] = static_cast<VMesh::Node::index_type>(i);
}

template <class MESH>
void
VPointCloudMesh<MESH>::get_elems(VMesh::Elem::array_type& elems,
                                 VMesh::Node::index_type i) const
{
  elems.resize(1); elems[0] = static_cast<VMesh::Elem::index_type>(i);
}


template <class MESH>
void
VPointCloudMesh<MESH>::get_center(Point &p, VMesh::Node::index_type idx) const
{
  p = this->mesh_->points_[idx];
}

template <class MESH>
void
VPointCloudMesh<MESH>::get_center(Point &p, VMesh::Elem::index_type idx) const
{
  p = this->mesh_->points_[idx];
}


template <class MESH>
void
VPointCloudMesh<MESH>::get_centers(Point* points, const VMesh::Node::array_type& array) const
{
  for (size_t j=0; j <array.size(); j++)
  {
    this->mesh_->get_center(points[j],typename MESH::Node::index_type(array[j]));
  }
}

template <class MESH>
void
VPointCloudMesh<MESH>::get_centers(Point* points, const VMesh::Elem::array_type& array) const
{
  for (size_t j=0; j <array.size(); j++)
  {
    this->mesh_->get_center(points[j],typename MESH::Node::index_type(array[j]));
  }
}



template <class MESH>
bool
VPointCloudMesh<MESH>::get_neighbor(VMesh::Elem::index_type& /*neighbor*/,
                                    VMesh::Elem::index_type /*elem*/,
                                    VMesh::DElem::index_type /*delem*/) const
{
  return(false);
}


template <class MESH>
bool
VPointCloudMesh<MESH>::get_neighbors(VMesh::Elem::array_type &array,
                                     VMesh::Elem::index_type /*elem*/,
                                     VMesh::DElem::index_type /*delem*/) const
{
  array.clear(); return (false);
}


template <class MESH>
void
VPointCloudMesh<MESH>::get_neighbors(VMesh::Elem::array_type &array,
                                     VMesh::Elem::index_type /*elem*/) const
{
  array.clear();
}


template <class MESH>
void
VPointCloudMesh<MESH>::get_neighbors(VMesh::Node::array_type &array,
                                     VMesh::Node::index_type /*node*/) const
{
  array.clear();
}


template <class MESH>
double
VPointCloudMesh<MESH>::get_size(VMesh::Node::index_type) const
{
  return (0.0);
}

template <class MESH>
double
VPointCloudMesh<MESH>::get_size(VMesh::Elem::index_type) const
{
  return (0.0);
}



template <class MESH>
bool
VPointCloudMesh<MESH>::locate(VMesh::Node::index_type &vi,
                              const Point &point) const
{
  typename MESH::Node::index_type i;
  bool ret = this->mesh_->locate(i,point);
  vi = static_cast<VMesh::Node::index_type>(i);
  return (ret);
}

template <class MESH>
bool
VPointCloudMesh<MESH>::locate(VMesh::Elem::index_type &vi,
                              const Point &point) const
{
  typename MESH::Elem::index_type i;
  bool ret = this->mesh_->locate(i,point);
  vi = static_cast<VMesh::Elem::index_type>(i);
  return (ret);
}

template <class MESH>
bool
VPointCloudMesh<MESH>::get_coords(VMesh::coords_type &coords,
                                  const Point &point,
                                  VMesh::Elem::index_type i) const
{
  return(this->mesh_->get_coords(coords,point,typename MESH::Elem::index_type(i)));
}

template <class MESH>
void
VPointCloudMesh<MESH>::interpolate(Point &p,
                                   const VMesh::coords_type &coords,
                                   VMesh::Elem::index_type i) const
{
  this->mesh_->interpolate(p,coords,typename MESH::Elem::index_type(i));
}

template <class MESH>
void
VPointCloudMesh<MESH>::derivate(VMesh::dpoints_type &p,
                                const VMesh::coords_type &coords,
                                VMesh::Elem::index_type i) const
{
  this->mesh_->derivate(coords,typename MESH::Elem::index_type(i),p);
}

template <class MESH>
void
VPointCloudMesh<MESH>::set_point(const Point &point, VMesh::Node::index_type i)
{
  this->mesh_->points_[i] = point;
}

template <class MESH>
Point*
VPointCloudMesh<MESH>::get_points_pointer() const
{
  if (this->mesh_->points_.empty())
    return 0;

  return(&(this->mesh_->points_[0]));
}


template <class MESH>
void
VPointCloudMesh<MESH>::add_node(const Point &point, VMesh::Node::index_type &vi)
{
  vi = static_cast<VMesh::Node::index_type>(this->mesh_->add_node(point));
}

template <class MESH>
void
VPointCloudMesh<MESH>::add_elem(const VMesh::Node::array_type &nodes,
                                VMesh::Elem::index_type &vi)
{
  typename MESH::Node::array_type nnodes;
  this->convert_vector(nnodes,nodes);
  vi = static_cast<VMesh::Elem::index_type>(this->mesh_->add_elem(nnodes));
}

template <class MESH>
void
VPointCloudMesh<MESH>::get_random_point(Point &p,
                                        VMesh::Elem::index_type i,
                                        FieldRNG &rng) const
{
  this->mesh_->get_random_point(p,typename MESH::Elem::index_type(i),rng);
}


template <class MESH>
double
VPointCloudMesh<MESH>::det_jacobian(const VMesh::coords_type& /*coords*/,
                                    VMesh::Elem::index_type /*idx*/) const
{
  return (1.0);
}

template <class MESH>
void
VPointCloudMesh<MESH>::jacobian(const VMesh::coords_type& /*coords*/,
                                VMesh::Elem::index_type /*idx*/, double* J) const
{
  J[0] = 1.0;
  J[1] = 0.0;
  J[2] = 0.0;
  J[3] = 0.0;
  J[4] = 1.0;
  J[5] = 0.0;
  J[6] = 0.0;
  J[7] = 0.0;
  J[8] = 1.0;
}

template <class MESH>
double
VPointCloudMesh<MESH>::inverse_jacobian(const VMesh::coords_type& /*coords*/,
                                        VMesh::Elem::index_type /*idx*/,
                                        double* Ji) const
{
  Ji[0] = 1.0;
  Ji[1] = 0.0;
  Ji[2] = 0.0;
  Ji[3] = 0.0;
  Ji[4] = 1.0;
  Ji[5] = 0.0;
  Ji[6] = 0.0;
  Ji[7] = 0.0;
  Ji[8] = 1.0;

  return (1.0);
}


template <class MESH>
void
VPointCloudMesh<MESH>::node_reserve(size_t size)
{
  this->mesh_->node_reserve(size);
}

template <class MESH>
void
VPointCloudMesh<MESH>::elem_reserve(size_t size)
{
  this->mesh_->elem_reserve(size);
}


template <class MESH>
void
VPointCloudMesh<MESH>::resize_nodes(size_t size)
{
  this->mesh_->resize_nodes(size);
}

template <class MESH>
void
VPointCloudMesh<MESH>::resize_elems(size_t size)
{
  this->mesh_->resize_elems(size);
}

template <class MESH>
bool
VPointCloudMesh<MESH>::find_closest_node(double& pdist, Point& result,
                      VMesh::Node::index_type &i, const Point &point) const
{

  return(this->mesh_->find_closest_node(pdist,result,i,point));
}


template <class MESH>
bool
VPointCloudMesh<MESH>::find_closest_node(double& pdist, Point& result,
                      VMesh::Node::index_type &i, const Point &point, double maxdist) const
{

  return(this->mesh_->find_closest_node(pdist,result,i,point,maxdist));
}


template <class MESH>
bool
VPointCloudMesh<MESH>::find_closest_elem(double& pdist, Point& result,
                                         VMesh::coords_type& coords,
                                         VMesh::Elem::index_type &i,
                                         const Point &point) const
{

  return(this->mesh_->find_closest_elem(pdist,result,coords,i,point));
}

template <class MESH>
bool
VPointCloudMesh<MESH>::find_closest_elem(double& pdist, Point& result,
                                         VMesh::coords_type& coords,
                                         VMesh::Elem::index_type &i,
                                         const Point &point,
                                         double maxdist) const
{
  return(this->mesh_->find_closest_elem(pdist,result,coords,i,point,maxdist));
}

template <class MESH>
bool
VPointCloudMesh<MESH>::find_closest_elems(double& pdist, Point& result,
                      VMesh::Elem::array_type &i, const Point &point) const
{

  return(this->mesh_->find_closest_elems(pdist,result,i,point));
}



template <class MESH>
void
VPointCloudMesh<MESH>::
get_interpolate_weights(const Point& point,
                        VMesh::ElemInterpolate& ei,
                        int basis_order) const
{
  typename MESH::Elem::index_type elem;

  if(this->mesh_->locate(elem,point))
  {
    ei.basis_order = basis_order;
    ei.elem_index = elem;
  }
  else
  {
    ei.elem_index  = -1;
    return;
  }

  StackVector<double,3> coords;
  this->mesh_->get_coords(coords,point,elem);

  if (basis_order == 0) return;

  ASSERTFAIL("Interpolation of unknown order requested");
}

template <class MESH>
void
VPointCloudMesh<MESH>::
get_interpolate_weights(const VMesh::coords_type& /*coords*/,
                        VMesh::Elem::index_type elem,
                        VMesh::ElemInterpolate& ei,
                        int basis_order) const
{
  ei.basis_order = basis_order;
  ei.elem_index = elem;

  if (basis_order == 0) return;

  ASSERTFAIL("Interpolation of unknown order requested");
}


template <class MESH>
void
VPointCloudMesh<MESH>::
get_minterpolate_weights(const std::vector<Point>& point,
                         VMesh::MultiElemInterpolate& ei,
                         int basis_order) const
{
  ei.resize(point.size());

  if (basis_order == 0)
  {
    for (size_t i=0; i<ei.size();i++)
    {
      typename MESH::Elem::index_type elem;
      if(this->mesh_->locate(elem,point[i]))
      {
        ei[i].basis_order = basis_order;
        ei[i].elem_index = elem;
      }
      else
      {
        ei[i].elem_index  = -1;
      }
    }
    return;
  }
  ASSERTFAIL("Interpolation of unknown order requested");
}

template <class MESH>
void
VPointCloudMesh<MESH>::
get_minterpolate_weights(const std::vector<VMesh::coords_type>& coords,
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

  if (basis_order == 0) return;

  ASSERTFAIL("Interpolation of unknown order requested");
}


template <class MESH>
void
VPointCloudMesh<MESH>::
get_gradient_weights(const VMesh::coords_type& /*coords*/,
                     VMesh::Elem::index_type elem,
                     VMesh::ElemGradient& eg,
                     int basis_order) const
{
  eg.basis_order = basis_order;
  eg.elem_index = elem;

  if (basis_order == 0) return;

  ASSERTFAIL("Gradient of unknown order requested");
}




template <class MESH>
void
VPointCloudMesh<MESH>::
get_gradient_weights(const Point& point,
                     VMesh::ElemGradient& eg,
                     int basis_order) const
{
  typename MESH::Elem::index_type elem;
  StackVector<double,3> coords;

  if(!(this->mesh_->locate(elem,point)))
  {
    eg.basis_order = basis_order;
    eg.elem_index = -1;
    return;
  }

  this->mesh_->get_coords(coords,point,elem);
  eg.basis_order = basis_order;
  eg.elem_index = elem;

  if (basis_order == 0) return;

  ASSERTFAIL("Gradient of unknown order requested");
}


template <class MESH>
void
VPointCloudMesh<MESH>::
get_mgradient_weights(const std::vector<Point>& point,
                      VMesh::MultiElemGradient& eg,
                      int basis_order) const
{
  eg.resize(point.size());
  if(basis_order==0) return;

  ASSERTFAIL("Gradient of unknown order requested");
}



template <class MESH>
void
VPointCloudMesh<MESH>::
get_mgradient_weights(const std::vector<VMesh::coords_type>& coords,
                      VMesh::Elem::index_type elem,
                      VMesh::MultiElemGradient& eg,
                      int basis_order) const
{
  eg.resize(coords.size());
  for (size_t i=0; i< coords.size(); i++)
  {
    eg[i].basis_order = basis_order;
    eg[i].elem_index =  elem;
  }

  if(basis_order == 0) return;
  ASSERTFAIL("Gradient of unknown order requested");
}

template <class MESH>
VMesh::index_type*
VPointCloudMesh<MESH>::
get_elems_pointer() const
{
  return (0);
}


} // namespace SCIRun

#endif

