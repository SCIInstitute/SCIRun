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

#include <Core/Datatypes/Legacy/Field/ImageMesh.h>
#include <Core/Datatypes/Legacy/Field/StructQuadSurfMesh.h>
#include <Core/Basis/QuadElementWeights.h>
#include <Core/Datatypes/Legacy/Field/VMeshShared.h>


/// Only include this class if we included Image Support
#if (SCIRUN_IMAGE_SUPPORT > 0) || (SCIRUN_STRUCTQUADSURF_SUPPORT > 0)

using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;

namespace SCIRun {


/// This is the virtual interface to the curve mesh
/// This class lives besides the real mesh class for now and solely provides
/// an interface. In the future however when dynamic compilation is gone
/// this should be put into the ImageMesh class.
template<class MESH> class VImageMesh;
template<class MESH> class VStructQuadSurfMesh;

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.

template<class MESH>
class VImageMesh : public VMeshShared<MESH> {
  public:

    virtual bool is_imagemesh()          { return (true); }

    VImageMesh(MESH* mesh) : VMeshShared<MESH>(mesh) 
    {
      DEBUG_CONSTRUCTOR("VImageMesh") 
      this->ni_ = this->mesh_->ni_;
      this->nj_ = this->mesh_->nj_;
    }
    
    virtual ~VImageMesh()
    {
      DEBUG_DESTRUCTOR("VImageMesh")     
    }
    
    virtual void size(VMesh::Node::size_type& size) const;
    virtual void size(VMesh::ENode::size_type& size) const;
    virtual void size(VMesh::Edge::size_type& size) const;
    virtual void size(VMesh::Face::size_type& size) const;
    virtual void size(VMesh::Elem::size_type& size) const;
    virtual void size(VMesh::DElem::size_type& size) const;
    
    virtual void get_nodes(VMesh::Node::array_type& nodes, 
                           VMesh::Edge::index_type i) const;
    virtual void get_nodes(VMesh::Node::array_type& nodes, 
                           VMesh::Face::index_type i) const;
    virtual void get_nodes(VMesh::Node::array_type& nodes, 
                           VMesh::Elem::index_type i) const;
    virtual void get_nodes(VMesh::Node::array_type& nodes, 
                           VMesh::DElem::index_type i) const;

    virtual void get_enodes(VMesh::ENode::array_type& edges, 
                            VMesh::Face::index_type i) const;
    virtual void get_enodes(VMesh::ENode::array_type& edges, 
                            VMesh::Elem::index_type i) const;
    virtual void get_enodes(VMesh::ENode::array_type& edges, 
                            VMesh::DElem::index_type i) const;
    
    virtual void get_edges(VMesh::Edge::array_type& edges, 
                           VMesh::Node::index_type i) const;
    virtual void get_edges(VMesh::Edge::array_type& edges, 
                           VMesh::Face::index_type i) const;
    virtual void get_edges(VMesh::Edge::array_type& edges, 
                           VMesh::Elem::index_type i) const;
    virtual void get_edges(VMesh::Edge::array_type& edges, 
                           VMesh::DElem::index_type i) const;

    virtual void get_faces(VMesh::Face::array_type& faces, 
                           VMesh::Node::index_type i) const;
    virtual void get_faces(VMesh::Face::array_type& faces, 
                           VMesh::Edge::index_type i) const;
    virtual void get_faces(VMesh::Face::array_type& faces, 
                           VMesh::Elem::index_type i) const;
    virtual void get_faces(VMesh::Face::array_type& faces, 
                           VMesh::DElem::index_type i) const;

    virtual void get_elems(VMesh::Elem::array_type& elems, 
                           VMesh::Node::index_type i) const;
    virtual void get_elems(VMesh::Elem::array_type& elems, 
                           VMesh::Edge::index_type i) const;
    virtual void get_elems(VMesh::Elem::array_type& elems, 
                           VMesh::Face::index_type i) const;
    virtual void get_elems(VMesh::Elem::array_type& elems, 
                           VMesh::DElem::index_type i) const;

    virtual void get_delems(VMesh::DElem::array_type& delems, 
                            VMesh::Edge::index_type i) const;
    virtual void get_delems(VMesh::DElem::array_type& delems, 
                            VMesh::Face::index_type i) const;
    virtual void get_delems(VMesh::DElem::array_type& delems, 
                            VMesh::Elem::index_type i) const;

    /// Get the center of a certain mesh element
    virtual void get_center(Point &point, VMesh::Node::index_type i) const;
    virtual void get_center(Point &point, VMesh::ENode::index_type i) const;
    virtual void get_center(Point &point, VMesh::Edge::index_type i) const;
    virtual void get_center(Point &point, VMesh::Face::index_type i) const;
    virtual void get_center(Point &point, VMesh::Elem::index_type i) const;
    virtual void get_center(Point &point, VMesh::DElem::index_type i) const;

    /// Get the centers of a series of nodes
    virtual void get_centers(Point* points, const VMesh::Node::array_type& array) const;
    virtual void get_centers(Point* points, const VMesh::Elem::array_type& array) const;

    virtual double get_size(VMesh::Node::index_type i) const;
    virtual double get_size(VMesh::Edge::index_type i) const;
    virtual double get_size(VMesh::Face::index_type i) const;
    virtual double get_size(VMesh::Elem::index_type i) const;
    virtual double get_size(VMesh::DElem::index_type i) const;
                                                      
    virtual bool locate(VMesh::Node::index_type &i, const Point &point) const;
    virtual bool locate(VMesh::Elem::index_type &i, const Point &point) const;
    virtual bool locate(VMesh::Elem::index_type &i,
                        VMesh::coords_type &coords, 
                        const Point &point) const;

    virtual bool find_closest_node(double& pdist, 
                                   Point &result,
                                   VMesh::Node::index_type& elem, 
                                   const Point &p) const;

    virtual bool find_closest_node(double& pdist, 
                                   Point &result,
                                   VMesh::Node::index_type& elem, 
                                   const Point &p,
                                   double maxdist) const;
                                   
    virtual bool find_closest_elem(double& pdist, 
                                   Point& result,
                                   VMesh::coords_type& coords,
                                   VMesh::Elem::index_type& elem, 
                                   const Point& p) const;
                                   
    virtual bool find_closest_elem(double& pdist, 
                                   Point& result,
                                   VMesh::coords_type& coords,
                                   VMesh::Elem::index_type& elem, 
                                   const Point& p,
                                   double maxdist) const;
                                   
    virtual bool find_closest_elems(double& pdist, Point &result, 
                                      VMesh::Elem::array_type& elems, 
                                      const Point &p) const;
                                      
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

    virtual void get_dimensions(VMesh::dimension_type& dims);
    virtual void get_elem_dimensions(VMesh::dimension_type& dims);

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
    void get_nodes_from_edge(ARRAY &array, INDEX idx) const
    {
      const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);

      array.resize(2);

      const VMesh::index_type j_idx = xidx - (this->ni_-1) * this->nj_;
      if (j_idx >= 0)
      {
        const VMesh::index_type i = j_idx / (this->nj_ - 1);
        const VMesh::index_type j = j_idx % (this->nj_ - 1);
        array[0] = typename ARRAY::value_type(i+this->ni_*j);
        array[1] = typename ARRAY::value_type(i+this->ni_*(j+1));
      }
      else
      {
        const VMesh::index_type i = idx % (this->ni_ - 1);
        const VMesh::index_type j = idx / (this->ni_ - 1);
        array[0] = typename ARRAY::value_type(i+this->ni_*j);
        array[1] = typename ARRAY::value_type(i+1+this->ni_*j);
      }
    }


    template <class ARRAY, class INDEX>
    void get_nodes_from_face(ARRAY &array, INDEX idx) const
    {
      const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);
      const VMesh::index_type i = xidx % (this->ni_-1);
      const VMesh::index_type j = xidx / (this->ni_-1);
      
      array.resize(4);
      array[0] = typename ARRAY::value_type(i+this->ni_*j);
      array[1] = typename ARRAY::value_type(i+1+this->ni_*j);
      array[2] = typename ARRAY::value_type(i+1+this->ni_*(j+1));
      array[3] = typename ARRAY::value_type(i+this->ni_*(j+1));
    }


    template <class ARRAY, class INDEX>
    void get_nodes_from_elem(ARRAY &array, INDEX idx) const
    {
      get_nodes_from_face(array,idx);
    }
    
    template <class ARRAY, class INDEX>
    void get_edges_from_face(ARRAY &array,INDEX idx) const
    {
      const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);
      const VMesh::index_type i = xidx % (this->ni_-1);
      const VMesh::index_type j = xidx / (this->ni_-1);
      
      array.resize(4);

      const VMesh::index_type j_idx = (this->ni_-1) * this->nj_;

      array[0] = typename ARRAY::value_type(i+j*(this->ni_-1));
      array[1] = typename ARRAY::value_type(i+(j+1)*(this->ni_-1));
      array[2] = typename ARRAY::value_type(i*(this->nj_-1)+j+j_idx);
      array[3] = typename ARRAY::value_type((i+1)*(this->nj_-1)+j+j_idx);
    }


    template <class ARRAY, class INDEX>
    void get_edges_from_node(ARRAY &array,INDEX idx) const
    {
      const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);
      const VMesh::index_type i = xidx % (this->ni_);
      const VMesh::index_type j = xidx / (this->ni_);

      const VMesh::index_type j_idx = (this->ni_-1) * this->nj_;
      
      array.clear();

      if (i < (this->ni_-1))
        array.push_back(typename ARRAY::value_type(i+j*(this->ni_-1)));
      if (i > 0)
        array.push_back(typename ARRAY::value_type(i-1+j*(this->ni_-1)));
      
      if (j < (this->nj_-1))
        array.push_back(typename ARRAY::value_type(i*(this->nj_-1)+j+j_idx));
      if (j > 0)
        array.push_back(typename ARRAY::value_type(i*(this->nj_-1)+j-1+j_idx));
    }

    template <class ARRAY, class INDEX>
    void get_edges_from_elem(ARRAY &array, INDEX idx) const
    {
      get_edges_from_face(array,idx);
    }
    
    template <class ARRAY, class INDEX>
    void get_faces_from_node(ARRAY &array, INDEX idx) const
    {
      const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);
      const VMesh::index_type i = xidx % this->ni_;
      const VMesh::index_type j = xidx / this->ni_;
      
      array.reserve(4);
      array.clear();

      const VMesh::index_type i0 = i > 0 ? i-1 : 0;
      const VMesh::index_type j0 = j > 0 ? j-1 : 0;

      const VMesh::index_type i1 = i < this->ni_-1 ? i+1 : this->ni_-1;
      const VMesh::index_type j1 = j < this->nj_-1 ? j+1 : this->nj_-1;

      const VMesh::index_type mj = (this->ni_-1);
      VMesh::index_type ii, jj;
      for (jj = j0; jj < j1; jj++)
        for (ii = i0; ii < i1; ii++)
          array.push_back(typename ARRAY::value_type(ii+jj*mj));
    }

    template <class ARRAY, class INDEX>
    void get_faces_from_edge(ARRAY &array, INDEX idx) const
    {
      VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);

      array.reserve(2);
      array.clear();
      
      const VMesh::index_type offset = (this->ni_-1)*this->nj_;
      const VMesh::index_type mj = (this->ni_-1);
      if (xidx < offset)
      {
        const VMesh::index_type j = xidx/(this->ni_-1); 
        const VMesh::index_type i = xidx - j*(this->ni_-1);
        if (j < (this->nj_-1)) array.push_back(typename ARRAY::value_type(i+mj*j));
        if (j > 0) array.push_back(typename ARRAY::value_type(i+mj*(j-1)));  
      }
      else
      {
        xidx = xidx - offset;
        const VMesh::index_type i = xidx/(this->nj_-1); 
        const VMesh::index_type j = xidx - i*(this->nj_-1);
        if (i < (this->ni_-1)) array.push_back(typename ARRAY::value_type(i+mj*j));
        if (i > 0) array.push_back(typename ARRAY::value_type(i-1+mj*j));  
      }
    }

    template <class INDEX>
    inline bool 
    locate_elem(INDEX idx,Point p) const
    {
      const Point r = this->mesh_->transform_.unproject(p);

      const double epsilon = 1e-8;
      double ii = r.x();
      double jj = r.y();

      const double nii = static_cast<double>(this->ni_-1);
      const double njj = static_cast<double>(this->nj_-1);

      if (ii>nii && (ii-epsilon)<nii) ii=nii-epsilon;
      if (jj>njj && (jj-epsilon)<njj) jj=njj-epsilon;
      if (ii<0 && ii>-epsilon) ii=0;
      if (jj<0 && jj>-epsilon) jj=0;
      
      const index_type i = static_cast<index_type>(floor(ii));
      const index_type j = static_cast<index_type>(floor(jj));

      if (i < (int)(this->ni_-1) && i >= 0 &&
          j < (int)(this->nj_-1) && j >= 0)
      {
        idx = i+(this->ni_-1)*j;
        return (true);
      }

      return (false);  
    }


    template <class INDEX>
    inline bool 
    locate_elem(INDEX idx,VMesh::coords_type coords, Point p) const
    {
      const Point r = this->mesh_->transform_.unproject(p);

      const double epsilon = 1e-8;
   
      double ii = r.x();
      double jj = r.y();

      const double nii = static_cast<double>(this->ni_-1);
      const double njj = static_cast<double>(this->nj_-1);

      if (ii>nii && (ii-epsilon)<nii) ii=nii-epsilon;
      if (jj>njj && (jj-epsilon)<njj) jj=njj-epsilon;
      if (ii<0 && ii>-epsilon) ii=0;
      if (jj<0 && jj>-epsilon) jj=0;
      
      const index_type i = static_cast<index_type>(floor(ii));
      const index_type j = static_cast<index_type>(floor(jj));

      coords.resize(2);
      coords[0] = ii - floor(ii);
      coords[1] = jj - floor(jj);

      if (i < (int)(this->ni_-1) && i >= 0 &&
          j < (int)(this->nj_-1) && j >= 0)
      {
        idx = i+(this->ni_-1)*j;
        return (true);
      }
      return (false);  
    }

    template <class INDEX>
    inline bool 
    locate_node(INDEX idx,Point p) const
    {
      if (this->ni_ == 0 || this->nj_ == 0) return (false);
    
      const Point r = this->mesh_->transform_.unproject(p);

      double rx = floor(r.x() + 0.5);
      double ry = floor(r.y() + 0.5);

      const double nii = static_cast<double>(this->ni_-1);
      const double njj = static_cast<double>(this->nj_-1);
      
      if (rx < 0.0) rx = 0.0; if (rx > nii) rx = nii;
      if (ry < 0.0) ry = 0.0; if (ry > njj) ry = njj;
      
      index_type i = static_cast<index_type>(rx);
      index_type j = static_cast<index_type>(ry);
    
      idx = i+this->ni_*j;

      return (true);
    }
    
    template <class INDEX>
    inline void 
    to_index(typename MESH::Node::index_type &index, INDEX idx) const
    {
      const VMesh::index_type i = idx % this->ni_;
      const VMesh::index_type j = idx / this->ni_;
      index = typename MESH::Node::index_type(this->mesh_, i, j);
    }

    template <class INDEX>
    inline void 
    to_index(typename MESH::Edge::index_type &index, INDEX idx) const
    {
      index = typename MESH::Edge::index_type(idx);
    }

    template <class INDEX>
    inline void 
    to_index(typename MESH::Face::index_type &index, INDEX idx) const
    {
      const VMesh::index_type i = idx % (this->ni_-1);
      const VMesh::index_type j = idx / (this->ni_-1);
      index = typename MESH::Face::index_type(this->mesh_, i, j);
    }


    template<class ARRAY, class INDEX>
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
VMesh* CreateVImageMesh(ImageMesh<QuadBilinearLgn<Point> >* mesh)
{
  return new VImageMesh<ImageMesh<QuadBilinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID ImageMesh_MeshID1(ImageMesh<QuadBilinearLgn<Point> >::type_name(-1),
                  ImageMesh<QuadBilinearLgn<Point> >::mesh_maker,
                  ImageMesh<QuadBilinearLgn<Point> >::image_maker);
                  

/// Most of the topology functions are completely redone as we now use single
/// valued iterators and indices


/// Functions for determining the number of indices that are used
template <class MESH>
void
VImageMesh<MESH>::
size(VMesh::Node::size_type& sz) const
{
  sz = VMesh::Node::size_type(this->ni_*this->nj_);
}

template <class MESH>
void
VImageMesh<MESH>::
size(VMesh::ENode::size_type& sz) const
{
  sz = VMesh::ENode::size_type((this->ni_-1) * 
                                (this->nj_) + (this->ni_) * (this->nj_ -1));
  if (sz < 0) sz = 0;
}

template <class MESH>
void
VImageMesh<MESH>::
size(VMesh::Edge::size_type& sz) const
{
  sz = VMesh::Edge::size_type((this->ni_-1) * 
                                  (this->nj_) + (this->ni_) * (this->nj_ -1));
  if (sz < 0) sz = 0;
}

template <class MESH>
void
VImageMesh<MESH>::
size(VMesh::Face::size_type& sz) const
{
  sz = VMesh::Face::size_type((this->ni_-1)*(this->nj_-1));
  if (sz < 0) sz = 0;
}


template <class MESH>
void
VImageMesh<MESH>::
size(VMesh::DElem::size_type& sz) const
{
  sz = VMesh::DElem::size_type((this->ni_-1) *
                                  (this->nj_) + (this->ni_) * (this->nj_ -1));
  if (sz < 0) sz = 0;
}

template <class MESH>
void
VImageMesh<MESH>::
size(VMesh::Elem::size_type& sz) const
{
  sz = VMesh::Elem::size_type((this->ni_-1)*(this->nj_-1));
  if (sz < 0) sz = 0;
}

/// Topology functions


template <class MESH>
void
VImageMesh<MESH>::
get_nodes(VMesh::Node::array_type &array, VMesh::Edge::index_type idx) const
{
  get_nodes_from_edge(array,idx);
}


template <class MESH>
void
VImageMesh<MESH>::
get_nodes(VMesh::Node::array_type &array, VMesh::Face::index_type idx) const
{
  get_nodes_from_face(array,idx);
}


template <class MESH>
void
VImageMesh<MESH>::
get_nodes(VMesh::Node::array_type &array, VMesh::Elem::index_type idx) const
{
  get_nodes_from_face(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_nodes(VMesh::Node::array_type &array, VMesh::DElem::index_type idx) const
{
  get_nodes_from_edge(array,idx);
}


template <class MESH>
void
VImageMesh<MESH>::
get_enodes(VMesh::ENode::array_type &array, VMesh::Face::index_type idx) const
{
  get_edges_from_face(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_enodes(VMesh::ENode::array_type &array, VMesh::Elem::index_type idx) const
{
  get_edges_from_face(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_enodes(VMesh::ENode::array_type &array,
                             VMesh::DElem::index_type idx) const
{
  array.resize(1); array[0] = VMesh::ENode::index_type(idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_edges(VMesh::Edge::array_type &array, VMesh::Node::index_type idx) const
{
  get_edges_from_node(array,idx);
}


template <class MESH>
void
VImageMesh<MESH>::
get_edges(VMesh::Edge::array_type &array, VMesh::Face::index_type idx) const
{
  get_edges_from_face(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_edges(VMesh::Edge::array_type &array, VMesh::DElem::index_type idx) const
{
  array.resize(1); array[0] = VMesh::Edge::index_type(idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_edges(VMesh::Edge::array_type &array, VMesh::Elem::index_type idx) const
{
  get_edges_from_face(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_faces(VMesh::Face::array_type &array, VMesh::Node::index_type idx) const
{
  get_faces_from_node(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_faces(VMesh::Face::array_type &array, VMesh::Edge::index_type idx) const
{
  get_faces_from_edge(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_faces(VMesh::Face::array_type &array, VMesh::Elem::index_type idx) const
{
  array.resize(1); array[0] = static_cast<VMesh::Face::index_type>(idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_faces(VMesh::Face::array_type &array, VMesh::DElem::index_type idx) const
{
  get_faces_from_edge(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_elems(VMesh::Elem::array_type &array, VMesh::Node::index_type idx) const
{
  get_faces_from_node(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_elems(VMesh::Elem::array_type &array, VMesh::Edge::index_type idx) const
{
  get_faces_from_edge(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_elems(VMesh::Elem::array_type &array, VMesh::Face::index_type idx) const
{
  array.resize(1); array[0] = static_cast<VMesh::Elem::index_type>(idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_elems(VMesh::Elem::array_type &array, VMesh::DElem::index_type idx) const
{
  get_faces_from_edge(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_delems(VMesh::DElem::array_type &array, VMesh::Face::index_type idx) const
{
  get_edges_from_face(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_delems(VMesh::DElem::array_type &array, VMesh::Edge::index_type idx) const
{
  array.resize(1); array[0] = VMesh::DElem::index_type(idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_delems(VMesh::DElem::array_type &array, VMesh::Elem::index_type idx) const
{
  get_edges_from_face(array,idx);
}

template <class MESH>
void
VImageMesh<MESH>::
get_center(Point &p, VMesh::Node::index_type idx) const
{
  const VMesh::index_type i = idx % this->ni_;
  const VMesh::index_type j = idx / this->ni_;
    
  Point pnt(static_cast<double>(i),static_cast<double>(j),0.0);
  p = this->mesh_->transform_.project(pnt);
}

template <class MESH>
void
VImageMesh<MESH>::
get_center(Point &p, VMesh::ENode::index_type idx) const
{
  get_center(p,VMesh::Edge::index_type(idx));
}

template <class MESH>
void
VImageMesh<MESH>::
get_center(Point &p, VMesh::Edge::index_type idx) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);
  const VMesh::index_type j_idx = xidx - (this->ni_-1) * this->nj_;
  
  if (j_idx >= 0)
  {
    const VMesh::index_type i = j_idx / (this->nj_ - 1);
    const VMesh::index_type j = j_idx % (this->nj_ - 1);
    
    Point pnt(static_cast<double>(i),static_cast<double>(j)+0.5,0.0);
    p = this->mesh_->transform_.project(pnt);
  }
  else
  {
    const VMesh::index_type i = idx % (this->ni_ - 1);
    const VMesh::index_type j = idx / (this->ni_ - 1);

    Point pnt(static_cast<double>(i)+0.5,static_cast<double>(j),0.0);
    p = this->mesh_->transform_.project(pnt);
  }
}

template <class MESH>
void
VImageMesh<MESH>::
get_center(Point &p, VMesh::Face::index_type idx) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);
  const VMesh::index_type i = xidx % (this->ni_-1);
  const VMesh::index_type j = xidx / (this->ni_-1);

  Point pnt(static_cast<double>(i)+0.5,static_cast<double>(j)+0.5,0.0);
  p = this->mesh_->transform_.project(pnt);
}

template <class MESH>
void
VImageMesh<MESH>::
get_center(Point &p, VMesh::Elem::index_type idx) const
{
  get_center(p,VMesh::Face::index_type(idx));
}

template <class MESH>
void
VImageMesh<MESH>::
get_center(Point &p, VMesh::DElem::index_type idx) const
{
  get_center(p,VMesh::Edge::index_type(idx));
}


template <class MESH>
void
VImageMesh<MESH>::get_centers(Point* points, const VMesh::Node::array_type& array) const
{
  for (size_t k=0; k <array.size(); k++)
  {
    const VMesh::index_type idx = array[k];
    const VMesh::index_type i = idx % this->ni_;
    const VMesh::index_type j = idx / this->ni_;
    
    Point pnt(static_cast<double>(i),static_cast<double>(j),0.0);
    points[k] = this->mesh_->transform_.project(pnt);  
  }
}                                     
 
template <class MESH>
void
VImageMesh<MESH>::get_centers(Point* points, const VMesh::Elem::array_type& array) const
{
  for (size_t k=0; k <array.size(); k++)
  {
    const VMesh::index_type idx = array[k];
    const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);
    const VMesh::index_type i = xidx % (this->ni_-1);
    const VMesh::index_type j = xidx / (this->ni_-1);

    Point pnt(static_cast<double>(i)+0.5,static_cast<double>(j)+0.5,0.0);
    points[k] = this->mesh_->transform_.project(pnt);
  }
} 


template <class MESH>
bool 
VImageMesh<MESH>::
locate(VMesh::Node::index_type &i, const Point &point) const
{
  return(locate_node(i,point));
}

template <class MESH>
bool 
VImageMesh<MESH>::
locate(VMesh::Elem::index_type &i, const Point &point) const
{
  return(locate_elem(i,point));
}

template <class MESH>
bool 
VImageMesh<MESH>::
locate(VMesh::Elem::index_type &i, 
       VMesh::coords_type& coords, 
       const Point &point) const
{
  return(locate_elem(i,coords,point));
}


template <class MESH>
bool 
VImageMesh<MESH>::
get_coords(VMesh::coords_type &coords, 
           const Point &point, 
           VMesh::Elem::index_type i) const
{
  typename MESH::Elem::index_type vi;
  to_index(vi,i);
  return(this->mesh_->get_coords(coords,point,vi));
}  
  
template <class MESH>
void 
VImageMesh<MESH>::
interpolate(Point &point, 
            const VMesh::coords_type &coords, 
            VMesh::Elem::index_type idx) const
{
  const VMesh::index_type i = idx % (this->ni_-1);
  const VMesh::index_type j = idx / (this->ni_-1);
  
  Point pnt(static_cast<double>(i)+static_cast<double>(coords[0]),
            static_cast<double>(j)+static_cast<double>(coords[1]),0.0);
  point = this->mesh_->transform_.project(pnt);
}

template <class MESH>
void 
VImageMesh<MESH>::
minterpolate(std::vector<Point> &point, 
             const std::vector<VMesh::coords_type> &coords, 
             VMesh::Elem::index_type idx) const
{
  const VMesh::index_type i = idx % (this->ni_-1);
  const VMesh::index_type j = idx / (this->ni_-1);

  point.resize(coords.size());
  for (size_t p=0; p<coords.size();p++)
  {
    Point pnt(static_cast<double>(i)+static_cast<double>(coords[p][0]),
              static_cast<double>(j)+static_cast<double>(coords[p][1]),0.0);
    point[p] = this->mesh_->transform_.project(pnt);  
  }
}

template <class MESH>
void 
VImageMesh<MESH>::
derivate(VMesh::dpoints_type &p, 
         const VMesh::coords_type &coords, 
         VMesh::Elem::index_type i) const
{
  typename MESH::Elem::index_type vi;
  to_index(vi,i);
  this->mesh_->derivate(coords,vi,p);
}

template <class MESH>
bool 
VImageMesh<MESH>::
get_neighbor(VMesh::Elem::index_type &neighbor, 
             VMesh::Elem::index_type from, 
             VMesh::DElem::index_type delem) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(from);
  const VMesh::index_type xdelem = static_cast<const VMesh::index_type>(delem);
  const VMesh::index_type i = xidx % (this->ni_-1);
  const VMesh::index_type j = xidx / (this->ni_-1);
  
  const int j_idx = (this->ni_-1) * this->nj_;
  
  if (xdelem == i+j*(this->ni_-1))
  {
    if (j <= 0) return (false);
    neighbor = VMesh::Elem::index_type(i+(j-1)*(this->ni_-1));
    return (true);
  }

  if (xdelem == i+(j+1)*(this->ni_-1))
  {
    if (j >= this->nj_-2) return (false);
    neighbor = VMesh::Elem::index_type(i+(j+1)*(this->ni_-1));
    return (true);
  }

  if (xdelem == i*(this->nj_-1)+j+j_idx)
  {
    if (i <= 0) return (false);
    neighbor = VMesh::Elem::index_type(i-1+j*(this->ni_-1));
    return (true);
  }

  if (xdelem == (i+1)*(this->nj_-1)+j+j_idx)
  {
    if (i >= this->ni_-2) return (false);
    neighbor = VMesh::Elem::index_type(i+1+j*(this->ni_-1));
    return (true);
  }

  return (false);
}


template <class MESH>
bool 
VImageMesh<MESH>::
get_neighbors(VMesh::Elem::array_type &neighbors, 
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
VImageMesh<MESH>::
get_neighbors(VMesh::Elem::array_type &array, 
              VMesh::Elem::index_type idx) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);
  const VMesh::index_type i = xidx % (this->ni_-1);
  const VMesh::index_type j = xidx / (this->ni_-1);
  
  array.reserve(4);
  array.clear();

  const VMesh::index_type mj = this->ni_-1;
  if (i > 0) array.push_back(VMesh::Elem::index_type((i-1)+mj*j));
  if (i < this->ni_-2) array.push_back(VMesh::Elem::index_type((i+1)+mj*j));

  if (j > 0) array.push_back(VMesh::Elem::index_type(i+mj*(j-1)));
  if (j < this->nj_-2) array.push_back(VMesh::Elem::index_type(i+mj*(j+1)));
}

template <class MESH>
void 
VImageMesh<MESH>::
get_neighbors(VMesh::Node::array_type &array, 
              VMesh::Node::index_type idx) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);
  const VMesh::index_type i = xidx % this->ni_;
  const VMesh::index_type j = xidx / this->ni_;
  
  array.reserve(4);
  array.clear();

  const VMesh::index_type mj = this->ni_;
  if (i > 0) array.push_back(VMesh::Node::index_type((i-1)+mj*j));
  if (i < this->ni_-1) array.push_back(VMesh::Node::index_type((i+1)+mj*j));

  if (j > 0) array.push_back(VMesh::Node::index_type(i+mj*(j-1)));
  if (j < this->nj_-1) array.push_back(VMesh::Node::index_type(i+mj*(j+1)));
}

// WE should prcompute these:
template <class MESH>
double
VImageMesh<MESH>::
get_size(VMesh::Node::index_type /*i*/) const
{
  return (0.0);
}

template <class MESH>
double
VImageMesh<MESH>::
get_size(VMesh::Edge::index_type i) const
{
  typename MESH::Edge::index_type vi; to_index(vi,i);
  return (this->mesh_->get_size(vi));
}

template <class MESH>
double
VImageMesh<MESH>::
get_size(VMesh::Face::index_type i) const
{
  typename MESH::Face::index_type vi; to_index(vi,i);
  return (this->mesh_->get_size(vi));
}

template <class MESH>
double
VImageMesh<MESH>::
get_size(VMesh::Elem::index_type i) const
{
  typename MESH::Elem::index_type vi; to_index(vi,i);
  return (this->mesh_->get_size(vi));
}

template <class MESH>
double
VImageMesh<MESH>::
get_size(VMesh::DElem::index_type i) const
{
  typename MESH::DElem::index_type vi; to_index(vi,i);
  return (this->mesh_->get_size(vi));
}


template <class MESH>
void 
VImageMesh<MESH>::
pwl_approx_edge(VMesh::coords_array_type &coords, 
                VMesh::Elem::index_type /*ci*/, 
                unsigned int which_edge,
                unsigned int div_per_unit) const
{
  this->basis_->approx_edge(which_edge, div_per_unit, coords);
}

template <class MESH>
void 
VImageMesh<MESH>::
pwl_approx_face(VMesh::coords_array2_type &coords, 
                VMesh::Elem::index_type /*ci*/, 
                unsigned int which_face,
                unsigned int div_per_unit) const
{
  this->basis_->approx_face(which_face, div_per_unit, coords);
}

template <class MESH>
void 
VImageMesh<MESH>::
get_random_point(Point &p, 
                 VMesh::Elem::index_type i,
                 FieldRNG &rng) const
{
  typename MESH::Elem::index_type vi; to_index(vi,i);
  this->mesh_->get_random_point(p,vi,rng);
}

template <class MESH>
void 
VImageMesh<MESH>::
get_dimensions(VMesh::dimension_type& dims)
{
  dims.resize(2);
  dims[0] = this->ni_;
  dims[1] = this->nj_;
}

template <class MESH>
void 
VImageMesh<MESH>::
get_elem_dimensions(VMesh::dimension_type& dims)
{
  dims.resize(2);
  dims[0] = this->ni_-1;
  dims[1] = this->nj_-1;
}

template <class MESH>
Transform
VImageMesh<MESH>::
get_transform() const
{
  return (this->mesh_->get_transform());
}

template <class MESH>
void
VImageMesh<MESH>::set_transform(const Transform& t)
{
  this->mesh_->set_transform(t);
}


template <class MESH>
double 
VImageMesh<MESH>::
det_jacobian(const VMesh::coords_type& /*coords*/, 
             VMesh::Elem::index_type /*idx*/) const
{
  return(this->mesh_->det_jacobian_);
}


template <class MESH>
void
VImageMesh<MESH>::
jacobian(const VMesh::coords_type& /*coords*/, 
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
VImageMesh<MESH>::
inverse_jacobian(const VMesh::coords_type& /*coords*/,
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
VImageMesh<MESH>::
scaled_jacobian_metric(const VMesh::Elem::index_type /*idx*/) const
{
  return(this->mesh_->scaled_jacobian_);
}

template <class MESH>
double 
VImageMesh<MESH>::
jacobian_metric(const VMesh::Elem::index_type /*idx*/) const
{
  return(this->mesh_->det_jacobian_);
}

template <class MESH>
bool
VImageMesh<MESH>::
find_closest_node(double& pdist,
                  Point &result, 
                  VMesh::Node::index_type& node,
                  const Point &p,
                  double maxdist) const
{
  bool ret = find_closest_node(pdist,result,node,p);
  if(!ret) return (false);
  if (maxdist < 0.0 || pdist < maxdist) return (true);
  return (false);
}

template <class MESH>
bool
VImageMesh<MESH>::
find_closest_node(double& pdist,
                  Point &result, 
                  VMesh::Node::index_type& node,
                  const Point &p) const
{
  if (this->ni_ == 0 || this->nj_ == 0) return (false);
  
  const Point r = this->mesh_->transform_.unproject(p);

  double rx = floor(r.x() + 0.5);
  double ry = floor(r.y() + 0.5);
  
  const double nii = static_cast<double>(this->ni_-1);
  const double njj = static_cast<double>(this->nj_-1);

  if (rx < 0.0) rx = 0.0; if (rx > nii) rx = nii;
  if (ry < 0.0) ry = 0.0; if (ry > njj) ry = njj;

  result = this->mesh_->transform_.project(Point(rx,ry,0.0)); 
  const index_type i = static_cast<index_type>(rx);
  const index_type j = static_cast<index_type>(ry);
  node = i+j*this->ni_;
  
  pdist = (p-result).length();
  return (true);
}


template <class MESH>
bool 
VImageMesh<MESH>::
find_closest_elem(double& pdist,
                  Point& result,
                  VMesh::coords_type& coords, 
                  VMesh::Elem::index_type& elem,
                  const Point& p) const
{
  if (this->ni_ == 0 || this->nj_ == 0) return (false);
  
  const Point r = this->mesh_->transform_.unproject(p);

  double ii = r.x();
  double jj = r.y();
  const double nii = static_cast<double>(this->ni_-2);
  const double njj = static_cast<double>(this->nj_-2);
   
  if (ii < 0.0) ii = 0.0; if (ii > nii) ii = nii;
  if (jj < 0.0) jj = 0.0; if (jj > njj) jj = njj;

  const double fi = floor(ii);
  const double fj = floor(jj);

  const index_type i = static_cast<index_type>(fi);
  const index_type j = static_cast<index_type>(fj);
  elem = i+j*(this->ni_-1);

  result = this->mesh_->transform_.project(Point(ii,jj,0));
  pdist = (p-result).length();

  coords.resize(2);
  coords[0] = (ii-fi);
  coords[1] = (jj-fj);

  return (true);
}

template <class MESH>
bool 
VImageMesh<MESH>::find_closest_elem(double& pdist, 
                                   Point &result,
                                   VMesh::coords_type& coords, 
                                   VMesh::Elem::index_type &elem,
                                   const Point &p,
                                   double maxdist) const
{
  bool ret = find_closest_elem(pdist,result,coords,elem,p);
  if(!ret) return (false);
  if (maxdist < 0.0 || pdist < maxdist) return (true);
  return (false);
}


template <class MESH>
bool 
VImageMesh<MESH>::
find_closest_elems(double& pdist,
                   Point &result,
                   VMesh::Elem::array_type &elems,
                   const Point &p) const
{
  if (this->ni_ == 0 || this->nj_ == 0) return (false);

  elems.clear();
    
  const double epsilon = 1e-8;
  
  const Point r = this->mesh_->transform_.unproject(p);

  double ii = r.x();
  double jj = r.y();
  const double nii = static_cast<double>(this->ni_-2);
  const double njj = static_cast<double>(this->nj_-2);
   
  if (ii < 0.0) ii = 0.0; if (ii > nii) ii = nii;
  if (jj < 0.0) jj = 0.0; if (jj > njj) jj = njj;

  const double fii = floor(ii);
  const double fjj = floor(jj);

  index_type i = static_cast<index_type>(fii);
  index_type j = static_cast<index_type>(fjj);
  
  index_type elem = i + j*(this->ni_-1);
  
  elems.push_back(elem);

  if ((fabs(fii-ii) < epsilon) && ((i-1)>0))
  {
    elem = i-1 + j*(this->ni_-1);
    elems.push_back(elem);  
  }
  
  if ((fabs(fii-(ii+1.0)) < epsilon) && (i<(this->ni_-1)))
  {
    elem = i+1 + j*(this->ni_-1);
    elems.push_back(elem);  
  }

  if ((fabs(fjj-jj) < epsilon) && ((j-1)>0))
  {
    elem = i + (j-1)*(this->ni_-1);
    elems.push_back(elem);  
  }
  
  if ((fabs(fjj-(jj+1.0)) < epsilon) && (j<(this->nj_-1)))
  {
    elem = i + (j+1)*(this->ni_-1);
    elems.push_back(elem);  
  }

  result = this->mesh_->transform_.project(Point(ii,jj,0));
  pdist = (p-result).length();

  return (true);
}


template <class MESH>
void
VImageMesh<MESH>::
get_interpolate_weights(const Point& point, 
                        VMesh::ElemInterpolate& ei,
                        int basis_order) const
{
  VMesh::Elem::index_type elem;
  StackVector<double,3> coords;

  ei.basis_order = basis_order;
  if(locate_elem(elem,coords,point))
  {
    ei.elem_index = elem;
  }
  else
  {
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
VImageMesh<MESH>::
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
VImageMesh<MESH>::
get_minterpolate_weights(const std::vector<Point>& point, 
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
VImageMesh<MESH>::
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
VImageMesh<MESH>::
get_gradient_weights(const VMesh::coords_type& coords, 
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
VImageMesh<MESH>::
get_gradient_weights(const Point& point, 
                     VMesh::ElemGradient& eg,
                     int basis_order) const
{
  double* inverse_jacobian = this->mesh_->inverse_jacobian_;

  VMesh::Elem::index_type elem;
  StackVector<double,3> coords;

  if(!(locate_elem(elem,coords,point)))
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


template <class MESH>
void
VImageMesh<MESH>::
get_mgradient_weights(const std::vector<Point>& point, 
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
VImageMesh<MESH>::
get_mgradient_weights(const std::vector<VMesh::coords_type>& coords, 
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




#if (SCIRUN_STRUCTQUADSURF_SUPPORT > 0)

template<class MESH>
class VStructQuadSurfMesh : public VImageMesh<MESH> {
public:

  virtual bool is_structquadsurfmesh() { return (true); }
  virtual bool is_imagemesh()          { return (false); }

  /// constructor and descructor
  VStructQuadSurfMesh(MESH* mesh) :
    VImageMesh<MESH>(mesh),
    points_(mesh->get_points())
  {
    DEBUG_CONSTRUCTOR("VStructQuadSurfMesh")  
  }

  virtual ~VStructQuadSurfMesh()
  {
    DEBUG_DESTRUCTOR("VStructQuadSurfMesh")  
  }

  friend class ElemData;

  class ElemData
  {
  public:
    ElemData(const VStructQuadSurfMesh<MESH>* vmesh,
             MESH* mesh, VMesh::Elem::index_type idx) :
      index_(idx),
      points_(mesh->get_points())    
    {
      vmesh->get_nodes_from_face(nodes_,idx);
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

  private:
    VMesh::Face::index_type  index_;
    const Array2<Point>&      points_;
    VMesh::Node::array_type  nodes_;
  };
  
  
  /// Get the center of a certain mesh element
  virtual void get_center(Point &point, VMesh::Node::index_type i) const;
  virtual void get_center(Point &point, VMesh::ENode::index_type i) const;
  virtual void get_center(Point &point, VMesh::Edge::index_type i) const;
  virtual void get_center(Point &point, VMesh::Face::index_type i) const;
  virtual void get_center(Point &point, VMesh::Elem::index_type i) const;
  virtual void get_center(Point &point, VMesh::DElem::index_type i) const;

  /// Get the centers of a series of nodes
  virtual void get_centers(Point* points, const VMesh::Node::array_type& array) const;
  virtual void get_centers(Point* points, const VMesh::Elem::array_type& array) const;
  
  virtual double get_size(VMesh::Node::index_type i) const;
  virtual double get_size(VMesh::Edge::index_type i) const;
  virtual double get_size(VMesh::Face::index_type i) const;
  virtual double get_size(VMesh::Elem::index_type i) const;
  virtual double get_size(VMesh::DElem::index_type i) const;
                                                  
  virtual bool locate(VMesh::Node::index_type &i, const Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, const Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, 
                      VMesh::coords_type& coords, 
                      const Point &point) const;

  virtual bool find_closest_node(double& pdist, Point &result,
                                 VMesh::Node::index_type& elem, 
                                 const Point &p) const;

  virtual bool find_closest_node(double& pdist, Point &result,
                                 VMesh::Node::index_type& elem, 
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

  virtual bool find_closest_elems(double& pdist, Point &result, 
                                  VMesh::Elem::array_type& elems, 
                                  const Point &p) const;

  virtual bool get_coords(VMesh::coords_type &coords, const Point &point, 
                                              VMesh::Elem::index_type i) const;
  virtual void interpolate(Point &p, 
                           const VMesh::coords_type &coords, 
                           VMesh::Elem::index_type i) const;
  virtual void minterpolate(std::vector<Point> &p, 
                            const std::vector<VMesh::coords_type> &coords, 
                            VMesh::Elem::index_type i) const;
                            
  virtual void derivate(VMesh::dpoints_type &p, const VMesh::coords_type &coords, 
                                              VMesh::Elem::index_type i) const;

  virtual void set_point(const Point &p, VMesh::Node::index_type i);
  
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
                                                                          
protected:
  template <class INDEX>
  inline void to_index(typename ImageMesh<typename MESH::basis_type>::
                       Node::index_type &index, INDEX idx) const
  {
    const VMesh::index_type i = idx % this->ni_;
    const VMesh::index_type j = idx / this->ni_;
    index = typename ImageMesh<typename MESH::basis_type>::Node::
                                    index_type(this->mesh_, i, j);  
  }

  template <class INDEX>
  inline void to_index(typename ImageMesh<typename MESH::basis_type>::
                       Edge::index_type &index, INDEX idx) const
    {index = typename ImageMesh<typename MESH::basis_type>::Edge::
                                                    index_type(idx);}

  template <class INDEX>
  inline  void to_index(typename ImageMesh<typename MESH::basis_type>::
                        Face::index_type &index, INDEX idx) const
  {
    const VMesh::index_type i = idx % (this->ni_-1);
    const VMesh::index_type j = idx / (this->ni_-1);
    index = typename ImageMesh<typename MESH::basis_type>::Face::
                                        index_type(this->mesh_, i, j); 
  }

  template<class ARRAY, class INDEX>
  void
  inv_jacobian(const ARRAY& coords, INDEX idx, double* Ji) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(this,this->mesh_,idx);
    this->basis_->derivate(coords,ed,Jv);
    double J[9];
    Vector Jv2 = Cross(Vector(Jv[0]),Vector(Jv[1]));
    Jv2.normalize();
    J[0] = Jv[0].x();
    J[1] = Jv[0].y();
    J[2] = Jv[0].z();
    J[3] = Jv[1].x();
    J[4] = Jv[1].y();
    J[5] = Jv[1].z();
    J[6] = Jv2.x();
    J[7] = Jv2.y();
    J[8] = Jv2.z();
    
    InverseMatrix3x3(J,Ji);
  }

  
  Array2<Point>&     points_;
};

/// Functions for creating the virtual interface for specific mesh types
/// These are similar to compare maker and only serve to instantiate the class

/// Currently there are only 3 variations of this mesh available
/// 1) linear interpolation

/// Add the LINEAR virtual interface and the meshid for creating it 

/// Create virtual interface 
VMesh* CreateVStructQuadSurfMesh(StructQuadSurfMesh<QuadBilinearLgn<Point> >* mesh)
{
  return new VStructQuadSurfMesh<StructQuadSurfMesh<QuadBilinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID StructQuadSurfMesh_MeshID1(StructQuadSurfMesh<QuadBilinearLgn<Point> >::type_name(-1),
                  StructQuadSurfMesh<QuadBilinearLgn<Point> >::mesh_maker,
                  StructQuadSurfMesh<QuadBilinearLgn<Point> >::structquadsurf_maker);


template <class MESH>
void
VStructQuadSurfMesh<MESH>::
get_center(Point &p, VMesh::Node::index_type idx) const
{
  p = points_[idx];
}

template <class MESH>
void
VStructQuadSurfMesh<MESH>::
get_center(Point &p, VMesh::ENode::index_type idx) const
{
  get_center(p,VMesh::Edge::index_type(idx));
}

template <class MESH>
void
VStructQuadSurfMesh<MESH>::
get_center(Point &p, VMesh::Edge::index_type idx) const
{
  StackVector<VMesh::index_type,2> nodes;
  this->get_nodes_from_edge(nodes,idx);
  
  p = points_[nodes[0]];
  p += points_[nodes[1]];
  p *= 0.5;
}

template <class MESH>
void
VStructQuadSurfMesh<MESH>::
get_center(Point &p, VMesh::Face::index_type idx) const
{
  StackVector<VMesh::index_type,4> nodes;
  this->get_nodes_from_face(nodes,idx);
  
  p = points_[nodes[0]];
  p += points_[nodes[1]];
  p += points_[nodes[2]];
  p += points_[nodes[3]];
  p *= 0.25;
}

template <class MESH>
void
VStructQuadSurfMesh<MESH>::
get_center(Point &p, VMesh::Elem::index_type idx) const
{
  get_center(p,VMesh::Face::index_type(idx));
}

template <class MESH>
void
VStructQuadSurfMesh<MESH>::
get_center(Point &p, VMesh::DElem::index_type idx) const
{
  get_center(p,VMesh::Edge::index_type(idx));
}


template <class MESH>
void
VStructQuadSurfMesh<MESH>::get_centers(Point* points, const VMesh::Node::array_type& array) const
{
  for (size_t j=0; j <array.size(); j++)
  {
    points[j] = points_[array[j]];
  }
}                                     
 
template <class MESH>
void
VStructQuadSurfMesh<MESH>::get_centers(Point* points, const VMesh::Elem::array_type& array) const
{
  Point p;
  StackVector<VMesh::index_type,4> nodes;
  for (size_t j=0; j <array.size(); j++)
  {
    VMesh::Elem::index_type idx = array[j];
    
    this->get_nodes_from_face(nodes,idx);  
    p = points_[nodes[0]];
    p += points_[nodes[1]];
    p += points_[nodes[2]];
    p += points_[nodes[3]];
    p *= 0.25;
  
    points[j] = p;
  }
} 


template <class MESH>
double
VStructQuadSurfMesh<MESH>::
get_size(VMesh::Node::index_type /*idx*/) const
{
  return (0.0);
}

template <class MESH>
double
VStructQuadSurfMesh<MESH>::
get_size(VMesh::Edge::index_type idx) const
{
  StackVector<VMesh::index_type,2> nodes;
  this->get_nodes_from_edge(nodes,idx);
  
  const Point &p0 = points_[nodes[0]];
  const Point &p1 = points_[nodes[1]];
  
  return (p1 - p0).length();
}

template <class MESH>
double
VStructQuadSurfMesh<MESH>::
get_size(VMesh::Face::index_type idx) const
{
  StackVector<VMesh::index_type,4> nodes;
  this->get_nodes_from_face(nodes,idx);
  
  const Point &p0 = points_[nodes[0]];
  const Point &p1 = points_[nodes[1]];
  const Point &p2 = points_[nodes[2]];
  const Point &p3 = points_[nodes[3]];
  
  return ((Cross(p0-p1,p2-p0)).length()+(Cross(p0-p3,p2-p0)).length())*0.5;
}

template <class MESH>
double
VStructQuadSurfMesh<MESH>::
get_size(VMesh::Elem::index_type idx) const
{
  return(get_size(VMesh::Face::index_type(idx)));
}

template <class Basis>
double
VStructQuadSurfMesh<Basis>::
get_size(VMesh::DElem::index_type idx) const
{
  return(get_size(VMesh::Edge::index_type(idx)));
}


template <class MESH>
bool 
VStructQuadSurfMesh<MESH>::
locate(VMesh::Node::index_type &idx, const Point &point) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename ImageMesh<typename MESH::basis_type>::Node::index_type i;
  to_index(i,idx);
  bool ret = this->mesh_->locate(i,point);
  idx = static_cast<VMesh::Node::index_type>(i);
  return (ret);
}

template <class MESH>
bool 
VStructQuadSurfMesh<MESH>::
locate(VMesh::Elem::index_type &idx, const Point &point) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename ImageMesh<typename MESH::basis_type>::Elem::index_type i;
  to_index(i,idx);
  bool ret = this->mesh_->locate_elem(i,point);
  idx = static_cast<VMesh::Elem::index_type>(i);
  return (ret);
}

template <class MESH>
bool 
VStructQuadSurfMesh<MESH>::
locate(VMesh::Elem::index_type& idx, 
       VMesh::coords_type& coords, 
       const Point &point) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename ImageMesh<typename MESH::basis_type>::Elem::index_type i;
  to_index(i,idx);
  bool ret = this->mesh_->locate_elem(i,coords,point);
  idx = static_cast<VMesh::Elem::index_type>(i);
  return (ret);
}

template <class MESH>
bool 
VStructQuadSurfMesh<MESH>::
find_closest_node(double& pdist, Point& result,
                  VMesh::Node::index_type &idx, 
                  const Point &point) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename ImageMesh<typename MESH::basis_type>::Node::index_type i;
  to_index(i,idx);
  bool ret = this->mesh_->find_closest_node(pdist,result,i,point,-1.0);
  idx = static_cast<VMesh::Node::index_type>(i);
  return (ret);
} 

template <class MESH>
bool 
VStructQuadSurfMesh<MESH>::
find_closest_node(double& pdist, Point& result,
                  VMesh::Node::index_type &idx, 
                  const Point &point,
                  double maxdist) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename ImageMesh<typename MESH::basis_type>::Node::index_type i;
  to_index(i,idx);
  bool ret = this->mesh_->find_closest_node(pdist,result,i,point,maxdist);
  idx = static_cast<VMesh::Node::index_type>(i);
  return (ret);
} 

template <class MESH>
bool 
VStructQuadSurfMesh<MESH>::
find_closest_elem(double& pdist, 
                  Point& result,
                  VMesh::coords_type& coords,
                  VMesh::Elem::index_type &idx, 
                  const Point &point) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename ImageMesh<typename MESH::basis_type>::Elem::index_type i;
  to_index(i,idx);
  bool ret = this->mesh_->find_closest_elem(pdist,result,coords,i,point,-1.0);
  idx = static_cast<VMesh::Elem::index_type>(i);
  return (ret);
} 

template <class MESH>
bool 
VStructQuadSurfMesh<MESH>::
find_closest_elem(double& pdist, 
                  Point& result,
                  VMesh::coords_type& coords,
                  VMesh::Elem::index_type &idx, 
                  const Point &point,
                  double maxdist) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename ImageMesh<typename MESH::basis_type>::Elem::index_type i;
  to_index(i,idx);
  bool ret = this->mesh_->find_closest_elem(pdist,result,coords,i,point,maxdist);
  idx = static_cast<VMesh::Elem::index_type>(i);
  return (ret);
} 

template <class MESH>
bool 
VStructQuadSurfMesh<MESH>::
find_closest_elems(double& pdist, Point& result,
                   VMesh::Elem::array_type &idx, 
                   const Point &point) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename ImageMesh<typename MESH::basis_type>::Elem::array_type i;
  bool ret = this->mesh_->find_closest_elems(pdist,result,i,point);
  this->convert_vector(idx,i);
  return (ret);
} 


template <class MESH>
bool 
VStructQuadSurfMesh<MESH>::
get_coords(VMesh::coords_type &coords, 
           const Point &point, 
           VMesh::Elem::index_type idx) const
{
  ElemData ed(this,this->mesh_, idx);
  return this->basis_->get_coords(coords, point, ed);
}  
  
template <class MESH>
void 
VStructQuadSurfMesh<MESH>::
interpolate(Point &p, 
            const VMesh::coords_type &coords, 
            VMesh::Elem::index_type idx) const
{
  ElemData ed(this,this->mesh_, idx);
  p = this->basis_->interpolate(coords, ed);
}

template <class MESH>
void 
VStructQuadSurfMesh<MESH>::
minterpolate(std::vector<Point> &point, 
             const std::vector<VMesh::coords_type> &coords, 
             VMesh::Elem::index_type idx) const
{
  point.resize(coords.size());
  ElemData ed(this,this->mesh_, idx);
  
  for (size_t j=0;j<coords.size();j++)
  {
    point[j] = this->basis_->interpolate(coords[j], ed);
  }
}

template <class MESH>
void 
VStructQuadSurfMesh<MESH>::
derivate(VMesh::dpoints_type &dp, 
         const VMesh::coords_type &coords, 
         VMesh::Elem::index_type idx) const
{
  ElemData ed(this,this->mesh_, idx);
  this->basis_->derivate(coords, ed, dp);
}

template <class MESH>
void 
VStructQuadSurfMesh<MESH>::
get_random_point(Point &p, 
                 VMesh::Elem::index_type idx,
                 FieldRNG &rng) const
{
  typename ImageMesh<typename MESH::basis_type>::Elem::index_type vi; 
  to_index(vi,idx);
  this->mesh_->get_random_point(p,vi,rng);
}

template <class MESH>
void 
VStructQuadSurfMesh<MESH>::
set_point(const Point &point, VMesh::Node::index_type idx)
{
  points_[idx] = point;
}

template <class MESH>
Point*
VStructQuadSurfMesh<MESH>::
get_points_pointer() const
{
  if (points_.size() == 0) return (0);
  return (&(points_[0]));
}

template <class Basis>
double 
VStructQuadSurfMesh<Basis>::
det_jacobian(const VMesh::coords_type& coords,
             VMesh::Elem::index_type idx) const
{
    double J[9];
    jacobian(coords,idx,J);
    return (DetMatrix3x3(J));
}


template <class Basis>
void
VStructQuadSurfMesh<Basis>::
jacobian(const VMesh::coords_type& coords,
         VMesh::Elem::index_type idx, 
         double* J) const
{
    StackVector<Point,2> Jv;
    ElemData ed(this,this->mesh_,idx);
    this->basis_->derivate(coords,ed,Jv);
    Vector Jv2 = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1]));
    Jv2.normalize();
    J[0] = Jv[0].x();
    J[1] = Jv[0].y();
    J[2] = Jv[0].z();
    J[3] = Jv[1].x();
    J[4] = Jv[1].y();
    J[5] = Jv[1].z();
    J[6] = Jv2.x();
    J[7] = Jv2.y();
    J[8] = Jv2.z();
}

template <class MESH>
double
VStructQuadSurfMesh<MESH>::
inverse_jacobian(const VMesh::coords_type& coords,
                 VMesh::Elem::index_type idx, 
                 double* Ji) const
{
  StackVector<Point,3> Jv;
  ElemData ed(this,this->mesh_,idx);
  this->basis_->derivate(coords,ed,Jv);
  double J[9];
  Vector Jv2 = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1]));
  Jv2.normalize();
  J[0] = Jv[0].x();
  J[1] = Jv[0].y();
  J[2] = Jv[0].z();
  J[3] = Jv[1].x();
  J[4] = Jv[1].y();
  J[5] = Jv[1].z();
  J[6] = Jv2.x();
  J[7] = Jv2.y();
  J[8] = Jv2.z();
  
  return (InverseMatrix3x3(J,Ji));
}



template <class MESH>
double 
VStructQuadSurfMesh<MESH>::
scaled_jacobian_metric(const VMesh::Elem::index_type idx) const
{
  StackVector<Point,3> Jv;
  ElemData ed(this,this->mesh_,idx);

  double temp;
  this->basis_->derivate(this->basis_->unit_center,ed,Jv);
  Jv.resize(3); 
  Vector v = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1]));
  v.normalize();
  Jv[2] = v.asPoint();
  double min_jacobian = ScaledDetMatrix3P(Jv);
  
  size_t num_vertices = this->basis_->number_of_vertices();
  for (size_t j=0;j < num_vertices;j++)
  {
    this->basis_->derivate(this->basis_->unit_vertices[j],ed,Jv);
    Jv.resize(3); 
    Vector v = Cross(Vector(Jv[0]),Vector(Jv[1])); v.normalize();
    Jv[2] = v.asPoint();    
    temp = ScaledDetMatrix3P(Jv);
    if(temp < min_jacobian) min_jacobian = temp;
  }
    
  return (min_jacobian);
}


template <class MESH>
double 
VStructQuadSurfMesh<MESH>::
jacobian_metric(VMesh::Elem::index_type idx) const
{
  StackVector<Point,3> Jv;
  ElemData ed(this,this->mesh_,idx);

  double temp;
  this->basis_->derivate(this->basis_->unit_center,ed,Jv);
  Jv.resize(3); 
  Vector v = Cross(Vector(Jv[0]),Vector(Jv[1])); v.normalize();
  Jv[2] = v.asPoint();
  double min_jacobian = DetMatrix3P(Jv);
  
  size_t num_vertices = this->basis_->number_of_vertices();
  for (size_t j=0;j < num_vertices;j++)
  {
    this->basis_->derivate(this->basis_->unit_vertices[j],ed,Jv);
    Jv.resize(3); 
    Vector v = Cross(Vector(Jv[0]),Vector(Jv[1])); v.normalize();
    Jv[2] = v.asPoint();
    temp = DetMatrix3P(Jv);
    if(temp < min_jacobian) min_jacobian = temp;
  }
    
  return (min_jacobian);
}


template <class MESH>
void
VStructQuadSurfMesh<MESH>::
get_interpolate_weights(const Point& point, 
                        VMesh::ElemInterpolate& ei,
                        int basis_order) const
{
  VMesh::Elem::index_type elem;
  
  if(locate(elem,point))
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
  get_coords(coords,point,elem);
  
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
VStructQuadSurfMesh<MESH>::
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
VStructQuadSurfMesh<MESH>::
get_minterpolate_weights(const std::vector<Point>& point, 
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
VStructQuadSurfMesh<MESH>::
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
VStructQuadSurfMesh<MESH>::
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
VStructQuadSurfMesh<MESH>::
get_gradient_weights(const Point& point, 
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
VStructQuadSurfMesh<MESH>::
get_mgradient_weights(const std::vector<Point>& point, 
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



template <class MESH>
void
VStructQuadSurfMesh<MESH>::
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

