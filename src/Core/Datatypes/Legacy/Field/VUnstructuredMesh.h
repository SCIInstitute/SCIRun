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


#ifndef CORE_DATATYPES_VUNSTRUCTUREDMESH_H
#define CORE_DATATYPES_VUNSTRUCTUREDMESH_H

#include <Core/Datatypes/Legacy/Field/VMeshShared.h>

/// Include needed for Windows: declares SCISHARE
#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

template <class MESH>
class SCISHARE VUnstructuredMesh : public VMeshShared<MESH> {
public:
  
  /// Constructor: This will initialize VMeshShared and VMesh
  explicit VUnstructuredMesh(MESH* mesh) :
    VMeshShared<MESH>(mesh)
  {}

  virtual void size(VMesh::Node::size_type& size) const;
  virtual void size(VMesh::ENode::size_type& size) const;
  virtual void size(VMesh::Edge::size_type& size) const;
  virtual void size(VMesh::Face::size_type& size) const;
  virtual void size(VMesh::Cell::size_type& size) const;
  virtual void size(VMesh::Elem::size_type& size) const;
  virtual void size(VMesh::DElem::size_type& size) const;

  virtual void get_center(Core::Geometry::Point &point, VMesh::Node::index_type i) const;
  virtual void get_center(Core::Geometry::Point &point, VMesh::ENode::index_type i) const;
  virtual void get_center(Core::Geometry::Point &point, VMesh::Edge::index_type i) const;
  virtual void get_center(Core::Geometry::Point &point, VMesh::Face::index_type i) const;
  virtual void get_center(Core::Geometry::Point &point, VMesh::Cell::index_type i) const;
  virtual void get_center(Core::Geometry::Point &point, VMesh::Elem::index_type i) const;
  virtual void get_center(Core::Geometry::Point &point, VMesh::DElem::index_type i) const;

  virtual void get_centers(Core::Geometry::Point* points, const VMesh::Node::array_type& array) const;
  virtual void get_centers(Core::Geometry::Point* points, const VMesh::Elem::array_type& array) const;

  virtual double get_size(VMesh::Edge::index_type i) const;
  virtual double get_size(VMesh::Face::index_type i) const;
  virtual double get_size(VMesh::Cell::index_type i) const;
  virtual double get_size(VMesh::Elem::index_type i) const;
  virtual double get_size(VMesh::DElem::index_type i) const;
  
  virtual bool locate(VMesh::Node::index_type &i, const Core::Geometry::Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, const Core::Geometry::Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i,
                      VMesh::coords_type& coords,
                      const Core::Geometry::Point &point) const;

  virtual bool locate(VMesh::Elem::array_type &i, const Core::Geometry::BBox &bbox) const;

  virtual void mlocate(std::vector<VMesh::Node::index_type> &i, const std::vector<Core::Geometry::Point> &point) const;
  virtual void mlocate(std::vector<VMesh::Elem::index_type> &i, const std::vector<Core::Geometry::Point> &point) const;
  
  virtual bool get_coords(VMesh::coords_type &coords, 
                          const Core::Geometry::Point &point, VMesh::Elem::index_type i) const;  
                          
  virtual void interpolate(Core::Geometry::Point &p, const VMesh::coords_type &coords, 
                           VMesh::Elem::index_type i) const;
  virtual void minterpolate(std::vector<Core::Geometry::Point> &p, 
                            const std::vector<VMesh::coords_type> &coords, 
                            VMesh::Elem::index_type i) const;

  virtual void derivate(VMesh::dpoints_type &p, const VMesh::coords_type &coords, 
                        VMesh::Elem::index_type i) const;

  virtual void get_random_point(Core::Geometry::Point &p, VMesh::Elem::index_type i,
                                FieldRNG &rng) const;
  virtual void set_point(const Core::Geometry::Point &point, VMesh::Node::index_type i);
  virtual void set_point(const Core::Geometry::Point &point, VMesh::ENode::index_type i);
  
  virtual Core::Geometry::Point* get_points_pointer() const;
  
  virtual void add_node(const Core::Geometry::Point &point,VMesh::Node::index_type &i);
  virtual void add_enode(const Core::Geometry::Point &point,VMesh::ENode::index_type &i);

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
                             
  virtual void pwl_approx_edge(VMesh::coords_array_type &coords, 
                               VMesh::Elem::index_type ci, 
                                unsigned int which_edge, 
                               unsigned int div_per_unit) const;  

  virtual void pwl_approx_face(VMesh::coords_array2_type &coords, 
                               VMesh::Elem::index_type ci, 
                               unsigned int which_face, 
                               unsigned int div_per_unit) const;
  
  virtual void get_dimensions(VMesh::dimension_type& dim);

  /// Get the jacobian for the global coordinates to local coordinates transformation 
  virtual double det_jacobian(const VMesh::coords_type& coords, 
                              VMesh::Elem::index_type idx) const; 

  virtual void jacobian(const VMesh::coords_type& coords, 
                        VMesh::Elem::index_type idx, 
                        double* J) const; 

  virtual double inverse_jacobian(const VMesh::coords_type& coords, 
                                  VMesh::Elem::index_type idx, 
                                  double* Ji) const;
  
  // The element metrics 
  virtual double scaled_jacobian_metric(const VMesh::Elem::index_type elem) const;
  virtual double jacobian_metric(const VMesh::Elem::index_type elem) const;
  
  
  /// Functions for forwarding call to the basis class of the
  /// mesh
  virtual void node_reserve(size_t size);
  virtual void elem_reserve(size_t size);
  virtual void resize_nodes(size_t size);
  virtual void resize_elems(size_t size);  

  /// Get normals, for surface meshes only
  virtual void get_normal(Core::Geometry::Vector& norm, VMesh::Node::index_type i) const;

  virtual void get_normal(Core::Geometry::Vector& norm, VMesh::coords_type& coords, 
                VMesh::Elem::index_type i, VMesh::DElem::index_type j) const;
  
  /// Get all the information needed for interpolation:
  /// this includes weights and node indices

  virtual void get_interpolate_weights(const Core::Geometry::Point& point, 
                                       VMesh::ElemInterpolate& ei,
                                       int basis_order) const;
  
  virtual void get_interpolate_weights(const VMesh::coords_type& coords, 
                                       VMesh::Elem::index_type elem, 
                                       VMesh::ElemInterpolate& ei,
                                       int basis_order) const;

  virtual void get_minterpolate_weights(const std::vector<Core::Geometry::Point>& point, 
                                        VMesh::MultiElemInterpolate& ei,
                                        int basis_order) const;
  
  virtual void get_minterpolate_weights(const std::vector<VMesh::coords_type>& coords, 
                                        VMesh::Elem::index_type elem, 
                                        VMesh::MultiElemInterpolate& ei,
                                        int basis_order) const;

  virtual void get_gradient_weights(const Core::Geometry::Point& point, 
                                    VMesh::ElemGradient& eg,
                                    int basis_order) const;
                                       
  virtual void get_gradient_weights(const VMesh::coords_type& coords, 
                                    VMesh::Elem::index_type elem, 
                                    VMesh::ElemGradient& eg,
                                    int basis_order) const;

  virtual void get_mgradient_weights(const std::vector<Core::Geometry::Point>& point, 
                                     VMesh::MultiElemGradient& eg,
                                     int basis_order) const;
                                       
  virtual void get_mgradient_weights(const std::vector<VMesh::coords_type>& coords, 
                                     VMesh::Elem::index_type elem, 
                                     VMesh::MultiElemGradient& eg,
                                     int basis_order) const;
                                     
  virtual bool find_closest_node(double& pdist, 
                                 Core::Geometry::Point& result, 
                                 VMesh::Node::index_type &i, 
                                 const Core::Geometry::Point &point) const;

  virtual bool find_closest_node(double& pdist, 
                                 Core::Geometry::Point& result, 
                                 VMesh::Node::index_type &i, 
                                 const Core::Geometry::Point &point,
                                 double maxdist) const;
                                 
  virtual bool find_closest_nodes(std::vector<VMesh::Node::index_type> &nodes, 
                                  const Core::Geometry::Point &point,
                                  double maxdist) const;

  virtual bool find_closest_nodes(std::vector<double> &distances,
                                  std::vector<VMesh::Node::index_type> &nodes, 
                                  const Core::Geometry::Point &point,
                                  double maxdist) const;
                                                                   
  virtual bool find_closest_elem(double& pdist, 
                                 Core::Geometry::Point& result, 
                                 VMesh::coords_type& coords,
                                 VMesh::Elem::index_type &i, 
                                 const Core::Geometry::Point &point) const;

  virtual bool find_closest_elem(double& pdist, 
                                 Core::Geometry::Point& result, 
                                 VMesh::coords_type& coords,
                                 VMesh::Elem::index_type &i, 
                                 const Core::Geometry::Point &point,
                                 double maxdist) const;
                                 
  virtual bool find_closest_elems(double& pdist, Core::Geometry::Point& result, 
                                  VMesh::Elem::array_type &i, 
                                  const Core::Geometry::Point &point) const;

};




template <class MESH>
void 
VUnstructuredMesh<MESH>::
size(VMesh::Node::size_type& sz) const
{
  typename MESH::Node::size_type s; 
  this->mesh_->size(s);
  sz = VMesh::Node::index_type(s);
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
size(VMesh::ENode::size_type& sz) const
{
  typename MESH::Edge::size_type s; 
  this->mesh_->size(s);
  sz = VMesh::ENode::index_type(s);
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
size(VMesh::Edge::size_type& sz) const
{
  typename MESH::Edge::size_type s; 
  this->mesh_->size(s);
  sz = VMesh::Edge::index_type(s);
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
size(VMesh::Face::size_type& sz) const
{
  typename MESH::Face::size_type s; 
  this->mesh_->size(s);
  sz = VMesh::Face::index_type(s);
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
size(VMesh::Cell::size_type& sz) const
{
  typename MESH::Cell::size_type s; 
  this->mesh_->size(s);
  sz = VMesh::Cell::index_type(s);
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
size(VMesh::DElem::size_type& sz) const
{
  typename MESH::DElem::size_type s; 
  this->mesh_->size(s);
  sz = VMesh::DElem::index_type(s);
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
size(VMesh::Elem::size_type& sz) const
{
  typename MESH::Elem::size_type s; 
  this->mesh_->size(s);
  sz = VMesh::Elem::index_type(s);

}



template <class MESH>
bool 
VUnstructuredMesh<MESH>::
locate(VMesh::Node::index_type &idx, const Core::Geometry::Point &point) const
{
  return(this->mesh_->locate_node(idx,point));
}

template <class MESH>
bool 
VUnstructuredMesh<MESH>::
locate(VMesh::Elem::index_type &idx, const Core::Geometry::Point &point) const
{
  return(this->mesh_->locate_elem(idx,point));
}


template <class MESH>
bool 
VUnstructuredMesh<MESH>::
locate(VMesh::Elem::array_type &array, const Core::Geometry::BBox &bbox) const
{
  return(this->mesh_->locate_elems(array,bbox));
}

template <class MESH>
bool 
VUnstructuredMesh<MESH>::
locate(VMesh::Elem::index_type& idx, 
       VMesh::coords_type& coords,
       const Core::Geometry::Point& point) const
{
  return(this->mesh_->locate_elem(idx,coords,point));
}


template <class MESH>
void 
VUnstructuredMesh<MESH>::
mlocate(std::vector<VMesh::Node::index_type> &idx, const std::vector<Core::Geometry::Point> &point) const
{
  idx.resize(point.size());
  for (size_t i=0; i<point.size(); i++)
  {
    if(!(this->mesh_->locate_node(idx[i],point[i]))) idx[i] = -1;
  }
}

template <class MESH>
void 
VUnstructuredMesh<MESH>::
mlocate(std::vector<VMesh::Elem::index_type> &idx, const std::vector<Core::Geometry::Point> &point) const
{
  idx.resize(point.size());
  for (size_t i=0; i<point.size(); i++)
  {
    if(!(this->mesh_->locate_elem(idx[i],point[i]))) idx[i] = -1;
  }
}

template <class MESH>
bool 
VUnstructuredMesh<MESH>::
get_coords(VMesh::coords_type& coords, 
           const Core::Geometry::Point &point, 
           VMesh::Elem::index_type i) const
{
  return(this->mesh_->get_coords(coords,point,typename MESH::Elem::index_type(i)));
}  
  
template <class MESH>
void 
VUnstructuredMesh<MESH>::
interpolate(Core::Geometry::Point &p, 
            const VMesh::coords_type& coords, 
            VMesh::Elem::index_type i) const
{
  this->mesh_->interpolate(p,coords,typename MESH::Elem::index_type(i));
}

template <class MESH>
void 
VUnstructuredMesh<MESH>::
minterpolate(std::vector<Core::Geometry::Point> &p, 
             const std::vector<VMesh::coords_type>& coords, 
             VMesh::Elem::index_type idx) const
{
  p.resize(coords.size());
  for (size_t i=0; i<coords.size(); i++)
    this->mesh_->interpolate(p[i],coords[i],typename MESH::Elem::index_type(idx));
}

template <class MESH>
void 
VUnstructuredMesh<MESH>::
derivate(VMesh::dpoints_type &p, 
         const VMesh::coords_type &coords, 
         VMesh::Elem::index_type i) const
{
  this->mesh_->derivate(coords,typename MESH::Elem::index_type(i),p);
}

template <class MESH>
bool 
VUnstructuredMesh<MESH>::
get_neighbor(VMesh::Elem::index_type &neighbor, 
             VMesh::Elem::index_type elem, 
             VMesh::DElem::index_type delem) const
{
  return(this->mesh_->get_elem_neighbor(neighbor,elem,delem));
}


template <class MESH>
bool 
VUnstructuredMesh<MESH>::
get_neighbors(VMesh::Elem::array_type &array, 
              VMesh::Elem::index_type elem, 
              VMesh::DElem::index_type delem) const
{
  return(this->mesh_->get_elem_neighbors(array,elem,delem));
}

template <class MESH>
void 
VUnstructuredMesh<MESH>::
get_neighbors(VMesh::Elem::array_type &array, 
              VMesh::Elem::index_type idx) const
{
  this->mesh_->get_elem_neighbors(array,idx);
}

template <class MESH>
void 
VUnstructuredMesh<MESH>::
get_neighbors(VMesh::Node::array_type &array, 
              VMesh::Node::index_type idx) const
{
  this->mesh_->get_node_neighbors(array,idx);
}


template <class MESH>
void 
VUnstructuredMesh<MESH>::pwl_approx_edge(VMesh::coords_array_type &coords, 
                                         VMesh::Elem::index_type /*ci*/, 
                                         unsigned int which_edge,
                                         unsigned int div_per_unit) const
{
  this->basis_->approx_edge(which_edge, div_per_unit, coords);
}

template <class MESH>
void 
VUnstructuredMesh<MESH>::pwl_approx_face(VMesh::coords_array2_type &coords, 
                                         VMesh::Elem::index_type /*ci*/, 
                                         unsigned int which_face,
                                         unsigned int div_per_unit) const
{
  this->basis_->approx_face(which_face, div_per_unit, coords);
}


template <class MESH>
void 
VUnstructuredMesh<MESH>::
get_dimensions(VMesh::dimension_type& dim)
{
  dim.resize(1);
  typename MESH::Node::size_type sz;
  this->mesh_->size(sz);
  dim[0] = sz;
}

template <class MESH>
double 
VUnstructuredMesh<MESH>::
det_jacobian(const VMesh::coords_type& coords, 
             VMesh::Elem::index_type idx) const
{
  return(this->mesh_->det_jacobian(coords,typename MESH::Elem::index_type(idx)));
}


template <class MESH>
void
VUnstructuredMesh<MESH>::
jacobian(const VMesh::coords_type& coords, 
         VMesh::Elem::index_type idx, 
         double* J) const
{
  this->mesh_->jacobian(coords,typename MESH::Elem::index_type(idx),J);
}


template <class MESH>
double
VUnstructuredMesh<MESH>::
inverse_jacobian(const VMesh::coords_type& coords, 
                 VMesh::Elem::index_type idx, 
                 double* Ji) const
{
  return(this->mesh_->inverse_jacobian(coords,typename MESH::Elem::index_type(idx),Ji));
}



template <class MESH>
double
VUnstructuredMesh<MESH>::
scaled_jacobian_metric(VMesh::Elem::index_type idx) const
{
  return(this->mesh_->scaled_jacobian_metric(typename MESH::Elem::index_type(idx)));
}
                                                            
template <class MESH>
double
VUnstructuredMesh<MESH>::
jacobian_metric(VMesh::Elem::index_type idx) const
{
  return(this->mesh_->jacobian_metric(typename MESH::Elem::index_type(idx)));
}
                                       

template <class MESH>
void
VUnstructuredMesh<MESH>::
node_reserve(size_t size)
{
  this->mesh_->node_reserve(size);
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
elem_reserve(size_t size)
{
  this->mesh_->elem_reserve(size);
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
resize_nodes(size_t size)
{
  this->mesh_->resize_nodes(size);
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
resize_elems(size_t size)
{
  this->mesh_->resize_elems(size);
}

template <class MESH>
double
VUnstructuredMesh<MESH>::
get_size(VMesh::Edge::index_type i) const
{
  return (this->mesh_->get_size(typename MESH::Edge::index_type(i)));
}

template <class MESH>
double
VUnstructuredMesh<MESH>::
get_size(VMesh::Face::index_type i) const
{
  return (this->mesh_->get_size(typename MESH::Face::index_type(i)));
}

template <class MESH>
double
VUnstructuredMesh<MESH>::
get_size(VMesh::Cell::index_type i) const
{
  return (this->mesh_->get_size(typename MESH::Cell::index_type(i)));
}

template <class MESH>
double
VUnstructuredMesh<MESH>::
get_size(VMesh::Elem::index_type i) const
{
  return (this->mesh_->get_size(typename MESH::Elem::index_type(i)));
}

template <class MESH>
double
VUnstructuredMesh<MESH>::
get_size(VMesh::DElem::index_type /*i*/) const
{
  return (0.0);
}


template <class MESH>
void
VUnstructuredMesh<MESH>::
get_center(Core::Geometry::Point &p, VMesh::Node::index_type idx) const
{
  p = this->mesh_->points_[idx]; 
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
get_center(Core::Geometry::Point &p, VMesh::ENode::index_type idx) const
{
  if (this->num_enodes_per_elem_)
  {
    this->basis_->get_node_value(p,static_cast<VMesh::index_type>(idx));
  }
  else
  {
    this->mesh_->get_center(p, typename MESH::Edge::index_type(idx));
  }
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
get_center(Core::Geometry::Point &p, VMesh::Edge::index_type idx) const
{
  this->mesh_->get_center(p, typename MESH::Edge::index_type(idx));
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
get_center(Core::Geometry::Point &p, VMesh::Face::index_type idx) const
{
  this->mesh_->get_center(p, typename MESH::Face::index_type(idx));
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
get_center(Core::Geometry::Point &p, VMesh::Cell::index_type idx) const
{
  this->mesh_->get_center(p, typename MESH::Cell::index_type(idx));
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
get_center(Core::Geometry::Point &p, VMesh::Elem::index_type idx) const
{
  this->mesh_->get_center(p, typename MESH::Elem::index_type(idx));
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
get_center(Core::Geometry::Point &p, VMesh::DElem::index_type idx) const
{
  this->mesh_->get_center(p, typename MESH::DElem::index_type(idx));
}

template <class MESH>
void
VUnstructuredMesh<MESH>::get_centers(Core::Geometry::Point* points, const VMesh::Node::array_type& array) const
{
  for (size_t j=0; j <array.size(); j++)
  {
    this->mesh_->get_center(points[j],typename MESH::Node::index_type(array[j]));
  }
}                                     
 
template <class MESH>
void
VUnstructuredMesh<MESH>::get_centers(Core::Geometry::Point* points, const VMesh::Elem::array_type& array) const
{
  for (size_t j=0; j <array.size(); j++)
  {
    this->mesh_->get_center(points[j],typename MESH::Node::index_type(array[j]));
  }
} 



template <class MESH>
void 
VUnstructuredMesh<MESH>::
set_point(const Core::Geometry::Point &point, VMesh::Node::index_type i)
{
  this->mesh_->points_[i] = point;
}

template <class MESH>
void 
VUnstructuredMesh<MESH>::
set_point(const Core::Geometry::Point &point, VMesh::ENode::index_type i)
{
  if (this->num_enodes_per_elem_)
  {
    if (this->mesh_->edges_.size() != this->basis_->size_node_values()) 
    {
      this->basis_->resize_node_values(this->mesh_->edges_.size());
    }
    this->basis_->set_node_value(point,i);
  }
  else
  {
    ASSERTFAIL("CurveMesh does not contain Lagrangian nodes that can be edited");
  }
}

template <class MESH>
Core::Geometry::Point*
VUnstructuredMesh<MESH>::
get_points_pointer() const
{
  if (this->mesh_->points_.size() == 0) return (0);
   return (&(this->mesh_->points_[0]));
}

template <class MESH>
void 
VUnstructuredMesh<MESH>::
add_node(const Core::Geometry::Point &point, VMesh::Node::index_type &vi)
{
  vi = static_cast<VMesh::Node::index_type>(this->mesh_->add_point(point));
}  
  
template <class MESH>
void 
VUnstructuredMesh<MESH>::
add_enode(const Core::Geometry::Point &point, VMesh::ENode::index_type &vi)
{
  vi = static_cast<VMesh::ENode::index_type>(this->basis_->size_node_values());
  this->basis_->add_node_value(point);
}    
  
template <class MESH>
void 
VUnstructuredMesh<MESH>::
add_elem(const VMesh::Node::array_type &nodes, 
         VMesh::Elem::index_type &vi)
{
  vi = static_cast<VMesh::Elem::index_type>(this->mesh_->add_elem(nodes));
}  


template <class MESH>
void 
VUnstructuredMesh<MESH>::
get_random_point(Core::Geometry::Point &p, 
                 VMesh::Elem::index_type i, 
                 FieldRNG &rng) const
{
  this->mesh_->get_random_point(p,typename MESH::Elem::index_type(i),rng);
}

template <class MESH>
void 
VUnstructuredMesh<MESH>::get_normal(Core::Geometry::Vector& norm, VMesh::Node::index_type i) const
{
  this->mesh_->get_normal(norm,typename MESH::Node::index_type(i));
}

template <class MESH>
void 
VUnstructuredMesh<MESH>::get_normal(Core::Geometry::Vector& norm, VMesh::coords_type& coords, 
  VMesh::Elem::index_type i, VMesh::DElem::index_type j) const
{
  this->mesh_->get_normal(norm,coords,i,j);
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
get_interpolate_weights(const Core::Geometry::Point& point, 
                        VMesh::ElemInterpolate& ei,
                        int basis_order) const
{
  typename MESH::Elem::index_type elem = ei.elem_index;
  
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
  
  switch (basis_order)
  {
    case 0:
      return;
    case 1:
      ei.weights.resize(this->basis_->num_linear_weights());
      this->basis_->get_linear_weights(coords,&(ei.weights[0]));
      this->mesh_->get_nodes_from_elem(ei.node_index,elem);
      return;
    case 2:
      ei.weights.resize(this->basis_->num_quadratic_weights());
      this->basis_->get_quadratic_weights(coords,&(ei.weights[0]));
      this->mesh_->get_nodes_from_elem(ei.node_index,elem);
      this->mesh_->get_edges_from_elem(ei.edge_index,elem);
      return;
    case 3:   
      ei.weights.resize(this->basis_->num_cubic_weights());
      this->basis_->get_cubic_weights(coords,&(ei.weights[0]));
      this->mesh_->get_nodes_from_elem(ei.node_index,elem);
      ei.num_hderivs = this->basis_->num_hderivs();
      return;
  }
  ASSERTFAIL("Interpolation of unknown order requested");
}

template <class MESH>
void
VUnstructuredMesh<MESH>::
get_interpolate_weights(const VMesh::coords_type& coords, 
                        VMesh::Elem::index_type elem, 
                        VMesh::ElemInterpolate& ei,
                        int basis_order) const
{
  ei.basis_order = basis_order;
  ei.elem_index = elem;
  
  switch (basis_order)
  {
    case 0:
      return;
    case 1:
      ei.weights.resize(this->basis_->num_linear_weights());
      this->basis_->get_linear_weights(coords,&(ei.weights[0]));
      this->mesh_->get_nodes_from_elem(ei.node_index,elem);
      return;
    case 2:
      ei.weights.resize(this->basis_->num_quadratic_weights());
      this->basis_->get_quadratic_weights(coords,&(ei.weights[0]));
      this->mesh_->get_nodes_from_elem(ei.node_index,elem);
      this->mesh_->get_edges_from_elem(ei.edge_index,elem);
      return;
    case 3:   
      ei.weights.resize(this->basis_->num_cubic_weights());
      this->basis_->get_cubic_weights(coords,&(ei.weights[0]));
      this->mesh_->get_nodes_from_elem(ei.node_index,elem);
      ei.num_hderivs = this->basis_->num_hderivs();
      return;
  }
  ASSERTFAIL("Interpolation of unknown order requested");
}
                                                     

template <class MESH>
void
VUnstructuredMesh<MESH>::
get_minterpolate_weights(const std::vector<Core::Geometry::Point>& point, 
                         VMesh::MultiElemInterpolate& ei,
                         int basis_order) const
{
  ei.resize(point.size());
  typename MESH::Elem::index_type elem;
  
  switch (basis_order)
  {
    case 0:
      {
        for (size_t i=0; i<ei.size();i++)
        {
          if (i == 0) elem = ei[0].elem_index;
          if(this->mesh_->locate(elem,point[i]))
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
        for (size_t i=0; i<ei.size();i++)
        {
          if (i == 0) elem = ei[0].elem_index;
          if(this->mesh_->locate(elem,point[i]))
          {
            this->mesh_->get_coords(coords,point[i],elem);
            ei[i].basis_order = basis_order;
            ei[i].elem_index = elem;
            ei[i].weights.resize(this->basis_->num_linear_weights());
            this->basis_->get_linear_weights(coords,&(ei[i].weights[0]));
            this->mesh_->get_nodes_from_elem(ei[i].node_index,elem);
          }
          else
          {
            ei[i].basis_order = basis_order;
            ei[i].elem_index  = -1;  
          }      
        }
      }
      return;
    case 2:
      {
        StackVector<double,3> coords;
        
        for (size_t i=0; i<ei.size();i++)
        {
          if (i == 0) elem = ei[0].elem_index;
          if(this->mesh_->locate(elem,point[i]))
          {
            this->mesh_->get_coords(coords,point[i],elem);
            ei[i].basis_order = basis_order;
            ei[i].elem_index = elem;
            ei[i].weights.resize(this->basis_->num_quadratic_weights());
            this->basis_->get_quadratic_weights(coords,&(ei[i].weights[0]));
            this->mesh_->get_nodes_from_elem(ei[i].node_index,elem);
            this->mesh_->get_edges_from_elem(ei[i].edge_index,elem);
          }
          else
          {
            ei[i].basis_order = basis_order;
            ei[i].elem_index  = -1;  
          }      
        }
      }
      return;    
    case 3:   
      {
        StackVector<double,3> coords;
        
        for (size_t i=0; i<ei.size();i++)
        {
          if (i == 0) elem = ei[0].elem_index;
          if(this->mesh_->locate(elem,point[i]))
          {
            this->mesh_->get_coords(coords,point[i],elem);
            ei[i].basis_order = basis_order;
            ei[i].elem_index = elem;
            ei[i].weights.resize(this->basis_->num_cubic_weights());
            this->basis_->get_cubic_weights(coords,&(ei[i].weights[0]));
            this->mesh_->get_nodes_from_elem(ei[i].node_index,elem);
            ei[i].num_hderivs = this->basis_->num_hderivs();
          }
          else
          {
            ei[i].basis_order = basis_order;
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
VUnstructuredMesh<MESH>::
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
  
  switch (basis_order)
  {
    case 0:
      return;
    case 1:
      for (size_t i=0; i<coords.size(); i++)
      {
        ei[i].weights.resize(this->basis_->num_linear_weights());
        this->basis_->get_linear_weights(coords[i],&(ei[i].weights[0]));
        this->mesh_->get_nodes_from_elem(ei[i].node_index,elem);
      }
      return;
    case 2:
      for (size_t i=0; i<coords.size(); i++)
      {
        ei[i].weights.resize(this->basis_->num_quadratic_weights());
        this->basis_->get_quadratic_weights(coords[i],&(ei[i].weights[0]));
        this->mesh_->get_nodes_from_elem(ei[i].node_index,elem);
        this->mesh_->get_edges_from_elem(ei[i].edge_index,elem);
      }
      return;
    case 3:   
      for (size_t i=0; i<coords.size(); i++)
      {
        ei[i].weights.resize(this->basis_->num_cubic_weights());
        this->basis_->get_cubic_weights(coords[i],&(ei[i].weights[0]));
        this->mesh_->get_nodes_from_elem(ei[i].node_index,elem);
        ei[i].num_hderivs = this->basis_->num_hderivs();
      }
      return;
  }
  ASSERTFAIL("Interpolation of unknown order requested");
}


template <class MESH>
void
VUnstructuredMesh<MESH>::
get_gradient_weights(const VMesh::coords_type& coords, 
                     VMesh::Elem::index_type elem, 
                     VMesh::ElemGradient& eg,
                     int basis_order) const
{
  eg.basis_order = basis_order;
  eg.elem_index = elem;
    
  switch (basis_order)
  {
    case 0:
      return;
    case 1:
      eg.weights.resize(this->basis_->num_linear_derivate_weights());
      this->basis_->get_linear_derivate_weights(coords,&(eg.weights[0]));
      this->mesh_->get_nodes_from_elem(eg.node_index,elem);

      eg.inverse_jacobian.resize(9);
      this->mesh_->inverse_jacobian(coords,typename MESH::Elem::index_type(elem),&(eg.inverse_jacobian[0]));
      eg.num_derivs = this->basis_->num_derivs();
      return;
    case 2:
      eg.weights.resize(this->basis_->num_quadratic_derivate_weights());
      this->basis_->get_quadratic_derivate_weights(coords,&(eg.weights[0]));
      this->mesh_->get_nodes_from_elem(eg.node_index,elem);
      this->mesh_->get_edges_from_elem(eg.edge_index,elem);

      eg.inverse_jacobian.resize(9);
      this->mesh_->inverse_jacobian(coords,typename MESH::Elem::index_type(elem),&(eg.inverse_jacobian[0]));
      eg.num_derivs = this->basis_->num_derivs();
      return;
    case 3:
      eg.weights.resize(this->basis_->num_cubic_derivate_weights());
      this->basis_->get_cubic_derivate_weights(coords,&(eg.weights[0]));
      this->mesh_->get_nodes_from_elem(eg.node_index,elem);
      eg.num_hderivs = this->basis_->num_hderivs();

      eg.inverse_jacobian.resize(9);
      this->mesh_->inverse_jacobian(coords,typename MESH::Elem::index_type(elem),&(eg.inverse_jacobian[0]));
      eg.num_derivs = this->basis_->num_derivs();
      return;
  }
  ASSERTFAIL("Gradient of unknown order requested");
}




template <class MESH>
void
VUnstructuredMesh<MESH>::
get_gradient_weights(const Core::Geometry::Point& point, 
                     VMesh::ElemGradient& eg,
                     int basis_order) const
{
  typename MESH::Elem::index_type elem;
  StackVector<double,3> coords;

  eg.basis_order = basis_order;

  if(!(this->mesh_->locate(elem,point)))
  {
    eg.elem_index = -1;
    return;
  }
  
  this->mesh_->get_coords(coords,point,elem);
  eg.basis_order = basis_order;
  eg.elem_index = elem;
    
  switch (basis_order)
  {
    case 0:
      return;
    case 1:
      eg.weights.resize(this->basis_->num_linear_derivate_weights());
      this->basis_->get_linear_derivate_weights(coords,&(eg.weights[0]));
      this->mesh_->get_nodes_from_elem(eg.node_index,elem);

      eg.inverse_jacobian.resize(9);
      this->mesh_->inverse_jacobian(coords,typename MESH::Elem::index_type(elem),&(eg.inverse_jacobian[0]));
      eg.num_derivs = this->basis_->num_derivs();
      return;
    case 2:
      eg.weights.resize(this->basis_->num_quadratic_derivate_weights());
      this->basis_->get_quadratic_derivate_weights(coords,&(eg.weights[0]));
      this->mesh_->get_nodes_from_elem(eg.node_index,elem);
      this->mesh_->get_edges_from_elem(eg.edge_index,elem);

      eg.inverse_jacobian.resize(9);
      this->mesh_->inverse_jacobian(coords,typename MESH::Elem::index_type(elem),&(eg.inverse_jacobian[0]));
      eg.num_derivs = this->basis_->num_derivs();
      return;
    case 3:
      eg.weights.resize(this->basis_->num_cubic_derivate_weights());
      this->basis_->get_cubic_derivate_weights(coords,&(eg.weights[0]));
      this->mesh_->get_nodes_from_elem(eg.node_index,elem);
      eg.num_hderivs = this->basis_->num_hderivs();

      eg.inverse_jacobian.resize(9);
      this->mesh_->inverse_jacobian(coords,typename MESH::Elem::index_type(elem),&(eg.inverse_jacobian[0]));
      eg.num_derivs = this->basis_->num_derivs();
      return;
  }
  ASSERTFAIL("Gradient of unknown order requested");
}


template <class MESH>
void
VUnstructuredMesh<MESH>::
get_mgradient_weights(const std::vector<Core::Geometry::Point>& point, 
                      VMesh::MultiElemGradient& eg,
                      int basis_order) const
{
  eg.resize(point.size());
  
  switch (basis_order)
  {
    case 0:
      {
        typename MESH::Elem::index_type elem;
        for (size_t i=0; i< point.size(); i++)
        {
          if(this->mesh_->locate(elem,point[i]))
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
        typename MESH::Elem::index_type elem;
        StackVector<double,3> coords;
        for (size_t i=0; i< point.size(); i++)
        {
          eg[i].basis_order = basis_order;            
          if(this->mesh_->locate(elem,point[i]))
          { 
            eg[i].elem_index = elem;
            this->mesh_->get_coords(coords,point[i],elem);
            eg[i].weights.resize(this->basis_->num_linear_derivate_weights());
            this->basis_->get_linear_derivate_weights(coords,&(eg[i].weights[0]));
            this->mesh_->get_nodes_from_elem(eg[i].node_index,elem);

            eg[i].inverse_jacobian.resize(9);
            this->mesh_->inverse_jacobian(coords,typename MESH::Elem::index_type(elem),&(eg[i].inverse_jacobian[0]));
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
        typename MESH::Elem::index_type elem;
        StackVector<double,3> coords;
        for (size_t i=0; i< point.size(); i++)
        {
          eg[i].basis_order = basis_order;            
          if(this->mesh_->locate(elem,point[i]))
          {      
            eg[i].elem_index = elem;
            this->mesh_->get_coords(coords,point[i],elem);
            eg[i].weights.resize(this->basis_->num_quadratic_derivate_weights());
            this->basis_->get_quadratic_derivate_weights(coords,&(eg[i].weights[0]));
            this->mesh_->get_nodes_from_elem(eg[i].node_index,elem);
            this->mesh_->get_edges_from_elem(eg[i].edge_index,elem);

            eg[i].inverse_jacobian.resize(9);
            this->mesh_->inverse_jacobian(coords,typename MESH::Elem::index_type(elem),&(eg[i].inverse_jacobian[0]));
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
        typename MESH::Elem::index_type elem;
        StackBasedVector<double,3> coords;
        for (size_t i=0; i< point.size(); i++)
        {
          eg[i].basis_order = basis_order;            
          if(this->mesh_->locate(elem,point[i]))
          {      
            eg[i].elem_index = elem;
            this->mesh_->get_coords(coords,point[i],elem);
            eg[i].weights.resize(this->basis_->num_cubic_derivate_weights());
            this->basis_->get_cubic_derivate_weights(coords,&(eg[i].weights[0]));
            this->mesh_->get_nodes_from_elem(eg[i].node_index,elem);
            eg[i].num_hderivs = this->basis_->num_hderivs();

            eg[i].inverse_jacobian.resize(9);
            this->mesh_->inverse_jacobian(coords,typename MESH::Elem::index_type(elem),&(eg[i].inverse_jacobian[0]));
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



template <class MESH>
void
VUnstructuredMesh<MESH>::
get_mgradient_weights(const std::vector<VMesh::coords_type>& coords, 
                      VMesh::Elem::index_type elem, 
                      VMesh::MultiElemGradient& eg,
                      int basis_order) const
{
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
        this->mesh_->get_nodes_from_elem(eg[i].node_index,elem);

        eg[i].inverse_jacobian.resize(9);
        this->mesh_->inverse_jacobian(coords[i],typename MESH::Elem::index_type(elem),&(eg[i].inverse_jacobian[0]));
        eg[i].num_derivs = this->basis_->num_derivs();
      }
      return;
    case 2:
      for (size_t i=0; i< coords.size(); i++)
      {
        eg[i].weights.resize(this->basis_->num_quadratic_derivate_weights());
        this->basis_->get_quadratic_derivate_weights(coords[i],&(eg[i].weights[0]));
        this->mesh_->get_nodes_from_elem(eg[i].node_index,elem);
        this->mesh_->get_edges_from_elem(eg[i].edge_index,elem);

        eg[i].inverse_jacobian.resize(9);
        this->mesh_->inverse_jacobian(coords[i],typename MESH::Elem::index_type(elem),&(eg[i].inverse_jacobian[0]));
        eg[i].num_derivs = this->basis_->num_derivs();
      }
      return;
    case 3:
      for (size_t i=0; i< coords.size(); i++)
      {
        eg[i].weights.resize(this->basis_->num_cubic_derivate_weights());
        this->basis_->get_cubic_derivate_weights(coords[i],&(eg[i].weights[0]));
        this->mesh_->get_nodes_from_elem(eg[i].node_index,elem);
        eg[i].num_hderivs = this->basis_->num_hderivs();

        eg[i].inverse_jacobian.resize(9);
        this->mesh_->inverse_jacobian(coords[i],typename MESH::Elem::index_type(elem),&(eg[i].inverse_jacobian[0]));
        eg[i].num_derivs = this->basis_->num_derivs();
      }
      return;
  }
  ASSERTFAIL("Gradient of unknown order requested");
}



template <class MESH>
bool 
VUnstructuredMesh<MESH>::
find_closest_node(double& pdist, Core::Geometry::Point& result,
                  VMesh::Node::index_type &i, 
                  const Core::Geometry::Point &point) const
{
  return(this->mesh_->find_closest_node(pdist,result,i,point));
} 


template <class MESH>
bool 
VUnstructuredMesh<MESH>::
find_closest_node(double& pdist, Core::Geometry::Point& result,
                  VMesh::Node::index_type &i, 
                  const Core::Geometry::Point &point,
                  double maxdist) const
{
  return(this->mesh_->find_closest_node(pdist,result,i,point,maxdist));
} 

template <class MESH>
bool 
VUnstructuredMesh<MESH>::
find_closest_nodes(std::vector<VMesh::Node::index_type> &nodes, 
                   const Core::Geometry::Point &point,
                   double maxdist) const
{
  return(this->mesh_->find_closest_nodes(nodes,point,maxdist));
}


template <class MESH>
bool 
VUnstructuredMesh<MESH>::
find_closest_nodes(std::vector<double> &distances,
                   std::vector<VMesh::Node::index_type> &nodes, 
                   const Core::Geometry::Point &point,
                   double maxdist) const
{
  return(this->mesh_->find_closest_nodes(distances,nodes,point,maxdist));
}

template <class MESH>
bool
VUnstructuredMesh<MESH>::
find_closest_elem(double& pdist, 
                  Core::Geometry::Point& result,
                  VMesh::coords_type& coords,
                  VMesh::Elem::index_type& i, 
                  const Core::Geometry::Point& point) const
{
  return(this->mesh_->find_closest_elem(pdist,result,coords,i,point));
} 

template <class MESH>
bool
VUnstructuredMesh<MESH>::
find_closest_elem(double& pdist, 
                  Core::Geometry::Point& result,
                  VMesh::coords_type& coords,
                  VMesh::Elem::index_type& i, 
                  const Core::Geometry::Point& point,
                  double maxdist) const
{
  return(this->mesh_->find_closest_elem(pdist,result,coords,i,point,maxdist));
} 

template <class MESH>
bool 
VUnstructuredMesh<MESH>::
find_closest_elems(double& pdist, Core::Geometry::Point& result,
                   VMesh::Elem::array_type &i, 
                   const Core::Geometry::Point &point) const
{
  return(this->mesh_->find_closest_elems(pdist,result,i,point));
} 


}

#endif
