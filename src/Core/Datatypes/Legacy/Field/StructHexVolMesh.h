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


///@details
///  A structured grid is a dataset with regular topology but with irregular
///  geometry.
///
///  The grid may have any shape but can not be overlapping or
///  self-intersecting.
///
///  The topology of a structured grid is represented using 2D, or 3D vector
///  with the points being stored in an index based array. The faces
///  (quadrilaterals) and cells (Hexahedron) are implicitly define based
///  based upon their indexing.
///
///  Structured grids are typically used in finite difference analysis.
///
///  For more information on datatypes see Schroeder, Martin, and Lorensen,
///  "The Visualization Toolkit", Prentice Hall, 1998.
///

#ifndef CORE_DATATYPES_STRUCTHEXVOLMESH_H
#define CORE_DATATYPES_STRUCTHEXVOLMESH_H 1

/// Include what kind of support we want to have
/// Need to fix this and couple it to sci-defs
#include <Core/Datatypes/Legacy/Field/MeshSupport.h>

#include <Core/Thread/Mutex.h>
#include <Core/Containers/Array3.h>
#include <Core/GeometryPrimitives/SearchGridT.h>

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/CompGeom.h>

#include <Core/Datatypes/Legacy/Field/LatVolMesh.h>

/// Incude needed for Windows: declares SCISHARE
#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

/// Declarations for virtual interface


/// Functions for creating the virtual interface
/// Declare the functions that instantiate the virtual interface
template <class Basis>
class StructHexVolMesh;

/// make sure any other mesh other than the preinstantiate ones
/// returns no virtual interface. Altering this behavior will allow
/// for dynamically compiling the interface if needed.
template<class MESH>
VMesh* CreateVStructHexVolMesh(MESH* mesh) { return (0); }

/// These declarations are needed for a combined dynamic compilation as
/// as well as virtual functions solution.
/// Declare that these can be found in a library that is already
/// precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_STRUCTHEXVOL_SUPPORT > 0)

SCISHARE VMesh* CreateVStructHexVolMesh(StructHexVolMesh<Core::Basis::HexTrilinearLgn<Core::Geometry::Point> >* mesh);

#endif


template <class Basis>
class StructHexVolMesh : public LatVolMesh<Basis>
{

template <class MESH>
friend class VStructHexVolMesh;

public:
  /// Types that change depending on 32 or 64bits
  typedef SCIRun::index_type                 under_type;
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::size_type                  size_type;
  typedef SCIRun::mask_type                  mask_type;

  typedef boost::shared_ptr<StructHexVolMesh<Basis> > handle_type;

  StructHexVolMesh();
  StructHexVolMesh(size_type i, size_type j, size_type k);
  StructHexVolMesh(const StructHexVolMesh<Basis> &copy);
  virtual StructHexVolMesh *clone() const override { return new StructHexVolMesh<Basis>(*this); }
  virtual ~StructHexVolMesh()
  {
    DEBUG_DESTRUCTOR("StructHexVolMesh")
  }

  class ElemData
  {
  public:
    typedef typename StructHexVolMesh<Basis>::index_type  index_type;

    ElemData(const StructHexVolMesh<Basis>& msh,
             const typename LatVolMesh<Basis>::Cell::index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    /// the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const {
      return (index_.i_ + mesh_.get_ni()*index_.j_ +
              mesh_.get_ni()*mesh_.get_nj()*index_.k_);
    }
    inline
    index_type node1_index() const {
      return (index_.i_+ 1 + mesh_.get_ni()*index_.j_ +
              mesh_.get_ni()*mesh_.get_nj()*index_.k_);
    }
    inline
    index_type node2_index() const {
      return (index_.i_ + 1 + mesh_.get_ni()*(index_.j_ + 1) +
              mesh_.get_ni()*mesh_.get_nj()*index_.k_);

    }
    inline
    index_type node3_index() const {
      return (index_.i_ + mesh_.get_ni()*(index_.j_ + 1) +
              mesh_.get_ni()*mesh_.get_nj()*index_.k_);
    }
    inline
    index_type node4_index() const {
      return (index_.i_ + mesh_.get_ni()*index_.j_ +
              mesh_.get_ni()*mesh_.get_nj()*(index_.k_ + 1));
    }
    inline
    index_type node5_index() const {
      return (index_.i_ + 1 + mesh_.get_ni()*index_.j_ +
              mesh_.get_ni()*mesh_.get_nj()*(index_.k_ + 1));
    }

    inline
    index_type node6_index() const {
      return (index_.i_ + 1 + mesh_.get_ni()*(index_.j_ + 1) +
              mesh_.get_ni()*mesh_.get_nj()*(index_.k_ + 1));
    }
    inline
    index_type node7_index() const {
      return (index_.i_ + mesh_.get_ni()*(index_.j_ + 1) +
              mesh_.get_ni()*mesh_.get_nj()*(index_.k_ + 1));
    }

    inline
    const Core::Geometry::Point &node0() const {
      return mesh_.points_(index_.k_, index_.j_, index_.i_);
    }
    inline
    const Core::Geometry::Point &node1() const {
      return mesh_.points_(index_.k_, index_.j_, index_.i_+1);
    }
    inline
    const Core::Geometry::Point &node2() const {
      return mesh_.points_(index_.k_, index_.j_+1, index_.i_+1);
    }
    inline
    const Core::Geometry::Point &node3() const {
      return mesh_.points_(index_.k_, index_.j_+1, index_.i_);
    }
    inline
    const Core::Geometry::Point &node4() const {
      return mesh_.points_(index_.k_+1, index_.j_, index_.i_);
    }
    inline
    const Core::Geometry::Point &node5() const {
      return mesh_.points_(index_.k_+1, index_.j_, index_.i_+1);
    }
    inline
    const Core::Geometry::Point &node6() const {
      return mesh_.points_(index_.k_+1, index_.j_+1, index_.i_+1);
    }
    inline
    const Core::Geometry::Point &node7() const {
      return mesh_.points_(index_.k_+1, index_.j_+1, index_.i_);
    }

  private:
    const StructHexVolMesh<Basis>                       &mesh_;
    const typename LatVolMesh<Basis>::Cell::index_type  index_;

  };

  friend class ElemData;

  /// get the mesh statistics
  virtual Core::Geometry::BBox get_bounding_box() const override;
  virtual void transform(const Core::Geometry::Transform &t) override;

  virtual bool get_dim(std::vector<size_type>&) const override;
  virtual void set_dim(const std::vector<size_type>& dims) override {
    LatVolMesh<Basis>::ni_ = dims[0];
    LatVolMesh<Basis>::nj_ = dims[1];
    LatVolMesh<Basis>::nk_ = dims[2];

    points_.resize(dims[2], dims[1], dims[0]);

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    LatVolMesh<Basis>::vmesh_.reset(CreateVStructHexVolMesh(this));
  }

  virtual int topology_geometry() const override
  {
    return (Mesh::STRUCTURED | Mesh::IRREGULAR);
  }

  /// get the center point (in object space) of an element
  void get_center(Core::Geometry::Point &,
                  const typename LatVolMesh<Basis>::Node::index_type &) const;
  void get_center(Core::Geometry::Point &, typename LatVolMesh<Basis>::Edge::index_type) const;
  void get_center(Core::Geometry::Point &, typename LatVolMesh<Basis>::Face::index_type) const;
  void get_center(Core::Geometry::Point &,
                  const typename LatVolMesh<Basis>::Cell::index_type &) const;

  double get_size(const typename LatVolMesh<Basis>::Node::index_type &idx) const;
  double get_size(typename LatVolMesh<Basis>::Edge::index_type idx) const;
  double get_size(typename LatVolMesh<Basis>::Face::index_type idx) const;
  double get_size(
               const typename LatVolMesh<Basis>::Cell::index_type &idx) const;
  double get_length(typename LatVolMesh<Basis>::Edge::index_type idx) const
  { return get_size(idx); };
  double get_area(typename LatVolMesh<Basis>::Face::index_type idx) const
  { return get_size(idx); };
  double get_volume(const
                    typename LatVolMesh<Basis>::Cell::index_type &i) const
  { return get_size(i); };

  bool locate(typename LatVolMesh<Basis>::Node::index_type &node,
              const Core::Geometry::Point &p) const
    { return (locate_node(node,p)); }

  bool locate(typename LatVolMesh<Basis>::Edge::index_type&,
              const Core::Geometry::Point &) const
    { return false; }
  bool locate(typename LatVolMesh<Basis>::Face::index_type &,
              const Core::Geometry::Point&) const
    { return false; }
  bool locate(typename LatVolMesh<Basis>::Cell::index_type &elem,
              const Core::Geometry::Point &p) const
    { return (locate_elem(elem,p)); }

  bool locate(typename LatVolMesh<Basis>::Cell::index_type &elem,
              std::vector<double>& coords,
              const Core::Geometry::Point &p) const
    { return (locate_elem(elem,coords,p)); }

  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         typename LatVolMesh<Basis>::Node::index_type &node,
                         const Core::Geometry::Point &p) const
    { return (find_closest_node(pdist,result,node,p,-1.0)); }

  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         typename LatVolMesh<Basis>::Node::index_type &node,
                         const Core::Geometry::Point &p, double maxdist) const;

  template<class INDEX>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point& result,
                         INDEX& elem,
                         const Core::Geometry::Point& p) const
  {
    StackVector<double,3> coords;
    return(find_closest_elem(pdist,result,coords,elem,p,-1.0));
  }

  template<class INDEX, class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point& result,
                         ARRAY& coords,
                         INDEX& elem,
                         const Core::Geometry::Point& p) const
  {
    return(find_closest_elem(pdist,result,coords,elem,p,-1.0));
  }

  template<class INDEX, class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point& result,
                         ARRAY& coords,
                         INDEX& elem,
                         const Core::Geometry::Point& p,
                         double maxdist) const
  {
    /// @todo: Generate bounding boxes for elements and integrate this into the
    // basis function code.

    /// If there are no nodes we cannot find a closest point
    if (this->ni_ == 0 || this->nj_ == 0 || this->nk_ == 0) return (false);

    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;

    /// Check whether the estimate given in idx is the point we are looking for
    if (elem.i_ >= 0 && elem.i_ < (this->ni_-1) &&
        elem.j_ >= 0 && elem.j_ < (this->nj_-1) &&
        elem.k_ >= 0 && elem.k_ < (this->nk_-1))
    {
      elem.mesh_ = this;
      if (inside(elem,p))
      {
        ElemData ed(*this,elem);
        this->basis_.get_coords(coords,p,ed);
        pdist = 0.0;
        result = p;
        return (true);
      }
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
                  "StructHexVolMesh: need to synchronize ELEM_LOCATE_E first");

// Find whether we are inside an element
    typename SearchGridT<typename LatVolMesh<Basis>::Cell::index_type>::iterator it, eit;
    if (elem_grid_->lookup(it, eit, p))
    {
      while (it != eit)
      {
        if (inside(*it, p))
        {
          elem = INDEX(*it);
          ElemData ed(*this,elem);
          this->basis_.get_coords(coords,p,ed);
          pdist = 0.0;
          result = p;
          return (true);
        }
        ++it;
      }
    }

    // Find where we are in terms of boundary elements

    // If not start searching for the closest outer boundary
    // get grid sizes
    const size_type ni = elem_grid_->get_ni()-1;
    const size_type nj = elem_grid_->get_nj()-1;
    const size_type nk = elem_grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, ei, bj, ej, bk, ek;
    elem_grid_->unsafe_locate(bi, bj, bk, p);

    // Clamp to closest point on the grid.
    if (bi > ni) bi = ni;
    if (bi < 0) bi = 0;
    if (bj > nj) bj = nj;
    if (bj < 0) bj = 0;
    if (bk > nk) bk = nk;
    if (bk < 0) bk = 0;

    ei = bi; ej = bj; ek = bk;

    double dmin = maxdist;
    bool found = true;
    bool found_one = false;

    do
    {
      found = true;
      /// We need to do a full shell without any elements that are closer
      /// to make sure there no closer elements in neighboring searchgrid cells
      for (index_type i = bi; i <= ei; i++)
      {
        if (i < 0 || i > ni) continue;
        for (index_type j = bj; j <= ej; j++)
        {
        if (j < 0 || j > nj) continue;
          for (index_type k = bk; k <= ek; k++)
          {
            if (k < 0 || k > nk) continue;
            if (i == bi || i == ei || j == bj || j == ej || k == bk || k == ek)
            {
              if (elem_grid_->min_distance_squared(p, i, j, k) < dmin)
              {
                found = false;
                typename SearchGridT<typename LatVolMesh<Basis>::Cell::index_type>::iterator it, eit;
                elem_grid_->lookup_ijk(it,eit, i, j, k);

                while (it != eit)
                {
                  Core::Geometry::Point r;
                  typename LatVolMesh<Basis>::Cell::index_type cidx = (*it);

                  const index_type ii = (*it).i_;
                  const index_type jj = (*it).j_;
                  const index_type kk = (*it).k_;
                  if (ii == 0)
                  {
                    est_closest_point_on_quad(r, p,
                                         points_(kk,jj,ii),
                                         points_(kk,jj+1,ii),
                                         points_(kk+1,jj+1,ii),
                                         points_(kk+1,jj,ii));
                    const double dtmp = (p - r).length2();
                    if (dtmp < dmin)
                    {
                      found_one = true;
                      result = r;
                      elem = INDEX(cidx);
                      dmin = dtmp;

                      if (dmin < epsilon2_)
                      {
                        ElemData ed(*this,elem);
                        this->basis_.get_coords(coords,result,ed);

                        result = this->basis_.interpolate(coords,ed);
                        pdist = sqrt((result-p).length2());
                        return (true);
                      }
                    }
                  }

                  if (ii == this->ni_-2)
                  {
                    est_closest_point_on_quad(r, p,
                                         points_(kk,jj,ii+1),
                                         points_(kk,jj+1,ii+1),
                                         points_(kk+1,jj+1,ii+1),
                                         points_(kk+1,jj,ii+1));
                    const double dtmp = (p - r).length2();
                    if (dtmp < dmin)
                    {
                      found_one = true;
                      result = r;
                      elem = INDEX(cidx);
                      dmin = dtmp;

                      if (dmin < epsilon2_)
                      {
                        ElemData ed(*this,elem);
                        this->basis_.get_coords(coords,result,ed);

                        result = this->basis_.interpolate(coords,ed);
                        pdist = sqrt((result-p).length2());
                        return (true);
                      }
                    }
                  }

                  if (jj == 0)
                  {
                    est_closest_point_on_quad(r, p,
                                         points_(kk,jj,ii),
                                         points_(kk,jj,ii+1),
                                         points_(kk+1,jj,ii+1),
                                         points_(kk+1,jj,ii));
                    const double dtmp = (p - r).length2();
                    if (dtmp < dmin)
                    {
                      found_one = true;
                      result = r;
                      elem = INDEX(cidx);
                      dmin = dtmp;

                      if (dmin < epsilon2_)
                      {
                        ElemData ed(*this,elem);
                        this->basis_.get_coords(coords,result,ed);

                        result = this->basis_.interpolate(coords,ed);
                        pdist = sqrt((result-p).length2());
                        return (true);
                      }
                    }
                  }

                  if (jj == this->nj_-2)
                  {
                    est_closest_point_on_quad(r, p,
                                         points_(kk,jj+1,ii),
                                         points_(kk,jj+1,ii+1),
                                         points_(kk+1,jj+1,ii+1),
                                         points_(kk+1,jj+1,ii));
                    const double dtmp = (p - r).length2();
                    if (dtmp < dmin)
                    {
                      found_one = true;
                      result = r;
                      elem = INDEX(cidx);
                      dmin = dtmp;

                      if (dmin < epsilon2_)
                      {
                        ElemData ed(*this,elem);
                        this->basis_.get_coords(coords,result,ed);

                        result = this->basis_.interpolate(coords,ed);
                        pdist = sqrt((result-p).length2());
                        return (true);
                      }
                    }
                  }

                  if (kk == 0)
                  {
                    est_closest_point_on_quad(r, p,
                                         points_(kk,jj,ii),
                                         points_(kk,jj,ii+1),
                                         points_(kk,jj+1,ii+1),
                                         points_(kk,jj+1,ii));
                    const double dtmp = (p - r).length2();
                    if (dtmp < dmin)
                    {
                      found_one = true;
                      result = r;
                      elem = INDEX(cidx);
                      dmin = dtmp;

                      if (dmin < epsilon2_)
                      {
                        ElemData ed(*this,elem);
                        this->basis_.get_coords(coords,result,ed);

                        result = this->basis_.interpolate(coords,ed);
                        pdist = sqrt((result-p).length2());
                        return (true);
                      }
                    }
                  }

                  if (kk == this->nk_-2)
                  {
                    est_closest_point_on_quad(r, p,
                                         points_(kk+1,jj,ii),
                                         points_(kk+1,jj,ii+1),
                                         points_(kk+1,jj+1,ii+1),
                                         points_(kk+1,jj+1,ii));
                    const double dtmp = (p - r).length2();
                    if (dtmp < dmin)
                    {
                      found_one = true;
                      result = r;
                      elem = INDEX(cidx);
                      dmin = dtmp;

                      if (dmin < epsilon2_)
                      {
                        ElemData ed(*this,elem);
                        this->basis_.get_coords(coords,result,ed);

                        result = this->basis_.interpolate(coords,ed);
                        pdist = sqrt((result-p).length2());
                        return (true);
                      }
                    }
                  }

                  ++it;
                }
              }
            }
          }
        }
      }
      bi--;ei++;
      bj--;ej++;
      bk--;ek++;
    }
    while (!found) ;

    if (!found_one) return (false);

    ElemData ed(*this,elem);
    this->basis_.get_coords(coords,result,ed);

    result = this->basis_.interpolate(coords,ed);
    dmin = (result-p).length2();
    pdist = sqrt(dmin);
    return (true);
  }


  bool find_closest_elems(double& pdist, Core::Geometry::Point &result,
                          std::vector<typename LatVolMesh<Basis>::Elem::index_type> &elems,
                          const Core::Geometry::Point &p) const;

  int get_weights(const Core::Geometry::Point &,
                  typename LatVolMesh<Basis>::Node::array_type &, double *);
  int get_weights(const Core::Geometry::Point &,
                  typename LatVolMesh<Basis>::Edge::array_type &, double *)
  { ASSERTFAIL("StructHexVolMesh::get_weights for edges isn't supported"); }
  int get_weights(const Core::Geometry::Point &,
                  typename LatVolMesh<Basis>::Face::array_type &, double *)
  { ASSERTFAIL("StructHexVolMesh::get_weights for faces isn't supported"); }
  int get_weights(const Core::Geometry::Point &,
                  typename LatVolMesh<Basis>::Cell::array_type &, double *);

  void get_point(Core::Geometry::Point &point,
              const typename LatVolMesh<Basis>::Node::index_type &index) const
  { get_center(point, index); }
  void set_point(const Core::Geometry::Point &point,
                 const typename LatVolMesh<Basis>::Node::index_type &index);


  void get_random_point(Core::Geometry::Point &p,
                        const typename LatVolMesh<Basis>::Elem::index_type & i,
                        FieldRNG &rng) const;

  /// Get the local coordinates for a certain point within an element
  /// This function uses a couple of newton iterations to find the local
  /// coordinate of a point
  template<class VECTOR>
  bool get_coords(VECTOR &coords, const Core::Geometry::Point &p,
		  typename LatVolMesh<Basis>::Elem::index_type idx) const
  {
    ElemData ed(*this, idx);
    return this->basis_.get_coords(coords, p, ed);
  }

  /// Find the location in the global coordinate system for a local
  /// coordinate ! This function is the opposite of get_coords.
  template<class VECTOR>
  void interpolate(Core::Geometry::Point &pt, const VECTOR &coords,
		   typename LatVolMesh<Basis>::Elem::index_type idx) const
  {
    ElemData ed(*this, idx);
    pt = this->basis_.interpolate(coords, ed);
  }

  /// Interpolate the derivate of the function, This infact will
  /// return the jacobian of the local to global coordinate
  /// transformation. This function is mainly intended for the non
  /// linear elements
  template<class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords,
		typename LatVolMesh<Basis>::Elem::index_type idx,
		VECTOR2 &J) const
  {
    ElemData ed(*this, idx);
    this->basis_.derivate(coords, ed, J);
  }

  /// Get the determinant of the jacobian, which is the local volume
  /// of an element and is intended to help with the integration of
  /// functions over an element.
  template<class VECTOR>
  double det_jacobian(const VECTOR& coords,
		      typename LatVolMesh<Basis>::Elem::index_type idx) const
  {
    double J[9];
    jacobian(coords,idx,J);
    return (DetMatrix3x3(J));
  }

  /// Get the jacobian of the transformation. In case one wants the
  /// non inverted version of this matrix. This is currentl here for
  /// completeness of the interface
  template<class VECTOR>
  void jacobian(const VECTOR& coords,
		typename LatVolMesh<Basis>::Elem::index_type idx,
		double* J) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);
    this->basis_.derivate(coords,ed,Jv);
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

  /// Get the inverse jacobian of the transformation. This one is
  /// needed to translate local gradients into global gradients. Hence
  /// it is crucial for calculating gradients of fields, or
  /// constructing finite elements.
  template<class VECTOR>
  double inverse_jacobian(const VECTOR& coords,
			  typename LatVolMesh<Basis>::Elem::index_type idx,
			  double* Ji) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);
    this->basis_.derivate(coords,ed,Jv);
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

  double scaled_jacobian_metric(typename LatVolMesh<Basis>::Elem::index_type idx) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;
    this->basis_.derivate(this->basis_.unit_center,ed,Jv);
    double min_jacobian = ScaledDetMatrix3P(Jv);

    size_t num_vertices = this->basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      this->basis_.derivate(this->basis_.unit_vertices[j],ed,Jv);
      temp = ScaledDetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }

    return (min_jacobian);
  }

  double jacobian_metric(typename LatVolMesh<Basis>::Elem::index_type idx) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;
    this->basis_.derivate(this->basis_.unit_center,ed,Jv);
    double min_jacobian = DetMatrix3P(Jv);

    size_t num_vertices = this->basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      this->basis_.derivate(this->basis_.unit_vertices[j],ed,Jv);
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }

    return (min_jacobian);
  }


  double get_epsilon() const
  { return (epsilon_); }

  virtual bool synchronize(mask_type) override;
  virtual bool unsynchronize(mask_type) override;
  bool clear_synchronization();

  /// Export this class using the old Pio system
  virtual void io(Piostream&) override;
  static PersistentTypeID structhexvol_typeid;
  /// Core functionality for getting the name of a templated mesh class
  static  const std::string type_name(int n = -1);

  /// Type description, used for finding names of the mesh class for
  /// dynamic compilation purposes. Soem of this should be obsolete
  virtual const TypeDescription *get_type_description() const override;
  static const TypeDescription* node_type_description();
  static const TypeDescription* edge_type_description();
  static const TypeDescription* face_type_description();
  static const TypeDescription* cell_type_description();
  static const TypeDescription* elem_type_description()
  { return cell_type_description(); }

  /// This function returns a maker for Pio.
  static Persistent *maker() { return new StructHexVolMesh<Basis>(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker()  { return boost::make_shared<StructHexVolMesh<Basis>>(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle structhexvol_maker(size_type x,
				       size_type y,
				       size_type z)
  {
    return boost::make_shared<StructHexVolMesh<Basis>>(x,y,z);
  }

  Array3<Core::Geometry::Point>& get_points() { return (points_); }

  bool inside(typename LatVolMesh<Basis>::Elem::index_type idx,
              const Core::Geometry::Point &p) const
  {
    // rewrote this function to more accurately deal with hexes that do not have
    // face aligned with the axes of the coordinate system

    // First a fast test to see whether we are inside the bounding box
    // (this code could be faster, by testing axis by axis)
    // Then if it is inside a tight bounding box compute the local coordinates
    // using the Newton search, this way we account for not planar surfaces of
    // the hexes.

    typename LatVolMesh<Basis>::Node::array_type nodes;
    this->get_nodes(nodes,idx);

    Core::Geometry::BBox bbox;
    bbox.extend(points_(nodes[0].k_,nodes[0].j_,nodes[0].i_));
    bbox.extend(points_(nodes[1].k_,nodes[1].j_,nodes[1].i_));
    bbox.extend(points_(nodes[2].k_,nodes[2].j_,nodes[2].i_));
    bbox.extend(points_(nodes[3].k_,nodes[3].j_,nodes[3].i_));
    bbox.extend(points_(nodes[4].k_,nodes[4].j_,nodes[4].i_));
    bbox.extend(points_(nodes[5].k_,nodes[5].j_,nodes[5].i_));
    bbox.extend(points_(nodes[6].k_,nodes[6].j_,nodes[6].i_));
    bbox.extend(points_(nodes[7].k_,nodes[7].j_,nodes[7].i_));
    bbox.extend(epsilon_);

    if (bbox.inside(p))
    {
      StackVector<double,3> coords;
      ElemData ed(*this, idx);
      if(this->basis_.get_coords(coords, p, ed)) return (true);
    }

    return (false);
  }



  template <class INDEX>
  inline bool locate_elem(INDEX &elem, const Core::Geometry::Point &p) const
  {
    /// @todo: Generate bounding boxes for elements and integrate this into the
    // basis function code.
    if (this->basis_.polynomial_order() > 1) return elem_locate(elem, *this, p);

    /// If there are no nodes we cannot find a closest point
    if (this->ni_ == 0 || this->nj_ == 0 || this->nk_ == 0) return (false);

    /// Check whether the estimate given in idx is the point we are looking for
    if (elem.i_ >= 0 && elem.i_ < (this->ni_-1) &&
        elem.j_ >= 0 && elem.j_ < (this->nj_-1) &&
        elem.k_ >= 0 && elem.k_ < (this->nk_-1))
    {
      elem.mesh_ = this;
      if (inside(elem,p)) return (true);
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
                  "StructHexVolMesh: need to synchronize ELEM_LOCATE_E first");

    typename SearchGridT<typename LatVolMesh<Basis>::Cell::index_type>::iterator it, eit;
    if (elem_grid_->lookup(it, eit, p))
    {
      while (it != eit)
      {
        if (inside(*it, p))
        {
          elem = INDEX(*it);
          return (true);
        }
        ++it;
      }
    }
    return (false);
  }

  template <class ARRAY>
  inline bool locate_elems(ARRAY &array, const Core::Geometry::BBox &b) const
  {

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
              "StructHexVolMesh::locate_elems requires synchronize(ELEM_LOCATE_E).")

    array.clear();
    index_type is,js,ks;
    index_type ie,je,ke;
    elem_grid_->locate_clamp(is,js,ks,b.get_min());
    elem_grid_->locate_clamp(ie,je,ke,b.get_max());
    for (index_type i=is; i<=ie;i++)
      for (index_type j=js; j<je;j++)
        for (index_type k=ks; k<ke; k++)
        {
          typename SearchGridT<typename LatVolMesh<Basis>::Cell::index_type>::iterator it, eit;
          elem_grid_->lookup_ijk(it, eit, i, j, k);
          while (it != eit)
          {
            size_t p=0;
            for (;p<array.size();p++) if (array[p] == typename ARRAY::value_type(*it)) break;
            if (p == array.size()) array.push_back(typename ARRAY::value_type(*it));
            ++it;
          }
        }

    return (array.size() > 0);
  }


  template <class INDEX, class ARRAY>
  inline bool locate_elem(INDEX &elem, ARRAY& coords, const Core::Geometry::Point &p) const
  {
    /// @todo: Generate bounding boxes for elements and integrate this into the
    // basis function code.
    if (this->basis_.polynomial_order() > 1) return elem_locate(elem, *this, p);

    /// If there are no nodes we cannot find a closest point
    if (this->ni_ == 0 || this->nj_ == 0 || this->nk_ == 0) return (false);

    /// Check whether the estimate given in idx is the point we are looking for
    if (elem.i_ >= 0 && elem.i_ < (this->ni_-1) &&
        elem.j_ >= 0 && elem.j_ < (this->nj_-1) &&
        elem.k_ >= 0 && elem.k_ < (this->nk_-1))
    {
      elem.mesh_ = this;
      if (inside(elem,p))
      {
        ElemData ed(*this,elem);
        this->basis_.get_coords(coords,p,ed);
        return (true);
      }
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
                  "StructHexVolMesh: need to synchronize ELEM_LOCATE_E first");

    typename SearchGridT<typename LatVolMesh<Basis>::Cell::index_type>::iterator it, eit;
    if (elem_grid_->lookup(it, eit, p))
    {
      while (it != eit)
      {
        if (inside(*it, p))
        {
          elem = INDEX(*it);
          ElemData ed(*this,elem);
          this->basis_.get_coords(coords,p,ed);
          return (true);
        }
        ++it;
      }
    }
    return (false);
  }


  template <class INDEX>
  bool inline locate_node(INDEX &node, const Core::Geometry::Point &p) const
  {
    /// If there are no nodes we cannot find a closest point
    if (this->ni_ == 0 || this->nj_ == 0 || this->nk_ == 0) return (false);

    /// Check first guess
    if (node.i_ >= 0 && node.i_ < this->ni_ &&
        node.j_ >= 0 && node.j_ < this->nj_ &&
        node.k_ >= 0 && node.k_ < this->nk_)
    {
      node.mesh_ = this;
      if ((p - points_(node.k_,node.j_,node.i_)).length2() < epsilon2_)
        return (true);
    }

    ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
          "StructHexVolMesh::locate_node requires synchronize(NODE_LOCATE_E).")

    // get grid sizes
    const size_type ni = node_grid_->get_ni()-1;
    const size_type nj = node_grid_->get_nj()-1;
    const size_type nk = node_grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, bj, bk, ei, ej, ek;
    node_grid_->unsafe_locate(bi, bj, bk, p);

    // Clamp to l;..;,closest point on the grid.
    if (bi > ni) bi =ni; if (bi < 0) bi = 0;
    if (bj > nj) bj =nj; if (bj < 0) bj = 0;
    if (bk > nk) bk =nk; if (bk < 0) bk = 0;

    ei = bi;
    ej = bj;
    ek = bk;

    double dmin = DBL_MAX;
    bool found = true;
    do
    {
      found = true;
      /// We need to do a full shell without any elements that are closer
      /// to make sure there no closer elements in neighboring searchgrid cells

      for (index_type i = bi; i <= ei; i++)
      {
        if (i < 0 || i > ni) continue;
        for (index_type j = bj; j <= ej; j++)
        {
          if (j < 0 || j > nj) continue;
          for (index_type k = bk; k <= ek; k++)
          {
            if (k < 0 || k > nk) continue;
            if (i == bi || i == ei || j == bj || j == ej || k == bk || k == ek)
            {
              if (node_grid_->min_distance_squared(p, i, j, k) < dmin)
              {
                found = false;
                typename SearchGridT<typename LatVolMesh<Basis>::Node::index_type>::iterator it, eit;
                node_grid_->lookup_ijk(it, eit, i, j, k);

                while (it != eit)
                {
                  const typename LatVolMesh<Basis>::Node::index_type idx = *it;
                  const Core::Geometry::Point point = points_(idx.k_,idx.j_,idx.i_);
                  const double dist = (p-point).length2();

                  if (dist < dmin)
                  {
                    node = idx;
                    dmin = dist;

                    if (dist < epsilon2_) return (true);
                  }
                  ++it;
                }
              }
            }
          }
        }
      }
      bi--;ei++;
      bj--;ej++;
      bk--;ek++;
    }
    while ((!found)||(dmin == DBL_MAX)) ;

    return (true);
  }




private:

  void insert_elem_into_grid(typename LatVolMesh<Basis>::Elem::index_type idx);
  void remove_elem_from_grid(typename LatVolMesh<Basis>::Elem::index_type idx);
  void insert_node_into_grid(typename LatVolMesh<Basis>::Node::index_type idx);
  void remove_node_from_grid(typename LatVolMesh<Basis>::Node::index_type idx);

  void compute_node_grid(Core::Geometry::BBox& bb);
  void compute_elem_grid(Core::Geometry::BBox& bb);
  void compute_epsilon(Core::Geometry::BBox& bb);

  double polygon_area(const typename LatVolMesh<Basis>::Node::array_type &,
                      const Core::Geometry::Vector) const;
  const Core::Geometry::Point &point(
                const typename LatVolMesh<Basis>::Node::index_type &idx) const
  { return points_(idx.k_, idx.j_, idx.i_); }


  Array3<Core::Geometry::Point> points_;

  boost::shared_ptr<SearchGridT<typename LatVolMesh<Basis>::Node::index_type> >  node_grid_;
  boost::shared_ptr<SearchGridT<typename LatVolMesh<Basis>::Elem::index_type> >  elem_grid_;

  mutable Core::Thread::Mutex                       synchronize_lock_;
  mask_type                           synchronized_;
  double                              epsilon_;
  double                              epsilon2_;
  double                              epsilon3_; /// for volumetric comparison
};

template <class Basis>
PersistentTypeID
StructHexVolMesh<Basis>::structhexvol_typeid(StructHexVolMesh<Basis>::type_name(-1),
                                 "Mesh", maker);

template <class Basis>
StructHexVolMesh<Basis>::StructHexVolMesh():
  synchronize_lock_("Synchronize lock"),
  synchronized_(Mesh::ALL_ELEMENTS_E),
  epsilon_(0.0),
  epsilon2_(0.0),
  epsilon3_(0.0)
{
  DEBUG_CONSTRUCTOR("StructHexVolMesh")

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  this->vmesh_.reset(CreateVStructHexVolMesh(this));
}

template <class Basis>
StructHexVolMesh<Basis>::StructHexVolMesh(size_type i,
                                          size_type j,
                                          size_type k) :
  LatVolMesh<Basis>(i, j, k, Core::Geometry::Point(0.0, 0.0, 0.0), Core::Geometry::Point(1.0, 1.0, 1.0)),
  points_(k, j, i),
  synchronize_lock_("Synchronize lock"),
  synchronized_(Mesh::ALL_ELEMENTS_E),
  epsilon_(0.0),
  epsilon2_(0.0),
  epsilon3_(0.0)
{
  DEBUG_CONSTRUCTOR("StructHexVolMesh")

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  this->vmesh_.reset(CreateVStructHexVolMesh(this));
}

template <class Basis>
StructHexVolMesh<Basis>::StructHexVolMesh(const StructHexVolMesh<Basis> &copy):
  LatVolMesh<Basis>(copy),
  synchronize_lock_("Synchronize lock"),
  synchronized_(Mesh::ALL_ELEMENTS_E),
  epsilon_(0.0),
  epsilon2_(0.0),
  epsilon3_(0.0)
{
  DEBUG_CONSTRUCTOR("StructHexVolMesh")

  copy.synchronize_lock_.lock();

  points_ = copy.points_;

  // Epsilon does not require much space, hence copy those
  synchronized_ |= copy.synchronized_ & Mesh::EPSILON_E;
  epsilon_  = copy.epsilon_;
  epsilon2_ = copy.epsilon2_;
  epsilon3_ = copy.epsilon3_;

  copy.synchronize_lock_.unlock();

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  this->vmesh_.reset(CreateVStructHexVolMesh(this));
}


template <class Basis>
bool
StructHexVolMesh<Basis>::get_dim(std::vector<size_type> &array) const
{
  array.resize(3);
  array.clear();

  array.push_back(this->ni_);
  array.push_back(this->nj_);
  array.push_back(this->nk_);

  return true;
}


template <class Basis>
Core::Geometry::BBox
StructHexVolMesh<Basis>::get_bounding_box() const
{
  Core::Geometry::BBox result;

  typename LatVolMesh<Basis>::Node::iterator ni, nie;
  this->begin(ni);
  this->end(nie);
  while (ni != nie)
  {
    Core::Geometry::Point p;
    get_center(p, *ni);
    result.extend(p);
    ++ni;
  }
  return result;
}

template <class Basis>
void
StructHexVolMesh<Basis>::transform(const Core::Geometry::Transform &t)
{
  typename LatVolMesh<Basis>::Node::iterator i, ie;
  this->begin(i);
  this->end(ie);

  while (i != ie)
  {
    points_((*i).k_,(*i).j_,(*i).i_) =
      t.project(points_((*i).k_,(*i).j_,(*i).i_));

    ++i;
  }

  synchronize_lock_.lock();
  if (node_grid_) { node_grid_->transform(t); }
  if (elem_grid_) { elem_grid_->transform(t); }
  synchronize_lock_.unlock();

}


template <class Basis>
void
StructHexVolMesh<Basis>::get_center(Core::Geometry::Point &result,
				    const typename LatVolMesh<Basis>::Node::index_type &idx) const
{
  result = points_(idx.k_, idx.j_, idx.i_);
}

template <class Basis>
void
StructHexVolMesh<Basis>::get_center(Core::Geometry::Point &result,
				    typename LatVolMesh<Basis>::Edge::index_type idx) const
{
  typename LatVolMesh<Basis>::Node::array_type arr;
  this->get_nodes(arr, idx);
  Core::Geometry::Point p1;
  get_center(result, arr[0]);
  get_center(p1, arr[1]);

  result += p1;
  result *= 0.5;
}


template <class Basis>
void
StructHexVolMesh<Basis>::get_center(Core::Geometry::Point &result,
				    typename LatVolMesh<Basis>::Face::index_type idx) const
{
  typename LatVolMesh<Basis>::Node::array_type nodes;
  this->get_nodes(nodes, idx);
  ASSERT(nodes.size() == 4);
  typename LatVolMesh<Basis>::Node::array_type::iterator nai = nodes.begin();
  get_point(result, *nai);
  ++nai;
  Core::Geometry::Point pp;
  while (nai != nodes.end())
  {
    get_point(pp, *nai);
    result += pp;
    ++nai;
  }
  result *= (1.0 / 4.0);
}


template <class Basis>
void
StructHexVolMesh<Basis>::get_center(Core::Geometry::Point &result,
				    const typename LatVolMesh<Basis>::Cell::index_type &idx) const
{
  typename LatVolMesh<Basis>::Node::array_type nodes;
  this->get_nodes(nodes, idx);
  ASSERT(nodes.size() == 8);
  typename LatVolMesh<Basis>::Node::array_type::iterator nai = nodes.begin();
  get_point(result, *nai);
  ++nai;
  Core::Geometry::Point pp;
  while (nai != nodes.end())
  {
    get_point(pp, *nai);
    result += pp;
    ++nai;
  }
  result *= (1.0 / 8.0);
}

template <class Basis>
bool
StructHexVolMesh<Basis>::find_closest_node(double& pdist, Core::Geometry::Point &result,
      typename LatVolMesh<Basis>::Node::index_type &node, const Core::Geometry::Point &p,
      double maxdist) const
{
  /// If there are no nodes we cannot find a closest point
  if (this->ni_ == 0 || this->nj_ == 0 || this->nk_ == 0) return (false);

  if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;

  /// Check first guess
  if (node.i_ >= 0 && node.i_ < this->ni_ &&
      node.j_ >= 0 && node.j_ < this->nj_ &&
      node.k_ >= 0 && node.k_ < this->nk_)
  {
    node.mesh_ = this;
    Core::Geometry::Point point = points_(node.k_,node.j_,node.i_);
    double dist = (point-p).length2();

    if ( dist < epsilon2_ )
    {
      result = point;
      pdist = sqrt(dist);
      return (true);
    }
  }

  ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
    "StructHexVolMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

  // get grid sizes
  const size_type ni = node_grid_->get_ni()-1;
  const size_type nj = node_grid_->get_nj()-1;
  const size_type nk = node_grid_->get_nk()-1;

  // Convert to grid coordinates.
  index_type bi, bj, bk, ei, ej, ek;
  node_grid_->unsafe_locate(bi, bj, bk, p);

  // Clamp to closest point on the grid.
  if (bi > ni) bi = ni;
  if (bi < 0) bi = 0;
  if (bj > nj) bj = nj;
  if (bj < 0) bj = 0;
  if (bk > nk) bk = nk;
  if (bk < 0) bk = 0;

  ei = bi; ej = bj; ek = bk;

  double dmin = maxdist;
  bool found = true;
  bool found_one = false;

  do
  {
    found = true;
    /// We need to do a full shell without any elements that are closer
    /// to make sure there no closer elements in neighboring searchgrid cells

    for (index_type i = bi; i <= ei; i++)
    {
      if (i < 0 || i > ni) continue;
      for (index_type j = bj; j <= ej; j++)
      {
        if (j < 0 || j > nj) continue;
        for (index_type k = bk; k <= ek; k++)
        {
          if (k < 0 || k > nk) continue;
          if (i == bi || i == ei || j == bj || j == ej || k == bk || k == ek)
          {
            if (node_grid_->min_distance_squared(p, i, j, k) < dmin)
            {
              typename SearchGridT<typename LatVolMesh<Basis>::Node::index_type>::iterator it, eit;
              found = false;
              node_grid_->lookup_ijk(it,eit, i, j, k);

              while (it != eit)
              {
                const typename LatVolMesh<Basis>::Node::index_type idx = *it;
                const Core::Geometry::Point point = points_(idx.k_,idx.j_,idx.i_);
                const double dist = (p-point).length2();

                if (dist < dmin)
                {
                  found_one = true;
                  result = point;
                  node = idx;
                  dmin = dist;
                  /// If we are closer than eps^2 we found a node close enough
                  if (dmin < epsilon2_)
                  {
                    pdist = sqrt(dmin);
                    return (true);
                  }
                }
                ++it;
              }
            }
          }
        }
      }
    }
    bi--;ei++;
    bj--;ej++;
    bk--;ek++;
  }
  while ((!found)||(dmin == DBL_MAX)) ;

  if (!found_one) return (false);

  pdist = sqrt(dmin);
  return (true);
}

  /// Find the closest elements to a point
template<class Basis>
bool
StructHexVolMesh<Basis>::find_closest_elems(double& /*pdist*/, Core::Geometry::Point& /*result*/,
  std::vector<typename LatVolMesh<Basis>::Elem::index_type>& /*elems*/,
  const Core::Geometry::Point& /*p*/) const
{
  ASSERTFAIL("StructHexVolMesh: Find closest element has not yet been implemented.");
  return (false);
}



template <class Basis>
int
StructHexVolMesh<Basis>::get_weights(const Core::Geometry::Point &p,
				     typename LatVolMesh<Basis>::Node::array_type &l,
                                     double *w)
{
  typename LatVolMesh<Basis>::Cell::index_type idx;
  if (locate_elem(idx, p))
  {
    this->get_nodes(l,idx);
    std::vector<double> coords(3);
    if (get_coords(coords, p, idx))
    {
      this->basis_.get_weights(coords, w);
      return this->basis_.dofs();
    }
  }
  return 0;
}


template <class Basis>
int
StructHexVolMesh<Basis>::get_weights(const Core::Geometry::Point &p,
				     typename LatVolMesh<Basis>::Cell::array_type &l,
                                     double *w)
{
  typename LatVolMesh<Basis>::Cell::index_type idx;
  if (locate_elem(idx, p))
  {
    l.resize(1);
    l[0] = idx;
    w[0] = 1.0;
    return 1;
  }
  return 0;
}


/// ===================================================================
/// area3D_Polygon(): computes the area of a 3D planar polygon
///    Input:  int n = the number of vertices in the polygon
///            Core::Geometry::Point* V = an array of n+2 vertices in a plane
///                       with V[n]=V[0] and V[n+1]=V[1]
///            Core::Geometry::Point N = unit normal vector of the polygon's plane
///    Return: the (float) area of the polygon

/// Copyright 2009, softSurfer (www.softsurfer.com)
/// This code may be freely used and modified for any purpose
/// providing that this copyright notice is included with it.
/// SoftSurfer makes no warranty for this code, and cannot be held
/// liable for any real or imagined damage resulting from its use.
/// Users of this code must verify correctness for their application.

template <class Basis>
double
StructHexVolMesh<Basis>::polygon_area(const typename LatVolMesh<Basis>::Node::array_type &ni,
				      const Core::Geometry::Vector N) const
{
  double area = 0;
  double an, ax, ay, az;  /// abs value of normal and its coords
  int   coord;           // coord to ignore: 1=x, 2=y, 3=z
  index_type   i, j, k;         /// loop indices
  const size_type n = ni.size();

  /// select largest abs coordinate to ignore for projection
  ax = (N.x()>0 ? N.x() : -N.x());     /// abs x-coord
  ay = (N.y()>0 ? N.y() : -N.y());     /// abs y-coord
  az = (N.z()>0 ? N.z() : -N.z());     /// abs z-coord

  coord = 3;                     /// ignore z-coord
  if (ax > ay)
  {
    if (ax > az) coord = 1;    /// ignore x-coord
  }
  else if (ay > az) coord = 2;   /// ignore y-coord

  /// compute area of the 2D projection
  for (i=1, j=2, k=0; i<=n; i++, j++, k++)
    switch (coord)
    {
    case 1:
      area += (point(ni[i%n]).y() *
               (point(ni[j%n]).z() - point(ni[k%n]).z()));
      continue;
    case 2:
      area += (point(ni[i%n]).x() *
               (point(ni[j%n]).z() - point(ni[k%n]).z()));
      continue;
    case 3:
      area += (point(ni[i%n]).x() *
               (point(ni[j%n]).y() - point(ni[k%n]).y()));
      continue;
    }

  /// scale to get area before projection
  an = sqrt( ax*ax + ay*ay + az*az);  /// length of normal vector
  switch (coord)
  {
  case 1:
    area *= (an / (2*ax));
    break;
  case 2:
    area *= (an / (2*ay));
    break;
  case 3:
    area *= (an / (2*az));
  }
  return area;
}


inline double tri_area(const Core::Geometry::Point &a, const Core::Geometry::Point &b, const Core::Geometry::Point &c)
{
  return (0.5*Cross((a-b),(b-c)).length());
}

template <class Basis>
double
StructHexVolMesh<Basis>::get_size(const typename LatVolMesh<Basis>::Node::index_type& /*idx*/) const
{
  return 0.0;
}


template <class Basis>
double
StructHexVolMesh<Basis>::get_size(typename LatVolMesh<Basis>::Edge::index_type idx) const
{
  typename LatVolMesh<Basis>::Node::array_type arr;
  this->get_nodes(arr, idx);
  Core::Geometry::Point p0, p1;
  get_center(p0, arr[0]);
  get_center(p1, arr[1]);

  return (p1 - p0).length();
}


template <class Basis>
double
StructHexVolMesh<Basis>::get_size(typename LatVolMesh<Basis>::Face::index_type idx) const
{
  typename LatVolMesh<Basis>::Node::array_type nodes;
  this->get_nodes(nodes, idx);
  Core::Geometry::Point p0, p1, p2, p3;
  get_point(p0, nodes[0]);
  get_point(p1, nodes[1]);
  get_point(p2, nodes[2]);
  get_point(p3, nodes[3]);

  return ((Cross(p0-p1,p2-p0)).length()+(Cross(p0-p3,p2-p0)).length())*0.5;
}


template <class Basis>
double
StructHexVolMesh<Basis>::get_size(const typename LatVolMesh<Basis>::Cell::index_type &idx) const
{
  typename LatVolMesh<Basis>::Node::array_type nodes;
  this->get_nodes(nodes, idx);
  Core::Geometry::Point p0, p1, p2, p3, p4, p5, p6, p7;
  get_point(p0, nodes[0]);
  get_point(p1, nodes[1]);
  get_point(p2, nodes[2]);
  get_point(p3, nodes[3]);
  get_point(p4, nodes[4]);
  get_point(p5, nodes[5]);
  get_point(p6, nodes[6]);
  get_point(p7, nodes[7]);

  const double a0 = tetrahedra_volume(p0, p1, p2, p5);
  const double a1 = tetrahedra_volume(p0, p2, p3, p7);
  const double a2 = tetrahedra_volume(p0, p5, p2, p7);
  const double a3 = tetrahedra_volume(p0, p5, p7, p4);
  const double a4 = tetrahedra_volume(p5, p2, p7, p6);

  return (a0 + a1 + a2 + a3 + a4);
}


template <class Basis>
void
StructHexVolMesh<Basis>::set_point(const Core::Geometry::Point &p,
				   const typename LatVolMesh<Basis>::Node::index_type &idx)
{
  points_(idx.k_, idx.j_, idx.i_) = p;
}

template<class Basis>
void
StructHexVolMesh<Basis>::get_random_point(Core::Geometry::Point &p,
					  const typename LatVolMesh<Basis>::Elem::index_type &ei,
                                          FieldRNG &rng) const
{
  const Core::Geometry::Point &p0 = points_(ei.k_+0, ei.j_+0, ei.i_+0);
  const Core::Geometry::Point &p1 = points_(ei.k_+0, ei.j_+0, ei.i_+1);
  const Core::Geometry::Point &p2 = points_(ei.k_+0, ei.j_+1, ei.i_+1);
  const Core::Geometry::Point &p3 = points_(ei.k_+0, ei.j_+1, ei.i_+0);
  const Core::Geometry::Point &p4 = points_(ei.k_+1, ei.j_+0, ei.i_+0);
  const Core::Geometry::Point &p5 = points_(ei.k_+1, ei.j_+0, ei.i_+1);
  const Core::Geometry::Point &p6 = points_(ei.k_+1, ei.j_+1, ei.i_+1);
  const Core::Geometry::Point &p7 = points_(ei.k_+1, ei.j_+1, ei.i_+0);

  const double a0 = tetrahedra_volume(p0, p1, p2, p5);
  const double a1 = tetrahedra_volume(p0, p2, p3, p7);
  const double a2 = tetrahedra_volume(p0, p5, p2, p7);
  const double a3 = tetrahedra_volume(p0, p5, p7, p4);
  const double a4 = tetrahedra_volume(p5, p2, p7, p6);

  const double w = rng() * (a0 + a1 + a2 + a3 + a4);

  double t = rng();
  double u = rng();
  double v = rng();

  /// Fold cube into prism.
  if (t + u > 1.0)
  {
    t = 1.0 - t;
    u = 1.0 - u;
  }

  /// Fold prism into tet.
  if (u + v > 1.0)
  {
    const double tmp = v;
    v = 1.0 - t - u;
    u = 1.0 - tmp;
  }
  else if (t + u + v > 1.0)
  {
    const double tmp = v;
    v = t + u + v - 1.0;
    t = 1.0 - u - tmp;
  }

  /// Convert to Barycentric and compute new point.
  const double a = 1.0 - t - u - v;

  if (w > (a0 + a1 + a2 + a3))
  {
    p = Core::Geometry::Point(p5*a + p2*t + p7*u + p6*v);
  }
  else if (w > (a0 + a1 + a2))
  {
    p = Core::Geometry::Point(p0*a + p5*t + p7*u + p5*v);
  }
  else if (w > (a0 + a1))
  {
    p = Core::Geometry::Point(p0*a + p5*t + p2*u + p7*v);
  }
  else if (w > a0)
  {
    p = Core::Geometry::Point(p0*a + p2*t + p3*u + p7*v);
  }
  else
  {
    p = Core::Geometry::Point(p0*a + p1*t + p2*u + p5*v);
  }
}

template <class Basis>
bool
StructHexVolMesh<Basis>::synchronize(mask_type sync)
{
  synchronize_lock_.lock();

  if (sync & (Mesh::NODE_LOCATE_E|Mesh::ELEM_LOCATE_E|Mesh::EPSILON_E
              |Mesh::FIND_CLOSEST_E|Mesh::ELEM_LOCATE_E) &&
      ( !(synchronized_ & Mesh::NODE_LOCATE_E) ||
        !(synchronized_ & Mesh::ELEM_LOCATE_E) ||
        !(synchronized_ & Mesh::EPSILON_E) ))
  {
    /// These computations share the evaluation of the bounding box
    Core::Geometry::BBox bb = get_bounding_box();

    /// Compute the epsilon for geometrical closeness comparisons
    /// Mainly used by the grid lookup tables
    if (sync & (Mesh::EPSILON_E|Mesh::LOCATE_E|Mesh::FIND_CLOSEST_E) &&
        !(synchronized_ & Mesh::EPSILON_E))
    {
      compute_epsilon(bb);
    }

    /// Table for finding nodes in space
    if (sync & (Mesh::NODE_LOCATE_E|Mesh::FIND_CLOSEST_NODE_E) &&
        !(synchronized_ & Mesh::NODE_LOCATE_E))
    {
      compute_node_grid(bb);
    }

    /// Table for finding elements in space
    if (sync & (Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E) &&
        !(synchronized_ & Mesh::ELEM_LOCATE_E))
    {
      compute_elem_grid(bb);
    }
  }

  synchronize_lock_.unlock();

  return (true);
}

template <class Basis>
bool
StructHexVolMesh<Basis>::unsynchronize(mask_type /*sync*/)
{
  return (true);
}

template <class Basis>
bool
StructHexVolMesh<Basis>::clear_synchronization()
{
  synchronize_lock_.lock();

  // Undo marking the synchronization
  synchronized_ = Mesh::NODES_E | Mesh::ELEMS_E | Mesh::CELLS_E;

  // Free memory where possible

  node_grid_.reset();
  elem_grid_.reset();

  synchronize_lock_.unlock();

  return (true);
}

template <class Basis>
void
StructHexVolMesh<Basis>::insert_elem_into_grid(typename LatVolMesh<Basis>::Elem::index_type idx)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.

  Core::Geometry::BBox box;
  box.extend(points_(idx.k_,idx.j_,idx.i_));
  box.extend(points_(idx.k_+1,idx.j_,idx.i_));
  box.extend(points_(idx.k_,idx.j_+1,idx.i_));
  box.extend(points_(idx.k_+1,idx.j_+1,idx.i_));
  box.extend(points_(idx.k_,idx.j_,idx.i_+1));
  box.extend(points_(idx.k_+1,idx.j_,idx.i_+1));
  box.extend(points_(idx.k_,idx.j_+1,idx.i_+1));
  box.extend(points_(idx.k_+1,idx.j_+1,idx.i_+1));
  box.extend(epsilon_);
  elem_grid_->insert(idx, box);
}


template <class Basis>
void
StructHexVolMesh<Basis>::remove_elem_from_grid(typename LatVolMesh<Basis>::Elem::index_type idx)
{
  Core::Geometry::BBox box;
  box.extend(points_(idx.k_,idx.j_,idx.i_));
  box.extend(points_(idx.k_+1,idx.j_,idx.i_));
  box.extend(points_(idx.k_,idx.j_+1,idx.i_));
  box.extend(points_(idx.k_+1,idx.j_+1,idx.i_));
  box.extend(points_(idx.k_,idx.j_,idx.i_+1));
  box.extend(points_(idx.k_+1,idx.j_,idx.i_+1));
  box.extend(points_(idx.k_,idx.j_+1,idx.i_+1));
  box.extend(points_(idx.k_+1,idx.j_+1,idx.i_+1));
  box.extend(epsilon_);
  elem_grid_->remove(idx, box);
}

template <class Basis>
void
StructHexVolMesh<Basis>::insert_node_into_grid(typename LatVolMesh<Basis>::Node::index_type ni)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.
  node_grid_->insert(ni,points_[ni]);
}


template <class Basis>
void
StructHexVolMesh<Basis>::remove_node_from_grid(typename LatVolMesh<Basis>::Node::index_type ni)
{
  node_grid_->remove(ni,points_[ni]);
}

template <class Basis>
void
StructHexVolMesh<Basis>::compute_elem_grid(Core::Geometry::BBox& bb)
{
  if (bb.valid())
  {
    // Cubed root of number of cells to get a subdivision ballpark.

    const size_type esz = (this->ni_-1)*(this->nj_-1)*(this->nk_-1);

    const size_type s =
      3*static_cast<size_type>((ceil(pow(static_cast<double>(esz) , (1.0/3.0))))/2.0 + 1.0);

    Core::Geometry::Vector diag  = bb.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(diag.z()/trace*s));

    Core::Geometry::BBox b = bb; b.extend(10*epsilon_);
    elem_grid_.reset(new SearchGridT<typename LatVolMesh<Basis>::Elem::index_type>(sx, sy, sz, b.get_min(), b.get_max()));

    typename LatVolMesh<Basis>::Elem::iterator ci, cie;
    this->begin(ci);
    this->end(cie);
    while(ci != cie)
    {
      insert_elem_into_grid(*ci);
      ++ci;
    }
  }

  synchronized_ |= Mesh::ELEM_LOCATE_E;
}

template <class Basis>
void
StructHexVolMesh<Basis>::compute_node_grid(Core::Geometry::BBox& bb)
{
  if (bb.valid())
  {
    // Cubed root of number of cells to get a subdivision ballpark.

    const size_type esz = (this->ni_)*(this->nj_)*(this->nk_);

    const size_type s =  3*static_cast<size_type>
                  ((ceil(pow(static_cast<double>(esz) , (1.0/3.0))))/2.0 + 1.0);

    Core::Geometry::Vector diag  = bb.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(diag.z()/trace*s));

    Core::Geometry::BBox b = bb; b.extend(10*epsilon_);
    node_grid_.reset(new SearchGridT<typename LatVolMesh<Basis>::Node::index_type>(sx, sy, sz, b.get_min(), b.get_max()));

    typename LatVolMesh<Basis>::Node::iterator ni, nie;
    this->begin(ni);
    this->end(nie);
    while(ni != nie)
    {
      insert_node_into_grid(*ni);
      ++ni;
    }
  }

  synchronized_ |= Mesh::NODE_LOCATE_E;
}



template <class Basis>
void
StructHexVolMesh<Basis>::compute_epsilon(Core::Geometry::BBox& bb)
{
  epsilon_ = bb.diagonal().length()*1e-8;
  epsilon2_ = epsilon_*epsilon_;
  epsilon3_ = epsilon_*epsilon_*epsilon_;
  synchronized_ |= Mesh::EPSILON_E;
}

#define STRUCT_HEX_VOL_MESH_VERSION 2

template <class Basis>
void
StructHexVolMesh<Basis>::io(Piostream& stream)
{
  int version =  stream.begin_class(type_name(-1), STRUCT_HEX_VOL_MESH_VERSION);
  LatVolMesh<Basis>::io(stream);
  /// IO data members, in order

  if (version == 1)
  {
    /// The dimensions of this array were swapped
    Array3<Core::Geometry::Point> tpoints;
    Pio(stream, tpoints);

    size_type dim1 = tpoints.dim1();
    size_type dim2 = tpoints.dim2();
    size_type dim3 = tpoints.dim3();

    points_.resize(dim3,dim2,dim1);
    for (size_type i=0; i<dim1; i++)
      for (size_type j=0; j<dim2; j++)
        for (size_type k=0; k<dim3; k++)
           points_(k,j,i) = tpoints(i,j,k);
  }
  else
  {
    Pio(stream, points_);
  }
  stream.end_class();

  if (stream.reading())
    this->vmesh_.reset(CreateVStructHexVolMesh(this));
}

template <class Basis>
const std::string
StructHexVolMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("StructHexVolMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}

template <class Basis>
const TypeDescription*
get_type_description(StructHexVolMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("StructHexVolMesh", subs,
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructHexVolMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((StructHexVolMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
StructHexVolMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructHexVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Node",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructHexVolMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructHexVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Edge",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructHexVolMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructHexVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Face",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructHexVolMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructHexVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Cell",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

} /// namespace SCIRun

#endif /// SCI_project_StructHexVolMesh_h
