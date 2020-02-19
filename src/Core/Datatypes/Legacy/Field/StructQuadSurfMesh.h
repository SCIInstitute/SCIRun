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
///  geometry.  The grid may have any shape but can not be overlapping or
///  self-intersecting. The topology of a structured grid is represented
///  using a 2D, or 3D vector with the points being stored in an index
///  based array. The faces (quadrilaterals) and  cells (Hexahedron) are
///  implicitly define based based upon their indexing.
///
///  Structured grids are typically used in finite difference analysis.
///
///  For more information on datatypes see Schroeder, Martin, and Lorensen,
///  "The Visualization Toolkit", Prentice Hall, 1998.
///

#ifndef CORE_DATATYPES_STRUCTQUADSURFMESH_H
#define CORE_DATATYPES_STRUCTQUADSURFMESH_H 1

/// Include what kind of support we want to have
/// Need to fix this and couple it to sci-defs
#include <Core/Datatypes/Legacy/Field/MeshSupport.h>

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Plane.h>
#include <Core/GeometryPrimitives/CompGeom.h>

#include <Core/Containers/Array2.h>
#include <Core/GeometryPrimitives/SearchGridT.h>
#include <Core/Thread/Mutex.h>

#include <Core/Datatypes/Legacy/Field/ImageMesh.h>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

/// Declarations for virtual interface

/// Functions for creating the virtual interface
/// Declare the functions that instantiate the virtual interface
template <class Basis>
class StructQuadSurfMesh;

/// make sure any other mesh other than the preinstantiate ones
/// returns no virtual interface. Altering this behavior will allow
/// for dynamically compiling the interface if needed.
template<class MESH>
VMesh* CreateVStructQuadSurfMesh(MESH* mesh) { return (0); }

/// These declarations are needed for a combined dynamic compilation as
/// as well as virtual functions solution.
/// Declare that these can be found in a library that is already
/// precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_STRUCTQUADSURF_SUPPORT > 0)

SCISHARE VMesh* CreateVStructQuadSurfMesh(StructQuadSurfMesh<Core::Basis::QuadBilinearLgn<Core::Geometry::Point> >* mesh);

#endif


template <class Basis>
class StructQuadSurfMesh : public ImageMesh<Basis>
{

/// Make sure the virtual interface has access
template <class MESH> friend class VStructQuadSurfMesh;

public:
  /// Types that change depending on 32 or 64bits
  typedef SCIRun::index_type                 under_type;
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::size_type                  size_type;
  typedef SCIRun::mask_type                  mask_type;

  typedef boost::shared_ptr<StructQuadSurfMesh<Basis> > handle_type;

  StructQuadSurfMesh();
  StructQuadSurfMesh(size_type x, size_type y);
  StructQuadSurfMesh(const StructQuadSurfMesh<Basis> &copy);
  virtual StructQuadSurfMesh *clone() const
  { return new StructQuadSurfMesh<Basis>(*this); }
  virtual ~StructQuadSurfMesh()
  {
    DEBUG_DESTRUCTOR("StructQuadSurfMesh")
  }

  /// get the mesh statistics
  virtual Core::Geometry::BBox get_bounding_box() const;
  virtual void transform(const Core::Geometry::Transform &t);

  virtual bool get_dim(std::vector<size_type>&) const;
  virtual void set_dim(std::vector<size_type> dims) {
    ImageMesh<Basis>::ni_ = dims[0];
    ImageMesh<Basis>::nj_ = dims[1];

    points_.resize(dims[1], dims[0]);
    normals_.resize(dims[1], dims[0]);

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    ImageMesh<Basis>::vmesh_.reset(CreateVStructQuadSurfMesh(this));
  }

  virtual int topology_geometry() const
  {
    return (Mesh::STRUCTURED | Mesh::IRREGULAR);
  }

  /// Get the size of an element (length, area, volume)
  double get_size(const typename ImageMesh<Basis>::Node::index_type &) const
  { return 0.0; }
  double get_size(typename ImageMesh<Basis>::Edge::index_type idx) const
  {
    typename ImageMesh<Basis>::Node::array_type arr;
    this->get_nodes(arr, idx);
    Core::Geometry::Point p0, p1;
    get_center(p0, arr[0]);
    get_center(p1, arr[1]);
    return (p1 - p0).length();
  }
  double get_size(const typename ImageMesh<Basis>::Face::index_type &idx) const
  {
    /// Sum the sizes of the two triangles.
    const Core::Geometry::Point &p0 = points_(idx.j_ + 0, idx.i_ + 0);
    const Core::Geometry::Point &p1 = points_(idx.j_ + 0, idx.i_ + 1);
    const Core::Geometry::Point &p2 = points_(idx.j_ + 1, idx.i_ + 1);
    const Core::Geometry::Point &p3 = points_(idx.j_ + 1, idx.i_ + 0);
    const double a0 = Cross(p0 - p1, p2 - p0).length();
    const double a1 = Cross(p2 - p3, p0 - p2).length();
    return (a0 + a1) * 0.5;
  }

  double get_size(typename ImageMesh<Basis>::Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }
  double get_length(typename ImageMesh<Basis>::Edge::index_type idx) const
  { return get_size(idx); }
  double get_area(const typename ImageMesh<Basis>::Face::index_type &idx) const
  { return get_size(idx); }
  double get_volume(typename ImageMesh<Basis>::Cell::index_type /*idx*/) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  void get_normal(Core::Geometry::Vector &,
                  const typename ImageMesh<Basis>::Node::index_type &) const;

  void get_normal(Core::Geometry::Vector &result, std::vector<double> &coords,
                  typename ImageMesh<Basis>::Elem::index_type eidx,
                  unsigned int)
  {
    ElemData ed(*this, eidx);
    std::vector<Core::Geometry::Point> Jv;
    this->basis_.derivate(coords, ed, Jv);
    result = Cross(Core::Geometry::Vector(Jv[0]), Core::Geometry::Vector(Jv[1]));
    result.normalize();
  }
  /// get the center point (in object space) of an element
  void get_center(Core::Geometry::Point &,
                  const typename ImageMesh<Basis>::Node::index_type &) const;
  void get_center(Core::Geometry::Point &,
                  typename ImageMesh<Basis>::Edge::index_type) const;
  void get_center(Core::Geometry::Point &,
                  const typename ImageMesh<Basis>::Face::index_type &) const;
  void get_center(Core::Geometry::Point &,
                  typename ImageMesh<Basis>::Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  bool locate(typename ImageMesh<Basis>::Node::index_type &node,
              const Core::Geometry::Point &p) const
    { return (locate_node(node,p)); }
  bool locate(typename ImageMesh<Basis>::Edge::index_type &edge,
              const Core::Geometry::Point &p) const
    { return (locate_edge(edge,p)); }
  bool locate(typename ImageMesh<Basis>::Face::index_type &face,
              const Core::Geometry::Point &p ) const
    { return (locate_elem(face,p)); }
  bool locate(typename ImageMesh<Basis>::Cell::index_type &,
              const Core::Geometry::Point &) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  template <class ARRAY>
  bool locate(typename ImageMesh<Basis>::Elem::index_type &elem,
              ARRAY& coords,
              const Core::Geometry::Point &p ) const
    { return (locate_elem(elem,coords,p)); }


  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         typename ImageMesh<Basis>::Node::index_type &node,
                         const Core::Geometry::Point &p) const
    {  return (find_closest_node(pdist,result,node,p,-1.0)); }

  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         typename ImageMesh<Basis>::Node::index_type &node,
                         const Core::Geometry::Point &p, double maxdist) const;

  bool find_closest_elem(double& pdist, Core::Geometry::Point &result,
                         typename ImageMesh<Basis>::Elem::index_type &elem,
                         const Core::Geometry::Point &p) const;

  bool find_closest_elems(double& pdist, Core::Geometry::Point &result,
                          std::vector<typename ImageMesh<Basis>::Elem::index_type> &elems,
                          const Core::Geometry::Point &p) const;

  int get_weights(const Core::Geometry::Point &p,
                  typename ImageMesh<Basis>::Node::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point & ,
                  typename ImageMesh<Basis>::Edge::array_type & , double * )
  { ASSERTFAIL("StructQuadSurfMesh::get_weights for edges isn't supported"); }
  int get_weights(const Core::Geometry::Point &p,
                  typename ImageMesh<Basis>::Face::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point & ,
                  typename ImageMesh<Basis>::Cell::array_type & , double * )
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }


  bool inside3_p(typename ImageMesh<Basis>::Face::index_type i,
                 const Core::Geometry::Point &p) const;


  void get_point(Core::Geometry::Point &point,
                 const typename ImageMesh<Basis>::Node::index_type &index) const
  { get_center(point, index); }
  void set_point(const Core::Geometry::Point &point,
                 const typename ImageMesh<Basis>::Node::index_type &index);

  void get_random_point(Core::Geometry::Point &,
                        const typename ImageMesh<Basis>::Elem::index_type &,
                        FieldRNG &rng) const;

  class ElemData
  {
  public:

    ElemData(const StructQuadSurfMesh<Basis>& msh,
             const typename ImageMesh<Basis>::Elem::index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    /// the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const
    {
      return (index_.i_ + mesh_.get_ni()*index_.j_);
    }
    inline
    index_type node1_index() const
    {
      return (index_.i_+ 1 + mesh_.get_ni()*index_.j_);
    }
    inline
    index_type node2_index() const
    {
      return (index_.i_ + 1 + mesh_.get_ni()*(index_.j_ + 1));

    }
    inline
    index_type node3_index() const
    {
      return (index_.i_ + mesh_.get_ni()*(index_.j_ + 1));
    }

    /// the following designed to coordinate with ::get_edges
    inline
    index_type edge0_index() const
    {
      return index_.i_ + index_.j_ * (mesh_.ni_- 1);
    }
    inline
    index_type edge1_index() const
    {
      return index_.i_ + (index_.j_ + 1) * (mesh_.ni_ - 1);
    }
    inline
    index_type edge2_index() const
    {
      return index_.i_    *(mesh_.nj_ - 1) + index_.j_ +
        ((mesh_.ni_ - 1) * mesh_.nj_);
     }
    inline
    index_type edge3_index() const
    {
      return (index_.i_ + 1) * (mesh_.nj_ - 1) + index_.j_ +
        ((mesh_.ni_ - 1) * mesh_.nj_);
    }


    inline
    const Core::Geometry::Point &node0() const
    {
      return mesh_.points_(index_.j_, index_.i_);
    }
    inline
    const Core::Geometry::Point &node1() const
    {
      return mesh_.points_(index_.j_, index_.i_+1);
    }
    inline
    const Core::Geometry::Point &node2() const
    {
      return mesh_.points_(index_.j_+1, index_.i_+1);
    }
    inline
    const Core::Geometry::Point &node3() const
    {
      return mesh_.points_(index_.j_+1, index_.i_);
    }
    inline
    const Core::Geometry::Point &node4() const
    {
      return mesh_.points_(index_.j_, index_.i_);
    }

  private:
    const StructQuadSurfMesh<Basis>                    &mesh_;
    const typename ImageMesh<Basis>::Elem::index_type  index_;
  };

  friend class ElemData;

  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an edge.
  void pwl_approx_edge(std::vector<std::vector<double> > &coords,
                       typename ImageMesh<Basis>::Elem::index_type /*ci*/,
                       unsigned which_edge,
                       unsigned div_per_unit) const
  {
    /// Needs to match unit_edges in Basis/QuadBilinearLgn.cc compare
    /// get_nodes order to the basis order
    int emap[] = {0, 2, 3, 1};
    this->basis_.approx_edge(emap[which_edge], div_per_unit, coords);
  }

  /// Get the local coordinates for a certain point within an element
  /// This function uses a couple of newton iterations to find the
  /// local ! coordinate of a point
  template<class VECTOR>
  bool get_coords(VECTOR &coords, const Core::Geometry::Point &p,
		  typename ImageMesh<Basis>::Elem::index_type idx) const
  {
    ElemData ed(*this, idx);
    return this->basis_.get_coords(coords, p, ed);
  }

  /// Find the location in the global coordinate system for a local
  /// coordinate ! This function is the opposite of get_coords.
  template<class VECTOR>
  void interpolate(Core::Geometry::Point &pt, const VECTOR &coords,
		   typename ImageMesh<Basis>::Elem::index_type idx) const
  {
    ElemData ed(*this, idx);
    pt = this->basis_.interpolate(coords, ed);
  }

  /// Interpolate the derivate of the function, This infact will
  /// return the ! jacobian of the local to global coordinate
  /// transformation. This function ! is mainly intended for the non
  /// linear elements
  template<class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords,
		typename ImageMesh<Basis>::Elem::index_type idx,
		VECTOR2 &J) const
  {
    ElemData ed(*this, idx);
    this->basis_.derivate(coords, ed, J);
  }

  /// Get the determinant of the jacobian, which is the local volume
  /// of an element ! and is intended to help with the integration of
  /// functions over an element.
  template<class VECTOR>
  double det_jacobian(const VECTOR& coords,
		      typename ImageMesh<Basis>::Elem::index_type idx) const
  {
    double J[9];
    jacobian(coords,idx,J);
    return (DetMatrix3x3(J));
  }

  /// Get the jacobian of the transformation. In case one wants the
  /// non inverted ! version of this matrix. This is currentl here for
  /// completeness of the ! interface
  template<class VECTOR>
  void jacobian(const VECTOR& coords,
		typename ImageMesh<Basis>::Elem::index_type idx,
		double* J) const
  {
    StackVector<Core::Geometry::Point,2> Jv;
    ElemData ed(*this,idx);
    this->basis_.derivate(coords,ed,Jv);
    Core::Geometry::Vector Jv2 = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1]));
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

  /// Get the inverse jacobian of the transformation. This one is
  /// needed to ! translate local gradients into global gradients. Hence
  /// it is crucial for ! calculating gradients of fields, or
  /// constructing finite elements.
  template<class VECTOR>
  double inverse_jacobian(const VECTOR& coords,
			  typename ImageMesh<Basis>::Elem::index_type idx,
			  double* Ji) const
  {
    StackVector<Core::Geometry::Point,2> Jv;
    ElemData ed(*this,idx);
    this->basis_.derivate(coords,ed,Jv);
    double J[9];
    Core::Geometry::Vector Jv2 = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1]));
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


  template<class INDEX>
  double scaled_jacobian_metric(const INDEX idx) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    this->basis_.derivate(this->basis_.unit_center,ed,Jv);
    Jv.resize(3);
    Core::Geometry::Vector v = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1])); v.normalize();
    Jv[2] = v.asPoint();
    double min_jacobian = ScaledDetMatrix3P(Jv);

    size_t num_vertices = this->basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      this->basis_.derivate(this->basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3);
      v = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1])); v.normalize();
      Jv[2] = v.asPoint();
      temp = ScaledDetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }

    return (min_jacobian);
  }


  template<class INDEX>
  double jacobian_metric(const INDEX idx) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    this->basis_.derivate(this->basis_.unit_center,ed,Jv);
    Jv.resize(3);
    Core::Geometry::Vector v = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1])); v.normalize();
    Jv[2] = v.asPoint();
    double min_jacobian = DetMatrix3P(Jv);

    size_t num_vertices = this->basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      this->basis_.derivate(this->basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3);
      v = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1])); v.normalize();
      Jv[2] = v.asPoint();
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }

    return (min_jacobian);
  }


  template <class INDEX>
  bool inside(INDEX idx, const Core::Geometry::Point &p) const
  {
    const Core::Geometry::Point &p0 = points_(idx.j_,idx.i_);
    const Core::Geometry::Point &p1 = points_(idx.j_+1,idx.i_);
    const Core::Geometry::Point &p2 = points_(idx.j_+1,idx.i_+1);
    const Core::Geometry::Point &p3 = points_(idx.j_,idx.i_+1);
    const double x0 = p0.x();
    const double y0 = p0.y();
    const double z0 = p0.z();
    const double x1 = p1.x();
    const double y1 = p1.y();
    const double z1 = p1.z();
    const double x2 = p2.x();
    const double y2 = p2.y();
    const double z2 = p2.z();
    const double x3 = p3.x();
    const double y3 = p3.y();
    const double z3 = p3.z();

    const double a0 = + x1*(y2*z3-y3*z2) + x2*(y3*z1-y1*z3) + x3*(y1*z2-y2*z1);
    const double a1 = - x2*(y3*z0-y0*z3) - x3*(y0*z2-y2*z0) - x0*(y2*z3-y3*z2);
    const double a2 = + x3*(y0*z1-y1*z0) + x0*(y1*z3-y3*z1) + x1*(y3*z0-y0*z3);
    const double a3 = - x0*(y1*z2-y2*z1) - x1*(y2*z0-y0*z2) - x2*(y0*z1-y1*z0);
    const double iV6 = 1.0 / (a0+a1+a2+a3);

    const double b0 = - (y2*z3-y3*z2) - (y3*z1-y1*z3) - (y1*z2-y2*z1);
    const double c0 = + (x2*z3-x3*z2) + (x3*z1-x1*z3) + (x1*z2-x2*z1);
    const double d0 = - (x2*y3-x3*y2) - (x3*y1-x1*y3) - (x1*y2-x2*y1);
    const double s0 = iV6 * (a0 + b0*p.x() + c0*p.y() + d0*p.z());
    if (s0 < -1e-8) return (false);

    const double b1 = + (y3*z0-y0*z3) + (y0*z2-y2*z0) + (y2*z3-y3*z2);
    const double c1 = - (x3*z0-x0*z3) - (x0*z2-x2*z0) - (x2*z3-x3*z2);
    const double d1 = + (x3*y0-x0*y3) + (x0*y2-x2*y0) + (x2*y3-x3*y2);
    const double s1 = iV6 * (a1 + b1*p.x() + c1*p.y() + d1*p.z());
    if (s1 < -1e-8) return (false);

    const double b2 = - (y0*z1-y1*z0) - (y1*z3-y3*z1) - (y3*z0-y0*z3);
    const double c2 = + (x0*z1-x1*z0) + (x1*z3-x3*z1) + (x3*z0-x0*z3);
    const double d2 = - (x0*y1-x1*y0) - (x1*y3-x3*y1) - (x3*y0-x0*y3);
    const double s2 = iV6 * (a2 + b2*p.x() + c2*p.y() + d2*p.z());
    if (s2 < -1e-8) return (false);

    const double b3 = +(y1*z2-y2*z1) + (y2*z0-y0*z2) + (y0*z1-y1*z0);
    const double c3 = -(x1*z2-x2*z1) - (x2*z0-x0*z2) - (x0*z1-x1*z0);
    const double d3 = +(x1*y2-x2*y1) + (x2*y0-x0*y2) + (x0*y1-x1*y0);
    const double s3 = iV6 * (a3 + b3*p.x() + c3*p.y() + d3*p.z());
    if (s3 < -1e-8) return (false);

    return (true);
  }


  template <class INDEX>
  inline bool locate_node(INDEX &node, const Core::Geometry::Point &p) const
  {
    /// If there are no nodes we cannot find a closest point
    if (this->ni_ == 0 || this->nj_ == 0) return (false);

    /// Check first guess
    if (node.i_ >= 0 && node.i_ < this->ni_ &&
        node.j_ >= 0 && node.j_ < this->nj_)
    {
      node.mesh_ = this;
      if ((p - points_(node.j_,node.i_)).length2() < epsilon2_) return (true);
    }

    ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
              "QuadSurfMesh::locate_node requires synchronize(NODE_LOCATE_E).")

    // get grid sizes
    const size_type ni = node_grid_->get_ni()-1;
    const size_type nj = node_grid_->get_nj()-1;
    const size_type nk = node_grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, bj, bk, ei, ej, ek;
    node_grid_->unsafe_locate(bi, bj, bk, p);

    // Clamp to closest point on the grid.
    if (bi > ni) bi =ni;
    if (bi < 0) bi = 0;
    if (bj > nj) bj =nj;
    if (bj < 0) bj = 0;
    if (bk > nk) bk =nk;
    if (bk < 0) bk = 0;

    ei = bi;
    ej = bj;
    ek = bk;

    double dmin = DBL_MAX;
    bool found;
    do
    {
      found = true;
      /// This looks incorrect - but it is correct
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
                typename SearchGridT<typename ImageMesh<Basis>::Node::index_type >::iterator it, eit;
                node_grid_->lookup_ijk(it,eit, i, j, k);

                while (it != eit)
                {
                  const Core::Geometry::Point point = points_((*it).j_,(*it).i_);
                  const double dist = (p-point).length2();

                  if (dist < dmin)
                  {
                    node = INDEX(*it);
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

  /// This is currently implemented as an exhaustive search
  template <class INDEX>
  inline bool locate_edge(INDEX &idx, const Core::Geometry::Point &p) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "QuadSurfMesh::locate_node requires synchronize(EDGES_E).")

    typename ImageMesh<Basis>::Edge::iterator bi, ei;
    typename ImageMesh<Basis>::Node::array_type nodes;
    this->begin(bi);
    this->end(ei);
    idx = 0;

    bool found = false;
    double mindist = 0.0;
    while (bi != ei)
    {
      this->get_nodes(nodes,*bi);
      const double dist = distance_to_line2(p, points_(nodes[0].j_,nodes[0].i_),
                                            points_(nodes[1].j_,nodes[1].i_),epsilon_);
      if (!found || dist < mindist)
      {
        idx = static_cast<INDEX>(*bi);
        mindist = dist;
        found = true;
      }
      ++bi;
    }
    return (found);
  }


  template <class INDEX>
  inline bool locate_elem(INDEX &elem, const Core::Geometry::Point &p) const
  {
    if (this->basis_.polynomial_order() > 1) return elem_locate(elem, *this, p);

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
              "StructQuadSurfMesh::locate_node requires synchronize(ELEM_LOCATE_E).")

    typename ImageMesh<Basis>::Elem::size_type sz;
    this->size(sz);
    if ((elem > 0)&&(elem < sz))
    {
      elem.mesh_ = this;
      if (inside(elem,p)) return (true);
    }

    if (!elem_grid_) return (false);

    typename SearchGridT<typename ImageMesh<Basis>::Elem::index_type >::iterator it, eit;
    if (elem_grid_->lookup(it,eit, p))
    {
      while (it != eit)
      {
        if (inside(typename ImageMesh<Basis>::Elem::index_type(*it), p))
        {
          elem = static_cast<INDEX>(*it);
          return (true);
        }
        ++it;
      }
    }
    return (false);
  }


  template <class INDEX, class ARRAY>
  inline bool locate_elem(INDEX &elem, ARRAY& coords, const Core::Geometry::Point &p) const
  {
    if (this->basis_.polynomial_order() > 1) return elem_locate(elem, *this, p);

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
              "StructQuadSurfMesh::locate_node requires synchronize(ELEM_LOCATE_E).")

    typename ImageMesh<Basis>::Elem::size_type sz;
    this->size(sz);
    if ((elem > 0)&&(elem < sz))
    {
      elem.mesh_ = this;
      if (inside(elem,p))
      {
        ElemData ed(*this,elem);
        this->basis_.get_coords(coords,p,ed);
        return (true);
      }
    }

    if (!elem_grid_) return (false);

    typename SearchGridT<typename ImageMesh<Basis>::Elem::index_type >::iterator it, eit;
    if (elem_grid_->lookup(it,eit, p))
    {
      while (it != eit)
      {
        if (inside(typename ImageMesh<Basis>::Elem::index_type(*it), p))
        {
          elem = static_cast<INDEX>(*it);
          ElemData ed(*this,elem);
          this->basis_.get_coords(coords,p,ed);
          return (true);
        }
        ++it;
      }
    }
    return (false);
  }

  template <class ARRAY>
  bool find_closest_elem(double& pdist, Core::Geometry::Point &result, ARRAY& coords,
    typename ImageMesh<Basis>::Elem::index_type &elem, const Core::Geometry::Point &p) const
  {
    return (find_closest_elem(pdist,result,coords,elem,p,-1.0));
  }

  template <class ARRAY>
  bool find_closest_elem(double& pdist, Core::Geometry::Point &result, ARRAY& coords,
    typename ImageMesh<Basis>::Elem::index_type &elem,
    const Core::Geometry::Point &p, double maxdist) const
  {
    /// If there are no nodes we cannot find the closest one
    if (this->ni_ < 2 || this->nj_ < 2) return (false);

    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;

    /// Test the one in face that is an initial guess
    if (elem.i_ >= 0 && elem.i_ < (this->ni_-1) &&
        elem.j_ >= 0 && elem.j_ < (this->nj_-1))
    {
      elem.mesh_ = this;
      typename ImageMesh<Basis>::Node::array_type nodes;

      this->get_nodes(nodes,elem);
      est_closest_point_on_quad(result, p,
                           points_(nodes[0].j_,nodes[0].i_),
                           points_(nodes[1].j_,nodes[1].i_),
                           points_(nodes[2].j_,nodes[2].i_),
                           points_(nodes[3].j_,nodes[3].i_));

      double dist = (p-result).length2();
      if ( dist < epsilon2_ )
      {
        /// As we computed an estimate, we use the Newton's method in the basis functions
        /// compute a more refined solution. This function may slow down computation.
        /// This piece of code will calculate the coordinates in the local element framework
        /// (the newton's method of finding a minimum), then it will project this back
        /// THIS CODE SHOULD BE FURTHER OPTIMIZED

        ElemData ed(*this,elem);
        this->basis_.get_coords(coords,result,ed);

        result = this->basis_.interpolate(coords,ed);
        pdist = sqrt((result-p).length2());
        return (true);
      }
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
              "StructQuadSurfMesh::find_closest_elem requires synchronize(ELEM_LOCATE_E).")

    // get grid sizes
    const size_type ni = elem_grid_->get_ni()-1;
    const size_type nj = elem_grid_->get_nj()-1;
    const size_type nk = elem_grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, bj, bk, ei, ej, ek;
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
    bool found;
    bool found_one = false;

    typename ImageMesh<Basis>::Node::array_type nodes;
    typename ImageMesh<Basis>::Elem::index_type idx;

    do
    {
      found = true;
      /// This looks incorrect - but it is correct
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
                typename SearchGridT<typename ImageMesh<Basis>::Elem::index_type>::iterator it, eit;
                elem_grid_->lookup_ijk(it, eit, i, j, k);

                while (it != eit)
                {
                  Core::Geometry::Point r;
                  this->get_nodes(nodes,*it);
                  est_closest_point_on_quad(r, p,
                                       points_(nodes[0].j_,nodes[0].i_),
                                       points_(nodes[1].j_,nodes[1].i_),
                                       points_(nodes[2].j_,nodes[2].i_),
                                       points_(nodes[3].j_,nodes[3].i_));
                  const double dtmp = (p - r).length2();
                  if (dtmp < dmin)
                  {
                    found_one = true;
                    result = r;
                    idx = *it;
                    dmin = dtmp;

                    if (dmin < epsilon2_)
                    {
                      /// As we computed an estimate, we use the Newton's method in the basis functions
                      /// compute a more refined solution. This function may slow down computation.
                      /// This piece of code will calculate the coordinates in the local element framework
                      /// (the newton's method of finding a minimum), then it will project this back
                      /// THIS CODE SHOULD BE FURTHER OPTIMIZED

                      ElemData ed(*this,idx);
                      this->basis_.get_coords(coords,result,ed);

                      result = this->basis_.interpolate(coords,ed);
                      dmin = (result-p).length2();

                      elem = idx;
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
    while (!found);

    if (!found_one) return (false);

    /// As we computed an estimate, we use the Newton's method in the basis functions
    /// compute a more refined solution. This function may slow down computation.
    /// This piece of code will calculate the coordinates in the local element framework
    /// (the newton's method of finding a minimum), then it will project this back
    /// THIS CODE SHOULD BE FURTHER OPTIMIZED

    ElemData ed(*this,idx);
    this->basis_.get_coords(coords,result,ed);

    result = this->basis_.interpolate(coords,ed);
    dmin = (result-p).length2();

    elem = idx;
    pdist = dmin;
    return (true);
  }



  /// Export this class using the old Pio system
  virtual void io(Piostream&);
  /// These IDs are created as soon as this class will be instantiated
  /// The first one is for Pio and the second for the virtual interface
  /// These are currently different as they serve different needs.  static PersistentTypeID type_idts;
  static PersistentTypeID type_idsqs;
  /// Core functionality for getting the name of a templated mesh class
  static  const std::string type_name(int n = -1);

  virtual bool synchronize(mask_type sync);
  virtual bool unsynchronize(mask_type sync);
  bool clear_synchronization();

  /// Type description, used for finding names of the mesh class for
  /// dynamic compilation purposes. Soem of this should be obsolete
  virtual const TypeDescription *get_type_description() const;
  static const TypeDescription* node_type_description();
  static const TypeDescription* edge_type_description();
  static const TypeDescription* face_type_description();
  static const TypeDescription* cell_type_description();
  static const TypeDescription* elem_type_description()
  { return face_type_description(); }

  /// This function returns a maker for Pio.
  static Persistent *maker() { return new StructQuadSurfMesh<Basis>(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return boost::make_shared<StructQuadSurfMesh<Basis>>(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle structquadsurf_maker(size_type x ,size_type y) { return boost::make_shared<StructQuadSurfMesh<Basis>>(x,y); }

  Array2<Core::Geometry::Point>& get_points() { return (points_); }



  double get_epsilon() const
    { return (epsilon_); }

protected:

  void compute_node_grid(Core::Geometry::BBox& bb);
  void compute_elem_grid(Core::Geometry::BBox& bb);
  void compute_epsilon(Core::Geometry::BBox& bb);

  void compute_normals();

  /// Used to recompute data for individual cells.
  void insert_elem_into_grid(typename ImageMesh<Basis>::Elem::index_type ci);
  void remove_elem_from_grid(typename ImageMesh<Basis>::Elem::index_type ci);

  void insert_node_into_grid(typename ImageMesh<Basis>::Node::index_type ci);
  void remove_node_from_grid(typename ImageMesh<Basis>::Node::index_type ci);

  /// compute_edge_neighbors is not defined anywhere... do don't
  /// declare it...  void compute_edge_neighbors(double err = 1.0e-8);

  const Core::Geometry::Point &point(const typename ImageMesh<Basis>::Node::index_type &idx)
  { return points_(idx.j_, idx.i_); }

  index_type next(index_type i) { return ((i%4)==3) ? (i-3) : (i+1); }
  index_type prev(index_type i) { return ((i%4)==0) ? (i+3) : (i-1); }

  Array2<Core::Geometry::Point>  points_;
  Array2<Core::Geometry::Vector> normals_; /// normalized per node

  boost::shared_ptr<SearchGridT<typename ImageMesh<Basis>::Node::index_type > > node_grid_;
  boost::shared_ptr<SearchGridT<typename ImageMesh<Basis>::Elem::index_type > > elem_grid_;

  mutable Core::Thread::Mutex  synchronize_lock_;
  mask_type      synchronized_;
  double         epsilon_;
  double         epsilon2_;


};


template <class Basis>
PersistentTypeID
StructQuadSurfMesh<Basis>::type_idsqs(StructQuadSurfMesh<Basis>::type_name(-1),
                                   "Mesh", maker);


template <class Basis>
StructQuadSurfMesh<Basis>::StructQuadSurfMesh()
  : synchronize_lock_("StructQuadSurfMesh Normals Lock"),
    synchronized_(Mesh::ALL_ELEMENTS_E),
    epsilon_(0.0),
    epsilon2_(0.0)
{
  DEBUG_CONSTRUCTOR("StructQuadSurfMesh")

  /// Create a new virtual interface for this copy ! all pointers have
  /// changed hence create a new ! virtual interface class
  this->vmesh_.reset(CreateVStructQuadSurfMesh(this));
}


template <class Basis>
StructQuadSurfMesh<Basis>::StructQuadSurfMesh(size_type x, size_type y)
  : ImageMesh<Basis>(x, y, Core::Geometry::Point(0.0, 0.0, 0.0), Core::Geometry::Point(1.0, 1.0, 1.0)),
    points_( y,x),
    normals_( y,x),
    synchronize_lock_("StructQuadSurfMesh Normals Lock"),
    synchronized_(Mesh::ALL_ELEMENTS_E),
    epsilon_(0.0),
    epsilon2_(0.0)
{
  DEBUG_CONSTRUCTOR("StructQuadSurfMesh")

  /// Create a new virtual interface for this copy ! all pointers have
  /// changed hence create a new ! virtual interface class
  this->vmesh_.reset(CreateVStructQuadSurfMesh(this));
}


template <class Basis>
StructQuadSurfMesh<Basis>::StructQuadSurfMesh(const StructQuadSurfMesh &copy)
  : ImageMesh<Basis>(copy),
    synchronize_lock_("StructQuadSurfMesh Normals Lock"),
    synchronized_(Mesh::NODES_E | Mesh::FACES_E | Mesh::CELLS_E)
{
  DEBUG_CONSTRUCTOR("StructQuadSurfMesh")

  copy.synchronize_lock_.lock();

  points_ = copy.points_;

  epsilon_ = copy.epsilon_;
  epsilon2_ = copy.epsilon2_;

  synchronized_ |= copy.synchronized_ & Mesh::EPSILON_E;

  copy.synchronize_lock_.unlock();

  /// Create a new virtual interface for this copy ! all pointers have
  /// changed hence create a new ! virtual interface class
  this->vmesh_.reset(CreateVStructQuadSurfMesh(this));
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::get_dim(std::vector<size_type> &array) const
{
  array.resize(2);
  array.clear();

  array.push_back(this->ni_);
  array.push_back(this->nj_);

  return true;
}


template <class Basis>
Core::Geometry::BBox
StructQuadSurfMesh<Basis>::get_bounding_box() const
{
  Core::Geometry::BBox result;

  typename ImageMesh<Basis>::Node::iterator ni, nie;
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
StructQuadSurfMesh<Basis>::transform(const Core::Geometry::Transform &t)
{
  synchronize_lock_.lock();

  typename ImageMesh<Basis>::Node::iterator i, ie;
  this->begin(i);
  this->end(ie);

  while (i != ie)
  {
    points_((*i).j_,(*i).i_) = t.project(points_((*i).j_,(*i).i_));
    ++i;
  }

  if (node_grid_) { node_grid_->transform(t); }
  if (elem_grid_) { elem_grid_->transform(t); }
  synchronize_lock_.unlock();
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::get_normal(Core::Geometry::Vector &result,
				      const typename ImageMesh<Basis>::Node::index_type &idx ) const
{
  result = normals_(idx.j_, idx.i_);
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::get_center(Core::Geometry::Point &result,
				      const typename ImageMesh<Basis>::Node::index_type &idx) const
{
  result = points_(idx.j_, idx.i_);
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::get_center(Core::Geometry::Point &result,
				      typename ImageMesh<Basis>::Edge::index_type idx) const
{
  typename ImageMesh<Basis>::Node::array_type arr;
  this->get_nodes(arr, idx);
  Core::Geometry::Point p1;
  get_center(result, arr[0]);
  get_center(p1, arr[1]);

  result += p1;
  result *= 0.5;
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::get_center(Core::Geometry::Point &p,
				      const typename ImageMesh<Basis>::Face::index_type &idx) const
{
  typename ImageMesh<Basis>::Node::array_type nodes;
  this->get_nodes(nodes, idx);
  ASSERT(nodes.size() == 4);
  typename ImageMesh<Basis>::Node::array_type::iterator nai = nodes.begin();
  get_point(p, *nai);
  ++nai;
  Core::Geometry::Point pp;
  while (nai != nodes.end())
  {
    get_point(pp, *nai);
    p += pp;
    ++nai;
  }
  p *= (1.0 / 4.0);
}

template <class Basis>
int
StructQuadSurfMesh<Basis>::get_weights(const Core::Geometry::Point &p,
				       typename ImageMesh<Basis>::Face::array_type &l,
                                       double *w)
{
  typename ImageMesh<Basis>::Face::index_type idx;
  if (locate_elem(idx, p))
  {
    l.resize(1);
    l[0] = idx;
    w[0] = 1.0;
    return 1;
  }
  return 0;
}


template <class Basis>
int
StructQuadSurfMesh<Basis>::get_weights(const Core::Geometry::Point &p,
				       typename ImageMesh<Basis>::Node::array_type &l,
                                       double *w)
{
  typename ImageMesh<Basis>::Face::index_type idx;
  if (locate_elem(idx, p))
  {
    this->get_nodes(l,idx);
    std::vector<double> coords(2);
    if (get_coords(coords, p, idx)) {
      this->basis_.get_weights(coords, w);
      return this->basis_.dofs();
    }
  }
  return 0;
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::inside3_p(typename ImageMesh<Basis>::Face::index_type i,
				     const Core::Geometry::Point &p) const
{
  typename ImageMesh<Basis>::Node::array_type nodes;
  this->get_nodes(nodes, i);

  size_type n = nodes.size();

  std::vector<Core::Geometry::Point> pts(n);

  for (index_type i = 0; i < n; i++)
  {
    get_center(pts[i], nodes[i]);
  }

  for (index_type i = 0; i < n; i+=2)
  {
    Core::Geometry::Point p0 = pts[(i+0)%n];
    Core::Geometry::Point p1 = pts[(i+1)%n];
    Core::Geometry::Point p2 = pts[(i+2)%n];

    Core::Geometry::Vector v01(p0-p1);
    Core::Geometry::Vector v02(p0-p2);
    Core::Geometry::Vector v0(p0-p);
    Core::Geometry::Vector v1(p1-p);
    Core::Geometry::Vector v2(p2-p);
    const double a = Cross(v01, v02).length(); /// area of the whole triangle (2x)
    const double a0 = Cross(v1, v2).length();  /// area opposite p0
    const double a1 = Cross(v2, v0).length();  /// area opposite p1
    const double a2 = Cross(v0, v1).length();  /// area opposite p2
    const double s = a0+a1+a2;

    /// If the area of any of the sub triangles is very small then the point
    /// is on the edge of the subtriangle.
    /// @todo : How small is small ???
///     if( a0 < MIN_ELEMENT_VAL ||
///      a1 < MIN_ELEMENT_VAL ||
///      a2 < MIN_ELEMENT_VAL )
///       return true;

    /// For the point to be inside a CONVEX quad it must be inside one
    /// of the four triangles that can be formed by using three of the
    /// quad vertices and the point in question.
    if( std::fabs(s - a) < epsilon2_*epsilon2_ && a > epsilon2_*epsilon2_ )
    {
      return true;
    }
  }
  return false;
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::get_random_point(Core::Geometry::Point &p,
					    const typename ImageMesh<Basis>::Elem::index_type &ei,
                                            FieldRNG &rng) const
{
  const Core::Geometry::Point &a0 = points_(ei.j_ + 0, ei.i_ + 0);
  const Core::Geometry::Point &a1 = points_(ei.j_ + 0, ei.i_ + 1);
  const Core::Geometry::Point &a2 = points_(ei.j_ + 1, ei.i_ + 1);
  const Core::Geometry::Point &a3 = points_(ei.j_ + 1, ei.i_ + 0);

  const double aarea = Cross(a1 - a0, a2 - a0).length();
  const double barea = Cross(a3 - a0, a2 - a0).length();

  /// Fold the quad sample into a triangle.
  double u = rng();
  double v = rng();
  if (u + v > 1.0) { u = 1.0 - u; v = 1.0 - v; }

  if (rng() * (aarea + barea) < aarea)
  {
    p = a0+((a1-a0)*u)+((a2-a0)*v);
  }
  else
  {
    p = a0+((a3-a0)*u)+((a2-a0)*v);
  }
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::set_point(const Core::Geometry::Point &point,
				     const typename ImageMesh<Basis>::Node::index_type &index)
{
  points_(index.j_, index.i_) = point;
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::synchronize(mask_type sync)
{
  synchronize_lock_.lock();
  if (sync & Mesh::NORMALS_E && !(synchronized_ & Mesh::NORMALS_E))
  {
    compute_normals();
    synchronized_ |= Mesh::NORMALS_E;
  }

  if (sync & (Mesh::NODE_LOCATE_E|Mesh::ELEM_LOCATE_E|Mesh::EPSILON_E) &&
      ( !(synchronized_ & Mesh::NODE_LOCATE_E) ||
        !(synchronized_ & Mesh::ELEM_LOCATE_E) ||
        !(synchronized_ & Mesh::EPSILON_E) ))
  {
    /// These computations share the evalution of the bounding box
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
  return(true);
}

template <class Basis>
bool
StructQuadSurfMesh<Basis>::unsynchronize(mask_type /*sync*/)
{
  return (true);
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::clear_synchronization()
{
  synchronize_lock_.lock();
  // Undo marking the synchronization
  synchronized_ = Mesh::NODES_E | Mesh::ELEMS_E | Mesh::FACES_E;

  // Free memory where possible
  node_grid_.reset();
  elem_grid_.reset();

  synchronize_lock_.unlock();
  return (true);
}

template <class Basis>
void
StructQuadSurfMesh<Basis>::insert_elem_into_grid(typename ImageMesh<Basis>::Elem::index_type idx)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.

  Core::Geometry::BBox box;
  box.extend(points_(idx.j_,idx.i_));
  box.extend(points_(idx.j_+1,idx.i_));
  box.extend(points_(idx.j_,idx.i_+1));
  box.extend(points_(idx.j_+1,idx.i_+1));
  box.extend(epsilon_);
  elem_grid_->insert(idx, box);
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::remove_elem_from_grid(typename ImageMesh<Basis>::Elem::index_type idx)
{
  Core::Geometry::BBox box;
  box.extend(points_(idx.j_,idx.i_));
  box.extend(points_(idx.j_+1,idx.i_));
  box.extend(points_(idx.j_,idx.i_+1));
  box.extend(points_(idx.j_+1,idx.i_+1));
  box.extend(epsilon_);
  elem_grid_->remove(idx, box);
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::insert_node_into_grid(typename ImageMesh<Basis>::Node::index_type ni)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.
  node_grid_->insert(ni,points_[ni]);
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::remove_node_from_grid(typename ImageMesh<Basis>::Node::index_type ni)
{
  node_grid_->remove(ni,points_[ni]);
}

template <class Basis>
void
StructQuadSurfMesh<Basis>::compute_node_grid(Core::Geometry::BBox& bb)
{
  if (bb.valid())
  {
    // Cubed root of number of elems to get a subdivision ballpark.

    typename ImageMesh<Basis>::Node::size_type esz;
    this->size(esz);

    const size_type s = 3*static_cast<size_type>
                  ((ceil(pow(static_cast<double>(esz) , (1.0/3.0))))/2.0 + 1.0);

    Core::Geometry::Vector diag  = bb.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(diag.z()/trace*s));

    Core::Geometry::BBox b = bb; b.extend(10*epsilon_);
    node_grid_.reset(new SearchGridT<typename ImageMesh<Basis>::Node::index_type >(sx, sy, sz, b.get_min(), b.get_max()));

    typename ImageMesh<Basis>::Node::iterator ni, nie;
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
StructQuadSurfMesh<Basis>::compute_elem_grid(Core::Geometry::BBox& bb)
{
  if (bb.valid())
  {
    // Cubed root of number of elems to get a subdivision ballpark.

    typename ImageMesh<Basis>::Elem::size_type esz;
    this->size(esz);

    const size_type s = 3*static_cast<size_type>
                  ((ceil(pow(static_cast<double>(esz) , (1.0/3.0))))/2.0 + 1.0);

    Core::Geometry::Vector diag  = bb.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(diag.z()/trace*s));

    Core::Geometry::BBox b = bb; b.extend(10*epsilon_);
    elem_grid_.reset(new SearchGridT<typename ImageMesh<Basis>::Elem::index_type>(sx, sy, sz, b.get_min(), b.get_max()));

    typename ImageMesh<Basis>::Elem::iterator ci, cie;
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
StructQuadSurfMesh<Basis>::compute_epsilon(Core::Geometry::BBox& bb)
{
  epsilon_ =  bb.diagonal().length();
  epsilon2_ = epsilon_*epsilon_;
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::compute_normals()
{
  normals_.resize(points_.dim1(), points_.dim2()); /// 1 per node

  /// build table of faces that touch each node
  Array2< std::vector<typename ImageMesh<Basis>::Face::index_type> >
    node_in_faces(points_.dim1(), points_.dim2());

  /// face normals (not normalized) so that magnitude is also the area.
  Array2<Core::Geometry::Vector> face_normals((points_.dim1()-1),(points_.dim2()-1));

  /// Computing normal per face.
  typename ImageMesh<Basis>::Node::array_type nodes(4);
  typename ImageMesh<Basis>::Face::iterator iter, iter_end;
  this->begin(iter);
  this->end(iter_end);
  while (iter != iter_end)
  {
    this->get_nodes(nodes, *iter);

    Core::Geometry::Point p0, p1, p2, p3;
    get_point(p0, nodes[0]);
    get_point(p1, nodes[1]);
    get_point(p2, nodes[2]);
    get_point(p3, nodes[3]);

    /// build table of faces that touch each node
    node_in_faces(nodes[0].j_,nodes[0].i_).push_back(*iter);
    node_in_faces(nodes[1].j_,nodes[1].i_).push_back(*iter);
    node_in_faces(nodes[2].j_,nodes[2].i_).push_back(*iter);
    node_in_faces(nodes[3].j_,nodes[3].i_).push_back(*iter);

    Core::Geometry::Vector v0 = p1 - p0;
    Core::Geometry::Vector v1 = p2 - p1;
    Core::Geometry::Vector n = Cross(v0, v1);
    face_normals((*iter).j_, (*iter).i_) = n;

    ++iter;
  }

  /// Averaging the normals.
  typename ImageMesh<Basis>::Node::iterator nif_iter, nif_iter_end;
  this->begin( nif_iter );
  this->end( nif_iter_end );

  while (nif_iter != nif_iter_end)
  {
    std::vector<typename ImageMesh<Basis>::Face::index_type> v =
      node_in_faces((*nif_iter).j_, (*nif_iter).i_);
    typename std::vector<typename ImageMesh<Basis>::Face::index_type>::const_iterator
          fiter = v.begin();
    Core::Geometry::Vector ave(0.L,0.L,0.L);
    while(fiter != v.end())
    {
      ave += face_normals((*fiter).j_,(*fiter).i_);
      ++fiter;
    }
    ave.safe_normalize();
    normals_((*nif_iter).j_, (*nif_iter).i_) = ave;
    ++nif_iter;
  }
}


#define STRUCT_QUAD_SURF_MESH_VERSION 3

template <class Basis>
void
StructQuadSurfMesh<Basis>::io(Piostream& stream)
{
  int version =
    stream.begin_class(type_name(-1), STRUCT_QUAD_SURF_MESH_VERSION);
  ImageMesh<Basis>::io(stream);

  if (version ==2)
  {
    /// The dimensions of this array were swapped
    Array2<Core::Geometry::Point> tpoints;
    Pio(stream, tpoints);

    size_t dim1 = tpoints.dim1();
    size_t dim2 = tpoints.dim2();

    points_.resize(dim2,dim1);
    for (size_t i=0; i<dim1; i++)
      for (size_t j=0; j<dim2; j++)
           points_(j,i) = tpoints(i,j);
  }
  else
  {
    Pio(stream, points_);
  }

  stream.end_class();

  if (stream.reading())
    this->vmesh_.reset(CreateVStructQuadSurfMesh(this));
}


template <class Basis>
const std::string
StructQuadSurfMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("StructQuadSurfMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
const TypeDescription*
get_type_description(StructQuadSurfMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("StructQuadSurfMesh", subs,
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructQuadSurfMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((StructQuadSurfMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
StructQuadSurfMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructQuadSurfMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Node",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructQuadSurfMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructQuadSurfMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Edge",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructQuadSurfMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructQuadSurfMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Face",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructQuadSurfMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructQuadSurfMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Cell",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::find_closest_node(double& pdist, Core::Geometry::Point &result,
   typename ImageMesh<Basis>::Node::index_type &node, const Core::Geometry::Point &p,
   double maxdist) const
{
  /// If there are no nodes we cannot find a closest point
  if (this->ni_ == 0 || this->nj_ == 0) return (false);
  if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;

  /// Check first guess
  if (node.i_ >= 0 && node.i_ < this->ni_ &&
      node.j_ >= 0 && node.j_ < this->nj_)
  {
    node.mesh_ = this;
    Core::Geometry::Point point = points_(node.j_,node.i_);
    double dist = (point-p).length2();

    if ( dist < epsilon2_ )
    {
      result = point;
      pdist = sqrt(dist);
      return (true);
    }
  }

  ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
            "StructQuadSurfMesh::find_closest_elem requires synchronize(NODELOCATE_E).")

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

  typename ImageMesh<Basis>::Node::array_type nodes;

  double dmin = maxdist;
  bool found = true;
  bool found_one = false;

  do
  {
    found = true;
    /// This looks incorrect - but it is correct
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
              typename SearchGridT<typename ImageMesh<Basis>::Node::index_type>::iterator it, eit;
              node_grid_->lookup_ijk(it,eit, i, j, k);

              while (it != eit)
              {
                const typename ImageMesh<Basis>::Node::index_type idx = *it;
                const Core::Geometry::Point pnt = points_(idx.j_,idx.i_);
                const double dist = (p-pnt).length2();
                if (dist < dmin)
                {
                  found_one = true;
                  result = pnt;
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
  while (!found) ;

  if (!found_one) return (false);

  pdist = sqrt(dmin);
  return (true);
}


/// This function will find the closest element and the location on that
/// element that is the closest

template <class Basis>
bool
StructQuadSurfMesh<Basis>::find_closest_elem(double& pdist,
                       Core::Geometry::Point &result,
                       typename ImageMesh<Basis>::Elem::index_type &elem,
                       const Core::Geometry::Point &p) const
{
  StackVector<double,2> coords;
  return(find_closest_elem(pdist,result,coords,elem,p,-1.0));
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::find_closest_elems(double& pdist,
  Core::Geometry::Point &result,
  std::vector<typename ImageMesh<Basis>::Elem::index_type> &elems,
  const Core::Geometry::Point &p) const
{
  elems.clear();

  /// If there are no nodes we cannot find the closest one
  if (this->ni_ < 2 || this->nj_ < 2) return (false);

  ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
            "StructQuadSurfMesh::find_closest_elem requires synchronize(ELEM_LOCATE_E).")

  // get grid sizes
  const size_type ni = elem_grid_->get_ni()-1;
  const size_type nj = elem_grid_->get_nj()-1;
  const size_type nk = elem_grid_->get_nk()-1;

  // Convert to grid coordinates.
  index_type bi, bj, bk, ei, ej, ek;
  elem_grid_->unsafe_locate(bi, bj, bk, p);

  // Clamp to closest point on the grid.
  if (bi > ni) bi = ni;
  if (bi < 0) bi = 0;
  if (bj > nj) bj = nj;
  if (bj < 0) bj = 0;
  if (bk > nk) bk = nk;
  if (bk < 0) bk = 0;

  ei = bi; ej = bj; ek = bk;

  double dmin = DBL_MAX;

  typename ImageMesh<Basis>::Node::array_type nodes;
  typename ImageMesh<Basis>::Elem::index_type last_idx;

  bool found = true;
  do
  {
    found = true;
    /// This looks incorrect - but it is correct
    /// We need to do a full shell without any elements that are closer
    /// to make sure there no closer elements
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
              typename SearchGridT<typename ImageMesh<Basis>::Elem::index_type>::iterator it, eit;
              elem_grid_->lookup_ijk(it,eit, i, j, k);

              while (it != eit)
              {
                Core::Geometry::Point r;
                this->get_nodes(nodes,*it);
                est_closest_point_on_quad(r, p,
                                     points_(nodes[0].j_,nodes[0].i_),
                                     points_(nodes[1].j_,nodes[1].i_),
                                     points_(nodes[2].j_,nodes[2].i_),
                                     points_(nodes[3].j_,nodes[3].i_));
                const double dtmp = (p - r).length2();

                if (dtmp < dmin - epsilon2_)
                {
                  elems.clear();
                  result = r;
                  last_idx = *it;
                  elems.push_back(*it);
                  dmin = dtmp;
                }
                else if (dtmp < dmin + epsilon2_)
                {
                  elems.push_back(*it);
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


  // As we computed an estimate, we use the Newton's method in the basis functions
  // compute a more refined solution. This function may slow down computation.
  // This piece of code will calculate the coordinates in the local element framework
  // (the newton's method of finding a minimum), then it will project this back
  // THIS CODE SHOULD BE FURTHER OPTIMIZED

  if (elems.size() == 1)
  {
    // if the number of faces is more then one the point we found is located
    // on the node or on the edge, which means the estimate is correct.
    StackVector<double,3> coords;
    ElemData ed(*this,last_idx);
    this->basis_.get_coords(coords,result,ed);
    result = this->basis_.interpolate(coords,ed);
    dmin = (result-p).length2();
  }

  pdist = sqrt(dmin);
  return (true);
}


} /// namespace SCIRun

#endif /// SCI_project_StructQuadSurfMesh_h
