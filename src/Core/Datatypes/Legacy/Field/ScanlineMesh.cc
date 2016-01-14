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

#include <Core/Datatypes/Legacy/Field/ScanlineMesh.h>
#include <Core/Datatypes/Legacy/Field/VMeshShared.h>
#include <Core/Datatypes/Legacy/Field/StructCurveMesh.h>
#include <Core/Basis/CrvElementWeights.h>

/// Only include this class if we included Scanline Support
#if (SCIRUN_SCANLINE_SUPPORT > 0 || SCIRUN_STRUCTCURVE_SUPPORT > 0)

using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;

namespace SCIRun {

/// This is the virtual interface to the curve mesh
/// This class lives besides the real mesh class for now and solely profides
/// an interface. In the future however when dynamic compilation is gone
/// this should be put into the ScanlineMesh class.
template<class MESH> class VScanlineMesh;
template<class MESH> class VStructCurveMesh;

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.

template<class MESH>
class VScanlineMesh : public VMeshShared<MESH> {
public:
  virtual bool is_scanlinemesh()       { return (true); }

  /// constructor and descructor
  VScanlineMesh(MESH* mesh) : VMeshShared<MESH>(mesh) 
  {
    DEBUG_CONSTRUCTOR("VScanlineMesh")   

    this->ni_ = mesh->ni_;
  }
  
  virtual ~VScanlineMesh() 
  {
    DEBUG_DESTRUCTOR("VScanlineMesh")     
  }

  virtual void size(VMesh::Node::size_type& size) const;
  virtual void size(VMesh::ENode::size_type& size) const;
  virtual void size(VMesh::Edge::size_type& size) const;
  virtual void size(VMesh::Elem::size_type& size) const;
  virtual void size(VMesh::DElem::size_type& size) const;
  
  virtual void get_nodes(VMesh::Node::array_type& nodes, 
                         VMesh::Edge::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes, 
                         VMesh::Elem::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes, 
                         VMesh::DElem::index_type i) const;

  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::Edge::index_type i) const;
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

  /// Get the center of a certain mesh element
  virtual void get_center(Point &point, VMesh::Node::index_type i) const;
  virtual void get_center(Point &point, VMesh::ENode::index_type i) const;
  virtual void get_center(Point &point, VMesh::Edge::index_type i) const;
  virtual void get_center(Point &point, VMesh::Elem::index_type i) const;
  virtual void get_center(Point &point, VMesh::DElem::index_type i) const;

  /// Get the centers of a series of nodes
  virtual void get_centers(Point* points, const VMesh::Node::array_type& array) const;
  virtual void get_centers(Point* points, const VMesh::Elem::array_type& array) const;
  
  virtual double get_size(VMesh::Node::index_type i) const;
  virtual double get_size(VMesh::Edge::index_type i) const;
  virtual double get_size(VMesh::Elem::index_type i) const;
  virtual double get_size(VMesh::DElem::index_type i) const;
                                                    
  virtual bool locate(VMesh::Node::index_type &i, const Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, const Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, 
                      VMesh::coords_type& coords, const Point &point) const;

  virtual bool get_coords(VMesh::coords_type &coords, 
                          const Point &point, 
                          VMesh::Elem::index_type i) const;  
  virtual void interpolate(Point &p, 
                           const VMesh::coords_type &coords, 
                           VMesh::Elem::index_type i) const;

  virtual void minterpolate(std::vector<Point> &p, 
                            const std::vector<VMesh::coords_type> &coords, 
                            VMesh::Elem::index_type i) const;
                            
  virtual void derivate(VMesh::dpoints_type &p, 
                        const VMesh::coords_type &coords, 
                        VMesh::Elem::index_type i) const;
                        
  virtual void get_random_point(Point &p,
				VMesh::Elem::index_type i, 
                                FieldRNG &rng) const;

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

  virtual double det_jacobian(const VMesh::coords_type& coords,
                              VMesh::Elem::index_type idx) const; 

  virtual void jacobian(const VMesh::coords_type& coords,
                        VMesh::Elem::index_type idx,
                        double* J) const; 

  virtual double  inverse_jacobian(const VMesh::coords_type& coords,
                                   VMesh::Elem::index_type idx,
                                   double* Ji) const;
                                   
  virtual double  scaled_jacobian_metric(const VMesh::Elem::index_type) const;
  virtual double  jacobian_metric(const VMesh::Elem::index_type) const;  

  virtual bool find_closest_node(double& pdist,
                                 Point &result,
                                 VMesh::Node::index_type& node, 
                                 const Point &p) const;

  virtual bool find_closest_node(double& pdist,
                                 Point &result,
                                 VMesh::Node::index_type& node, 
                                 const Point &p,
                                 double maxdist) const;
  
  virtual bool find_closest_elem(double& pdist,
                                 Point &result,
                                 VMesh::coords_type& coords,
                                 VMesh::Elem::index_type& elem, 
                                 const Point &p) const;

  virtual bool find_closest_elem(double& pdist,
                                 Point &result,
                                 VMesh::coords_type& coords,
                                 VMesh::Elem::index_type& elem, 
                                 const Point &p,
                                 double maxdist) const;
                                 
  virtual bool find_closest_elems(double& pdist,
                                  Point &result, 
                                  VMesh::Elem::array_type& elems, 
                                  const Point &p) const;  
  
  virtual void get_dimensions(VMesh::dimension_type& dims);

  virtual void get_elem_dimensions(VMesh::dimension_type& dim);  

  virtual Transform get_transform() const;
  virtual void set_transform(const Transform& t);

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
                                       
  virtual void get_mgradient_weights(const std::vector<VMesh::coords_type>& coords, 
                                     VMesh::Elem::index_type elem, 
                                     VMesh::MultiElemGradient& eg,
                                     int basis_order) const;  

  virtual void get_mgradient_weights(const std::vector<Point>& point, 
                                     VMesh::MultiElemGradient& eg,
                                     int basis_order) const;  

protected:


  template <class ARRAY, class INDEX>
  inline void 
  get_nodes_from_edge(ARRAY &array, INDEX idx) const
  {
    array.resize(2);
    array[0] = typename ARRAY::value_type(idx);
    array[1] = typename ARRAY::value_type(idx + 1);
  }

  template <class ARRAY, class INDEX>
  inline void 
  get_edges_from_node(ARRAY &array,INDEX idx) const
  {
    array.reserve(2);
    array.clear();
    if (idx > 0)
    {
      array.push_back(typename ARRAY::value_type(idx-1));
    }
    if (idx< this->ni_-1)
    {
      array.push_back(typename ARRAY::value_type(idx));
    }
  }

  template <class ARRAY, class INDEX>
  inline void 
  get_nodes_from_elem(ARRAY &array, INDEX idx) const
  {
    get_nodes_from_edge(array,idx);
  }

  template <class ARRAY, class INDEX>
  inline void get_edges_from_elem(ARRAY &array, INDEX idx) const
  {
    array.resize(1); array[0] = typename ARRAY::value_type(idx);
  }

  template <class INDEX>
  inline void 
  to_index(typename MESH::Node::index_type &index, INDEX i) const
    { index = typename MESH::Node::index_type(i); }

  template <class INDEX>
  inline void 
  to_index(typename MESH::Edge::index_type &index, INDEX i) const
    { index = typename MESH::Edge::index_type(i); }


  template <class INDEX>
  inline bool 
  locate_elem(INDEX idx,Point p) const
  {
    if (this->ni_ < 2) return (true);
    const Point r = this->mesh_->transform_.unproject(p);

    double epsilon = this->mesh_->get_epsilon();
    double ii = r.x();

    const double nii = static_cast<double>(this->ni_-1);

    if (ii>nii && (ii-epsilon)<nii) ii=nii-epsilon;
    if (ii<0 && ii>-epsilon) ii=0;
    
    idx = static_cast<index_type>(floor(ii));

    if (idx < (this->ni_-1) && idx >= 0) return (true);

    return (false);  
  }


  template <class INDEX, class ARRAY>
  inline bool 
  locate_elem(INDEX idx, ARRAY coords, Point p) const
  {
    if (this->ni_ < 2) return (true);
    const Point r = this->mesh_->transform_.unproject(p);

    double epsilon = this->mesh_->get_epsilon();
    double ii = r.x();

    const double nii = static_cast<double>(this->ni_-1);

    if (ii>nii && (ii-epsilon)<nii) ii=nii-epsilon;
    if (ii<0 && ii>-epsilon) ii=0;
    
    idx = static_cast<index_type>(floor(ii));

    coords.resize(1);
    coords[0] = static_cast<typename ARRAY::value_type>(ii - floor(ii));

    if (idx < (this->ni_-1) && idx >= 0) return (true);

    return (false);  
  }

  template <class INDEX>
  inline bool 
  locate_node(INDEX idx, Point p) const
  {
    if (this->ni_ == 0) return (false);
  
    const Point r = this->mesh_->transform_.unproject(p);

    double rx = floor(r.x() + 0.5);
    const double nii = static_cast<double>(this->ni_-1);

    if (rx < 0.0) rx = 0.0; if (rx > nii) rx = nii;

    idx = static_cast<index_type>(rx);

    return (true);
  }

  template <class ARRAY, class INDEX>
  inline void
  inv_jacobian(const ARRAY& coords, INDEX idx, double* Ji) const
  {
    double* inverse_jacobian = this->mesh_->inverse_jacobian_;
    Ji[0] = inverse_jacobian[0];
    Ji[1] = inverse_jacobian[1];
    Ji[2] = inverse_jacobian[2];
    Ji[3] = inverse_jacobian[3];
    Ji[4] = inverse_jacobian[4];
    Ji[5] = inverse_jacobian[5];
    Ji[6] = inverse_jacobian[6];
    Ji[7] = inverse_jacobian[7];
    Ji[8] = inverse_jacobian[8];
  }

};

/// Functions for creating the virtual interface for specific mesh types
/// These are similar to compare maker and only serve to instantiate the class

/// Currently there are only 3 variations of this mesh available
/// 1) linear interpolation

/// Add the LINEAR virtual interface and the meshid for creating it 

/// Create virtual interface 
VMesh* CreateVScanlineMesh(ScanlineMesh<CrvLinearLgn<Point> >* mesh)
{
  return new VScanlineMesh<ScanlineMesh<CrvLinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID ScanlineMesh_MeshID1(ScanlineMesh<CrvLinearLgn<Point> >::type_name(-1),
                  ScanlineMesh<CrvLinearLgn<Point> >::mesh_maker,
                  ScanlineMesh<CrvLinearLgn<Point> >::scanline_maker);
                  


/// Functions for determining the number of indices that are used
template <class MESH>
void
VScanlineMesh<MESH>::size(VMesh::Node::size_type& sz) const
{
  sz = static_cast<VMesh::Node::size_type>(this->ni_);
}

template <class MESH>
void
VScanlineMesh<MESH>::size(VMesh::ENode::size_type& sz) const
{
  sz = static_cast<VMesh::ENode::size_type>(this->ni_-1);
  if (sz < 0) sz = 0;
}

template <class MESH>
void
VScanlineMesh<MESH>::size(VMesh::Edge::size_type& sz) const
{
  sz = static_cast<VMesh::Edge::size_type>(this->ni_-1);
  if (sz < 0) sz = 0;
}

template <class MESH>
void
VScanlineMesh<MESH>::size(VMesh::DElem::size_type& sz) const
{
  sz = static_cast<VMesh::DElem::size_type>(this->ni_);
}

template <class MESH>
void
VScanlineMesh<MESH>::size(VMesh::Elem::size_type& sz) const
{
  sz = static_cast<VMesh::Elem::size_type>(this->ni_-1);
  if (sz < 0) sz = 0;
}

/// Topology functions


template <class MESH>
void
VScanlineMesh<MESH>::get_nodes(VMesh::Node::array_type &array,
                               VMesh::Edge::index_type idx) const
{
  get_nodes_from_edge(array,idx);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_nodes(VMesh::Node::array_type &array,
                               VMesh::Elem::index_type idx) const
{
  get_nodes_from_edge(array,idx);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_nodes(VMesh::Node::array_type &array,
                               VMesh::DElem::index_type idx) const
{
  array.resize(1); array[0] = VMesh::Node::index_type(idx);
}


template <class MESH>
void
VScanlineMesh<MESH>::get_enodes(VMesh::ENode::array_type &array,
                                VMesh::Edge::index_type idx) const
{
  array.resize(1); array[0] = VMesh::ENode::index_type(idx);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_enodes(VMesh::ENode::array_type &array,
                                VMesh::Elem::index_type idx) const
{
  array.resize(1); array[0] = VMesh::ENode::index_type(idx);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_enodes(VMesh::ENode::array_type &array,
                                VMesh::DElem::index_type idx) const
{
  get_edges_from_node(array,idx);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_edges(VMesh::Edge::array_type &array,
                               VMesh::Node::index_type idx) const
{
  get_edges_from_node(array,idx);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_edges(VMesh::Edge::array_type &array,
                               VMesh::Elem::index_type idx) const
{
  array.resize(1); array[0] = VMesh::Edge::index_type(idx);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_edges(VMesh::Edge::array_type &array,
                               VMesh::DElem::index_type idx) const
{
  get_edges_from_node(array,idx);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_elems(VMesh::Elem::array_type &array,
                               VMesh::Node::index_type idx) const
{
  get_edges_from_node(array,idx);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_elems(VMesh::Elem::array_type &array,
                               VMesh::Edge::index_type idx) const
{
  array.resize(1); array[0] = VMesh::Elem::index_type(idx);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_elems(VMesh::Elem::array_type &array,
                               VMesh::DElem::index_type idx) const
{
  get_edges_from_node(array,idx);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_delems(VMesh::DElem::array_type &array,
                                VMesh::Edge::index_type idx) const
{
  get_nodes_from_edge(array,idx);
}


template <class MESH>
void
VScanlineMesh<MESH>::get_delems(VMesh::DElem::array_type &array,
                                VMesh::Elem::index_type idx) const
{
  get_nodes_from_edge(array,idx);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_delems(VMesh::DElem::array_type &array,
                                VMesh::Node::index_type idx) const
{
  array.resize(1); array[0] = VMesh::DElem::index_type(idx);
}


template <class MESH>
void
VScanlineMesh<MESH>::get_center(Point &p, VMesh::Node::index_type idx) const
{
  typename MESH::Node::index_type i(idx);
  this->mesh_->get_center(p,i); 
}

template <class MESH>
void
VScanlineMesh<MESH>::get_center(Point &p, VMesh::ENode::index_type idx) const
{
  typename MESH::Edge::index_type i(idx);
  this->mesh_->get_center(p,i);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_center(Point &p, VMesh::Edge::index_type idx) const
{
  typename MESH::Edge::index_type i(idx);
  this->mesh_->get_center(p,i);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_center(Point &p, VMesh::Elem::index_type idx) const
{
  typename MESH::Elem::index_type i(idx);
  this->mesh_->get_center(p, i);
}

template <class MESH>
void
VScanlineMesh<MESH>::get_center(Point &p, VMesh::DElem::index_type idx) const
{
  typename MESH::DElem::index_type i(idx);
  this->mesh_->get_center(p, i);
}


template <class MESH>
void
VScanlineMesh<MESH>::get_centers(Point* points, const VMesh::Node::array_type& array) const
{
  for (size_t j=0; j <array.size(); j++)
  {
    typename MESH::Node::index_type idx(array[j]);
    this->mesh_->get_center(points[j],idx);
  }
}                                     
 
template <class MESH>
void
VScanlineMesh<MESH>::get_centers(Point* points, const VMesh::Elem::array_type& array) const
{
  for (size_t j=0; j <array.size(); j++)
  {
    typename MESH::DElem::index_type idx(array[j]);
    this->mesh_->get_center(points[j],idx);
  }
} 


template <class MESH>
bool 
VScanlineMesh<MESH>::locate(VMesh::Node::index_type &i, const Point &point) const
{
  return(locate_node(i,point));
}

template <class MESH>
bool 
VScanlineMesh<MESH>::locate(VMesh::Elem::index_type &i, const Point &point) const
{
  return(locate_elem(i,point));
}

template <class MESH>
bool 
VScanlineMesh<MESH>::locate(VMesh::Elem::index_type &i, 
                            VMesh::coords_type& coords, 
                            const Point &point) const
{
  return(locate_elem(i,coords,point));
}

template <class MESH>
bool 
VScanlineMesh<MESH>::get_coords(VMesh::coords_type &coords, 
                                const Point &point, 
                                VMesh::Elem::index_type i) const
{
  typename MESH::Elem::index_type vi(i);
  return(this->mesh_->get_coords(coords,point,vi));
}  
  
template <class MESH>
void 
VScanlineMesh<MESH>::interpolate(Point &point, 
                                 const VMesh::coords_type &coords, 
                                 VMesh::Elem::index_type idx) const
{
  Point pnt(static_cast<double>(idx)+static_cast<double>(coords[0]),0.0,0.0);
  point = this->mesh_->transform_.project(pnt);
}

template <class MESH>
void 
VScanlineMesh<MESH>::minterpolate(std::vector<Point> &point, 
                                 const std::vector<VMesh::coords_type> &coords, 
                                 VMesh::Elem::index_type idx) const
{
  point.resize(coords.size());
  for (size_t p=0; p<coords.size();p++)
  {
    Point pnt(static_cast<double>(idx)+static_cast<double>(coords[p][0]),0.0,0.0);
    point[p] = this->mesh_->transform_.project(pnt);
  }
}

template <class MESH>
void 
VScanlineMesh<MESH>::derivate(VMesh::dpoints_type &p, 
                              const VMesh::coords_type &coords, 
                              VMesh::Elem::index_type i) const
{
  typename MESH::Elem::index_type vi(i);
  this->mesh_->derivate(coords,vi,p);
}

template <class MESH>
bool 
VScanlineMesh<MESH>::get_neighbor(VMesh::Elem::index_type &neighbor, 
                                  VMesh::Elem::index_type from, 
                                  VMesh::DElem::index_type delem) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(from);
  const VMesh::index_type xdelem = static_cast<const VMesh::index_type>(delem);

  if (xdelem == xidx)
  {
    if (xidx <= 0) return (false);
    neighbor = VMesh::Elem::index_type(xidx-1);
    return (true);
  }
  
  if (xdelem == xidx+1)
  {
    if (xidx >= this->ni_-2) return (false);
    neighbor = VMesh::Elem::index_type(xidx+1);
    return (true);  
  }
  
  return (false);
}


template <class MESH>
bool 
VScanlineMesh<MESH>::get_neighbors(VMesh::Elem::array_type &neighbors, 
                                   VMesh::Elem::index_type from, 
                                   VMesh::DElem::index_type delem) const
{
  VMesh::Elem::index_type n;
  if(get_neighbor(n,from,delem))
  {
    neighbors.resize(1);
    neighbors[0] = n;
    return (true);
  }
  return (false);
}                                

template <class MESH>
void 
VScanlineMesh<MESH>::get_neighbors(VMesh::Elem::array_type &array, 
                                   VMesh::Elem::index_type idx) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);

  array.reserve(2);
  array.clear();
  if (xidx > 0) array.push_back(VMesh::Elem::index_type(xidx-1));
  if (xidx < this->ni_-2) array.push_back(VMesh::Elem::index_type(xidx+1));   
}

template <class MESH>
void 
VScanlineMesh<MESH>::get_neighbors(VMesh::Node::array_type &array, 
                                   VMesh::Node::index_type idx) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);
 
  array.reserve(2);
  array.clear();
  if (xidx > 0) array.push_back(VMesh::Node::index_type(xidx-1));
  if (xidx < this->ni_-1) array.push_back(VMesh::Node::index_type(xidx+1));  
}

// WE should prcompute these:
template <class MESH>
double
VScanlineMesh<MESH>::get_size(VMesh::Node::index_type /*i*/) const
{
  return (0.0);
}

template <class MESH>
double
VScanlineMesh<MESH>::get_size(VMesh::Edge::index_type i) const
{
  typename MESH::Edge::index_type vi(i);
  return (this->mesh_->get_size(vi));
}

template <class MESH>
double
VScanlineMesh<MESH>::get_size(VMesh::Elem::index_type i) const
{
  typename MESH::Elem::index_type vi(i);
  return (this->mesh_->get_size(vi));
}

template <class MESH>
double
VScanlineMesh<MESH>::get_size(VMesh::DElem::index_type i) const
{
  typename MESH::DElem::index_type vi(i);
  return (this->mesh_->get_size(vi));
}


template <class MESH>
void 
VScanlineMesh<MESH>::pwl_approx_edge(VMesh::coords_array_type &coords, 
                                     VMesh::Elem::index_type /*ci*/, 
                                     unsigned int which_edge,
                                     unsigned int div_per_unit) const
{
  this->basis_->approx_edge(which_edge, div_per_unit, coords);
}

template <class MESH>
void 
VScanlineMesh<MESH>::pwl_approx_face(VMesh::coords_array2_type &coords, 
                                     VMesh::Elem::index_type /*ci*/, 
                                     unsigned int which_face,
                                     unsigned int div_per_unit) const
{
  this->basis_->approx_face(which_face, div_per_unit, coords);
}

template <class MESH>
void 
VScanlineMesh<MESH>::get_random_point(Point &p,
				      VMesh::Elem::index_type i,
				      FieldRNG &rng) const
{
  typename MESH::Elem::index_type vi(i);
  this->mesh_->get_random_point(p,vi,rng);
}

template <class MESH>
void 
VScanlineMesh<MESH>::get_dimensions(VMesh::dimension_type& dims)
{
  dims.resize(1);
  dims[0] = this->ni_;
}

template <class MESH>
void 
VScanlineMesh<MESH>::get_elem_dimensions(VMesh::dimension_type& dims)
{
  dims.resize(1);
  dims[0] = this->ni_-1;
}

template <class MESH>
Transform
VScanlineMesh<MESH>::get_transform() const
{
  return (this->mesh_->get_transform());
}

template <class MESH>
void
VScanlineMesh<MESH>::set_transform(const Transform& t)
{
  this->mesh_->set_transform(t);
}

template <class MESH>
double 
VScanlineMesh<MESH>::det_jacobian(const VMesh::coords_type& /*coords*/, 
                                  VMesh::Elem::index_type /*idx*/) const
{
  return(this->mesh_->det_jacobian_);
}


template <class MESH>
void
VScanlineMesh<MESH>::jacobian(const VMesh::coords_type& /*coords*/, 
                              VMesh::Elem::index_type /*idx*/, 
                              double* J) const
{
  double* jacobian = this->mesh_->jacobian_;
  J[0] = jacobian[0];
  J[1] = jacobian[1];
  J[2] = jacobian[2];
  J[3] = jacobian[3];
  J[4] = jacobian[4];
  J[5] = jacobian[5];
  J[6] = jacobian[6];
  J[7] = jacobian[7];
  J[8] = jacobian[8];
}


template <class MESH>
double
VScanlineMesh<MESH>::inverse_jacobian(const VMesh::coords_type& /*coords*/,
                                      VMesh::Elem::index_type /*idx*/, 
                                      double* Ji) const
{
  double* inverse_jacobian = this->mesh_->inverse_jacobian_;
  Ji[0] = inverse_jacobian[0];
  Ji[1] = inverse_jacobian[1];
  Ji[2] = inverse_jacobian[2];
  Ji[3] = inverse_jacobian[3];
  Ji[4] = inverse_jacobian[4];
  Ji[5] = inverse_jacobian[5];
  Ji[6] = inverse_jacobian[6];
  Ji[7] = inverse_jacobian[7];
  Ji[8] = inverse_jacobian[8];

  return (this->mesh_->det_inverse_jacobian_);
}

template <class MESH>
double 
VScanlineMesh<MESH>::scaled_jacobian_metric(const VMesh::Elem::index_type /*idx*/) const
{
  return(this->mesh_->scaled_jacobian_);
}

template <class MESH>
double 
VScanlineMesh<MESH>::jacobian_metric(const VMesh::Elem::index_type /*idx*/) const
{
  return(this->mesh_->det_jacobian_);
}


template <class MESH>
bool
VScanlineMesh<MESH>::find_closest_node(double& pdist,
                                       Point &result, 
                                       VMesh::Node::index_type& node,
                                       const Point &p) const
{
  return(this->mesh_->find_closest_node(pdist, result,node,p));
}

template <class MESH>
bool
VScanlineMesh<MESH>::find_closest_node(double& pdist,
                                       Point &result, 
                                       VMesh::Node::index_type& node,
                                       const Point &p,
                                       double maxdist) const
{
  return(this->mesh_->find_closest_node(pdist, result,node,p,maxdist));
}

template <class MESH>
bool 
VScanlineMesh<MESH>::find_closest_elem(double& pdist,
                                       Point &result,
                                       VMesh::coords_type& coords, 
                                       VMesh::Elem::index_type& elem,
                                       const Point &p) const
{
  return(this->mesh_->find_closest_elem(pdist,result,coords,elem,p));
}

template <class MESH>
bool 
VScanlineMesh<MESH>::find_closest_elem(double& pdist,
                                       Point &result,
                                       VMesh::coords_type& coords, 
                                       VMesh::Elem::index_type& elem,
                                       const Point &p,
                                       double maxdist) const
{
  return(this->mesh_->find_closest_elem(pdist,result,coords,elem,p,maxdist));
}

template <class MESH>
bool 
VScanlineMesh<MESH>::find_closest_elems(double& pdist,
                                        Point &result,
                                        VMesh::Elem::array_type &elems,
                                        const Point &p) const
{
  return(this->mesh_->find_closest_elems(pdist,result,elems,p));
}



template <class MESH>
void
VScanlineMesh<MESH>::get_interpolate_weights(const Point& point, 
                                             VMesh::ElemInterpolate& ei,
                                             int basis_order) const
{
  VMesh::Elem::index_type elem;
  StackVector<double,3> coords;

  if(locate_elem(elem,coords,point))
  {
    ei.basis_order = basis_order;
    ei.elem_index = elem;
  }
  else
  {
    ei.basis_order = basis_order;
    ei.elem_index  = -1;  
    return;
  }
    
  switch (basis_order)
  {
    case 0:
      return;
    case 1:
      ei.weights.resize(this->basis_->num_linear_weights());
      this->basis_->get_linear_weights(coords,&(ei.weights[0]));
      get_nodes_from_elem(ei.node_index,elem);
      return;
    case 2:
      ei.weights.resize(this->basis_->num_quadratic_weights());
      this->basis_->get_quadratic_weights(coords,&(ei.weights[0]));
      get_nodes_from_elem(ei.node_index,elem);
      get_edges_from_elem(ei.edge_index,elem);
      return;
    case 3:   
      ei.weights.resize(this->basis_->num_cubic_weights());
      this->basis_->get_cubic_weights(coords,&(ei.weights[0]));
      get_nodes_from_elem(ei.node_index,elem);
      ei.num_hderivs = this->basis_->num_hderivs();
      return;
  }
  ASSERTFAIL("Interpolation of unknown order requested");
}


template <class MESH>
void
VScanlineMesh<MESH>::get_interpolate_weights(const VMesh::coords_type& coords, 
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
      get_nodes_from_elem(ei.node_index,elem);
      return;
    case 2:
      ei.weights.resize(this->basis_->num_quadratic_weights());
      this->basis_->get_quadratic_weights(coords,&(ei.weights[0]));
      get_nodes_from_elem(ei.node_index,elem);
      get_edges_from_elem(ei.edge_index,elem);
      return;
    case 3:   
      ei.weights.resize(this->basis_->num_cubic_weights());
      this->basis_->get_cubic_weights(coords,&(ei.weights[0]));
      get_nodes_from_elem(ei.node_index,elem);
      ei.num_hderivs = this->basis_->num_hderivs();
      return;
  }
  ASSERTFAIL("Interpolation of unknown order requested");
}
                                       

template <class MESH>
void
VScanlineMesh<MESH>::get_minterpolate_weights(const std::vector<Point>& point, 
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
          ei[i].basis_order = basis_order;
          VMesh::Elem::index_type elem;
          if(locate_elem(elem,point[i]))
          {
            ei[i].elem_index = elem;
          }
          else
          {
            ei[i].elem_index  = -1;  
          }      
        }
      }
      return;
    case 1:
      {
        StackVector<double,3> coords;
        typename VMesh::Elem::index_type elem;
        
        for (size_t i=0; i<ei.size();i++)
        {
          ei[i].basis_order = basis_order;
          if(locate_elem(elem,coords,point[i]))
          {
            get_coords(coords,point[i],elem);
            ei[i].elem_index = elem;
            ei[i].weights.resize(this->basis_->num_linear_weights());
            this->basis_->get_linear_weights(coords,&(ei[i].weights[0]));
            get_nodes_from_elem(ei[i].node_index,elem);
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
          if(locate_elem(elem,coords,point[i]))
          {
            get_coords(coords,point[i],elem);
            ei[i].elem_index = elem;
            ei[i].weights.resize(this->basis_->num_quadratic_weights());
            this->basis_->get_quadratic_weights(coords,&(ei[i].weights[0]));
            get_nodes_from_elem(ei[i].node_index,elem);
            get_edges_from_elem(ei[i].edge_index,elem);
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
          if(locate_elem(elem,coords,point[i]))
          {
            get_coords(coords,point[i],elem);
            ei[i].elem_index = elem;
            ei[i].weights.resize(this->basis_->num_cubic_weights());
            this->basis_->get_cubic_weights(coords,&(ei[i].weights[0]));
            get_nodes_from_elem(ei[i].node_index,elem);
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
VScanlineMesh<MESH>::get_minterpolate_weights(const std::vector<VMesh::coords_type>& coords, 
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
VScanlineMesh<MESH>::get_gradient_weights(const VMesh::coords_type& coords, 
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
VScanlineMesh<MESH>::get_gradient_weights(const Point& point, 
                                          VMesh::ElemGradient& eg,
                                          int basis_order) const
{
  double* inverse_jacobian = this->mesh_->inverse_jacobian_;

  VMesh::Elem::index_type elem;
  StackVector<double,3> coords;

  if(!(locate_elem(elem,coords,point)))
  {
    eg.basis_order = basis_order;
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


template <class MESH>
void
VScanlineMesh<MESH>::get_mgradient_weights(const std::vector<Point>& point, 
                                           VMesh::MultiElemGradient& eg,
                                           int basis_order) const
{
  double* inverse_jacobian = this->mesh_->inverse_jacobian_;

  eg.resize(point.size());
  
  switch (basis_order)
  {
    case 0:
      {
        VMesh::Elem::index_type elem;
        for (size_t i=0; i< point.size(); i++)
        {
          if(locate_elem(elem,point[i]))
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
          if(locate_elem(elem,coords,point[i]))
          {      
            eg[i].elem_index = elem;
            eg[i].weights.resize(this->basis_->num_linear_derivate_weights());
            this->basis_->get_linear_derivate_weights(coords,&(eg[i].weights[0]));
            get_nodes_from_elem(eg[i].node_index,elem);

            eg[i].inverse_jacobian.resize(9);
            for(size_t j=0; j<9;j++) eg[i].inverse_jacobian[j] = inverse_jacobian[j];
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
          if(locate_elem(elem,coords,point[i]))
          { 
            eg[i].elem_index = elem;
            eg[i].weights.resize(this->basis_->num_quadratic_derivate_weights());
            this->basis_->get_quadratic_derivate_weights(coords,&(eg[i].weights[0]));
            get_nodes_from_elem(eg[i].node_index,elem);
            get_edges_from_elem(eg[i].edge_index,elem);

            eg[i].inverse_jacobian.resize(9);
            for(size_t j=0; j<9;j++) eg[i].inverse_jacobian[j] = inverse_jacobian[j];
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
          if(locate_elem(elem,coords,point[i]))
          {      
            eg[i].elem_index = elem;
            eg[i].weights.resize(this->basis_->num_cubic_derivate_weights());
            this->basis_->get_cubic_derivate_weights(coords,&(eg[i].weights[0]));
            get_nodes_from_elem(eg[i].node_index,elem);
            eg[i].num_hderivs = this->basis_->num_hderivs();

            eg[i].inverse_jacobian.resize(9);
            for(size_t j=0; j<9;j++) eg[i].inverse_jacobian[j] = inverse_jacobian[j];
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
VScanlineMesh<MESH>::get_mgradient_weights(const std::vector<VMesh::coords_type>& coords, 
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




#if (SCIRUN_STRUCTCURVE_SUPPORT > 0)

template<class MESH>
class VStructCurveMesh : public VScanlineMesh<MESH> {
public:

  virtual bool is_scanlinemesh()         { return (false); }
  virtual bool is_structcurvemesh()      { return (true); }

  /// constructor and descructor
  VStructCurveMesh(MESH* mesh) :
    VScanlineMesh<MESH>(mesh),
    points_(mesh->get_points())
  {
    DEBUG_CONSTRUCTOR("VStructCurveMesh") 
  }
  
  virtual ~VStructCurveMesh()
  {
    DEBUG_DESTRUCTOR("VStructCurveMesh")   
  }
  
  friend class ElemData;

  class ElemData
  {
  public:
    ElemData(const VStructCurveMesh<MESH>* vmesh,MESH* mesh, VMesh::Elem::index_type idx) :
      index_(idx),
      points_(mesh->get_points())    
    {
      vmesh->get_nodes_from_edge(nodes_,idx);
    }

    // the following designed to coordinate with ::get_nodes
    inline
    unsigned node0_index() const {
      return (nodes_[0]);
    }
    inline
    unsigned node1_index() const {
      return (nodes_[1]);
    }
    inline
    unsigned node2_index() const {
      return (nodes_[2]);
    }
    inline
    unsigned node3_index() const {
      return (nodes_[3]);
    }
    inline
    unsigned node4_index() const {
      return (nodes_[4]);
    }
    inline
    unsigned node5_index() const {
      return (nodes_[5]);
    }

    inline
    unsigned node6_index() const {
      return (nodes_[6]);
    }
    inline
    unsigned node7_index() const {
      return (nodes_[7]);
    }

    inline
    const Point &node0() const {
      return points_[nodes_[0]];
    }
    inline
    const Point &node1() const {
      return points_[nodes_[1]];
    }
    inline
    const Point &node2() const {
      return points_[nodes_[2]];
    }
    inline
    const Point &node3() const {
      return points_[nodes_[3]];
    }
    inline
    const Point &node4() const {
      return points_[nodes_[4]];
    }
    inline
    const Point &node5() const {
      return points_[nodes_[5]];
    }
    inline
    const Point &node6() const {
      return points_[nodes_[6]];
    }
    inline
    const Point &node7() const {
      return points_[nodes_[7]];
    }

  private:
    VMesh::Cell::index_type  index_;
    const std::vector<Point>&     points_;
    VMesh::Node::array_type  nodes_;
  };


  virtual void get_center(Point &point, VMesh::Node::index_type i) const;
  virtual void get_center(Point &point, VMesh::ENode::index_type i) const;
  virtual void get_center(Point &point, VMesh::Edge::index_type i) const;
  virtual void get_center(Point &point, VMesh::Elem::index_type i) const;
  virtual void get_center(Point &point, VMesh::DElem::index_type i) const;

  /// Get the centers of a series of nodes
  virtual void get_centers(Point* points, const VMesh::Node::array_type& array) const;
  virtual void get_centers(Point* points, const VMesh::Elem::array_type& array) const;
  
  virtual double get_size(VMesh::Node::index_type i) const;
  virtual double get_size(VMesh::Edge::index_type i) const;
  virtual double get_size(VMesh::Elem::index_type i) const;
  virtual double get_size(VMesh::DElem::index_type i) const;
  

  virtual bool locate(VMesh::Node::index_type &i, const Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, const Point &point) const;

  virtual bool locate(VMesh::Elem::index_type& i,
                      VMesh::coords_type& coords,
                      const Point& point) const;

  virtual bool get_coords(VMesh::coords_type &coords, const Point &point, 
			  VMesh::Elem::index_type i) const;  
                                              
  virtual void interpolate(Point &p, 
                           const VMesh::coords_type &coords, 
                           VMesh::Elem::index_type i) const;
                           
  virtual void minterpolate(std::vector<Point> &p, 
                            const std::vector<VMesh::coords_type> &coords, 
                            VMesh::Elem::index_type i) const;
                                              
  virtual void derivate(VMesh::dpoints_type &p,
			const VMesh::coords_type &coords, 
			VMesh::Elem::index_type i) const;

  virtual void set_point(const Point &point, VMesh::Node::index_type i);
  
  virtual Point* get_points_pointer() const;
  
  virtual void get_random_point(Point &p,
				VMesh::Elem::index_type i,
				FieldRNG &rng) const;  

  virtual double det_jacobian(const VMesh::coords_type& coords,
                              VMesh::Elem::index_type idx) const; 

  virtual void jacobian(const VMesh::coords_type& coords,
                        VMesh::Elem::index_type idx,
                        double* J) const; 

  virtual double  inverse_jacobian(const VMesh::coords_type& coords,
                                   VMesh::Elem::index_type idx,
                                   double* Ji) const;

  virtual double scaled_jacobian_metric(const VMesh::Elem::index_type idx) const;
  virtual double jacobian_metric(const VMesh::Elem::index_type idx) const;

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
                                       
  virtual void get_mgradient_weights(const std::vector<VMesh::coords_type>& coords, 
                                     VMesh::Elem::index_type elem, 
                                     VMesh::MultiElemGradient& eg,
                                     int basis_order) const;  

  virtual void get_mgradient_weights(const std::vector<Point>& point, 
                                     VMesh::MultiElemGradient& eg,
                                     int basis_order) const;
                                     
  virtual bool find_closest_node(double& pdist,
                                 Point& result, 
                                 VMesh::Node::index_type &i, 
                                 const Point &point) const;

  virtual bool find_closest_node(double& pdist,
                                 Point& result, 
                                 VMesh::Node::index_type &i, 
                                 const Point &point,
                                 double maxdist) const;
                                 
  virtual bool find_closest_elem(double& pdist,
                                 Point& result, 
                                 VMesh::coords_type& coords,
                                 VMesh::Elem::index_type &i, 
                                 const Point &point) const;

  virtual bool find_closest_elem(double& pdist,
                                 Point& result, 
                                 VMesh::coords_type& coords,
                                 VMesh::Elem::index_type &i, 
                                 const Point &point,
                                 double maxdist) const;
                                 
  virtual bool find_closest_elems(double& pdist,
                                  Point& result, 
                                  VMesh::Elem::array_type &i, 
                                  const Point &point) const;                                     
                                     
protected:

  template <class ARRAY, class INDEX>
  inline void 
  inv_jacobian(const ARRAY& coords, INDEX idx, double* Ji) const
  {
    this->mesh_->inverse_jacobian(coords, typename MESH::Elem::index_type(idx),Ji);
  }

  std::vector<Point>&        points_;
};


/// Functions for creating the virtual interface for specific mesh types
/// These are similar to compare maker and only serve to instantiate the class

/// Currently there are only 3 variations of this mesh available
/// 1) linear interpolation

/// Add the LINEAR virtual interface and the meshid for creating it 

/// Create virtual interface 
VMesh* CreateVStructCurveMesh(StructCurveMesh<CrvLinearLgn<Point> >* mesh)
{
  return new VStructCurveMesh<StructCurveMesh<CrvLinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID StructCurve_MeshID1(StructCurveMesh<CrvLinearLgn<Point> >::type_name(-1),
                  StructCurveMesh<CrvLinearLgn<Point> >::mesh_maker,
                  StructCurveMesh<CrvLinearLgn<Point> >::structcurve_maker);


template <class MESH>
void
VStructCurveMesh<MESH>::get_center(Point &p, VMesh::Node::index_type idx) const
{
  p = points_[idx];
}

template <class MESH>
void
VStructCurveMesh<MESH>::get_center(Point &p,VMesh::ENode::index_type idx) const
{
  get_center(p,VMesh::Edge::index_type(idx));
}

template <class MESH>
void
VStructCurveMesh<MESH>::get_center(Point &p,VMesh::Edge::index_type idx) const
{
  const Point &p0 = points_[idx];
  const Point &p1 = points_[idx+1];

  p = Point(p0+p1)*0.5;
}

template <class MESH>
void
VStructCurveMesh<MESH>::get_center(Point &p, VMesh::Elem::index_type idx) const
{
  const Point &p0 = points_[idx];
  const Point &p1 = points_[idx+1];

  p = Point(p0+p1)*0.5;
}

template <class MESH>
void
VStructCurveMesh<MESH>::get_center(Point &p, VMesh::DElem::index_type idx) const
{
  p = points_[idx];
}

template <class MESH>
void
VStructCurveMesh<MESH>::get_centers(Point* points, const VMesh::Node::array_type& array) const
{
  for (size_t j=0; j <array.size(); j++)
  {
    points[j] = points_[array[j]];
  }
}                                     
 
template <class MESH>
void
VStructCurveMesh<MESH>::get_centers(Point* points, const VMesh::Elem::array_type& array) const
{
  for (size_t j=0; j <array.size(); j++)
  {
    VMesh::Elem::index_type idx = array[j];
    const Point &p0 = points_[idx];
    const Point &p1 = points_[idx+1];
    points[j] = Point(p0+p1)*0.5;
  }
} 


template <class MESH>
double
VStructCurveMesh<MESH>::get_size(VMesh::Node::index_type /*i*/) const
{
  return (0.0);
}

template <class MESH>
double
VStructCurveMesh<MESH>::get_size(VMesh::Edge::index_type i) const
{
  return (this->mesh_->get_size(typename MESH::Edge::index_type(i)));
}

template <class MESH>
double
VStructCurveMesh<MESH>::get_size(VMesh::Elem::index_type i) const
{
  return (this->mesh_->get_size(typename MESH::Elem::index_type(i)));
}

template <class MESH>
double
VStructCurveMesh<MESH>::get_size(VMesh::DElem::index_type i) const
{
  return (this->mesh_->get_size(typename MESH::DElem::index_type(i)));
}

template <class MESH>
bool 
VStructCurveMesh<MESH>::locate(VMesh::Node::index_type &idx, 
                               const Point &point) const
{
  return(this->mesh_->locate_node(idx,point));
}

template <class MESH>
bool 
VStructCurveMesh<MESH>::locate(VMesh::Elem::index_type &idx,
                               const Point &point) const
{
  return(this->mesh_->locate_elem(idx,point));
}

template <class MESH>
bool 
VStructCurveMesh<MESH>::locate(VMesh::Elem::index_type &idx,
                               VMesh::coords_type& coords,
                               const Point &point) const
{
  return(this->mesh_->locate_elem(idx,coords,point));
}

template <class MESH>
bool 
VStructCurveMesh<MESH>::find_closest_node(double& pdist,
                                          Point& result,
                                          VMesh::Node::index_type &i, 
                                          const Point &point) const
{
  return(this->mesh_->find_closest_node(pdist,result,i,point,-1.0));
} 

template <class MESH>
bool 
VStructCurveMesh<MESH>::find_closest_node(double& pdist,
                                          Point& result,
                                          VMesh::Node::index_type &i, 
                                          const Point &point,
                                          double maxdist) const
{
  return(this->mesh_->find_closest_node(pdist,result,i,point,maxdist));
} 

template <class MESH>
bool
VStructCurveMesh<MESH>::find_closest_elem(double& pdist,
                                          Point& result,
                                          VMesh::coords_type& coords,
                                          VMesh::Elem::index_type &i, 
                                          const Point &point) const
{
  return(this->mesh_->find_closest_elem(pdist,result,coords,i,point,-1.0));
} 

template <class MESH>
bool
VStructCurveMesh<MESH>::find_closest_elem(double& pdist,
                                          Point& result,
                                          VMesh::coords_type& coords,
                                          VMesh::Elem::index_type &i, 
                                          const Point &point,
                                          double maxdist) const
{
  return(this->mesh_->find_closest_elem(pdist,result,coords,i,point,maxdist));
} 

template <class MESH>
bool 
VStructCurveMesh<MESH>::find_closest_elems(double& pdist,
                                           Point& result,
                                           VMesh::Elem::array_type &i, 
                                           const Point &point) const
{
  return(this->mesh_->find_closest_elems(pdist,result,i,point));
} 



template <class MESH>
bool 
VStructCurveMesh<MESH>::get_coords(VMesh::coords_type &coords, 
                                   const Point &point, 
                                   VMesh::Elem::index_type idx) const
{
  return(this->mesh_->get_coords(coords,point,idx));
}  
  
template <class MESH>
void 
VStructCurveMesh<MESH>::interpolate(Point &point, 
                                    const VMesh::coords_type &coords, 
                                    VMesh::Elem::index_type i) const
{
	this->mesh_->interpolate(point,coords,typename MESH::Elem::index_type(i));
}

template <class MESH>
void 
VStructCurveMesh<MESH>::minterpolate(std::vector<Point> &point, 
                                     const std::vector<VMesh::coords_type> &coords, 
                                     VMesh::Elem::index_type i) const
{
  point.resize(coords.size());
  for (size_t p=0; p <point.size(); p++)
  {
    this->mesh_->interpolate(point[p],coords[p],typename MESH::Elem::index_type(i));
  }
}

template <class MESH>
void 
VStructCurveMesh<MESH>::derivate(VMesh::dpoints_type &p, 
                                 const VMesh::coords_type &coords, 
                                 VMesh::Elem::index_type i) const
{
  this->mesh_->derivate(coords,typename MESH::Elem::index_type(i),p);
}


template <class MESH>
void 
VStructCurveMesh<MESH>::set_point(const Point &point, VMesh::Node::index_type i)
{
  points_[i] = point;
}

template <class MESH>
Point*
VStructCurveMesh<MESH>::
get_points_pointer() const
{
  if (points_.size() == 0) return (0);
  return (&(points_[0]));
}


template <class MESH>
void 
VStructCurveMesh<MESH>::get_random_point(Point &p,
					 VMesh::Elem::index_type i,
					 FieldRNG &rng) const
{
  typename MESH::Elem::index_type vi(i);
  this->mesh_->get_random_point(p,vi,rng);
}


template <class MESH>
double 
VStructCurveMesh<MESH>::det_jacobian(const VMesh::coords_type& coords,
                                     VMesh::Elem::index_type idx) const
{
  return(this->mesh_->det_jacobian(coords,
				   typename MESH::Elem::index_type(idx)));
}


template <class MESH>
void
VStructCurveMesh<MESH>::jacobian(const VMesh::coords_type& coords,
                                 VMesh::Elem::index_type idx, 
                                 double* J) const
{
  this->mesh_->jacobian(coords,
			typename MESH::Elem::index_type(idx),J);
}


template <class MESH>
double
VStructCurveMesh<MESH>::inverse_jacobian(const VMesh::coords_type& coords,
                                         VMesh::Elem::index_type idx, 
                                         double* Ji) const
{
  return(this->mesh_->inverse_jacobian(coords,
                    typename MESH::Elem::index_type(idx),Ji));
}


template <class MESH>
double 
VStructCurveMesh<MESH>::scaled_jacobian_metric(const VMesh::Elem::index_type idx) const
{
  StackVector<Point,3> Jv;
  ElemData ed(this,this->mesh_,idx);

  double temp;
  this->basis_->derivate(this->basis_->unit_center,ed,Jv);
  Jv.resize(3); 
  Vector v,w;
  Vector(Jv[0]).find_orthogonal(v,w);
  Jv[1] = v.asPoint();
  Jv[2] = w.asPoint();
  double min_jacobian = ScaledDetMatrix3P(Jv);
  
  size_t num_vertices = this->basis_->number_of_vertices();
  for (size_t j=0;j < num_vertices;j++)
  {
    this->basis_->derivate(this->basis_->unit_vertices[j],ed,Jv);
    Jv.resize(3); 
    Vector v,w;
    Vector(Jv[0]).find_orthogonal(v,w);
    Jv[1] = v.asPoint();
    Jv[2] = w.asPoint();
    temp = ScaledDetMatrix3P(Jv);
    if(temp < min_jacobian) min_jacobian = temp;
  }
    
  return (min_jacobian);
}

template <class MESH>
double 
VStructCurveMesh<MESH>::jacobian_metric(const  VMesh::Elem::index_type idx) const
{
  StackVector<Point,3> Jv;
  ElemData ed(this,this->mesh_,idx);

  double temp;
  this->basis_->derivate(this->basis_->unit_center,ed,Jv);
  Jv.resize(3); 
  Vector v,w;
  Vector(Jv[0]).find_orthogonal(v,w);
  Jv[1] = v.asPoint();
  Jv[2] = w.asPoint();
  double min_jacobian = DetMatrix3P(Jv);
  
  size_t num_vertices = this->basis_->number_of_vertices();
  for (size_t j=0;j < num_vertices;j++)
  {
    this->basis_->derivate(this->basis_->unit_vertices[j],ed,Jv);
    Jv.resize(3); 
    Vector v,w;
    Vector(Jv[0]).find_orthogonal(v,w);
    Jv[1] = v.asPoint();
    Jv[2] = w.asPoint();
    temp = DetMatrix3P(Jv);
    if(temp < min_jacobian) min_jacobian = temp;
  }
    
  return (min_jacobian);
}



template <class MESH>
void
VStructCurveMesh<MESH>::get_interpolate_weights(const Point& point, 
                                                 VMesh::ElemInterpolate& ei,
                                                 int basis_order) const
{
  VMesh::Elem::index_type elem;
  
  if(this->mesh_->locate_elem(elem,point))
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
      this->get_nodes_from_elem(ei.node_index,elem);
      return;
    case 2:
      ei.weights.resize(this->basis_->num_quadratic_weights());
      this->basis_->get_quadratic_weights(coords,&(ei.weights[0]));
      this->get_nodes_from_elem(ei.node_index,elem);
      this->get_edges_from_elem(ei.edge_index,elem);
      return;
    case 3:   
      ei.weights.resize(this->basis_->num_cubic_weights());
      this->basis_->get_cubic_weights(coords,&(ei.weights[0]));
      this->get_nodes_from_elem(ei.node_index,elem);
      ei.num_hderivs = this->basis_->num_hderivs();
      return;
  }
  ASSERTFAIL("Interpolation of unknown order requested");
}

template <class MESH>
void
VStructCurveMesh<MESH>::get_interpolate_weights(const VMesh::coords_type& coords, 
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
      this->get_nodes_from_elem(ei.node_index,elem);
      return;
    case 2:
      ei.weights.resize(this->basis_->num_quadratic_weights());
      this->basis_->get_quadratic_weights(coords,&(ei.weights[0]));
      this->get_nodes_from_elem(ei.node_index,elem);
      this->get_edges_from_elem(ei.edge_index,elem);
      return;
    case 3:   
      ei.weights.resize(this->basis_->num_cubic_weights());
      this->basis_->get_cubic_weights(coords,&(ei.weights[0]));
      this->get_nodes_from_elem(ei.node_index,elem);
      ei.num_hderivs = this->basis_->num_hderivs();
      return;
  }
  ASSERTFAIL("Interpolation of unknown order requested");
}
                                                     

template <class MESH>
void
VStructCurveMesh<MESH>::get_minterpolate_weights(const std::vector<Point>& point, 
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
          if(this->mesh_->locate_elem(elem,point[i]))
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
          if(this->mesh_->locate_elem(elem,point[i]))
          {
            this->mesh_->get_coords(coords,point[i],elem);
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
          if(this->mesh_->locate_elem(elem,point[i]))
          {
            this->mesh_->get_coords(coords,point[i],elem);
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
          if(this->mesh_->locate_elem(elem,point[i]))
          {
            this->mesh_->get_coords(coords,point[i],elem);
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
VStructCurveMesh<MESH>::get_minterpolate_weights(const std::vector<VMesh::coords_type>& coords, 
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
        this->get_nodes_from_elem(ei[i].node_index,elem);
      }
      return;
    case 2:
      for (size_t i=0; i<coords.size(); i++)
      {
        ei[i].weights.resize(this->basis_->num_quadratic_weights());
        this->basis_->get_quadratic_weights(coords[i],&(ei[i].weights[0]));
        this->get_nodes_from_elem(ei[i].node_index,elem);
        this->get_edges_from_elem(ei[i].edge_index,elem);
      }
      return;
    case 3:   
      for (size_t i=0; i<coords.size(); i++)
      {
        ei[i].weights.resize(this->basis_->num_cubic_weights());
        this->basis_->get_cubic_weights(coords[i],&(ei[i].weights[0]));
        this->get_nodes_from_elem(ei[i].node_index,elem);
        ei[i].num_hderivs = this->basis_->num_hderivs();
      }
      return;
  }
  ASSERTFAIL("Interpolation of unknown order requested");
}


template <class MESH>
void
VStructCurveMesh<MESH>::get_gradient_weights(const VMesh::coords_type& coords, 
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
VStructCurveMesh<MESH>::get_gradient_weights(const Point& point, 
                                             VMesh::ElemGradient& eg,
                                             int basis_order) const
{
  VMesh::Elem::index_type elem;
  StackVector<double,3> coords;

  if(!(this->mesh_->locate_elem(elem,point)))
  {
    eg.basis_order = basis_order;
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
VStructCurveMesh<MESH>::get_mgradient_weights(const std::vector<Point>& point, 
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
          if(this->mesh_->locate_elem(elem,point[i]))
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
          if(this->mesh_->locate_elem(elem,point[i]))
          {      
            eg[i].elem_index = elem;
            this->mesh_->get_coords(coords,point[i],elem);
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
          if(this->mesh_->locate_elem(elem,point[i]))
          {      
            eg[i].elem_index = elem;
            this->mesh_->get_coords(coords,point[i],elem);
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
          if(this->mesh_->locate_elem(elem,point[i]))
          {      
            eg[i].elem_index = elem;
            this->mesh_->get_coords(coords,point[i],elem);
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



template <class MESH>
void
VStructCurveMesh<MESH>::get_mgradient_weights(const std::vector<VMesh::coords_type>& coords, 
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
        this->get_nodes_from_elem(eg[i].node_index,elem);

        eg[i].inverse_jacobian.resize(9);
        inv_jacobian(coords[i],elem,&(eg[i].inverse_jacobian[0]));
        eg[i].num_derivs = this->basis_->num_derivs();
      }
      return;
    case 2:
      for (size_t i=0; i< coords.size(); i++)
      {
        eg[i].weights.resize(this->basis_->num_quadratic_derivate_weights());
        this->basis_->get_quadratic_derivate_weights(coords[i],&(eg[i].weights[0]));
        this->get_nodes_from_elem(eg[i].node_index,elem);
        this->get_edges_from_elem(eg[i].edge_index,elem);

        eg[i].inverse_jacobian.resize(9);
        inv_jacobian(coords[i],elem,&(eg[i].inverse_jacobian[0]));
        eg[i].num_derivs = this->basis_->num_derivs();
      }
      return;
    case 3:
      for (size_t i=0; i< coords.size(); i++)
      {
        eg[i].weights.resize(this->basis_->num_cubic_derivate_weights());
        this->basis_->get_cubic_derivate_weights(coords[i],&(eg[i].weights[0]));
        this->get_nodes_from_elem(eg[i].node_index,elem);
        eg[i].num_hderivs = this->basis_->num_hderivs();

        eg[i].inverse_jacobian.resize(9);
        inv_jacobian(coords[i],elem,&(eg[i].inverse_jacobian[0]));
        eg[i].num_derivs = this->basis_->num_derivs();
      }
      return;
  }
  ASSERTFAIL("Gradient of unknown order requested");
}

#endif

} // namespace
#endif

