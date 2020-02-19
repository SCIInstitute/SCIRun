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
///  A structured curve is a dataset with regular topology but with
///  irregular geometry.  The line defined may have any shape but can not
///  be overlapping or self-intersecting.
///
///  The topology of structured curve is represented using a 1D vector with
///  the points being stored in an index based array. The ordering of the curve
///  is implicity defined based based upon its indexing.
///
///  For more information on datatypes see Schroeder, Martin, and Lorensen,
///  "The Visualization Toolkit", Prentice Hall, 1998.

#ifndef CORE_DATATYPES_STRUCTCURVEMESH_H
#define CORE_DATATYPES_STRUCTCURVEMESH_H 1

/// Include what kind of support we want to have
/// Need to fix this and couple it to sci-defs
#include <Core/Datatypes/Legacy/Field/MeshSupport.h>

#include <Core/Persistent/PersistentSTL.h>
#include <Core/Thread/Mutex.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Plane.h>
#include <Core/GeometryPrimitives/CompGeom.h>

#include <Core/Containers/Array2.h>
#include <Core/Datatypes/Legacy/Field/ScanlineMesh.h>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

/// Declarations for virtual interface

/// Functions for creating the virtual interface
/// Declare the functions that instantiate the virtual interface
template <class Basis>
class StructCurveMesh;

/// make sure any other mesh other than the preinstantiate ones
/// returns no virtual interface. Altering this behavior will allow
/// for dynamically compiling the interface if needed.
template<class MESH>
VMesh* CreateVStructCurveMesh(MESH* mesh) { return (0); }

/// These declarations are needed for a combined dynamic compilation as
/// as well as virtual functions solution.
/// Declare that these can be found in a library that is already
/// precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_STRUCTCURVE_SUPPORT > 0)

SCISHARE VMesh* CreateVStructCurveMesh(StructCurveMesh<Core::Basis::CrvLinearLgn<Core::Geometry::Point> >* mesh);

#endif


template <class Basis>
class StructCurveMesh : public ScanlineMesh<Basis>
{

/// Make sure the virtual interface has access
template <class MESH> friend class VStructCurveMesh;

public:
  /// Types that change depending on 32 or 64bits
  typedef SCIRun::index_type            under_type;
  typedef SCIRun::index_type            index_type;
  typedef SCIRun::size_type             size_type;
  typedef SCIRun::mask_type             mask_type;

  StructCurveMesh();
  explicit StructCurveMesh(size_type n);
  StructCurveMesh(const StructCurveMesh &copy);
  virtual StructCurveMesh *clone() const { return new StructCurveMesh(*this); }
  virtual ~StructCurveMesh()
  {
    DEBUG_DESTRUCTOR("StructCurveMesh")
  }

  /// get the mesh statistics
  double get_cord_length() const;
  virtual Core::Geometry::BBox get_bounding_box() const;
  virtual void transform(const Core::Geometry::Transform &t);

  virtual bool get_dim(std::vector<size_type>&) const;
  virtual void set_dim(std::vector<size_type> dims) {
    ScanlineMesh<Basis>::ni_ = dims[0];

    points_.resize(dims[0]);

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    ScanlineMesh<Basis>::vmesh_.reset(CreateVStructCurveMesh(this));
  }

  virtual int topology_geometry() const
  {
    return (Mesh::STRUCTURED | Mesh::IRREGULAR);
  }

  /// get the child elements of the given index
  void get_nodes(typename ScanlineMesh<Basis>::Node::array_type &,
                 typename ScanlineMesh<Basis>::Edge::index_type) const;
  void get_nodes(typename ScanlineMesh<Basis>::Node::array_type &,
                 typename ScanlineMesh<Basis>::Face::index_type) const {}
  void get_nodes(typename ScanlineMesh<Basis>::Node::array_type &,
                 typename ScanlineMesh<Basis>::Cell::index_type) const {}
  void get_edges(typename ScanlineMesh<Basis>::Edge::array_type &,
                 typename ScanlineMesh<Basis>::Face::index_type) const {}
  void get_edges(typename ScanlineMesh<Basis>::Edge::array_type &,
                 typename ScanlineMesh<Basis>::Cell::index_type) const {}
  void get_edges(typename ScanlineMesh<Basis>::Edge::array_type &a,
                 typename ScanlineMesh<Basis>::Edge::index_type idx) const
  { a.push_back(idx);}

  /// Get the size of an elemnt (length, area, volume)
  double get_size(typename ScanlineMesh<Basis>::Node::index_type) const
  { return 0.0; }
  double get_size(typename ScanlineMesh<Basis>::Edge::index_type idx) const
  {
    typename ScanlineMesh<Basis>::Node::array_type arr;
    get_nodes(arr, idx);
    Core::Geometry::Point p0, p1;
    get_center(p0, arr[0]);
    get_center(p1, arr[1]);
    return Core::Geometry::Vector(p1 - p0).length();
  }

  double get_size(typename ScanlineMesh<Basis>::Face::index_type) const
  { return 0.0; }
  double get_size(typename ScanlineMesh<Basis>::Cell::index_type) const
  { return 0.0; }
  double get_length(typename ScanlineMesh<Basis>::Edge::index_type idx) const
  { return get_size(idx); }
  double get_area(typename ScanlineMesh<Basis>::Face::index_type idx) const
  { return get_size(idx); }
  double get_volume(typename ScanlineMesh<Basis>::Cell::index_type idx) const
  { return get_size(idx); }

  int get_valence(typename ScanlineMesh<Basis>::Node::index_type idx) const
  { return (idx == 0 || idx == static_cast<index_type>((points_.size()-1)) ? 1 : 2); }

  int get_valence(typename ScanlineMesh<Basis>::Edge::index_type) const
  { return 0; }
  int get_valence(typename ScanlineMesh<Basis>::Face::index_type) const
  { return 0; }
  int get_valence(typename ScanlineMesh<Basis>::Cell::index_type) const
  { return 0; }

  /// get the center point (in object space) of an element
  void get_center(Core::Geometry::Point &,
                  const typename ScanlineMesh<Basis>::Node::index_type&) const;
  void get_center(Core::Geometry::Point &,
                  const typename ScanlineMesh<Basis>::Edge::index_type&) const;
  void get_center(Core::Geometry::Point &,
                  const typename ScanlineMesh<Basis>::Face::index_type&) const
  {}
  void get_center(Core::Geometry::Point &,
                  const typename ScanlineMesh<Basis>::Cell::index_type&) const
  {}

  bool locate(typename ScanlineMesh<Basis>::Node::index_type &idx,
              const Core::Geometry::Point &point) const
    { return(locate_node(idx,point)); }
  bool locate(typename ScanlineMesh<Basis>::Edge::index_type &idx,
              const Core::Geometry::Point &point) const
    { return(locate_elem(idx,point)); }
  bool locate(typename ScanlineMesh<Basis>::Face::index_type&,
              const Core::Geometry::Point&) const
    { return (false); }
  bool locate(typename ScanlineMesh<Basis>::Cell::index_type&,
              const Core::Geometry::Point&) const
    { return (false); }

  bool locate(typename ScanlineMesh<Basis>::Elem::index_type &idx,
              std::vector<double>& coords,
              const Core::Geometry::Point& point )
    { return(locate_elem(idx,coords,point)); }

  int get_weights(const Core::Geometry::Point &,
                  typename ScanlineMesh<Basis>::Node::array_type &, double *w);
  int get_weights(const Core::Geometry::Point &,
                  typename ScanlineMesh<Basis>::Edge::array_type &, double *w);
  int get_weights(const Core::Geometry::Point &,
                  typename ScanlineMesh<Basis>::Face::array_type &, double *)
    {ASSERTFAIL("StructCurveMesh::get_weights for faces isn't supported"); }
  int get_weights(const Core::Geometry::Point &,
                  typename ScanlineMesh<Basis>::Cell::array_type &, double *)
    {ASSERTFAIL("StructCurveMesh::get_weights for cells isn't supported"); }

  void get_point(Core::Geometry::Point &p, typename ScanlineMesh<Basis>::Node::index_type i) const
  { get_center(p,i); }
  void set_point(const Core::Geometry::Point &p, typename ScanlineMesh<Basis>::Node::index_type i)
  { points_[i] = p; }

  void get_random_point(Core::Geometry::Point &p,
                        const typename ScanlineMesh<Basis>::Elem::index_type idx,
                        FieldRNG &rng) const;

  void get_normal(Core::Geometry::Vector &,
                  typename ScanlineMesh<Basis>::Node::index_type) const
  { ASSERTFAIL("This mesh type does not have node normals."); }

  void get_normal(Core::Geometry::Vector &, std::vector<double> &,
                  typename ScanlineMesh<Basis>::Elem::index_type,
                  unsigned int)
  { ASSERTFAIL("This mesh type does not have element normals."); }

  class ElemData
  {
  public:
    typedef typename StructCurveMesh<Basis>::index_type  index_type;

    ElemData(const StructCurveMesh<Basis>& msh,
             const typename ScanlineMesh<Basis>::Elem::index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    /// the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const
    {
      return (index_);
    }
    inline
    index_type node1_index() const
    {
      return (index_ + 1);
    }


    /// the following designed to coordinate with ::get_edges
    inline
    index_type edge0_index() const
    {
      return index_;
    }

    inline
    const Core::Geometry::Point &node0() const
    {
      return mesh_.points_[index_];
    }
    inline
    const Core::Geometry::Point &node1() const
    {
      return mesh_.points_[index_+1];
    }

  private:
    const StructCurveMesh<Basis>                             &mesh_;
    const typename ScanlineMesh<Basis>::Elem::index_type     index_;
  };

  friend class ElemData;

 /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an edge.
  void pwl_approx_edge(std::vector<std::vector<double> > &coords,
                       typename ScanlineMesh<Basis>::Elem::index_type /*ci*/,
                       unsigned int,
                       unsigned int div_per_unit) const
  {
    /// Needs to match unit_edges in Basis/QuadBilinearLgn.cc
    /// compare get_nodes order to the basis order
    this->basis_.approx_edge(0, div_per_unit, coords);
  }

  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an face.
  void pwl_approx_face(std::vector<std::vector<std::vector<double> > >& /*coords*/,
                       typename ScanlineMesh<Basis>::Elem::index_type /*ci*/,
                       typename ScanlineMesh<Basis>::Face::index_type /*fi*/,
                       unsigned int /*div_per_unit*/) const
  {
    ASSERTFAIL("ScanlineMesh has no faces");
  }


  /// Get the local coordinates for a certain point within an element
  /// This function uses a couple of newton iterations to find the
  /// local ! coordinate of a point
  template<class VECTOR, class INDEX>
  bool get_coords(VECTOR &coords, const Core::Geometry::Point &p, INDEX idx) const
  {
    ElemData ed(*this, typename ScanlineMesh<Basis>::Elem::index_type(idx));
    return this->basis_.get_coords(coords, p, ed);
  }

  /// Find the location in the global coordinate system for a local
  /// coordinate ! This function is the opposite of get_coords.
  template<class VECTOR, class INDEX>
  void interpolate(Core::Geometry::Point &pt, const VECTOR &coords, INDEX idx) const
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
		typename ScanlineMesh<Basis>::Elem::index_type idx,
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
		      typename ScanlineMesh<Basis>::Elem::index_type idx) const
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
		typename ScanlineMesh<Basis>::Elem::index_type idx,
		double* J) const
  {
    StackVector<Core::Geometry::Point,1> Jv;
    ElemData ed(*this,idx);
    this->basis_.derivate(coords,ed,Jv);
    Core::Geometry::Vector Jv1, Jv2;
    Core::Geometry::Vector(Jv[0]).find_orthogonal(Jv1,Jv2);
    J[0] = Jv[0].x();
    J[1] = Jv[0].y();
    J[2] = Jv[0].z();
    J[3] = Jv1.x();
    J[4] = Jv1.y();
    J[5] = Jv1.z();
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
			  typename ScanlineMesh<Basis>::Elem::index_type idx,
			  double* Ji) const
  {
    StackVector<Core::Geometry::Point,1> Jv;
    ElemData ed(*this,idx);
    this->basis_.derivate(coords,ed,Jv);
    double J[9];
    Core::Geometry::Vector Jv1, Jv2;
    Core::Geometry::Vector(Jv[0]).find_orthogonal(Jv1,Jv2);
    J[0] = Jv[0].x();
    J[1] = Jv[0].y();
    J[2] = Jv[0].z();
    J[3] = Jv1.x();
    J[4] = Jv1.y();
    J[5] = Jv1.z();
    J[6] = Jv2.x();
    J[7] = Jv2.y();
    J[8] = Jv2.z();

    return (InverseMatrix3x3(J,Ji));
  }

  template<class INDEX>
  double scaled_jacobian_metric(INDEX idx) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    this->basis_.derivate(this->basis_.unit_center,ed,Jv);
    Jv.resize(3);
    Core::Geometry::Vector v,w;
    Core::Geometry::Vector(Jv[0]).find_orthogonal(v,w);
    Jv[1] = v.asPoint();
    Jv[2] = w.asPoint();
    double min_jacobian = ScaledDetMatrix3P(Jv);

    size_t num_vertices = this->basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      this->basis_.derivate(this->basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3);
      Core::Geometry::Vector(Jv[0]).find_orthogonal(v,w);
      Jv[1] = v.asPoint();
      Jv[2] = w.asPoint();
      temp = ScaledDetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }

    return (min_jacobian);
  }


  template<class INDEX>
  double jacobian_metric(INDEX idx) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    this->basis_.derivate(this->basis_.unit_center,ed,Jv);
    Jv.resize(3);
    Core::Geometry::Vector v,w;
    Core::Geometry::Vector(Jv[0]).find_orthogonal(v,w);
    Jv[1] = v.asPoint();
    Jv[2] = w.asPoint();
    double min_jacobian = DetMatrix3P(Jv);

    size_t num_vertices = this->basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      this->basis_.derivate(this->basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3);
      Core::Geometry::Vector(Jv[0]).find_orthogonal(v,w);
      Jv[1] = v.asPoint();
      Jv[2] = w.asPoint();
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }

    return (min_jacobian);
  }

  template <class INDEX>
  bool locate_node(INDEX &idx, const Core::Geometry::Point &p) const
  {
    /// If there are no nodes, return false, otherwise there will always be
    /// a node that is closest
    if (points_.size() == 0) return (false);

    typename ScanlineMesh<Basis>::Node::iterator ni, nie;
    this->begin(ni);
    this->end(nie);

    double mindist = DBL_MAX;

    while(ni != nie)
    {
      double dist = (p-points_[*ni]).length2();
      if ( dist < mindist )
      {
        mindist = dist;
        idx = static_cast<INDEX>(*ni);

        if (mindist < epsilon2_) return (true);
      }
      ++ni;
    }

    return (true);
  }


  template <class INDEX>
  bool locate_elem(INDEX &idx, const Core::Geometry::Point &p) const
  {
    if (this->basis_.polynomial_order() > 1) return elem_locate(idx, *this, p);

    typename ScanlineMesh<Basis>::Elem::size_type sz;
    this->size(sz);

    /// If there are no elements, one cannot find a point inside
    if (sz == 0) return (false);

    double alpha;

    /// Check whether the estimate given in idx is the point we are looking for
    if (idx >= 0 && idx < sz)
    {
      if (inside2_p(idx,p,alpha)) return (true);
    }

    /// Loop over all nodes to find one that finds
    typename ScanlineMesh<Basis>::Elem::iterator ei;
    this->begin(ei);
    typename ScanlineMesh<Basis>::Elem::iterator eie;
    this->end(eie);

    while (ei != eie)
    {
      if (inside2_p(*ei,p,alpha))
      {
        idx = static_cast<INDEX>(*ei);
        return (true);
      }
      ++ei;
    }

    return (false);
  }

  template <class INDEX, class ARRAY>
  bool locate_elem(INDEX& idx, ARRAY& coords, const Core::Geometry::Point& p) const
  {
    if (this->basis_.polynomial_order() > 1) return elem_locate(idx, *this, p);

    typename ScanlineMesh<Basis>::Elem::size_type sz;
    this->size(sz);

    /// If there are no elements, one cannot find a point inside
    if (sz == 0) return (false);

    coords.resize(1);
    double alpha;

    /// Check whether the estimate given in idx is the point we are looking for
    if (idx >= 0 && idx < sz)
    {
      if (inside2_p(idx,p,coords[0])) return (true);
    }

    /// Loop over all nodes to find one that finds
    typename ScanlineMesh<Basis>::Elem::iterator ei;
    this->begin(ei);
    typename ScanlineMesh<Basis>::Elem::iterator eie;
    this->end(eie);

    while (ei != eie)
    {
      if (inside2_p(*ei,p,alpha))
      {
        coords[0] = alpha;
        idx = static_cast<INDEX>(*ei);
        return (true);
      }
      ++ei;
    }

    return (false);
  }


  template <class INDEX>
  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         INDEX &idx, const Core::Geometry::Point &point) const
  {
    return(find_closest_node(pdist,result,idx,point,-1.0));
  }

  /// Find the closest element to a point
  template <class INDEX>
  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         INDEX &idx, const Core::Geometry::Point &point,
                         double maxdist) const
  {
    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;
    typename ScanlineMesh<Basis>::Node::size_type sz;
    this->size(sz);

    /// No nodes, so none is closest
    if (sz == 0) return (false);

    typename ScanlineMesh<Basis>::Node::iterator ni;
    this->begin(ni);
    typename ScanlineMesh<Basis>::Node::iterator nie;
    this->end(nie);

    Core::Geometry::Point p2;
    double dist;

    /// Check whether first estimate is the one we are looking for
    if (idx >= 0 && idx < sz)
    {
      p2 = points_[*ni];
      dist = (point-p2).length2();

      if ( dist < epsilon2_ )
      {
        result = point;
        pdist= sqrt(dist);
        return (true);
      }
    }

    /// Loop through all nodes to find the one that is closest
    double mindist = maxdist;

    while(ni != nie)
    {
      p2 = points_[*ni];
      dist = (point-p2).length2();

      if ( dist < mindist )
      {
        mindist = dist;
        idx = static_cast<INDEX>(*ni);
        result = p2;
        if (mindist < epsilon2_)
        {
          pdist = sqrt(mindist);
          return (true);
        }
      }
      ++ni;
    }

    if (maxdist == mindist)  return (false);

    pdist = sqrt(mindist);
    return (true);
  }


  /// Find the closest element to a point
  template <class INDEX, class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         ARRAY &coords,
                         INDEX &idx,
                         const Core::Geometry::Point &p) const
  {
    return find_closest_elem(pdist,result,coords,idx,p,-1.0);
  }

  /// Find the closest element to a point
  template <class INDEX, class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         ARRAY &coords,
                         INDEX &idx,
                         const Core::Geometry::Point &p,
                         double maxdist) const
  {
    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;
    typename ScanlineMesh<Basis>::Elem::size_type sz;
    this->size(sz);

    /// No elements, none that can be closest
    if (sz == 0) return (false);

    typename ScanlineMesh<Basis>::Elem::iterator ni;
    this->begin(ni);
    typename ScanlineMesh<Basis>::Elem::iterator nie;
    this->end(nie);

    Core::Geometry::Point p2;
    double dist;

    coords.resize(1);
    /// Check whether first guess is OK
    if (idx >= 0 && idx < sz)
    {
      double alpha;
      dist = distance2_p(idx,p,result,alpha);
      if ( dist < epsilon2_ )
      {
        coords[0] = alpha;
        pdist = sqrt(dist);
        return (true);
      }
    }

    /// Loop through all elements to find closest
    double mindist = maxdist;
    Core::Geometry::Point res;

    while(ni != nie)
    {
      double alpha;
      dist = distance2_p(*ni,p,res,alpha);
      if ( dist < mindist )
      {
        coords[0] = alpha;
        mindist = dist;
        idx = static_cast<INDEX>(*ni);
        result = res;
        if (mindist < epsilon2_)
        {
          pdist = sqrt(mindist);
          return (true);
        }
      }
      ++ni;
    }

    if (mindist == maxdist) return (false);

    pdist = sqrt(mindist);
    return (true);
  }

  template <class INDEX>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         INDEX &elem,
                         const Core::Geometry::Point &p) const
  {
    StackVector<double,1> coords;
    return(find_closest_elem(pdist,result,coords,elem,p,-1.0));
  }

  /// Find the closest elements to a point
  template<class ARRAY>
  bool find_closest_elems(double& pdist, Core::Geometry::Point &result,
                          ARRAY &elems, const Core::Geometry::Point &p) const
  {
    elems.clear();

    typename ScanlineMesh<Basis>::Elem::size_type sz;
    this->size(sz);

    /// No elements, none that is closest
    if (sz == 0) return (false);

    typename ScanlineMesh<Basis>::Elem::iterator ni;
    this->begin(ni);
    typename ScanlineMesh<Basis>::Elem::iterator nie;
    this->end(nie);

    Core::Geometry::Point p2;

    double dist;
    double mindist = DBL_MAX;
    Core::Geometry::Point res;

    while(ni != nie)
    {
      double dummy;
      dist = distance2_p(*ni,p,res,dummy);

      if (dist < mindist - epsilon2_)
      {
        elems.clear();
        result = res;
        elems.push_back(static_cast<typename ARRAY::value_type>(*ni));
        mindist = dist;
      }
      else if (dist < mindist)
      {
        elems.push_back(static_cast<typename ARRAY::value_type>(*ni));
      }
      ++ni;
    }

    pdist = sqrt(mindist);
    return (true);
  }

  /// Export this class using the old Pio system
  virtual void io(Piostream&);
  /// These IDs are created as soon as this class will be instantiated
  /// The first one is for Pio and the second for the virtual
  /// interface ! These are currently different as they serve different
  /// needs.  static PersistentTypeID type_idts;
  static PersistentTypeID scanline_typeid;
  /// Core functionality for getting the name of a templated mesh class
  static  const std::string type_name(int n = -1);

  /// Type description, used for finding names of the mesh class for
  /// dynamic compilation purposes. Soem of this should be obsolete
  virtual const TypeDescription *get_type_description() const;

  /// This function returns a maker for Pio.
  static Persistent *maker() { return new StructCurveMesh<Basis>(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return boost::make_shared<StructCurveMesh<Basis>>();}
  /// This function returns a handle for the virtual interface.
  static MeshHandle structcurve_maker(size_type x) { return boost::make_shared<StructCurveMesh<Basis>>(x);}

  std::vector<Core::Geometry::Point>& get_points() { return (points_); }

  virtual bool synchronize(mask_type sync);
  virtual bool unsynchronize(mask_type sync);
  bool clear_synchronization();

  double get_epsilon() const;

private:

  bool inside2_p(index_type idx, const Core::Geometry::Point &p, double& alpha) const;
  double distance2_p(index_type idx, const Core::Geometry::Point &p,
                     Core::Geometry::Point& projection, double& alpha) const;

  void compute_epsilon();

  std::vector<Core::Geometry::Point> points_;
  mutable Core::Thread::Mutex synchronize_lock_;
  mask_type     synchronized_;
  double        epsilon_;
  double        epsilon2_;
};


template <class Basis>
PersistentTypeID
StructCurveMesh<Basis>::scanline_typeid(StructCurveMesh<Basis>::type_name(-1),
                                "Mesh", maker);


template <class Basis>
StructCurveMesh<Basis>::StructCurveMesh():
  points_(0),
  synchronize_lock_("Synchronize lock"),
  synchronized_(Mesh::ALL_ELEMENTS_E),
  epsilon_(0.0),
  epsilon2_(0.0)
{
  DEBUG_CONSTRUCTOR("StructCurveMesh")

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  this->vmesh_.reset(CreateVStructCurveMesh(this));
}


template <class Basis>
StructCurveMesh<Basis>::StructCurveMesh(size_type n)
  : ScanlineMesh<Basis>(n, Core::Geometry::Point(0.0, 0.0, 0.0), Core::Geometry::Point(1.0, 1.0, 1.0)),
    points_(n),
    synchronize_lock_("StructCurveMesh lock"),
    synchronized_(Mesh::EDGES_E | Mesh::NODES_E),
    epsilon_(0.0),
    epsilon2_(0.0)
{
  DEBUG_CONSTRUCTOR("StructCurveMesh")

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  this->vmesh_.reset(CreateVStructCurveMesh(this));
}


template <class Basis>
StructCurveMesh<Basis>::StructCurveMesh(const StructCurveMesh &copy)
  : ScanlineMesh<Basis>(copy),
    points_(copy.points_),
    synchronize_lock_("StructCurveMesh lock"),
    synchronized_(copy.synchronized_)
{
  DEBUG_CONSTRUCTOR("StructCurveMesh")

  copy.synchronize_lock_.lock();

  synchronized_ |= copy.synchronized_ & Mesh::EPSILON_E;

  // Make sure we got the synchronized version
  epsilon_ = copy.epsilon_;
  epsilon2_ = copy.epsilon2_;

  copy.synchronize_lock_.unlock();

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  this->vmesh_.reset(CreateVStructCurveMesh(this));
}


template <class Basis>
bool
StructCurveMesh<Basis>::get_dim(std::vector<size_type> &array) const
{
  array.resize(1);
  array.clear();

  array.push_back(this->ni_);

  return true;
}


template <class Basis>
double
StructCurveMesh<Basis>::get_epsilon() const
{
  return(epsilon_);
}

template <class Basis>
void
StructCurveMesh<Basis>::compute_epsilon()
{
  epsilon_ = get_bounding_box().diagonal().length()*1e-8;
  epsilon2_ = epsilon_*epsilon_;
}

template <class Basis>
Core::Geometry::BBox
StructCurveMesh<Basis>::get_bounding_box() const
{
  Core::Geometry::BBox result;

  typename ScanlineMesh<Basis>::Node::iterator i, ie;
  this->begin(i);
  this->end(ie);

  while (i != ie)
  {
    result.extend(points_[*i]);
    ++i;
  }

  return result;
}


template <class Basis>
void
StructCurveMesh<Basis>::transform(const Core::Geometry::Transform &t)
{
  typename ScanlineMesh<Basis>::Node::iterator i, ie;
  this->begin(i);
  this->end(ie);

  while (i != ie)
  {
    points_[*i] = t.project(points_[*i]);

    ++i;
  }
}


template <class Basis>
double
StructCurveMesh<Basis>::get_cord_length() const
{
  double result = 0.0;

  typename ScanlineMesh<Basis>::Node::iterator i, i1, ie;
  this->begin(i);
  this->begin(i1);
  this->end(ie);

  while (i1 != ie)
  {
    ++i1;
    result += (points_[*i] - points_[*i1]).length();
    ++i;
  }

  return result;
}


template <class Basis>
void
StructCurveMesh<Basis>::get_nodes(typename ScanlineMesh<Basis>::Node::array_type &array, typename ScanlineMesh<Basis>::Edge::index_type idx) const
{
  array.resize(2);
  array[0] = typename ScanlineMesh<Basis>::Node::index_type(idx);
  array[1] = typename ScanlineMesh<Basis>::Node::index_type(idx + 1);
}


template <class Basis>
void
StructCurveMesh<Basis>::get_center(Core::Geometry::Point &result, const typename ScanlineMesh<Basis>::Node::index_type &idx) const
{
  result = points_[idx];
}


template <class Basis>
void
StructCurveMesh<Basis>::get_center(Core::Geometry::Point &result, const typename ScanlineMesh<Basis>::Edge::index_type &idx) const
{
  const Core::Geometry::Point &p0 = points_[typename ScanlineMesh<Basis>::Node::index_type(idx)];
  const Core::Geometry::Point &p1 = points_[typename ScanlineMesh<Basis>::Node::index_type(idx+1)];

  result = Core::Geometry::Point(p0+p1)/2.0;
}


template <class Basis>
bool
StructCurveMesh<Basis>::inside2_p(index_type i, const Core::Geometry::Point &p, double& alpha) const
{
  const Core::Geometry::Point &p0 = points_[i];
  const Core::Geometry::Point &p1 = points_[i+1];

  const Core::Geometry::Vector v = p1-p0;
  alpha = Dot(p0-p,v)/v.length2();

  Core::Geometry::Point point;
  if (alpha < 0.0) { point = p0; alpha = 0.0; }
  else if (alpha > 1.0) { point = p1; alpha = 1.0; }
  else { point = (alpha*p0 + (1.0-alpha)*p1).asPoint(); }

  if ((point - p).length2() < epsilon2_) return (true);

  return (false);
}


template <class Basis>
double
StructCurveMesh<Basis>::distance2_p(index_type i,
                                    const Core::Geometry::Point &p,
                                    Core::Geometry::Point& result,
                                    double& alpha) const
{
  const Core::Geometry::Point &p0 = points_[i];
  const Core::Geometry::Point &p1 = points_[i+1];

  const Core::Geometry::Vector v = p1-p0;
  alpha = Dot(p0-p,v)/v.length2();

  if (alpha < 0.0) { result = p0; alpha = 0.0; }
  else if (alpha > 1.0) { result = p1; alpha = 1.0; }
  else { result = (alpha*p0 + (1.0-alpha)*p1).asPoint(); }

  double dist = (result - p).length2();
  return (sqrt(dist));
}


template <class Basis>
int
StructCurveMesh<Basis>::get_weights(const Core::Geometry::Point &p,
				    typename ScanlineMesh<Basis>::Node::array_type &l,
				    double *w)
{
  typename ScanlineMesh<Basis>::Edge::index_type idx;
  if (locate(idx, p))
  {
    get_nodes(l,idx);
    std::vector<double> coords(1);
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
StructCurveMesh<Basis>::get_weights(const Core::Geometry::Point &p,
				    typename ScanlineMesh<Basis>::Edge::array_type &l,
                                    double *w)
{
  typename ScanlineMesh<Basis>::Edge::index_type idx;
  if (locate(idx, p))
  {
    l.resize(1);
    l[0] = idx;
    w[0] = 1.0;
    return 1;
  }
  return 0;
}


template <class Basis>
void
StructCurveMesh<Basis>::get_random_point(Core::Geometry::Point &p,
					 const typename ScanlineMesh<Basis>::Elem::index_type i,
                                         FieldRNG &rng) const
{
  const Core::Geometry::Point &p0 =points_[typename ScanlineMesh<Basis>::Node::index_type(i)];
  const Core::Geometry::Point &p1=points_[typename ScanlineMesh<Basis>::Node::index_type(i+1)];

  p = p0 + (p1 - p0) * rng();
}


template <class Basis>
bool
StructCurveMesh<Basis>::synchronize(mask_type sync)
{
  synchronize_lock_.lock();
  if (sync & (Mesh::EPSILON_E|Mesh::LOCATE_E|Mesh::FIND_CLOSEST_E) &&
      !(synchronized_ & Mesh::EPSILON_E))
  {
    compute_epsilon();
    synchronized_ |= Mesh::EPSILON_E;
    synchronized_ |= Mesh::LOCATE_E;
  }
  synchronize_lock_.unlock();
  return (true);
}

template <class Basis>
bool
StructCurveMesh<Basis>::unsynchronize(mask_type /*sync*/)
{
  return (true);
}


template <class Basis>
bool
StructCurveMesh<Basis>::clear_synchronization()
{
  synchronize_lock_.lock();

  // Undo marking the synchronization
  synchronized_ = Mesh::NODES_E | Mesh::ELEMS_E | Mesh::EDGES_E;

  // Free memory where possible

  synchronize_lock_.unlock();

  return (true);
}

#define STRUCT_CURVE_MESH_VERSION 1

template <class Basis>
void
StructCurveMesh<Basis>::io(Piostream& stream)
{
  stream.begin_class(type_name(-1), STRUCT_CURVE_MESH_VERSION);
  ScanlineMesh<Basis>::io(stream);

  /// IO data members, in order
  Pio(stream, points_);

  stream.end_class();

  if (stream.reading())
    this->vmesh_.reset(CreateVStructCurveMesh(this));
}


template <class Basis>
const std::string
StructCurveMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("StructCurveMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
const TypeDescription*
get_type_description(StructCurveMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("StructCurveMesh", subs,
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructCurveMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((StructCurveMesh<Basis> *)0);
}


} /// namespace SCIRun

#endif /// SCI_project_StructCurveMesh_h
