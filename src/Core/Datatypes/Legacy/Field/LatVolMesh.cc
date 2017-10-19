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

#include <Core/Datatypes/Legacy/Field/LatVolMesh.h>
#include <Core/Datatypes/Legacy/Field/VMeshShared.h>
#include <Core/Datatypes/Legacy/Field/StructHexVolMesh.h>
#include <Core/Basis/HexElementWeights.h>

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Basis;


/// Only include this class if we included LatVol Support
#if (SCIRUN_LATVOL_SUPPORT > 0) || (SCIRUN_STRUCTHEXVOL_SUPPORT > 0)

namespace SCIRun {

/// This is the virtual interface to the curve mesh
/// This class lives besides the real mesh class for now and solely profides
/// an interface. In the future however when dynamic compilation is gone
/// this should be put into the LatVolMesh class.
template<class MESH> class VLatVolMesh;
template<class MESH> class VStructHexVolMesh;

/// This class is not exposed to the general interface and only the VMesh class
/// is accessed by other classes.

template<class MESH>
class VLatVolMesh : public VMeshShared<MESH> {
public:
  virtual bool is_latvolmesh()         { return (true); }

  /// constructor and descructor
  VLatVolMesh(MESH* mesh) : VMeshShared<MESH>(mesh) 
  {
    DEBUG_CONSTRUCTOR("VLatVolMesh")    
    // All we need to do most topology
    this->ni_ = mesh->ni_;
    this->nj_ = mesh->nj_;
    this->nk_ = mesh->nk_;
  }
  
  virtual ~VLatVolMesh() 
  {
    DEBUG_DESTRUCTOR("VLatVolMesh")      
  }

  virtual void size(VMesh::Node::size_type& size) const;
  virtual void size(VMesh::ENode::size_type& size) const;
  virtual void size(VMesh::Edge::size_type& size) const;
  virtual void size(VMesh::Face::size_type& size) const;
  virtual void size(VMesh::Cell::size_type& size) const;
  virtual void size(VMesh::Elem::size_type& size) const;
  virtual void size(VMesh::DElem::size_type& size) const;
  
  virtual void get_nodes(VMesh::Node::array_type& nodes, 
                         VMesh::Edge::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes, 
                         VMesh::Face::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes, 
                         VMesh::Cell::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes, 
                         VMesh::Elem::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes, 
                         VMesh::DElem::index_type i) const;

  virtual void get_enodes(VMesh::ENode::array_type& edges, 
                          VMesh::Face::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges, 
                          VMesh::Cell::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges, 
                          VMesh::Elem::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges, 
                          VMesh::DElem::index_type i) const;
                          
  virtual void get_edges(VMesh::Edge::array_type& edges, 
                         VMesh::Node::index_type i) const;  
  virtual void get_edges(VMesh::Edge::array_type& edges, 
                         VMesh::Face::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges, 
                         VMesh::Cell::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges, 
                         VMesh::Elem::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges, 
                         VMesh::DElem::index_type i) const;

  virtual void get_faces(VMesh::Face::array_type& faces, 
                         VMesh::Cell::index_type i) const;
  virtual void get_faces(VMesh::Face::array_type& faces, 
                         VMesh::Elem::index_type i) const;
  virtual void get_faces(VMesh::Face::array_type& faces, 
                         VMesh::DElem::index_type i) const;

  virtual void get_cells(VMesh::Cell::array_type& cells, 
                         VMesh::Node::index_type i) const;  
  virtual void get_cells(VMesh::Cell::array_type& cells, 
                         VMesh::Edge::index_type i) const;  
  virtual void get_cells(VMesh::Cell::array_type& cells, 
                         VMesh::Face::index_type i) const;  
  virtual void get_cells(VMesh::Cell::array_type& cells,
                         VMesh::Elem::index_type i) const;  
  virtual void get_cells(VMesh::Cell::array_type& cells, 
                         VMesh::DElem::index_type i) const;  
  
  virtual void get_elems(VMesh::Elem::array_type& elems, 
                         VMesh::Node::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems, 
                         VMesh::Edge::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems, 
                         VMesh::Face::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems, 
                         VMesh::Cell::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems, 
                          VMesh::DElem::index_type i) const;

  virtual void get_delems(VMesh::DElem::array_type& delems, 
                          VMesh::Face::index_type i) const;
  virtual void get_delems(VMesh::DElem::array_type& delems, 
                          VMesh::Cell::index_type i) const;
  virtual void get_delems(VMesh::DElem::array_type& delems, 
                          VMesh::Elem::index_type i) const;

  virtual bool get_elem(VMesh::Elem::index_type& elem, 
                        VMesh::Node::array_type& nodes) const;
  virtual bool get_delem(VMesh::DElem::index_type& delem, 
                         VMesh::Node::array_type& nodes) const;
  virtual bool get_cell(VMesh::Cell::index_type& cell, 
                        VMesh::Node::array_type& nodes) const;
  virtual bool get_face(VMesh::Face::index_type& face, 
                        VMesh::Node::array_type& nodes) const;
  virtual bool get_edge(VMesh::Edge::index_type& edge, 
                        VMesh::Node::array_type& nodes) const;

  /// Get the center of a certain mesh element
  virtual void get_center(Point &point, VMesh::Node::index_type i) const;
  virtual void get_center(Point &point, VMesh::ENode::index_type i) const;
  virtual void get_center(Point &point, VMesh::Edge::index_type i) const;
  virtual void get_center(Point &point, VMesh::Face::index_type i) const;
  virtual void get_center(Point &point, VMesh::Cell::index_type i) const;
  virtual void get_center(Point &point, VMesh::Elem::index_type i) const;
  virtual void get_center(Point &point, VMesh::DElem::index_type i) const;

  /// Get the centers of a series of nodes
  virtual void get_centers(Point* points, const VMesh::Node::array_type& array) const;
  virtual void get_centers(Point* points, const VMesh::Elem::array_type& array) const;

  virtual double get_size(VMesh::Node::index_type i) const;
  virtual double get_size(VMesh::Edge::index_type i) const;
  virtual double get_size(VMesh::Face::index_type i) const;
  virtual double get_size(VMesh::Cell::index_type i) const;
  virtual double get_size(VMesh::Elem::index_type i) const;
  virtual double get_size(VMesh::DElem::index_type i) const;
                                                    
  virtual bool locate(VMesh::Node::index_type &i, const Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, const Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, VMesh::coords_type &coords,
                      const Point &point) const;

  virtual bool locate(VMesh::Elem::array_type &i, const BBox &bbox) const;
  
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
 
  virtual bool find_closest_node(double& pdist, Point &result,
                                 VMesh::Node::index_type& elem, 
                                 const Point &p) const;

  virtual bool find_closest_node(double& pdist, Point &result,
                                 VMesh::Node::index_type& elem, 
                                 const Point &p, double maxdist) const;
                                 
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
    
  virtual void get_dimensions(VMesh::dimension_type& dim);
  virtual void get_elem_dimensions(VMesh::dimension_type& dim);  

  virtual Transform get_transform() const;
  virtual void set_transform(const Transform& t);

  /// Get all the information needed for interpolation:
  /// this includes weights and node indices
  /// As linear interpolation needs far less data from the mesh
  /// these classes are spilt out in linear, quadratic and cubic
  
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
    array.resize(2);
    const VMesh::index_type xidx = static_cast<VMesh::index_type>(idx);
    
    if (xidx < (this->ni_ - 1) * this->nj_ * this->nk_)
    {
      const VMesh::index_type i = xidx % (this->ni_ - 1);
      const VMesh::index_type jk = xidx / (this->ni_ - 1);
      const VMesh::index_type j = jk % this->nj_;
      const VMesh::index_type k = jk / this->nj_;
      const VMesh::index_type a = i+j*this->ni_+(this->ni_*this->nj_)*k;
      array[0] = typename ARRAY::value_type(a);
      array[1] = typename ARRAY::value_type(a+1);
    }
    else
    {
      const VMesh::index_type yidx = xidx - (this->ni_ - 1) * this->nj_ * this->nk_;
      if (yidx < (this->ni_ * (this->nj_ - 1) * this->nk_))
      {
        const VMesh::index_type j = yidx % (this->nj_ - 1);
        const VMesh::index_type ik = yidx / (this->nj_ - 1);
        const VMesh::index_type i = ik / this->nk_;
        const VMesh::index_type k = ik % this->nk_;

        const VMesh::index_type a = i+j*this->ni_+(this->ni_*this->nj_)*k;
        array[0] = typename ARRAY::value_type(a);
        array[1] = typename ARRAY::value_type(a+this->ni_);
      }
      else
      {
        const VMesh::index_type zidx = yidx - (this->ni_ * (this->nj_ - 1) * this->nk_);
        const VMesh::index_type k = zidx % (this->nk_ - 1);
        const VMesh::index_type ij = zidx / (this->nk_ - 1);
        const VMesh::index_type i = ij % this->ni_;
        const VMesh::index_type j = ij / this->ni_;

        const VMesh::index_type a = i+j*this->ni_+(this->ni_*this->nj_)*k;
        array[0] = typename ARRAY::value_type(a);
        array[1] = typename ARRAY::value_type(a+(this->nj_*this->ni_));
      }
    }
  }

  template<class ARRAY, class INDEX>
  inline bool get_cell_from_nodes(INDEX& /*idx*/, ARRAY& /*array*/) const
  {
    ASSERTFAIL("Get cell from nodes has not yet been implemented");
  }

  template<class ARRAY, class INDEX>
  inline bool get_edge_from_nodes(INDEX& /*idx*/, ARRAY& /*array*/) const
  {
    ASSERTFAIL("Get edge from nodes has not yet been implemented");
  }

  template <class ARRAY, class INDEX>
  inline bool get_face_from_nodes(INDEX& idx, ARRAY& array) const
  {
    VMesh::index_type f = array[0];
    for (size_t j=1;j<array.size();j++)
    {
      if (array[j] < f) f = array[j];
    }

    bool xdir = false;
    bool ydir = false;
    
    for (size_t j=0; j<array.size(); j++)
    {
      if (array[j] == f+1) xdir = true;
      if (array[j] == f+this->ni_) ydir = true;
    }

    VMesh::index_type i=-1,j=-1,jk=-1,k=-1;
    
    if (xdir && ydir)
    {
      i = f % this->ni_;
      jk = f / this->ni_;
      j = jk % this->nj_;
      k = jk / this->nj_;
      
      idx = i + j*(this->ni_-1)+k*(this->ni_-1)*(this->nj_-1);
      return (true);
    }
    else if (!xdir && ydir )
    {
      i = f % this->ni_;
      jk = f / this->ni_;
      j = jk % this->nj_;
      k = jk / this->nj_;

      idx = j + k*(this->nj_-1)+i*(this->nk_-1)*(this->nj_-1) + (this->ni_ - 1) * (this->nj_ - 1) * this->nk_;
      return (true);
    }
    else if (xdir && !ydir)
    {
      i = f % this->ni_;
      jk = f / this->ni_;
      j = jk % this->nj_;
      k = jk / this->nj_;

      idx = k + i*(this->nk_-1)+ j*(this->nk_-1)*(this->ni_-1) + (this->ni_ - 1) * (this->nj_ - 1) * this->nk_ + this->ni_ * (this->nj_ - 1) * (this->nk_ - 1);;
      return (true);    
    }
    else
    {
      ASSERTFAIL("get_delem failed");
      return (false);
    }
  }


  template <class ARRAY, class INDEX>
  void get_nodes_from_face(ARRAY &array, INDEX idx) const
  {
    array.resize(4);

    const VMesh::index_type xidx = static_cast<VMesh::index_type>(idx);
    if (xidx < (this->ni_ - 1) * (this->nj_ - 1) * this->nk_)
    {
      const VMesh::index_type i = xidx % (this->ni_ - 1);
      const VMesh::index_type jk = xidx / (this->ni_ - 1);
      const VMesh::index_type j = jk % (this->nj_ - 1);
      const VMesh::index_type k = jk / (this->nj_ - 1);
      const VMesh::index_type nij_ = this->ni_*this->nj_; 
      const VMesh::index_type a = i+j*this->ni_+k*nij_;
      
      if (k < (this->nk_-1))
      {
        array[0] = typename ARRAY::value_type(a);
        array[1] = typename ARRAY::value_type(a+1);
        array[2] = typename ARRAY::value_type(a+this->ni_+1);
        array[3] = typename ARRAY::value_type(a+this->ni_);
      }
      else
      {
        array[0] = typename ARRAY::value_type(a);
        array[3] = typename ARRAY::value_type(a+1);
        array[2] = typename ARRAY::value_type(a+this->ni_+1);
        array[1] = typename ARRAY::value_type(a+this->ni_);      
      }
    }
    else
    {
      const VMesh::index_type yidx = xidx - (this->ni_ - 1) * (this->nj_ - 1) * this->nk_;
      if (yidx < this->ni_ * (this->nj_ - 1) * (this->nk_ - 1))
      {
        const VMesh::index_type j = yidx % (this->nj_ - 1);
        const VMesh::index_type ik = yidx / (this->nj_ - 1);
        const VMesh::index_type k = ik % (this->nk_ - 1);
        const VMesh::index_type i = ik / (this->nk_ - 1);
        
        const VMesh::index_type nij_ = this->ni_*this->nj_; 
        const VMesh::index_type a = i+j*this->ni_+k*nij_;
        if (i < (this->ni_-1))
        {
          array[0] = typename ARRAY::value_type(a);
          array[1] = typename ARRAY::value_type(a+this->ni_);
          array[2] = typename ARRAY::value_type(a+this->ni_+nij_);
          array[3] = typename ARRAY::value_type(a+nij_);    
        }
        else
        {
          array[0] = typename ARRAY::value_type(a);
          array[3] = typename ARRAY::value_type(a+this->ni_);
          array[2] = typename ARRAY::value_type(a+this->ni_+nij_);
          array[1] = typename ARRAY::value_type(a+nij_);        
        }
      }
      else
      {
        const VMesh::index_type zidx = yidx - this->ni_ * (this->nj_ - 1) * (this->nk_ - 1);
        const VMesh::index_type k = zidx % (this->nk_ - 1);
        const VMesh::index_type ij = zidx / (this->nk_ - 1);
        const VMesh::index_type i = ij % (this->ni_ - 1);
        const VMesh::index_type j = ij / (this->ni_ - 1);

        const VMesh::index_type nij_ = this->ni_*this->nj_; 
        const VMesh::index_type a = i+j*this->ni_+k*nij_;

        if (j < (this->nj_-1))
        {
          array[0] = typename ARRAY::value_type(a);
          array[1] = typename ARRAY::value_type(a+nij_);
          array[2] = typename ARRAY::value_type(a+nij_+1);
          array[3] = typename ARRAY::value_type(a+1); 
        }
        else
        {
          array[0] = typename ARRAY::value_type(a);
          array[3] = typename ARRAY::value_type(a+nij_);
          array[2] = typename ARRAY::value_type(a+nij_+1);
          array[1] = typename ARRAY::value_type(a+1);         
        }
      }
    }
  }


  template <class ARRAY, class INDEX>
  void get_nodes_from_cell(ARRAY &array, INDEX idx) const
  {
    const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);

    const VMesh::index_type i = xidx % (this->ni_-1);
    const VMesh::index_type jk = xidx / (this->ni_-1);
    const VMesh::index_type j = jk % (this->nj_-1);
    const VMesh::index_type k = jk / (this->nj_-1);

    array.resize(8);
    const VMesh::index_type nij_ = this->ni_*this->nj_;
    const VMesh::index_type a = i+this->ni_*j+nij_*k;
    array[0] = typename ARRAY::value_type(a);
    array[1] = typename ARRAY::value_type(a+1);
    array[2] = typename ARRAY::value_type(a+1+this->ni_);
    array[3] = typename ARRAY::value_type(a+this->ni_);
    array[4] = typename ARRAY::value_type(a+nij_);
    array[5] = typename ARRAY::value_type(a+1+nij_);
    array[6] = typename ARRAY::value_type(a+1+this->ni_+nij_);
    array[7] = typename ARRAY::value_type(a+this->ni_+nij_);
  }

  template <class ARRAY, class INDEX>
  void get_nodes_from_elem(ARRAY &array, INDEX idx) const
  {
    get_nodes_from_cell(array,idx);
  }
  
  template <class ARRAY, class INDEX>
  void get_edges_from_face(ARRAY &array, INDEX idx) const
  {
    array.resize(4);
    const VMesh::index_type num_i_faces = (this->ni_-1)*(this->nj_-1)*this->nk_;  // lie in ij plane ijk
    const VMesh::index_type num_j_faces = this->ni_*(this->nj_-1)*(this->nk_-1);  // lie in jk plane jki
    const VMesh::index_type num_k_faces = (this->ni_-1)*this->nj_*(this->nk_-1);  // lie in ki plane kij

    const VMesh::index_type num_i_edges = (this->ni_-1)*this->nj_*this->nk_; // ijk
    const VMesh::index_type num_j_edges = this->ni_*(this->nj_-1)*this->nk_; // jki

    VMesh::index_type facei, facej, facek;
    VMesh::index_type face = static_cast<VMesh::index_type>(idx);

    if (face < num_i_faces)
    {
      facei = face % (this->ni_-1);
      facej = (face / (this->ni_-1)) % (this->nj_-1);
      facek = face / ((this->ni_-1)*(this->nj_-1));
      array[0] = typename ARRAY::value_type(facei+facej*(this->ni_-1)+facek*(this->ni_-1)*(this->nj_));
      array[1] = typename ARRAY::value_type(facei+(facej+1)*(this->ni_-1)+facek*(this->ni_-1)*(this->nj_));
      array[2] = typename ARRAY::value_type(num_i_edges + facei*(this->nj_-1)*(this->nk_)+facej+facek*(this->nj_-1));
      array[3] = typename ARRAY::value_type(num_i_edges + (facei+1)*(this->nj_-1)*(this->nk_)+facej+facek*(this->nj_-1));
    }
    else if (face - num_i_faces < num_j_faces)
    {
      face -= num_i_faces;
      facei = face / ((this->nj_-1) *(this->nk_-1));
      facej = face % (this->nj_-1);
      facek = (face / (this->nj_-1)) % (this->nk_-1);
      array[0] = typename ARRAY::value_type(num_i_edges + facei*(this->nj_-1)*(this->nk_)+facej+facek*(this->nj_-1));
      array[1] = typename ARRAY::value_type(num_i_edges + facei*(this->nj_-1)*(this->nk_)+facej+(facek+1)*(this->nj_-1));
      array[2] = typename ARRAY::value_type((num_i_edges + num_j_edges + facei*(this->nk_-1)+facej*(this->ni_)*(this->nk_-1)+facek));
      array[3] = typename ARRAY::value_type((num_i_edges + num_j_edges + facei*(this->nk_-1)+(facej+1)*(this->ni_)*(this->nk_-1)+facek));
    }
    else if (face - num_i_faces - num_j_faces < num_k_faces)
    {
      face -= (num_i_faces + num_j_faces);
      facei = (face / (this->nk_-1)) % (this->ni_-1);
      facej = face / ((this->ni_-1) * (this->nk_-1));
      facek = face % (this->nk_-1);
      array[0] = typename ARRAY::value_type(facei+facej*(this->ni_-1)+facek*(this->ni_-1)*(this->nj_));
      array[1] = typename ARRAY::value_type(facei+facej*(this->ni_-1)+(facek+1)*(this->ni_-1)*(this->nj_));
      array[2] = typename ARRAY::value_type((num_i_edges + num_j_edges + facei*(this->nk_-1)+facej*(this->ni_)*(this->nk_-1)+facek));
      array[3] = typename ARRAY::value_type((num_i_edges + num_j_edges + (facei+1)*(this->nk_-1)+facej*(this->ni_)*(this->nk_-1)+facek));
    }
    else 
    {
      ASSERTFAIL("VLatVolMesh: get_edges face index out of bounds");
    }
  }

  template <class ARRAY, class INDEX>
  void get_edges_from_cell(ARRAY &array, INDEX idx) const
  {
    const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);

    const VMesh::index_type i = xidx % (this->ni_-1);
    const VMesh::index_type jk = xidx / (this->ni_-1);
    const VMesh::index_type j = jk % (this->nj_-1);
    const VMesh::index_type k = jk / (this->nj_-1);
    
    array.resize(12);
    const VMesh::index_type j_start= (this->ni_-1)*this->nj_*this->nk_;
    const VMesh::index_type k_start = this->ni_*(this->nj_-1)*this->nk_ + j_start;

    array[0] = typename ARRAY::value_type(i+j*(this->ni_-1)+k*(this->ni_-1)*(this->nj_));
    array[1] = typename ARRAY::value_type(i+(j+1)*(this->ni_-1)+k*(this->ni_-1)*(this->nj_));
    array[2] = typename ARRAY::value_type(i+j*(this->ni_-1)+(k+1)*(this->ni_-1)*(this->nj_));
    array[3] = typename ARRAY::value_type(i+(j+1)*(this->ni_-1)+(k+1)*(this->ni_-1)*(this->nj_));

    array[4] = typename ARRAY::value_type(j_start+i*(this->nj_-1)*(this->nk_)+j+k*(this->nj_-1));
    array[5] = typename ARRAY::value_type(j_start+(i+1)*(this->nj_-1)*(this->nk_)+j+k*(this->nj_-1));
    array[6] = typename ARRAY::value_type(j_start+i*(this->nj_-1)*(this->nk_)+j+(k+1)*(this->nj_-1));
    array[7] = typename ARRAY::value_type(j_start+(i+1)*(this->nj_-1)*(this->nk_)+j+(k+1)*(this->nj_-1));

    array[8] =  typename ARRAY::value_type(k_start+i*(this->nk_-1)+j*(this->ni_)*(this->nk_-1)+k);
    array[9] =  typename ARRAY::value_type(k_start+(i+1)*(this->nk_-1)+j*(this->ni_)*(this->nk_-1)+k);
    array[10] = typename ARRAY::value_type(k_start+i*(this->nk_-1)+(j+1)*(this->ni_)*(this->nk_-1)+k);
    array[11] = typename ARRAY::value_type(k_start+(i+1)*(this->nk_-1)+(j+1)*(this->ni_)*(this->nk_-1)+k);
  }

  template <class ARRAY, class INDEX>
  void get_edges_from_node(ARRAY &array, INDEX idx) const
  {
    const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);

    const VMesh::index_type i = xidx % (this->ni_);
    const VMesh::index_type jk = xidx / (this->ni_);
    const VMesh::index_type j = jk % (this->nj_);
    const VMesh::index_type k = jk / (this->nj_);
    
    array.clear();
    const VMesh::index_type j_start= (this->ni_-1)*this->nj_*this->nk_;
    const VMesh::index_type k_start = this->ni_*(this->nj_-1)*this->nk_ + j_start;

    if (i < (this->ni_-1))
      array.push_back(typename ARRAY::value_type(i+j*(this->ni_-1)+k*(this->ni_-1)*(this->nj_)));
    if (i > 0)
      array.push_back(typename ARRAY::value_type(i-1+j*(this->ni_-1)+k*(this->ni_-1)*(this->nj_)));

    if (j < (this->nj_-1))
      array.push_back(typename ARRAY::value_type(j_start+i*(this->nj_-1)*(this->nk_)+j+k*(this->nj_-1)));
    if (j > 0)
      array.push_back(typename ARRAY::value_type(j_start+(i+1)*(this->nj_-1)*(this->nk_)+j-1+k*(this->nj_-1)));

    if (k < (this->nk_-1))
      array.push_back(typename ARRAY::value_type(k_start+i*(this->nk_-1)+j*(this->ni_)*(this->nk_-1)+k));
    if (k > 0)
      array.push_back(typename ARRAY::value_type(k_start+(i+1)*(this->nk_-1)+j*(this->ni_)*(this->nk_-1)+k-1));
  }

  template <class ARRAY, class INDEX>
  void get_edges_from_elem(ARRAY &array, INDEX idx) const
  {
    get_edges_from_cell(array,idx);
  }


  template <class ARRAY, class INDEX>
  void get_faces_from_node(ARRAY &array, INDEX idx) const
  {
    const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);

    const VMesh::index_type i = xidx % (this->ni_);
    const VMesh::index_type jk = xidx / (this->ni_);
    const VMesh::index_type j = jk % (this->nj_);
    const VMesh::index_type k = jk / (this->nj_);
    
    const VMesh::index_type offset1 = (this->ni_ - 1) * (this->nj_ - 1) * this->nk_;
    const VMesh::index_type offset2 = offset1 + this->ni_ * (this->nj_ - 1) * (this->nk_ - 1);

    array.clear();
    
    if ((i < (this->ni_-1))&&(j < (this->nj_-1)))
      array.push_back(typename ARRAY::value_type(i+(j+k*(this->nj_-1))*(this->ni_-1)));
    if ((i < (this->ni_-1))&&(j > 0))
      array.push_back(typename ARRAY::value_type(i+(j-1+k*(this->nj_-1))*(this->ni_-1)));
    if ((i > 0)&&(j < (this->nj_-1)))
      array.push_back(typename ARRAY::value_type(i-1+(j+k*(this->nj_-1))*(this->ni_-1)));
    if ((i > 0)&&(j > 0))
      array.push_back(typename ARRAY::value_type(i-1+(j-1+k*(this->nj_-1))*(this->ni_-1)));
      
    if ((j < (this->nj_-1))&&(k < (this->nk_-1)))
      array.push_back(typename ARRAY::value_type(offset1+j+(k+i*(this->nk_-1))*(this->nj_-1)));
    if ((j < (this->nj_-1))&&(k > 0))
      array.push_back(typename ARRAY::value_type(offset1+j+(k-1+i*(this->nk_-1))*(this->nj_-1)));
    if ((j > 0)&&(k < (this->nk_-1)))
      array.push_back(typename ARRAY::value_type(offset1+j-1+(k+i*(this->nk_-1))*(this->nj_-1)));
    if ((j > 0)&&(k > 0))
      array.push_back(typename ARRAY::value_type(offset1+j-1+(k-1+i*(this->nk_-1))*(this->nj_-1)));
      
    if ((i < (this->ni_-1))&&(k < (this->nk_-1)))
      array.push_back(typename ARRAY::value_type(offset2+k+(i+j*(this->ni_-1))*(this->nk_-1)));
    if ((i < (this->ni_-1))&&(k > 0))
      array.push_back(typename ARRAY::value_type(offset2+k-1+(i+j*(this->ni_-1))*(this->nk_-1)));
    if ((i > 0)&&(k < (this->nk_-1)))
      array.push_back(typename ARRAY::value_type(offset2+k+(i-1+j*(this->ni_-1))*(this->nk_-1)));
    if ((i > 0)&&(k > 0))
      array.push_back(typename ARRAY::value_type(offset2+k-1+(i-1+j*(this->ni_-1))*(this->nk_-1)));
  }


  template <class ARRAY, class INDEX>
  void get_faces_from_edge(ARRAY &array, INDEX idx) const
  {
    array.clear();
    const VMesh::index_type offset1 = (this->ni_ - 1) * (this->nj_ - 1) * this->nk_;
    const VMesh::index_type offset2 = offset1 + this->ni_ * (this->nj_ - 1) * (this->nk_ - 1);

    const VMesh::index_type num_i_edges = (this->ni_-1)*this->nj_*this->nk_; // ijk
    const VMesh::index_type num_j_edges = this->ni_*(this->nj_-1)*this->nk_; // jki
    const VMesh::index_type num_k_edges = this->ni_*this->nj_*(this->nk_-1); // jki

    VMesh::index_type facei, facej, facek;
    VMesh::index_type edge = static_cast<VMesh::index_type>(idx);

    if (idx < num_i_edges)
    {
      VMesh::index_type k = idx/((this->nj_)*(this->ni_-1)); idx -= k*(this->nj_)*(this->ni_-1);
      VMesh::index_type j = idx/(this->ni_-1); idx -= j*(this->ni_-1);
      VMesh::index_type i = idx;
      
      if (j < (this->nj_-1))
        array.push_back(typename ARRAY::value_type(i+(j+k*(this->nj_-1))*(this->ni_-1)));
      if (j > 0)
        array.push_back(typename ARRAY::value_type(i+(j-1+k*(this->nj_-1))*(this->ni_-1)));
      if (k < (this->nk_-1))
        array.push_back(typename ARRAY::value_type(offset2+k+(i+j*(this->ni_-1))*(this->nk_-1)));
      if (k > 0)
        array.push_back(typename ARRAY::value_type(offset2+k-1+(i+j*(this->ni_-1))*(this->nk_-1)));
    }
    else if (idx - num_i_edges < num_j_edges)
    {
      idx -= num_i_edges;
      VMesh::index_type i = idx/((this->nk_)*(this->nj_-1)); idx -= i*(this->nk_)*(this->nj_-1);
      VMesh::index_type k = idx/(this->nj_-1); idx -= k*(this->nj_-1);
      VMesh::index_type j = idx;
      
      if (k < (this->nk_-1))
        array.push_back(typename ARRAY::value_type(offset1+j+(k+i*(this->nk_-1))*(this->nj_-1)));
      if (k > 0)
        array.push_back(typename ARRAY::value_type(offset1+j+(k-1+i*(this->nk_-1))*(this->nj_-1)));
      if (i < (this->ni_-1))
        array.push_back(typename ARRAY::value_type(i+(j+k*(this->nj_-1))*(this->ni_-1)));
      if (i > 0)
        array.push_back(typename ARRAY::value_type(i-1+(j+k*(this->nj_-1))*(this->ni_-1)));
    }
    else if (idx - num_i_edges - num_j_edges < num_k_edges)
    {
      idx -= (num_i_edges + num_j_edges);
      VMesh::index_type j = idx/((this->nk_-1)*(this->ni_)); idx -= j*(this->nk_-1)*(this->ni_);
      VMesh::index_type i = idx/(this->nk_-1); idx -= i*(this->nk_-1);
      VMesh::index_type k = idx;

      if (j < (this->nj_-1))
        array.push_back(typename ARRAY::value_type(offset1+j+(k+i*(this->nk_-1))*(this->nj_-1)));
      if (j > 0)
        array.push_back(typename ARRAY::value_type(offset1+j-1+(k+i*(this->nk_-1))*(this->nj_-1)));
      if (i < (this->ni_-1))
        array.push_back(typename ARRAY::value_type(offset2+k+(i+j*(this->ni_-1))*(this->nk_-1)));
      if (i > 0)
        array.push_back(typename ARRAY::value_type(offset2+k+(i-1+j*(this->ni_-1))*(this->nk_-1)));
    }
    else 
    {
      ASSERTFAIL("VLatVolMesh: get_faces edge index out of bounds");
    }
  }

  template <class ARRAY, class INDEX>
  void get_faces_from_cell(ARRAY &array, INDEX idx) const
  {
    const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);

    const VMesh::index_type i = xidx % (this->ni_-1);
    const VMesh::index_type jk = xidx / (this->ni_-1);
    const VMesh::index_type j = jk % (this->nj_-1);
    const VMesh::index_type k = jk / (this->nj_-1);
    

    const VMesh::index_type offset1 = (this->ni_ - 1) * (this->nj_ - 1) * this->nk_;
    const VMesh::index_type offset2 = offset1 + this->ni_ * (this->nj_ - 1) * (this->nk_ - 1);

    array.resize(6);
    array[0] = typename ARRAY::value_type(i+(j+k*(this->nj_-1))*(this->ni_-1));
    array[1] = typename ARRAY::value_type(i+(j+(k+1)*(this->nj_-1))*(this->ni_-1));

    array[2] = typename ARRAY::value_type(offset1+j+(k+i*(this->nk_-1))*(this->nj_-1));
    array[3] = typename ARRAY::value_type(offset1+j+(k+(i+1)*(this->nk_-1))*(this->nj_-1));

    array[4] = typename ARRAY::value_type(offset2+k+(i+j*(this->ni_-1))*(this->nk_-1));
    array[5] = typename ARRAY::value_type(offset2+k+(i+(j+1)*(this->ni_-1))*(this->nk_-1));
  }
  
  template <class ARRAY, class INDEX>
  void get_cells_from_node(ARRAY &array, INDEX idx) const
  {

    const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);

    const VMesh::index_type ii = xidx % this->ni_;
    const VMesh::index_type jk = xidx / this->ni_;
    const VMesh::index_type jj = jk % this->nj_;
    const VMesh::index_type kk = jk / this->nj_;
    
    array.reserve(8);
    array.clear();
    const VMesh::index_type i0 = ii ? ii-1 : 0;
    const VMesh::index_type j0 = jj ? jj- 1 : 0;
    const VMesh::index_type k0 = kk ? kk- 1 : 0;

    const VMesh::index_type i1 = ii < this->ni_-1 ? ii+1 : this->ni_-1;
    const VMesh::index_type j1 = jj < this->nj_-1 ? jj+1 : this->nj_-1;
    const VMesh::index_type k1 = kk < this->nk_-1 ? kk+1 : this->nk_-1;

    VMesh::index_type i, j, k;
    for (k = k0; k < k1; k++)
      for (j = j0; j < j1; j++)
        for (i = i0; i < i1; i++)
        {
          array.push_back(typename ARRAY::value_type(i+j*(this->ni_-1)+k*(this->ni_-1)*(this->nj_-1)));
        }
  }

  template <class ARRAY, class INDEX>
  void get_cells_from_edge(ARRAY &array, INDEX eidx) const
  {
    array.reserve(4);
    array.clear();
    
    const VMesh::index_type offset1 = (this->ni_-1)*this->nj_*this->nk_;
    const VMesh::index_type offset2 = offset1 + this->ni_*(this->nj_-1)*this->nk_;
    VMesh::index_type idx = eidx;

    const VMesh::index_type mj = this->ni_-1;
    const VMesh::index_type mk = (this->ni_-1)*(this->nj_-1);
    
    if (idx < offset1)
    {
      VMesh::index_type k = idx/((this->nj_)*(this->ni_-1)); idx -= k*(this->nj_)*(this->ni_-1);
      VMesh::index_type j = idx/(this->ni_-1); idx -= j*(this->ni_-1);
      VMesh::index_type i = idx;

      if (j > 0)
      {
        if (k < (this->nk_-1)) array.push_back(typename ARRAY::value_type(i+mj*(j-1)+mk*k));
        if (k > 0) array.push_back(typename ARRAY::value_type(i+mj*(j-1)+mk*(k-1)));
      }
      if (j < (this->nj_-1))
      {
        if (k < (this->nk_-1)) array.push_back(typename ARRAY::value_type(i+mj*j+mk*k));
        if (k > 0) array.push_back(typename ARRAY::value_type(i+mj*j+mk*(k-1)));
      }
    }
    else if (idx >= offset2)
    {
      idx -= offset2;
      VMesh::index_type j = idx/((this->nk_-1)*(this->ni_)); idx -= j*(this->nk_-1)*(this->ni_);
      VMesh::index_type i = idx/(this->nk_-1); idx -= i*(this->nk_-1);
      VMesh::index_type k = idx;

      if (i > 0)
      {
        if (j < (this->nj_-1)) array.push_back(typename ARRAY::value_type((i-1)+mj*j+mk*k));    
        if (j > 0) array.push_back(typename ARRAY::value_type((i-1)+mj*(j-1)+mk*k));
      }
      if (i < (this->ni_-1))
      {
        if (j < (this->nj_-1)) array.push_back(typename ARRAY::value_type(i+mj*j+mk*k));    
        if (j > 0) array.push_back(typename ARRAY::value_type(i+mj*(j-1)+mk*k));
      }
    }
    else
    {
      idx -= offset1;
      VMesh::index_type i = idx/((this->nk_)*(this->nj_-1)); idx -= i*(this->nk_)*(this->nj_-1);
      VMesh::index_type k = idx/(this->nj_-1); idx -= k*(this->nj_-1);
      VMesh::index_type j = idx;

      if (k > 0)
      {
        if (i < (this->nk_-1)) array.push_back(typename ARRAY::value_type(i+mj*j+mk*(k-1)));
        if (i > 0) array.push_back(typename ARRAY::value_type((i-1)+mj*j+mk*(k-1)));
      }

      if (k < (this->nk_-1))
      {
        if (i < (this->ni_-1)) array.push_back(typename ARRAY::value_type(i+mj*j+mk*k));        
        if (i > 0) array.push_back(typename ARRAY::value_type((i-1)+mj*j+mk*k));
      }
    }
  }

  template <class ARRAY, class INDEX>
  void get_cells_from_face(ARRAY &array, INDEX fidx) const
  {
    array.reserve(2);
    array.clear();
    
    const VMesh::index_type offset1 = (this->ni_ - 1) * (this->nj_ - 1) * this->nk_;
    const VMesh::index_type offset2 = offset1 + this->ni_ * (this->nj_ - 1) * (this->nk_ - 1);
    VMesh::index_type idx = fidx;

    const VMesh::index_type mj = this->ni_-1;
    const VMesh::index_type mk = (this->ni_-1)*(this->nj_-1);

    if (idx < offset1)
    {
      VMesh::index_type k = idx/((this->nj_-1)*(this->ni_-1)); idx -= k*(this->nj_-1)*(this->ni_-1);
      VMesh::index_type j = idx/(this->ni_-1); idx -= j*(this->ni_-1);
      VMesh::index_type i = idx;

      if (k < (this->nk_-1)) array.push_back(typename ARRAY::value_type(i+mj*j+mk*k));
      if (k > 0) array.push_back(typename ARRAY::value_type(i+mj*j+mk*(k-1)));
    }
    else if (idx >= offset2)
    {
      idx -= offset2;
      VMesh::index_type j = idx/((this->nk_-1)*(this->ni_-1)); idx -= j*(this->nk_-1)*(this->ni_-1);
      VMesh::index_type i = idx/(this->nk_-1); idx -= i*(this->nk_-1);
      VMesh::index_type k = idx;

      if (j < (this->nj_-1)) array.push_back(typename ARRAY::value_type(i+mj*j+mk*k));    
      if (j > 0) array.push_back(typename ARRAY::value_type(i+mj*(j-1)+mk*k));
    }
    else
    {
      idx -= offset1;
      VMesh::index_type i = idx/((this->nk_-1)*(this->nj_-1)); idx -= i*(this->nk_-1)*(this->nj_-1);
      VMesh::index_type k = idx/(this->nj_-1); idx -= k*(this->nj_-1);
      VMesh::index_type j = idx;

      if (i < (this->ni_-1)) array.push_back(typename ARRAY::value_type(i+mj*j+mk*k));        
      if (i > 0) array.push_back(typename ARRAY::value_type((i-1)+mj*j+mk*k));
    }
  }

  template <class INDEX>
  bool elem_locate(INDEX &idx,Point p) const
  {
    const Point r = this->mesh_->transform_.unproject(p);

    double epsilon = this->mesh_->get_epsilon();
    double ii = r.x();
    double jj = r.y();
    double kk = r.z();

    const double nii = static_cast<double>(this->ni_-1);
    const double njj = static_cast<double>(this->nj_-1);
    const double nkk = static_cast<double>(this->nk_-1);
     
    if (ii>nii && (ii-epsilon)<nii) ii=nii-epsilon;
    if (jj>njj && (jj-epsilon)<njj) jj=njj-epsilon;
    if (kk>nkk && (kk-epsilon)<nkk) kk=nkk-epsilon;
    if (ii<0 && ii>-epsilon) ii=0;
    if (jj<0 && jj>-epsilon) jj=0;
    if (kk<0 && kk>-epsilon) kk=0;
    
    const index_type i = static_cast<index_type>(floor(ii));
    const index_type j = static_cast<index_type>(floor(jj));
    const index_type k = static_cast<index_type>(floor(kk));

    if (i < (this->ni_-1) && i >= 0 &&
        j < (this->nj_-1) && j >= 0 &&
        k < (this->nk_-1) && k >= 0)
    {
      idx = i+(this->ni_-1)*j+(this->ni_-1)*(this->nj_-1)*k;
      return (true);
    }

    return (false);  
  }


  template <class ARRAY>
  bool elems_locate(ARRAY &array,BBox b) const
  {
    array.clear();
    const Point r1 = this->mesh_->transform_.unproject(b.get_min());
    const Point r2 = this->mesh_->transform_.unproject(b.get_max());
    double epsilon = this->mesh_->get_epsilon();
    
    index_type i1 = static_cast<index_type>(floor(r1.x()-epsilon)); 
    index_type j1 = static_cast<index_type>(floor(r1.y()-epsilon));
    index_type k1 = static_cast<index_type>(floor(r1.z()-epsilon));
    if (i1 < 0) i1 = 0;
    if (j1 < 0) j1 = 0;
    if (k1 < 0) k1 = 0;
    
    const index_type nii = this->ni_-1;
    const index_type njj = this->nj_-1;
    const index_type nkk = this->nk_-1;

    index_type i2 = static_cast<index_type>(ceil(r2.x()+epsilon)); 
    index_type j2 = static_cast<index_type>(ceil(r2.y()+epsilon));
    index_type k2 = static_cast<index_type>(ceil(r2.z()+epsilon));
  
    if (i2 > nii) i2 = nii;
    if (j2 > njj) i2 = njj;
    if (k2 > nkk) i2 = nkk;
     
    for (index_type i=i1;i<i2;i++)
      for (index_type j=j1;j<j2;j++)
        for (index_type k=k1;k<k2;k++)
        {
          array.push_back(typename ARRAY::value_type(i+nii*j+nii*njj*k));
        }

    return (array.size() > 0);  
  }


  template <class INDEX>
  bool elem_locate(INDEX &idx, VMesh::coords_type& coords, Point p) const
  {
    const Point r = this->mesh_->transform_.unproject(p);

    double epsilon = this->mesh_->get_epsilon();
 
    double ii = r.x();
    double jj = r.y();
    double kk = r.z();
    
    const double nii = static_cast<double>(this->ni_-1);
    const double njj = static_cast<double>(this->nj_-1);
    const double nkk = static_cast<double>(this->nk_-1);
    
    if (ii>nii && (ii-epsilon)<nii) ii=nii-epsilon;
    if (jj>njj && (jj-epsilon)<njj) jj=njj-epsilon;
    if (kk>nkk && (kk-epsilon)<nkk) kk=nkk-epsilon;
    if (ii<0 && ii>-epsilon) ii=0;
    if (jj<0 && jj>-epsilon) jj=0;
    if (kk<0 && kk>-epsilon) kk=0;
    
    const index_type i = static_cast<index_type>(floor(ii));
    const index_type j = static_cast<index_type>(floor(jj));
    const index_type k = static_cast<index_type>(floor(kk));

    coords.resize(3);
    coords[0] = ii - floor(ii);
    coords[1] = jj - floor(jj);
    coords[2] = kk - floor(kk);

    if (i < (this->ni_-1) && i >= 0 &&
        j < (this->nj_-1) && j >= 0 &&
        k < (this->nk_-1) && k >= 0)
    {
      idx = i+(this->ni_-1)*j+(this->ni_-1)*(this->nj_-1)*k;
      return true;
    }

    return false;  
  }

  template <class INDEX>
  bool node_locate(INDEX &idx,Point p) const
  {
    if (this->ni_ == 0 || this->nj_ == 0 || this->nk_ == 0) return (false);
  
    const Point r = this->mesh_->transform_.unproject(p);

    double rx = floor(r.x() + 0.5);
    double ry = floor(r.y() + 0.5);
    double rz = floor(r.z() + 0.5);

    const double nii = static_cast<double>(this->ni_-1);
    const double njj = static_cast<double>(this->nj_-1);
    const double nkk = static_cast<double>(this->nk_-1);

    if (rx < 0.0) rx = 0.0; if (rx > nii) rx = nii;
    if (ry < 0.0) ry = 0.0; if (ry > njj) ry = njj;
    if (rz < 0.0) rz = 0.0; if (rz > nkk) rz = nkk;
    
    index_type i = static_cast<index_type>(rx);
    index_type j = static_cast<index_type>(ry);
    index_type k = static_cast<index_type>(rz);
    idx = i+this->ni_*j+this->ni_*this->nj_*k;

    return (true);
  }
  
  template <class INDEX>
  inline void to_index(typename MESH::Node::index_type &index, INDEX idx) const
  {
    const VMesh::index_type i = idx % this->ni_;
    const VMesh::index_type jk = idx / this->ni_;
    const VMesh::index_type j = jk % this->nj_;
    const VMesh::index_type k = jk / this->nj_;
    index = typename MESH::Node::index_type(this->mesh_, i, j, k);  
  }

  template <class INDEX>
  inline void to_index(typename MESH::Edge::index_type &index, INDEX idx) const
    {index = typename MESH::Edge::index_type(idx);}

  template <class INDEX>
  inline void to_index(typename MESH::Face::index_type &index, INDEX idx) const
    {index = typename MESH::Face::index_type(idx);}

  template <class INDEX>
  inline  void to_index(typename MESH::Cell::index_type &index, INDEX idx) const
  {
    const VMesh::index_type i = idx % (this->ni_-1);
    const VMesh::index_type jk = idx / (this->ni_-1);
    const VMesh::index_type j = jk % (this->nj_-1);
    const VMesh::index_type k = jk / (this->nj_-1);
    index = typename MESH::Cell::index_type(this->mesh_, i, j, k); 
  }
  
  template <class ARRAY, class INDEX>
  void
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
VMesh* CreateVLatVolMesh(LatVolMesh<HexTrilinearLgn<Point> >* mesh)
{
  return new VLatVolMesh<LatVolMesh<HexTrilinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID LatVolMesh_MeshID1(LatVolMesh<HexTrilinearLgn<Point> >::type_name(-1),
                  LatVolMesh<HexTrilinearLgn<Point> >::mesh_maker,
                  LatVolMesh<HexTrilinearLgn<Point> >::latvol_maker);
                  

/// Functions for determining the number of indices that are used
template <class MESH>
void
VLatVolMesh<MESH>::size(VMesh::Node::size_type& sz) const
{
  sz = VMesh::Node::size_type(this->ni_*this->nj_*this->nk_);
}

template <class MESH>
void
VLatVolMesh<MESH>::size(VMesh::ENode::size_type& sz) const
{
  sz = VMesh::ENode::size_type(((this->ni_-1) * this->nj_ * this->nk_) + 
                                (this->ni_ * (this->nj_-1) * this->nk_) + 
                                (this->ni_ * this->nj_ * (this->nk_-1)));
  if (sz < 0) sz = 0;
}

template <class MESH>
void
VLatVolMesh<MESH>::size(VMesh::Edge::size_type& sz) const
{
  sz = VMesh::Edge::size_type(((this->ni_-1) * this->nj_ * this->nk_) + 
                                (this->ni_ * (this->nj_-1) * this->nk_) + 
                                (this->ni_ * this->nj_ * (this->nk_-1)));
  if (sz < 0) sz = 0;
}

template <class MESH>
void
VLatVolMesh<MESH>::size(VMesh::Face::size_type& sz) const
{
  sz = VMesh::Face::size_type((this->ni_-1) * (this->nj_-1) * this->nk_ +
                  this->ni_ * (this->nj_ - 1 ) * (this->nk_ - 1) + 
                  (this->ni_ - 1) * this->nj_ * (this->nk_ - 1));
  if (sz < 0) sz = 0;
}

template <class MESH>
void
VLatVolMesh<MESH>::size(VMesh::Cell::size_type& sz) const
{
  sz = VMesh::Cell::size_type((this->ni_-1)*(this->nj_-1)*(this->nk_-1));
  if (sz < 0) sz = 0;
}

template <class MESH>
void
VLatVolMesh<MESH>::size(VMesh::DElem::size_type& sz) const
{
  sz = VMesh::DElem::size_type((this->ni_-1) * (this->nj_-1) * this->nk_
                + this->ni_ * (this->nj_ - 1 ) * (this->nk_ - 1) + 
                (this->ni_ - 1) * this->nj_ * (this->nk_ - 1));
  if (sz < 0) sz = 0;
}

template <class MESH>
void
VLatVolMesh<MESH>::size(VMesh::Elem::size_type& sz) const
{
  sz = VMesh::Elem::size_type((this->ni_-1)*(this->nj_-1)*(this->nk_-1));
  if (sz < 0) sz = 0;
}

/// Topology functions


template <class MESH>
void
VLatVolMesh<MESH>::get_nodes(VMesh::Node::array_type &array,
                             VMesh::Edge::index_type idx) const
{
  get_nodes_from_edge(array,idx);
}


template <class MESH>
void
VLatVolMesh<MESH>::get_nodes(VMesh::Node::array_type &array,
                             VMesh::Face::index_type idx) const
{
  get_nodes_from_face(array,idx);
}


template <class MESH>
void
VLatVolMesh<MESH>::get_nodes(VMesh::Node::array_type &array,
                             VMesh::Cell::index_type idx) const
{
  get_nodes_from_cell(array,idx);
}


template <class MESH>
void
VLatVolMesh<MESH>::get_nodes(VMesh::Node::array_type &array,
                             VMesh::Elem::index_type idx) const
{
  get_nodes_from_cell(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_nodes(VMesh::Node::array_type &array,
                             VMesh::DElem::index_type idx) const
{
  get_nodes_from_face(array,idx);
}


template <class MESH>
void
VLatVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &array,
                              VMesh::Face::index_type idx) const
{
  get_edges_from_face(array,idx);
}


template <class MESH>
void
VLatVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &array,
                              VMesh::Cell::index_type idx) const
{
  get_edges_from_cell(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &array,
                              VMesh::Elem::index_type idx) const
{
  get_edges_from_cell(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &array,
                              VMesh::DElem::index_type idx) const
{
  get_edges_from_face(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_edges(VMesh::Edge::array_type &array,
                             VMesh::Node::index_type idx) const
{
  get_edges_from_node(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_edges(VMesh::Edge::array_type &array,
                             VMesh::Face::index_type idx) const
{
  get_edges_from_face(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_edges(VMesh::Edge::array_type &array,
                             VMesh::DElem::index_type idx) const
{
  get_edges_from_face(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_edges(VMesh::Edge::array_type &array,
                             VMesh::Cell::index_type idx) const
{
  get_edges_from_cell(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_edges(VMesh::Edge::array_type &array,
                             VMesh::Elem::index_type idx) const
{
  get_edges_from_face(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_faces(VMesh::Face::array_type &array,
                             VMesh::Cell::index_type idx) const
{
  get_faces_from_cell(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_faces(VMesh::Face::array_type &array,
                             VMesh::Elem::index_type idx) const
{
  get_faces_from_cell(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_faces(VMesh::Face::array_type &array,
                             VMesh::DElem::index_type idx) const
{
  array.resize(1); array[0] = static_cast<VMesh::Face::index_type>(idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_cells(VMesh::Cell::array_type &array,
                             VMesh::Node::index_type idx) const
{
  get_cells_from_node(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_cells(VMesh::Cell::array_type &array,
                             VMesh::Edge::index_type idx) const
{
  get_cells_from_edge(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_cells(VMesh::Cell::array_type &array,
                             VMesh::Face::index_type idx) const
{
  get_cells_from_face(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_cells(VMesh::Cell::array_type &array,
                             VMesh::Elem::index_type idx) const
{
  array.resize(1); array[0] = VMesh::Cell::index_type(idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_cells(VMesh::Cell::array_type &array,
                             VMesh::DElem::index_type idx) const
{
  get_cells_from_face(array,idx);
}


template <class MESH>
void
VLatVolMesh<MESH>::get_elems(VMesh::Elem::array_type &array,
                             VMesh::Node::index_type idx) const
{
  get_cells_from_node(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_elems(VMesh::Elem::array_type &array,
                             VMesh::Edge::index_type idx) const
{
  get_cells_from_edge(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_elems(VMesh::Elem::array_type &array,
                             VMesh::Face::index_type idx) const
{
  get_cells_from_face(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_elems(VMesh::Elem::array_type &array,
                             VMesh::Cell::index_type idx) const
{
  array.resize(1); array[0] = VMesh::Elem::index_type(idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_elems(VMesh::Elem::array_type &array,
                             VMesh::DElem::index_type idx) const
{
  get_cells_from_face(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_delems(VMesh::DElem::array_type &array,
                              VMesh::Cell::index_type idx) const
{
  get_faces_from_cell(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_delems(VMesh::DElem::array_type &array,
                              VMesh::Elem::index_type idx) const
{
  get_faces_from_cell(array,idx);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_delems(VMesh::DElem::array_type &array,
                              VMesh::Face::index_type idx) const
{
  array.resize(1); array[0] = VMesh::DElem::index_type(idx);
}


template <class MESH>
bool
VLatVolMesh<MESH>::get_elem(VMesh::Elem::index_type& elem, 
                        VMesh::Node::array_type& nodes) const
{
  return(this->get_cell_from_nodes(elem,nodes));
}

template <class MESH>
bool
VLatVolMesh<MESH>::get_delem(VMesh::DElem::index_type& delem, 
                         VMesh::Node::array_type& nodes) const
{
  return(this->get_face_from_nodes(delem,nodes));
}


template <class MESH>
bool
VLatVolMesh<MESH>::get_cell(VMesh::Cell::index_type& cell, 
                        VMesh::Node::array_type& nodes) const
{
  return(this->get_cell_from_nodes(cell,nodes));
}

template <class MESH>
bool
VLatVolMesh<MESH>::get_face(VMesh::Face::index_type& face, 
                        VMesh::Node::array_type& nodes) const
{
  return(this->get_face_from_nodes(face,nodes));
}

template <class MESH>
bool
VLatVolMesh<MESH>::get_edge(VMesh::Edge::index_type& edge, 
                        VMesh::Node::array_type& nodes) const
{
  return(this->get_edge_from_nodes(edge,nodes));
}


template <class MESH>
void
VLatVolMesh<MESH>::get_center(Point &p, VMesh::Node::index_type idx) const
{
  const VMesh::index_type i = idx % this->ni_;
  const VMesh::index_type jk = idx / this->ni_;
  const VMesh::index_type j = jk % this->nj_;
  const VMesh::index_type k = jk / this->nj_;
  
  Point pnt(static_cast<double>(i),static_cast<double>(j),static_cast<double>(k));
  p = this->mesh_->transform_.project(pnt);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_center(Point &p, VMesh::ENode::index_type idx) const
{
  get_center(p,VMesh::Edge::index_type(idx));
}

template <class MESH>
void
VLatVolMesh<MESH>::get_center(Point &p, VMesh::Edge::index_type idx) const
{
  const VMesh::index_type xidx = static_cast<VMesh::index_type>(idx);
  
  if (xidx < (this->ni_ - 1) * this->nj_ * this->nk_)
  {
    const VMesh::index_type i = xidx % (this->ni_ - 1);
    const VMesh::index_type jk = xidx / (this->ni_ - 1);
    const VMesh::index_type j = jk % this->nj_;
    const VMesh::index_type k = jk / this->nj_;
    Point pnt(static_cast<double>(i)+0.5,static_cast<double>(j),static_cast<double>(k));
    p = this->mesh_->transform_.project(pnt);
  }
  else
  {
    const VMesh::index_type yidx = xidx - (this->ni_ - 1) * this->nj_ * this->nk_;
    if (yidx < (this->ni_ * (this->nj_ - 1) * this->nk_))
    {
      const VMesh::index_type j = yidx % (this->nj_ - 1);
      const VMesh::index_type ik = yidx / (this->nj_ - 1);
      const VMesh::index_type i = ik / this->nk_;
      const VMesh::index_type k = ik % this->nk_;

      Point pnt(static_cast<double>(i),static_cast<double>(j)+0.5,static_cast<double>(k));
      p = this->mesh_->transform_.project(pnt);

    }
    else
    {
      const VMesh::index_type zidx = yidx - (this->ni_ * (this->nj_ - 1) * this->nk_);
      const VMesh::index_type k = zidx % (this->nk_ - 1);
      const VMesh::index_type ij = zidx / (this->nk_ - 1);
      const VMesh::index_type i = ij % this->ni_;
      const VMesh::index_type j = ij / this->ni_;

      Point pnt(static_cast<double>(i),static_cast<double>(j),static_cast<double>(k)+0.5);
      p = this->mesh_->transform_.project(pnt);
    }
  }
}

template <class MESH>
void
VLatVolMesh<MESH>::get_center(Point &p, VMesh::Face::index_type idx) const
{
  const VMesh::index_type xidx = static_cast<VMesh::index_type>(idx);
  if (xidx < (this->ni_ - 1) * (this->nj_ - 1) * this->nk_)
  {
    const VMesh::index_type i = xidx % (this->ni_ - 1);
    const VMesh::index_type jk = xidx / (this->ni_ - 1);
    const VMesh::index_type j = jk % (this->nj_ - 1);
    const VMesh::index_type k = jk / (this->nj_ - 1);

    Point pnt(static_cast<double>(i)+0.5,static_cast<double>(j)+0.5,static_cast<double>(k));
    p = this->mesh_->transform_.project(pnt);
  }
  else
  {
    const VMesh::index_type yidx = xidx - (this->ni_ - 1) * (this->nj_ - 1) * this->nk_;
    if (yidx < this->ni_ * (this->nj_ - 1) * (this->nk_ - 1))
    {
      const VMesh::index_type j = yidx % (this->nj_ - 1);
      const VMesh::index_type ik = yidx / (this->nj_ - 1);
      const VMesh::index_type k = ik % (this->nk_ - 1);
      const VMesh::index_type i = ik / (this->nk_ - 1);
      
      Point pnt(static_cast<double>(i),static_cast<double>(j)+0.5,static_cast<double>(k)+0.5);
      p = this->mesh_->transform_.project(pnt);
    }
    else
    {
      const VMesh::index_type zidx = yidx - this->ni_ * (this->nj_ - 1) * (this->nk_ - 1);
      const VMesh::index_type k = zidx % (this->nk_ - 1);
      const VMesh::index_type ij = zidx / (this->nk_ - 1);
      const VMesh::index_type i = ij % (this->ni_ - 1);
      const VMesh::index_type j = ij / (this->ni_ - 1);

      Point pnt(static_cast<double>(i)+0.5,static_cast<double>(j),static_cast<double>(k)+0.5);
      p = this->mesh_->transform_.project(pnt);
    }
  }
}

template <class MESH>
void
VLatVolMesh<MESH>::get_center(Point &p, VMesh::Cell::index_type idx) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);

  const VMesh::index_type i = xidx % (this->ni_-1);
  const VMesh::index_type jk = xidx / (this->ni_-1);
  const VMesh::index_type j = jk % (this->nj_-1);
  const VMesh::index_type k = jk / (this->nj_-1);

  Point pnt(static_cast<double>(i)+0.5,static_cast<double>(j)+0.5,static_cast<double>(k)+0.5);
  p = this->mesh_->transform_.project(pnt);
}

template <class MESH>
void
VLatVolMesh<MESH>::get_center(Point &p, VMesh::Elem::index_type idx) const
{
  get_center(p,VMesh::Cell::index_type(idx));
}

template <class MESH>
void
VLatVolMesh<MESH>::get_center(Point &p, VMesh::DElem::index_type idx) const
{
  get_center(p,VMesh::Face::index_type(idx));
}


template <class MESH>
void
VLatVolMesh<MESH>::get_centers(Point* points, const VMesh::Node::array_type& array) const
{
  for (size_t p=0; p <array.size(); p++)
  {
    const VMesh::index_type idx = array[p];
    const VMesh::index_type i = idx % this->ni_;
    const VMesh::index_type jk = idx / this->ni_;
    const VMesh::index_type j = jk % this->nj_;
    const VMesh::index_type k = jk / this->nj_;
  
    Point pnt(static_cast<double>(i),static_cast<double>(j),static_cast<double>(k));
    points[p] = this->mesh_->transform_.project(pnt);  
  }
}                                     
 
template <class MESH>
void
VLatVolMesh<MESH>::get_centers(Point* points, const VMesh::Elem::array_type& array) const
{
  for (size_t p=0; p <array.size(); p++)
  {
    const VMesh::index_type xidx = array[p];
    const VMesh::index_type i = xidx % (this->ni_-1);
    const VMesh::index_type jk = xidx / (this->ni_-1);
    const VMesh::index_type j = jk % (this->nj_-1);
    const VMesh::index_type k = jk / (this->nj_-1);

    Point pnt(static_cast<double>(i)+0.5,static_cast<double>(j)+0.5,static_cast<double>(k)+0.5);

    points[p] = this->mesh_->transform_.project(pnt);
  }
} 



template <class MESH>
bool 
VLatVolMesh<MESH>::locate(VMesh::Node::index_type &i, const Point &point) const
{
  return(this->node_locate(i,point));
}

template <class MESH>
bool 
VLatVolMesh<MESH>::locate(VMesh::Elem::index_type &i, const Point &point) const
{
  return(this->elem_locate(i,point));
}

template <class MESH>
bool 
VLatVolMesh<MESH>::locate(VMesh::Elem::array_type &array, const BBox &bbox) const
{
  return(this->elems_locate(array,bbox));
}


template <class MESH>
bool 
VLatVolMesh<MESH>::locate(VMesh::Elem::index_type &i, 
                          VMesh::coords_type &coords, const Point &point) const
{
  return(this->elem_locate(i,coords,point));
}

template <class MESH>
bool 
VLatVolMesh<MESH>::get_coords(VMesh::coords_type &coords, 
                              const Point &point, 
                              VMesh::Elem::index_type idx) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);

  const VMesh::index_type i = xidx % (this->ni_-1);
  const VMesh::index_type jk = xidx / (this->ni_-1);
  const VMesh::index_type j = jk % (this->nj_-1);
  const VMesh::index_type k = jk / (this->nj_-1);
  
  const Point r = this->mesh_->transform_.unproject(point);
  
  coords.resize(3);
  coords[0] = static_cast<VMesh::coords_type::value_type>(r.x()-static_cast<double>(i));
  coords[1] = static_cast<VMesh::coords_type::value_type>(r.y()-static_cast<double>(j));
  coords[2] = static_cast<VMesh::coords_type::value_type>(r.z()-static_cast<double>(k));
  
  const double min_element_val = 1e-8;
  
  if (static_cast<double>(coords[0]) < 0.0) if (static_cast<double>(coords[0]) > -(min_element_val)) 
        coords[0] = static_cast<VMesh::coords_type::value_type>(0.0); else return (false);
  if (static_cast<double>(coords[0]) > 1.0) if (static_cast<double>(coords[0]) < 1.0+(min_element_val)) 
        coords[0] = static_cast<VMesh::coords_type::value_type>(1.0); else return (false);
  if (static_cast<double>(coords[1]) < 0.0) if (static_cast<double>(coords[1]) > -(min_element_val)) 
        coords[1] = static_cast<VMesh::coords_type::value_type>(0.0); else return (false);
  if (static_cast<double>(coords[1]) > 1.0) if (static_cast<double>(coords[1]) < 1.0+(min_element_val)) 
        coords[1] = static_cast<VMesh::coords_type::value_type>(1.0); else return (false);
  if (static_cast<double>(coords[2]) < 0.0) if (static_cast<double>(coords[2]) > -(min_element_val)) 
        coords[2] = static_cast<VMesh::coords_type::value_type>(0.0); else return (false);
  if (static_cast<double>(coords[2]) > 1.0) if (static_cast<double>(coords[2]) < 1.0+(min_element_val)) 
        coords[2] = static_cast<VMesh::coords_type::value_type>(1.0); else return (false);
  
  return (true);
}  
  
  
template <class MESH>
void 
VLatVolMesh<MESH>::interpolate(Point &point, 
                               const VMesh::coords_type &coords, 
                               VMesh::Elem::index_type idx) const
{
  const VMesh::index_type i = idx % (this->ni_-1);
  const VMesh::index_type jk = idx / (this->ni_-1);
  const VMesh::index_type j = jk % (this->nj_-1);
  const VMesh::index_type k = jk / (this->nj_-1);
  
  Point pnt(static_cast<double>(i)+static_cast<double>(coords[0]),
          static_cast<double>(j)+static_cast<double>(coords[1]),
          static_cast<double>(k)+static_cast<double>(coords[2]));
  point = this->mesh_->transform_.project(pnt);
}



template <class MESH>
void 
VLatVolMesh<MESH>::minterpolate(std::vector<Point> &point, 
                                const std::vector<VMesh::coords_type> &coords, 
                                VMesh::Elem::index_type idx) const
{
  point.resize(coords.size());
  const VMesh::index_type i = idx % (this->ni_-1);
  const VMesh::index_type jk = idx / (this->ni_-1);
  const VMesh::index_type j = jk % (this->nj_-1);
  const VMesh::index_type k = jk / (this->nj_-1);

  for (size_t p=0; p<coords.size(); p++)
  {
    
    Point pnt(static_cast<double>(i)+static_cast<double>(coords[p][0]),
            static_cast<double>(j)+static_cast<double>(coords[p][1]),
            static_cast<double>(k)+static_cast<double>(coords[p][2]));
    point[p] = this->mesh_->transform_.project(pnt);
  }
}


template <class MESH>
void 
VLatVolMesh<MESH>::derivate(VMesh::dpoints_type &dp, 
                            const VMesh::coords_type& /*coords*/, 
                            VMesh::Elem::index_type /*i*/) const
{
  dp.resize(3);
  dp[0] = (this->mesh_->transform_.project(Vector(1.0,0.0,0.0))).asPoint(); 
  dp[1] = (this->mesh_->transform_.project(Vector(0.0,1.0,0.0))).asPoint(); 
  dp[2] = (this->mesh_->transform_.project(Vector(0.0,0.0,1.0))).asPoint();
}

template <class MESH>
bool 
VLatVolMesh<MESH>::get_neighbor(VMesh::Elem::index_type &neighbor, 
                                VMesh::Elem::index_type from, 
                                VMesh::DElem::index_type delem) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(from);
  const VMesh::index_type xdelem = static_cast<const VMesh::index_type>(delem);

  const VMesh::index_type i = xidx % (this->ni_-1);
  const VMesh::index_type jk = xidx / (this->ni_-1);
  const VMesh::index_type j = jk % (this->nj_-1);
  const VMesh::index_type k = jk / (this->nj_-1);

  const VMesh::index_type offset1 = (this->ni_ - 1) * (this->nj_ - 1) * this->nk_;
  const VMesh::index_type offset2 = offset1 + this->ni_ * (this->nj_ - 1) * (this->nk_ - 1);

  const VMesh::index_type mj = this->ni_-1;
  const VMesh::index_type mk = (this->ni_-1)*(this->nj_-1);
  
  if (xdelem == (i+(j+k*(this->nj_-1))*(this->ni_-1))) 
  {
    if (k <= 0) return (false);
    neighbor = static_cast<VMesh::Elem::index_type>(i+mj*j+mk*(k-1));
    return (true);
  }

  if (xdelem == (i+(j+(k+1)*(this->nj_-1))*(this->ni_-1))) 
  {
    if (k >= this->nk_-2) return (false);
    neighbor = static_cast<VMesh::Elem::index_type>(i+mj*j+mk*(k+1));
    return (true);
  }

  if (xdelem == (offset1+j+(k+i*(this->nk_-1))*(this->nj_-1))) 
  {
    if (i <= 0) return (false);
    neighbor = static_cast<VMesh::Elem::index_type>((i-1)+mj*j+mk*k);
    return (true);
  }

  if (xdelem == (offset1+j+(k+(i+1)*(this->nk_-1))*(this->nj_-1))) 
  {
    if (i >= this->ni_-2) return (false);
    neighbor = static_cast<VMesh::Elem::index_type>((i+1)+mj*j+mk*k);
    return (true);
  }

  if (xdelem == (offset2+k+(i+j*(this->ni_-1))*(this->nk_-1))) 
  {
    if (j <= 0) return (false);
    neighbor = static_cast<VMesh::Elem::index_type>(i+mj*(j-1)+mk*k);
    return (true);
  }

  if (xdelem == (offset2+k+(i+(j+1)*(this->ni_-1))*(this->nk_-1))) 
  {
    if (j >= this->nj_-2) return (false);
    neighbor = static_cast<VMesh::Elem::index_type>(i+mj*(j+1)+mk*k);
    return (true);
  }  

  return (false);
}


template <class MESH>
bool 
VLatVolMesh<MESH>::get_neighbors(VMesh::Elem::array_type &neighbors, 
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
VLatVolMesh<MESH>::get_neighbors(VMesh::Elem::array_type &array, 
                                 VMesh::Elem::index_type idx) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);
  const VMesh::index_type i = xidx % (this->ni_-1);
  const VMesh::index_type jk = xidx / (this->ni_-1);
  const VMesh::index_type j = jk % (this->nj_-1);
  const VMesh::index_type k = jk / (this->nj_-1);
  
  array.reserve(6);
  array.clear();
  
  const VMesh::index_type mj = this->ni_-1;
  const VMesh::index_type mk = (this->ni_-1)*(this->nj_-1);
  if (i > 0) array.push_back(static_cast<VMesh::Elem::index_type>((i-1)+j*mj+k*mk));
  if (i < this->ni_-2) array.push_back(static_cast<VMesh::Elem::index_type>((i+1)+j*mj+k*mk));
  if (j > 0) array.push_back(static_cast<VMesh::Elem::index_type>(i+(j-1)*mj+k*mk));
  if (j < this->nj_-2) array.push_back(static_cast<VMesh::Elem::index_type>(i+(j+1)*mj+k*mk));
  if (k > 0) array.push_back(static_cast<VMesh::Elem::index_type>(i+j*mj+(k-1)*mk));
  if (k < this->nk_-2) array.push_back(static_cast<VMesh::Elem::index_type>(i+j*mj+(k+1)*mk));
}

template <class MESH>
void 
VLatVolMesh<MESH>::get_neighbors(VMesh::Node::array_type &array, 
                                 VMesh::Node::index_type idx) const
{
  const VMesh::index_type xidx = static_cast<const VMesh::index_type>(idx);
  const VMesh::index_type i = xidx % this->ni_;
  const VMesh::index_type jk = xidx / this->ni_;
  const VMesh::index_type j = jk % this->nj_;
  const VMesh::index_type k = jk / this->nj_;
  
  array.reserve(6);
  array.clear();
  
  const VMesh::index_type mj = this->ni_;
  const VMesh::index_type mk = this->ni_*this->nj_;
  if (i > 0) array.push_back(static_cast<VMesh::Node::index_type>((i-1)+j*mj+k*mk));
  if (i < this->ni_-1) array.push_back(static_cast<VMesh::Node::index_type>((i+1)+j*mj+k*mk));
  if (j > 0) array.push_back(static_cast<VMesh::Node::index_type>(i+(j-1)*mj+k*mk));
  if (j < this->nj_-1) array.push_back(static_cast<VMesh::Node::index_type>(i+(j+1)*mj+k*mk));
  if (k > 0) array.push_back(static_cast<VMesh::Node::index_type>(i+j*mj+(k-1)*mk));
  if (k < this->nk_-1) array.push_back(static_cast<VMesh::Node::index_type>(i+j*mj+(k+1)*mk));
}

// WE should prcompute these:
template <class MESH>
double
VLatVolMesh<MESH>::get_size(VMesh::Node::index_type /*i*/) const
{
  return (0.0);
}

template <class MESH>
double
VLatVolMesh<MESH>::get_size(VMesh::Edge::index_type i) const
{
  typename MESH::Edge::index_type vi; to_index(vi,i);
  return (this->mesh_->get_size(vi));
}

template <class MESH>
double
VLatVolMesh<MESH>::get_size(VMesh::Face::index_type i) const
{
  typename MESH::Face::index_type vi; to_index(vi,i);
  return (this->mesh_->get_size(vi));
}

template <class MESH>
double
VLatVolMesh<MESH>::get_size(VMesh::Cell::index_type i) const
{
  typename MESH::Cell::index_type vi; to_index(vi,i);
  return (this->mesh_->get_size(vi));
}

template <class MESH>
double
VLatVolMesh<MESH>::get_size(VMesh::Elem::index_type i) const
{
  typename MESH::Elem::index_type vi; to_index(vi,i);
  return (this->mesh_->get_size(vi));
}

template <class MESH>
double
VLatVolMesh<MESH>::get_size(VMesh::DElem::index_type i) const
{
  typename MESH::DElem::index_type vi; to_index(vi,i);
  return (this->mesh_->get_size(vi));
}


template <class MESH>
void 
VLatVolMesh<MESH>::pwl_approx_edge(std::vector<VMesh::coords_type > &coords, 
                                  VMesh::Elem::index_type /*ci*/, 
                                  unsigned int which_edge,
                                  unsigned int div_per_unit) const
{
  this->basis_->approx_edge(which_edge, div_per_unit, coords);
}

template <class MESH>
void 
VLatVolMesh<MESH>::pwl_approx_face(std::vector<std::vector<VMesh::coords_type > > &coords, 
                                  VMesh::Elem::index_type /*ci*/, 
                                  unsigned int which_face,
                                  unsigned int div_per_unit) const
{
  this->basis_->approx_face(which_face, div_per_unit, coords);
}

template <class MESH>
void 
VLatVolMesh<MESH>::get_random_point(Point &p,
				    VMesh::Elem::index_type i,
				    FieldRNG &rng) const
{
  typename MESH::Elem::index_type vi; 
  to_index(vi,i);
  this->mesh_->get_random_point(p,vi,rng);
}

template <class MESH>
void 
VLatVolMesh<MESH>::get_dimensions(VMesh::dimension_type& dims)
{
  dims.resize(3);
  dims[0] = this->ni_;
  dims[1] = this->nj_;
  dims[2] = this->nk_;
}

template <class MESH>
void 
VLatVolMesh<MESH>::get_elem_dimensions(VMesh::dimension_type& dims)
{
  dims.resize(3);
  dims[0] = this->ni_-1;
  dims[1] = this->nj_-1;
  dims[2] = this->nk_-1;
 }

template <class MESH>
double 
VLatVolMesh<MESH>::det_jacobian(const VMesh::coords_type& /*coords*/, 
                                VMesh::Elem::index_type /*idx*/) const
{
  return(this->mesh_->det_jacobian_);
}


template <class MESH>
double 
VLatVolMesh<MESH>::scaled_jacobian_metric(const VMesh::Elem::index_type /*idx*/) const
{
  return(this->mesh_->scaled_jacobian_);
}

template <class MESH>
double 
VLatVolMesh<MESH>::jacobian_metric(const VMesh::Elem::index_type /*idx*/) const
{
  return(this->mesh_->det_jacobian_);
}

template <class MESH>
bool
VLatVolMesh<MESH>::find_closest_node(double& pdist,
                                     Point &result, 
                                     VMesh::Node::index_type &node,
                                     const Point &p, 
                                     double maxdist) const
{
  bool ret = find_closest_node(pdist,result,node,p);
  if (!ret)  return (false);
  if (maxdist < 0.0 || pdist < maxdist) return (true);
  return (false);
}

template <class MESH>
bool 
VLatVolMesh<MESH>::find_closest_node(double& pdist,
                                     Point& result, 
                                     VMesh::Node::index_type& node,
                                     const Point& p) const
{
  if (this->ni_ == 0 || this->nj_ == 0 || this->nk_ == 0) return (false);
  
  const Point r = this->mesh_->transform_.unproject(p);

  double rx = floor(r.x() + 0.5);
  double ry = floor(r.y() + 0.5);
  double rz = floor(r.z() + 0.5);
  
  const double nii = static_cast<double>(this->ni_-1);
  const double njj = static_cast<double>(this->nj_-1);
  const double nkk = static_cast<double>(this->nk_-1);

  if (rx < 0.0) rx = 0.0; if (rx > nii) rx = nii;
  if (ry < 0.0) ry = 0.0; if (ry > njj) ry = njj;
  if (rz < 0.0) rz = 0.0; if (rz > nkk) rz = nkk;

  result = this->mesh_->transform_.project(Point(rx,ry,rz)); 
  index_type i = static_cast<index_type>(rx);
  index_type j = static_cast<index_type>(ry);
  index_type k = static_cast<index_type>(rz);
  
  node = i + j*(this->ni_)+k*(this->ni_*this->nk_);  
  pdist = (p-result).length();
  return (true);
}

template <class MESH>
bool 
VLatVolMesh<MESH>::find_closest_elem(double& pdist, 
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
VLatVolMesh<MESH>::find_closest_elem(double& pdist, 
                                     Point &result, 
                                     VMesh::coords_type& coords,
                                     VMesh::Elem::index_type& elem,
                                     const Point &p) const
{
  if (this->ni_ == 0 || this->nj_ == 0 || this->nk_ == 0) return (false);
  
  const Point r = this->mesh_->transform_.unproject(p);

  double ii = r.x();
  double jj = r.y();
  double kk = r.z();
  const double nii = static_cast<double>(this->ni_-1);
  const double njj = static_cast<double>(this->nj_-1);
  const double nkk = static_cast<double>(this->nk_-1);
   
  if (ii < 0.0) ii = 0.0; if (ii >= nii) ii = nii;
  if (jj < 0.0) jj = 0.0; if (jj >= njj) jj = njj;
  if (kk < 0.0) kk = 0.0; if (kk >= nkk) kk = nkk;

  double fi = floor(ii); if (fi == nii) fi--;
  double fj = floor(jj); if (fj == njj) fj--;
  double fk = floor(kk); if (fk == nkk) fk--;

  index_type i = static_cast<index_type>(fi);
  index_type j = static_cast<index_type>(fj);
  index_type k = static_cast<index_type>(fk);
  
  elem = i+ j*(this->ni_-1) + k*(this->ni_-1)*(this->nj_-1);
  result = this->mesh_->transform_.project(Point(ii,jj,kk));
  pdist = (p-result).length();

  coords.resize(3);
  coords[0] = ii-fi;
  coords[1] = jj-fj;
  coords[2] = kk-fk;

  return (true);
}

template <class MESH>
bool 
VLatVolMesh<MESH>::find_closest_elems(double& pdist, Point &result,
                                     VMesh::Elem::array_type &elems,
                                     const Point &p) const
{
  // For calculations inside the local element
  const double epsilon = 1e-8;
  elems.clear();
  
  if (this->ni_ == 0 || this->nj_ == 0 || this->nk_ == 0) return (false);
  
  const Point r = this->mesh_->transform_.unproject(p);

  double ii = r.x();
  double jj = r.y();
  double kk = r.z();
  const double nii = static_cast<double>(this->ni_-2);
  const double njj = static_cast<double>(this->nj_-2);
  //const double nkk = static_cast<double>(this->nk_-2);
   
  if (ii < 0.0) ii = 0.0; if (ii > nii) ii = nii;
  if (jj < 0.0) jj = 0.0; if (jj > njj) jj = njj;
  if (jj < 0.0) jj = 0.0; if (jj > njj) jj = njj;
  const double fii = floor(ii);
  const double fjj = floor(jj);
  const double fkk = floor(kk);

  index_type i = static_cast<index_type>(fii);
  index_type j = static_cast<index_type>(fjj);
  index_type k = static_cast<index_type>(fkk);
  
  index_type elem = i + j*(this->ni_-1)+ k*(this->ni_-1)*(this->nj_-1);
  elems.push_back(elem);

  if ((fabs(fii-ii) < epsilon) && ((i-1)>0))
  {
    elem = i-1 + j*(this->ni_-1)+ k*(this->ni_-1)*(this->nj_-1);
    elems.push_back(elem);  
  }
  
  if ((fabs(fii-(ii+1.0)) < epsilon) && (i<(this->ni_-1)))
  {
    elem = i+1 + j*(this->ni_-1)+ k*(this->ni_-1)*(this->nj_-1);
    elems.push_back(elem);  
  }

  if ((fabs(fjj-jj) < epsilon) && ((j-1)>0))
  {
    elem = i + (j-1)*(this->ni_-1)+ k*(this->ni_-1)*(this->nj_-1);
    elems.push_back(elem);  
  }
  
  if ((fabs(fjj-(jj+1.0)) < epsilon) && (j<(this->nj_-1)))
  {
    elem = i + (j-1)*(this->ni_-1)+ k*(this->ni_-1)*(this->nj_-1);
    elems.push_back(elem);  
  }

  if ((fabs(fkk-kk) < epsilon) && ((k-1)>0))
  {
    elem = i + j*(this->ni_-1)+ (k-1)*(this->ni_-1)*(this->nj_-1);
    elems.push_back(elem);  
  }
  
  if ((fabs(fkk-(kk+1.0)) < epsilon) && (k<(this->nk_-1)))
  {
    elem = i + j*(this->ni_-1)+ (k+1)*(this->ni_-1)*(this->nj_-1);
    elems.push_back(elem);  
  }
  
  result = this->mesh_->transform_.project(Point(ii,jj,kk));
  pdist = (p-result).length();
  
  return (true);
}

template <class MESH>
void
VLatVolMesh<MESH>::jacobian(const VMesh::coords_type& /*coords*/, 
                            VMesh::Elem::index_type /*idx*/, double* J) const
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
VLatVolMesh<MESH>::inverse_jacobian(const VMesh::coords_type& /*coords*/,
                  VMesh::Elem::index_type /*idx*/, double* Ji) const
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
Transform
VLatVolMesh<MESH>::get_transform() const
{
  return (this->mesh_->get_transform());
}

template <class MESH>
void
VLatVolMesh<MESH>::set_transform(const Transform& t)
{
  this->mesh_->set_transform(t);
}



template <class MESH>
void
VLatVolMesh<MESH>::get_interpolate_weights(const Point& point, 
                                           VMesh::ElemInterpolate& ei,
                                           int basis_order) const
{
  VMesh::Elem::index_type elem;
  StackVector<double,3> coords;
  ei.basis_order = basis_order;

  if(elem_locate(elem,coords,point))
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
VLatVolMesh<MESH>::get_interpolate_weights(const VMesh::coords_type& coords, 
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
VLatVolMesh<MESH>::get_minterpolate_weights(const std::vector<Point>& point, 
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
          if(elem_locate(elem,point[i]))
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
        typename VMesh::Elem::index_type elem;
        
        for (size_t i=0; i<ei.size();i++)
        {
          ei[i].basis_order = basis_order;
          if(elem_locate(elem,coords,point[i]))
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
          if(elem_locate(elem,coords,point[i]))
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
          if(elem_locate(elem,coords,point[i]))
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
VLatVolMesh<MESH>::get_minterpolate_weights(const std::vector<VMesh::coords_type>& coords, 
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
VLatVolMesh<MESH>::get_gradient_weights(const VMesh::coords_type& coords, 
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
VLatVolMesh<MESH>::get_gradient_weights(const Point& point, 
                                    VMesh::ElemGradient& eg,
                                    int basis_order) const
{
  double* inverse_jacobian = this->mesh_->inverse_jacobian_;

  VMesh::Elem::index_type elem;
  StackVector<double,3> coords;

  if(!(elem_locate(elem,coords,point)))
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
VLatVolMesh<MESH>::get_mgradient_weights(const std::vector<Point>& point, 
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
          if(elem_locate(elem,point[i]))
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
          if(elem_locate(elem,coords,point[i]))
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
          if(elem_locate(elem,coords,point[i]))
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
          if(elem_locate(elem,coords,point[i]))
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
VLatVolMesh<MESH>::get_mgradient_weights(const std::vector<VMesh::coords_type>& coords, 
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



#if (SCIRUN_STRUCTHEXVOL_SUPPORT > 0)

template<class MESH>
class VStructHexVolMesh : public VLatVolMesh<MESH> {
public:

  virtual bool is_latvolmesh()         { return (false); }
  virtual bool is_structhexvolmesh()   { return (true); }

  /// constructor and descructor
  VStructHexVolMesh(MESH* mesh) :
    VLatVolMesh<MESH>(mesh),
    points_(mesh->get_points())
  {
    DEBUG_CONSTRUCTOR("VStructHexVolMesh")   
  }
  
  virtual ~VStructHexVolMesh()
  {
    DEBUG_DESTRUCTOR("VStructHexVolMesh")     
  }
  
  friend class ElemData;

  class ElemData
  {
  public:
    ElemData(const VStructHexVolMesh<MESH>* vmesh,MESH* mesh, VMesh::Elem::index_type idx) :
      index_(idx),
      points_(mesh->get_points())    
    {
      vmesh->get_nodes_from_cell(nodes_,idx);
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
    const Array3<Point>&     points_;
    VMesh::Node::array_type  nodes_;
  };
  
  virtual void get_center(Point &point, VMesh::Node::index_type i) const;
  virtual void get_center(Point &point, VMesh::ENode::index_type i) const;
  virtual void get_center(Point &point, VMesh::Edge::index_type i) const;
  virtual void get_center(Point &point, VMesh::Face::index_type i) const;
  virtual void get_center(Point &point, VMesh::Cell::index_type i) const;
  virtual void get_center(Point &point, VMesh::Elem::index_type i) const;
  virtual void get_center(Point &point, VMesh::DElem::index_type i) const;

  /// Get the centers of a series of nodes
  virtual void get_centers(Point* points, const VMesh::Node::array_type& array) const;
  virtual void get_centers(Point* points, const VMesh::Elem::array_type& array) const;

  virtual double get_size(VMesh::Node::index_type i) const;
  virtual double get_size(VMesh::Edge::index_type i) const;
  virtual double get_size(VMesh::Face::index_type i) const;
  virtual double get_size(VMesh::Cell::index_type i) const;
  virtual double get_size(VMesh::Elem::index_type i) const;
  virtual double get_size(VMesh::DElem::index_type i) const;
                                                    
  virtual bool locate(VMesh::Node::index_type &i, const Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, const Point &point) const;

  virtual bool locate(VMesh::Elem::array_type &i, const BBox &bbox) const;
  
  virtual bool find_closest_node(double& pdist, 
                                 Point& result,
                                 VMesh::Node::index_type& elem, 
                                 const Point& p) const;

  virtual bool find_closest_node(double& pdist, 
                                 Point& result,
                                 VMesh::Node::index_type& elem, 
                                 const Point& p,
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
                                                    
  virtual void derivate(VMesh::dpoints_type &p,
			const VMesh::coords_type &coords, 
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
  inline void to_index(typename LatVolMesh<typename MESH::basis_type>::Node::index_type &index, INDEX idx) const
  {
    const VMesh::index_type i = idx % this->ni_;
    const VMesh::index_type jk = idx / this->ni_;
    const VMesh::index_type j = jk % this->nj_;
    const VMesh::index_type k = jk / this->nj_;
    index = typename LatVolMesh<typename LatVolMesh<typename MESH::basis_type>::basis_type>::Node::index_type(this->mesh_, i, j, k);  
  }

  template <class INDEX>
  inline void to_index(typename LatVolMesh<typename MESH::basis_type>::Edge::index_type &index, INDEX idx) const
    {index = typename LatVolMesh<typename MESH::basis_type>::Edge::index_type(idx);}

  template <class INDEX>
  inline void to_index(typename LatVolMesh<typename MESH::basis_type>::Face::index_type &index, INDEX idx) const
    {index = typename LatVolMesh<typename MESH::basis_type>::Face::index_type(idx);}

  template <class INDEX>
  inline  void to_index(typename LatVolMesh<typename MESH::basis_type>::Cell::index_type &index, INDEX idx) const
  {
    const VMesh::index_type i = idx % (this->ni_-1);
    const VMesh::index_type jk = idx / (this->ni_-1);
    const VMesh::index_type j = jk % (this->nj_-1);
    const VMesh::index_type k = jk / (this->nj_-1);
    index = typename LatVolMesh<typename MESH::basis_type>::Cell::index_type(this->mesh_, i, j, k); 
  }
  
  template<class ARRAY, class INDEX>
  void
  inv_jacobian(const ARRAY& coords, INDEX idx, double* Ji) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(this,this->mesh_,idx);
    this->basis_->derivate(coords,ed,Jv);
    double J[9];
    J[0] = Jv[0].x();
    J[1] = Jv[0].y();
    J[2] = Jv[0].z();
    J[3] = Jv[1].x();
    J[4] = Jv[1].y();
    J[5] = Jv[1].z();
    J[6] = Jv[2].x();
    J[7] = Jv[2].y();
    J[8] = Jv[2].z();
    InverseMatrix3x3(J,Ji);
  }

  Array3<Point>&     points_;
};


/// Functions for creating the virtual interface for specific mesh types
/// These are similar to compare maker and only serve to instantiate the class

/// Currently there are only 3 variations of this mesh available
/// 1) linear interpolation

/// Add the LINEAR virtual interface and the meshid for creating it 

/// Create virtual interface 
VMesh* CreateVStructHexVolMesh(StructHexVolMesh<HexTrilinearLgn<Point> >* mesh)
{
  return new VStructHexVolMesh<StructHexVolMesh<HexTrilinearLgn<Point> > >(mesh);
}

/// Register class maker, so we can instantiate it
static MeshTypeID StructHexVolMesh_MeshID1(StructHexVolMesh<HexTrilinearLgn<Point> >::type_name(-1),
                  StructHexVolMesh<HexTrilinearLgn<Point> >::mesh_maker,
                  StructHexVolMesh<HexTrilinearLgn<Point> >::structhexvol_maker);


template <class MESH>
void
VStructHexVolMesh<MESH>::get_center(Point &p, VMesh::Node::index_type idx) const
{
  p = points_[idx];
}

template <class MESH>
void
VStructHexVolMesh<MESH>::get_center(Point &p,VMesh::ENode::index_type idx) const
{
  get_center(p,VMesh::Edge::index_type(idx));
}

template <class MESH>
void
VStructHexVolMesh<MESH>::get_center(Point &p,VMesh::Edge::index_type idx) const
{
  StackVector<VMesh::index_type,2> nodes;
  this->get_nodes_from_edge(nodes,idx);
  
  p = points_[nodes[0]];
  p += points_[nodes[1]];
  p *= 0.5;
}

template <class MESH>
void
VStructHexVolMesh<MESH>::get_center(Point &p, VMesh::Face::index_type idx) const
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
VStructHexVolMesh<MESH>::get_center(Point &p, VMesh::Cell::index_type idx) const
{
  StackVector<VMesh::index_type,8> nodes;
  this->get_nodes_from_cell(nodes,idx);
  
  p = points_[nodes[0]];
  p += points_[nodes[1]];
  p += points_[nodes[2]];
  p += points_[nodes[3]];
  p += points_[nodes[4]];
  p += points_[nodes[5]];
  p += points_[nodes[6]];
  p += points_[nodes[7]];
  p *= 0.125;
}

template <class MESH>
void
VStructHexVolMesh<MESH>::get_center(Point &p, VMesh::Elem::index_type idx) const
{
  get_center(p, VMesh::Cell::index_type(idx));
}

template <class MESH>
void
VStructHexVolMesh<MESH>::get_center(Point &p, VMesh::DElem::index_type idx) const
{
  get_center(p, VMesh::Face::index_type(idx));
}


template <class MESH>
void
VStructHexVolMesh<MESH>::get_centers(Point* points, const VMesh::Node::array_type& array) const
{
  for (size_t j=0; j <array.size(); j++)
  {
    points[j] = points_[array[j]];
  }
}                                     
 
template <class MESH>
void
VStructHexVolMesh<MESH>::get_centers(Point* points, const VMesh::Elem::array_type& array) const
{
  Point p;
  StackVector<VMesh::index_type,8> nodes;
  for (size_t j=0; j <array.size(); j++)
  {
    VMesh::Elem::index_type idx = array[j];
    
    this->get_nodes_from_cell(nodes,idx);  
    p = points_[nodes[0]];
    p += points_[nodes[1]];
    p += points_[nodes[2]];
    p += points_[nodes[3]];
    p += points_[nodes[4]];
    p += points_[nodes[5]];
    p += points_[nodes[6]];
    p += points_[nodes[7]];
    p *= 0.125;  

    points[j] = p;
  }
} 




template <class MESH>
double
VStructHexVolMesh<MESH>::get_size(VMesh::Node::index_type /*idx*/) const
{
  return (0.0);
}

template <class MESH>
double
VStructHexVolMesh<MESH>::get_size(VMesh::Edge::index_type idx) const
{
  StackVector<VMesh::index_type,2> nodes;
  this->get_nodes_from_edge(nodes,idx);
  
  const Point &p0 = points_[nodes[0]];
  const Point &p1 = points_[nodes[1]];
  
  return (p1 - p0).length();
}

template <class MESH>
double
VStructHexVolMesh<MESH>::get_size(VMesh::Face::index_type idx) const
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
VStructHexVolMesh<MESH>::get_size(VMesh::Cell::index_type idx) const
{
  StackVector<VMesh::index_type,8> nodes;
  this->get_nodes_from_cell(nodes,idx);
  const Point &p0 = points_[nodes[0]];
  const Point &p1 = points_[nodes[1]];
  const Point &p2 = points_[nodes[2]];
  const Point &p3 = points_[nodes[3]];
  const Point &p4 = points_[nodes[4]];
  const Point &p5 = points_[nodes[5]];
  const Point &p6 = points_[nodes[6]];
  const Point &p7 = points_[nodes[7]];

  const double a0 = tetrahedra_volume(p0, p1, p2, p5);
  const double a1 = tetrahedra_volume(p0, p2, p3, p7);
  const double a2 = tetrahedra_volume(p0, p5, p2, p7);
  const double a3 = tetrahedra_volume(p0, p5, p7, p4);
  const double a4 = tetrahedra_volume(p5, p2, p7, p6);
  
  return (a0 + a1 + a2 + a3 + a4);
}


template <class MESH>
double
VStructHexVolMesh<MESH>::get_size(VMesh::Elem::index_type idx) const
{
  return(get_size(VMesh::Cell::index_type(idx)));
}

template <class MESH>
double
VStructHexVolMesh<MESH>::get_size(VMesh::DElem::index_type idx) const
{
  return(get_size(VMesh::Face::index_type(idx)));
}


template <class MESH>
bool 
VStructHexVolMesh<MESH>::locate(VMesh::Node::index_type &vi, const Point &point) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename LatVolMesh<typename MESH::basis_type>::Node::index_type i(this->mesh_,0,0,0);
  to_index(i,vi);
  bool ret = this->mesh_->locate(i,point);
  vi = VMesh::Node::index_type(i);
  return (ret);
}


template <class MESH>
bool 
VStructHexVolMesh<MESH>::locate(VMesh::Elem::index_type &vi, const Point &point) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename LatVolMesh<typename MESH::basis_type>::Elem::index_type i(this->mesh_,0,0,0);
  to_index(i,vi);
  bool ret = this->mesh_->locate(i,point);
  vi = VMesh::Elem::index_type(i);
  return (ret);
}

template <class MESH>
bool 
VStructHexVolMesh<MESH>::locate(VMesh::Elem::array_type &va, const BBox &bbox) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename LatVolMesh<typename MESH::basis_type>::Elem::array_type a;
  bool ret = this->mesh_->locate_elems(a,bbox);
  this->convert_vector(va,a);
  return (ret);
}


template <class MESH>
bool 
VStructHexVolMesh<MESH>::
find_closest_node(double& pdist, Point& result,
                  VMesh::Node::index_type &idx, 
                  const Point &point) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename LatVolMesh<typename MESH::basis_type>::Node::index_type i(this->mesh_,0,0,0);
  to_index(i,idx);
  bool ret = this->mesh_->find_closest_node(pdist,result,i,point,-1.0);
  idx = VMesh::Node::index_type(i);
  return (ret);
} 

template <class MESH>
bool 
VStructHexVolMesh<MESH>::
find_closest_node(double& pdist, Point& result,
                  VMesh::Node::index_type &idx, 
                  const Point &point,
                  double maxdist) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename LatVolMesh<typename MESH::basis_type>::Node::index_type i;
  to_index(i,idx);
  bool ret = this->mesh_->find_closest_node(pdist,result,i,point,maxdist);
  idx = static_cast<VMesh::Node::index_type>(i);
  return (ret);
} 


template <class MESH>
bool 
VStructHexVolMesh<MESH>::
find_closest_elem(double& pdist, 
                  Point& result,
                  VMesh::coords_type& coords,
                  VMesh::Elem::index_type &idx, 
                  const Point &point) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename LatVolMesh<typename MESH::basis_type>::Elem::index_type i(this->mesh_,0,0,0);
  to_index(i,idx);
  bool ret = this->mesh_->find_closest_elem(pdist,result,coords,i,point,-1.0);
  idx = VMesh::Elem::index_type(i);
  return (ret);
} 


template <class MESH>
bool 
VStructHexVolMesh<MESH>::
find_closest_elem(double& pdist, 
                  Point& result,
                  VMesh::coords_type& coords,
                  VMesh::Elem::index_type &idx, 
                  const Point &point,
                  double maxdist) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename LatVolMesh<typename MESH::basis_type>::Elem::index_type i;
  to_index(i,idx);
  bool ret = this->mesh_->find_closest_elem(pdist,result,coords,i,point,maxdist);
  idx = static_cast<VMesh::Elem::index_type>(i);
  return (ret);
} 

template <class MESH>
bool 
VStructHexVolMesh<MESH>::
find_closest_elems(double& pdist, Point& result,
                   VMesh::Elem::array_type &idx, 
                   const Point &point) const
{
  // NEED TO MAKE THIS MORE EFFICIENT
  typename LatVolMesh<typename MESH::basis_type>::Elem::array_type i;
  bool ret = this->mesh_->find_closest_elems(pdist,result,i,point);
  this->convert_vector(idx,i);
  return (ret);
} 



template <class MESH>
bool 
VStructHexVolMesh<MESH>::get_coords(VMesh::coords_type &coords, const Point &point, 
                                                    VMesh::Elem::index_type idx) const
{
  ElemData ed(this,this->mesh_, idx);
  return (this->basis_->get_coords(coords, point, ed));
}  
  
template <class MESH>
void 
VStructHexVolMesh<MESH>::interpolate(Point &pt, const VMesh::coords_type &coords, 
                                                    VMesh::Elem::index_type idx) const
{
  ElemData ed(this,this->mesh_, idx);
  pt = this->basis_->interpolate(coords, ed);
}

template <class MESH>
void 
VStructHexVolMesh<MESH>::minterpolate(std::vector<Point> &point, 
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
VStructHexVolMesh<MESH>::derivate(VMesh::dpoints_type &dp, const VMesh::coords_type &coords, 
                                                    VMesh::Elem::index_type idx) const
{
  ElemData ed(this,this->mesh_, idx);
  this->basis_->derivate(coords, ed, dp);
}

template <class MESH>
void 
VStructHexVolMesh<MESH>::get_random_point(Point &p,
					  VMesh::Elem::index_type i,
					  FieldRNG &rng) const
{
  typename LatVolMesh<typename MESH::basis_type>::Elem::index_type vi;
  to_index(vi,i);
  this->mesh_->get_random_point(p,vi,rng);
}

template <class MESH>
void 
VStructHexVolMesh<MESH>::set_point(const Point &point, VMesh::Node::index_type idx)
{
  points_[idx] = point;
}

template <class MESH>
Point*
VStructHexVolMesh<MESH>::get_points_pointer() const
{
  if (points_.size() == 0) return (0);
  return (&(points_[0]));
}

template <class MESH>
double 
VStructHexVolMesh<MESH>::det_jacobian(const VMesh::coords_type& coords,
				      VMesh::Elem::index_type idx) const
{
  double J[9];
  jacobian(coords,idx,J);
  return (DetMatrix3x3(J));
}


template <class MESH>
void
VStructHexVolMesh<MESH>::jacobian(const VMesh::coords_type& coords,
                  VMesh::Elem::index_type idx, double* J) const
{
  StackVector<Point,3> Jv;
  ElemData ed(this,this->mesh_,idx);
  this->basis_->derivate(coords,ed,Jv);
  J[0] = Jv[0].x();
  J[1] = Jv[0].y();
  J[2] = Jv[0].z();
  J[3] = Jv[1].x();
  J[4] = Jv[1].y();
  J[5] = Jv[1].z();
  J[6] = Jv[2].x();
  J[7] = Jv[2].y();
  J[8] = Jv[2].z();
}


template <class MESH>
double
VStructHexVolMesh<MESH>::inverse_jacobian(const VMesh::coords_type& coords,
                  VMesh::Elem::index_type idx, double* Ji) const
{
  StackVector<Point,3> Jv;
  ElemData ed(this,this->mesh_,idx);
  this->basis_->derivate(coords,ed,Jv);
  double J[9];
  J[0] = Jv[0].x();
  J[1] = Jv[0].y();
  J[2] = Jv[0].z();
  J[3] = Jv[1].x();
  J[4] = Jv[1].y();
  J[5] = Jv[1].z();
  J[6] = Jv[2].x();
  J[7] = Jv[2].y();
  J[8] = Jv[2].z();
  
  return (InverseMatrix3x3(J,Ji));
}


template <class MESH>
double 
VStructHexVolMesh<MESH>::scaled_jacobian_metric(VMesh::Elem::index_type idx) const
{
  StackVector<Point,3> Jv;
  ElemData ed(this,this->mesh_,idx);

  double temp;
  this->basis_->derivate(this->basis_->unit_center,ed,Jv);
  double min_jacobian = ScaledDetMatrix3P(Jv);
  
  size_t num_vertices = this->basis_->number_of_vertices();
  for (size_t j=0;j < num_vertices;j++)
  {
    this->basis_->derivate(this->basis_->unit_vertices[j],ed,Jv);
    temp = ScaledDetMatrix3P(Jv);
    if(temp < min_jacobian) min_jacobian = temp;
  }
    
  return (min_jacobian);
}

template <class MESH>
double 
VStructHexVolMesh<MESH>::jacobian_metric(VMesh::Elem::index_type idx) const
{
  StackVector<Point,3> Jv;
  ElemData ed(this,this->mesh_,idx);

  double temp;
  this->basis_->derivate(this->basis_->unit_center,ed,Jv);
  double min_jacobian = DetMatrix3P(Jv);
  
  size_t num_vertices = this->basis_->number_of_vertices();
  for (size_t j=0;j < num_vertices;j++)
  {
    this->basis_->derivate(this->basis_->unit_vertices[j],ed,Jv);
    temp = DetMatrix3P(Jv);
    if(temp < min_jacobian) min_jacobian = temp;
  }
    
  return (min_jacobian);
}


template <class MESH>
void
VStructHexVolMesh<MESH>::get_interpolate_weights(const Point& point, 
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
VStructHexVolMesh<MESH>::get_interpolate_weights(const VMesh::coords_type& coords, 
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
VStructHexVolMesh<MESH>::get_minterpolate_weights(const std::vector<Point>& point, 
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
VStructHexVolMesh<MESH>::get_minterpolate_weights(const std::vector<VMesh::coords_type>& coords, 
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
VStructHexVolMesh<MESH>::get_gradient_weights(const VMesh::coords_type& coords, 
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
VStructHexVolMesh<MESH>::get_gradient_weights(const Point& point, 
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
VStructHexVolMesh<MESH>::get_mgradient_weights(const std::vector<Point>& point, 
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
VStructHexVolMesh<MESH>::get_mgradient_weights(const std::vector<VMesh::coords_type>& coords, 
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
//#endif

#endif

} // namespace
#endif

