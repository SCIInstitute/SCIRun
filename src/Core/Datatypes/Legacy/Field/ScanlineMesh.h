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


///
///@file  ScanlineMesh.h
///@brief Templated Mesh defined on a 3D Regular Grid
///
///@author
///       Michael Callahan
///       Department of Computer Science
///       University of Utah
///@date  January 2001
///

#ifndef CORE_DATATYPES_SCANLINEMESH_H
#define CORE_DATATYPES_SCANLINEMESH_H 1

/// Include what kind of support we want to have
/// Need to fix this and couple it sci-defs
#include <Core/Datatypes/Legacy/Field/MeshSupport.h>

#include <Core/Containers/StackVector.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/CrvLinearLgn.h>

#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/BBox.h>

#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Datatypes/Legacy/Field/FieldRNG.h>
#include <Core/Datatypes/Legacy/Field/FieldIterator.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

/////////////////////////////////////////////////////
// Declarations for virtual interface


/// Functions for creating the virtual interface
/// Declare the functions that instantiate the virtual interface
template <class Basis>
class ScanlineMesh;

/// make sure any other mesh other than the preinstantiate ones
/// returns no virtual interface. Altering this behavior will allow
/// for dynamically compiling the interface if needed.
template<class MESH>
VMesh* CreateVScanlineMesh(MESH* mesh) { return (0); }

/// These declarations are needed for a combined dynamic compilation as
/// as well as virtual functions solution.
/// Declare that these can be found in a library that is already
/// precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_SCANLINE_SUPPORT > 0)

SCISHARE VMesh* CreateVScanlineMesh(ScanlineMesh<Core::Basis::CrvLinearLgn<Core::Geometry::Point> >* mesh);

#endif
/////////////////////////////////////////////////////


template <class Basis>
class ScanlineMesh : public Mesh
{

/// Make sure the virtual interface has access
template <class MESH> friend class VScanlineMesh;

public:

  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type            under_type;
  typedef SCIRun::index_type            index_type;
  typedef SCIRun::size_type             size_type;
  typedef SCIRun::mask_type             mask_type;

  typedef boost::shared_ptr<ScanlineMesh<Basis> > handle_type;
  typedef Basis           basis_type;

  /// Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef NodeIndex<under_type>       index_type;
    typedef NodeIterator<under_type>    iterator;
    typedef NodeIndex<under_type>       size_type;
    typedef StackVector<index_type, 8>  array_type;
  };

  struct Edge {
    typedef EdgeIndex<under_type>       index_type;
    typedef EdgeIterator<under_type>    iterator;
    typedef EdgeIndex<under_type>       size_type;
    typedef std::vector<index_type>          array_type;
  };

  struct Face {
    typedef FaceIndex<under_type>       index_type;
    typedef FaceIterator<under_type>    iterator;
    typedef FaceIndex<under_type>       size_type;
    typedef std::vector<index_type>          array_type;
  };

  struct Cell {
    typedef CellIndex<under_type>       index_type;
    typedef CellIterator<under_type>    iterator;
    typedef CellIndex<under_type>       size_type;
    typedef std::vector<index_type>          array_type;
  };

  typedef Edge Elem;
  typedef Node DElem;

  friend class ElemData;

  class ElemData
  {
  public:
    typedef typename ScanlineMesh<Basis>::index_type  index_type;

    ElemData(const ScanlineMesh<Basis>& msh,
             const typename Elem::index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    // the following designed to coordinate with ::get_nodes
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


    // the following designed to coordinate with ::get_edges
    inline
    index_type edge0_index() const
    {
      return index_;
    }

    inline
    const Core::Geometry::Point node0() const
    {
      Core::Geometry::Point p(index_, 0.0, 0.0);
      return mesh_.transform_.project(p);
    }
    inline
    const Core::Geometry::Point node1() const
    {
      Core::Geometry::Point p(index_ + 1, 0.0, 0.0);
      return mesh_.transform_.project(p);
    }

  private:
    const ScanlineMesh<Basis>          &mesh_;
    const index_type                   index_;
  };

  ScanlineMesh() : min_i_(0), ni_(0)
  {
    DEBUG_CONSTRUCTOR("ScanlineMesh")

    vmesh_.reset(CreateVScanlineMesh(this));
    compute_jacobian();
  }

  ScanlineMesh(index_type nx, const Core::Geometry::Point &min, const Core::Geometry::Point &max);
  ScanlineMesh(ScanlineMesh* mh, index_type offset, index_type nx)
    : min_i_(offset), ni_(nx), transform_(mh->transform_)
  {
    DEBUG_CONSTRUCTOR("ScanlineMesh")

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    vmesh_.reset(CreateVScanlineMesh(this));

    compute_jacobian();
  }
  ScanlineMesh(const ScanlineMesh &copy)
    : Mesh(copy),
      min_i_(copy.get_min_i()), ni_(copy.get_ni()),
      transform_(copy.transform_)
  {
    DEBUG_CONSTRUCTOR("ScanlineMesh")

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    vmesh_.reset(CreateVScanlineMesh(this));

    compute_jacobian();
  }
  virtual ScanlineMesh *clone() const { return new ScanlineMesh(*this); }
  virtual ~ScanlineMesh()
  {
    DEBUG_DESTRUCTOR("ScanlineMesh")
  }

  /// Access point to virtual interface
  virtual VMesh* vmesh() { return (vmesh_.get()); }

  virtual MeshFacadeHandle getFacade() const
  {
     return boost::make_shared<Core::Datatypes::VirtualMeshFacade<VMesh>>(vmesh_);
  }

  virtual int basis_order() { return basis_.polynomial_order(); }

  virtual bool has_normals() const { return false; }
  virtual bool has_face_normals() const { return false; }
  virtual bool is_editable() const { return false; }

  /// get the mesh statistics
  index_type get_min_i() const { return min_i_; }
  bool get_min(std::vector<index_type>&) const;
  size_type get_ni() const { return ni_; }
  virtual bool get_dim(std::vector<size_type>&) const;
  Core::Geometry::Vector diagonal() const;
  virtual Core::Geometry::BBox get_bounding_box() const;
  virtual void transform(const Core::Geometry::Transform &t);
  virtual void get_canonical_transform(Core::Geometry::Transform &t);

  /// set the mesh statistics
  void set_min_i(index_type i) {min_i_ = i; }
  void set_min(std::vector<index_type> mins);
  void set_ni(size_type i)
  {
    ni_ = i;

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    vmesh_.reset(CreateVScanlineMesh(this));
  }
  virtual void set_dim(std::vector<size_type> dims);

  void begin(typename Node::iterator &) const;
  void begin(typename Edge::iterator &) const;
  void begin(typename Face::iterator &) const;
  void begin(typename Cell::iterator &) const;

  void end(typename Node::iterator &) const;
  void end(typename Edge::iterator &) const;
  void end(typename Face::iterator &) const;
  void end(typename Cell::iterator &) const;

  void size(typename Node::size_type &) const;
  void size(typename Edge::size_type &) const;
  void size(typename Face::size_type &) const;
  void size(typename Cell::size_type &) const;

  void to_index(typename Node::index_type &index, index_type i) const
  { index = i; }
  void to_index(typename Edge::index_type &index, index_type i) const
  { index = i; }
  void to_index(typename Face::index_type &, index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\".");  }
  void to_index(typename Cell::index_type &, index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\".");  }

  /// get the child elements of the given index
  void get_nodes(typename Node::array_type &, typename Edge::index_type) const;
  void get_nodes(typename Node::array_type &, typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\".");  }
  void get_nodes(typename Node::array_type &, typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\".");  }
  void get_edges(typename Edge::array_type &, typename Edge::index_type) const
  {}
  void get_edges(typename Edge::array_type &, typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\".");  }
  void get_edges(typename Edge::array_type &, typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\".");  }

  //Stub, used by ShowField.
  void get_faces(typename Face::array_type &, typename Elem::index_type) const
  {}

  /// get the parent element(s) of the given index
  void get_elems(typename Elem::array_type &result,
                 typename Node::index_type idx) const;
  void get_elems(typename Elem::array_type&,
                 typename Edge::index_type) const {}
  void get_elems(typename Elem::array_type&,
                 typename Face::index_type) const {}

  /// Wrapper to get the derivative elements from this element.
  void get_delems(typename DElem::array_type &result,
                  typename Elem::index_type idx) const
  {
    get_nodes(result, idx);
  }

  /// Get the size of an elemnt (length, area, volume)
  double get_size(typename Node::index_type) const { return 0.0; }
  double get_size(typename Edge::index_type idx) const
  {
    typename Node::array_type ra;
    get_nodes(ra,idx);
    Core::Geometry::Point p0,p1;
    get_point(p0,ra[0]);
    get_point(p1,ra[1]);
    return (p0-p1).length();
  }
  double get_size(typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  double get_size(typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }
  double get_length(typename Edge::index_type idx) const
  { return get_size(idx); }
  double get_area(typename Face::index_type idx) const
  { return get_size(idx); }
  double get_volume(typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  int get_valence(typename Node::index_type idx) const
  { return (idx == 0 || idx == ni_ - 1) ? 1 : 2; }
  int get_valence(typename Edge::index_type) const { return 0; }
  int get_valence(typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  int get_valence(typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  /// get the center point (in object space) of an element
  void get_center(Core::Geometry::Point &, typename Node::index_type) const;
  void get_center(Core::Geometry::Point &, typename Edge::index_type) const;
  void get_center(Core::Geometry::Point &, typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  void get_center(Core::Geometry::Point &, typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  bool locate(typename Node::index_type &, const Core::Geometry::Point &) const;
  bool locate(typename Edge::index_type &, const Core::Geometry::Point &) const;
  bool locate(typename Face::index_type &, const Core::Geometry::Point &) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  bool locate(typename Cell::index_type &, const Core::Geometry::Point &) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  bool locate(typename Elem::index_type&, std::vector<double>& coords,
              const Core::Geometry::Point &) const;

  int get_weights(const Core::Geometry::Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point &p, typename Edge::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point & , typename Face::array_type & , double * )
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  int get_weights(const Core::Geometry::Point & , typename Cell::array_type & , double * )
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  void get_random_point(Core::Geometry::Point &p, typename Elem::index_type i, FieldRNG &rng) const;

  void get_point(Core::Geometry::Point &p, typename Node::index_type i) const { get_center(p, i); }
  void get_normal(Core::Geometry::Vector &, typename Node::index_type) const
  { ASSERTFAIL("This mesh type does not have node normals."); }
  void get_normal(Core::Geometry::Vector &, std::vector<double> &, typename Elem::index_type,
                  unsigned int)
  { ASSERTFAIL("This mesh type does not have element normals."); }



  // Unsafe due to non-constness of unproject.
  Core::Geometry::Transform &get_transform() { return transform_; }
  Core::Geometry::Transform &set_transform(const Core::Geometry::Transform &trans)
  { transform_ = trans; compute_jacobian(); return transform_; }

  virtual int dimensionality() const { return 1; }
  virtual int topology_geometry() const { return (Mesh::STRUCTURED | Mesh::REGULAR); }
  Basis& get_basis() { return basis_; }

 /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an edge.
  void pwl_approx_edge(std::vector<std::vector<double> > &coords,
                       typename Elem::index_type /*ci*/,
                       unsigned int,
                       unsigned int div_per_unit) const
  {
    // Needs to match unit_edges in Basis/QuadBilinearLgn.cc
    // compare get_nodes order to the basis order
    basis_.approx_edge(0, div_per_unit, coords);
  }

  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an face.
  void pwl_approx_face(std::vector<std::vector<std::vector<double> > >& /*coords*/,
                       typename Elem::index_type /*ci*/,
                       typename Face::index_type /*fi*/,
                       unsigned int /*div_per_unit*/) const
  {
    ASSERTFAIL("ScanlineMesh has no faces");
  }

  /// Synchronize functions, as there is nothing to synchronize, these
  /// functions always succeed

  virtual bool synchronize(mask_type /*sync*/) { return (true); }
  virtual bool unsynchronize(mask_type /*sync*/) { return (true); }
  bool clear_synchronization() { return (true); }

  /// Get the local coordinates for a certain point within an element
  /// This function uses a couple of newton iterations to find the local
  /// coordinate of a point
  template<class VECTOR>
  bool get_coords(VECTOR &coords, const Core::Geometry::Point &p, typename Elem::index_type idx) const
  {
    // If polynomial order is larger, use the complicated HO basis implementation
    // Since this is a latvol and most probably linear, this function is to expensive
    if (basis_.polynomial_order() > 1)
    {
      ElemData ed(*this, idx);
      return (basis_.get_coords(coords, p, ed));
    }

    // Cheap implementation that assumes it is a regular grid
    // This implementation should be faster then the interpolate of the linear
    // basis which needs to work as well for the unstructured hexvol :(
    const Core::Geometry::Point r = transform_.unproject(p);

    coords.resize(1);
    coords[0] = static_cast<typename VECTOR::value_type>(r.x()-static_cast<double>(idx));

    if (static_cast<double>(coords[0]) < 0.0)
    {
      if (static_cast<double>(coords[0]) > -(1e-8))
        coords[0] = static_cast<typename VECTOR::value_type>(0.0);
      else
        return (false);
    }
    if (static_cast<double>(coords[0]) > 1.0)
    {
      if (static_cast<double>(coords[0]) < 1.0+(1e-8))
        coords[0] = static_cast<typename VECTOR::value_type>(1.0);
      else
        return (false);
    }

    return (true);
  }

  /// Find the location in the global coordinate system for a local coordinate
  /// This function is the opposite of get_coords.
  template<class VECTOR>
  void interpolate(Core::Geometry::Point &pt, const VECTOR &coords, typename Elem::index_type idx) const
  {
    // only makes sense for higher order
    if (basis_.polynomial_order() > 1)
    {
      ElemData ed(*this, idx);
      pt = basis_.interpolate(coords, ed);
      return;
    }

    Core::Geometry::Point p(static_cast<double>(idx)+static_cast<double>(coords[0]),0.0,0.0);
    pt = transform_.project(p);
  }

  /// Interpolate the derivate of the function, This infact will return the
  /// jacobian of the local to global coordinate transformation. This function
  /// is mainly intended for the non linear elements
  template<class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, typename Elem::index_type idx, VECTOR2 &J) const
  {
    // only makes sense for higher order
    if (basis_.polynomial_order() > 1)
    {
      ElemData ed(*this, idx);
      basis_.derivate(coords, ed, J);
      return;
    }

    // Cheaper implementation
    J.resize(1);
    J[0] = transform_.project(Core::Geometry::Point(1.0,0.0,0.0));
  }

  /// Get the determinant of the jacobian, which is the local volume of an element
  /// and is intended to help with the integration of functions over an element.
  template<class VECTOR>
  double det_jacobian(const VECTOR& coords,
                typename Elem::index_type idx) const
  {
    if (basis_.polynomial_order() > 1)
    {
      double J[9];
      jacobian(coords,idx,J);
      return (DetMatrix3x3(J));
    }

    return (det_jacobian_);
  }

  /// Get the jacobian of the transformation. In case one wants the non inverted
  /// version of this matrix. This is currentl here for completeness of the
  /// interface
  template<class VECTOR>
  void jacobian(const VECTOR& coords, typename Elem::index_type idx, double* J) const
  {
    if (basis_.polynomial_order() > 1)
    {
      StackVector<Core::Geometry::Point,3> Jv;
      ElemData ed(*this,idx);
      basis_.derivate(coords,ed,Jv);
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

      return;
    }

    J[0] = jacobian_[0];
    J[1] = jacobian_[1];
    J[2] = jacobian_[2];
    J[3] = jacobian_[3];
    J[4] = jacobian_[4];
    J[5] = jacobian_[5];
    J[6] = jacobian_[6];
    J[7] = jacobian_[7];
    J[8] = jacobian_[8];
  }

  /// Get the inverse jacobian of the transformation. This one is needed to
  /// translate local gradients into global gradients. Hence it is crucial for
  /// calculating gradients of fields, or constructing finite elements.
  template<class VECTOR>
  double inverse_jacobian(const VECTOR& coords, typename Elem::index_type idx, double* Ji) const
  {
    if (basis_.polynomial_order() > 1)
    {
      StackVector<Core::Geometry::Point,2> Jv;
      ElemData ed(*this,idx);
      basis_.derivate(coords,ed,Jv);
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

    Ji[0] = inverse_jacobian_[0];
    Ji[1] = inverse_jacobian_[1];
    Ji[2] = inverse_jacobian_[2];
    Ji[3] = inverse_jacobian_[3];
    Ji[4] = inverse_jacobian_[4];
    Ji[5] = inverse_jacobian_[5];
    Ji[6] = inverse_jacobian_[6];
    Ji[7] = inverse_jacobian_[7];
    Ji[8] = inverse_jacobian_[8];

    return (det_inverse_jacobian_);
  }

  double scaled_jacobian_metric(typename Elem::index_type /*idx*/) const
    { return (scaled_jacobian_); }

  double jacobian_metric(typename Elem::index_type /*idx*/) const
    { return (det_jacobian_); }


  double get_epsilon() const;

  /// Export this class using the old Pio system
  virtual void io(Piostream&);
  /// These IDs are created as soon as this class will be instantiated
  /// The first one is for Pio and the second for the virtual interface
  /// These are currently different as they serve different needs.
  static PersistentTypeID scanline_typeid;
  /// Core functionality for getting the name of a templated mesh class
  static const std::string type_name(int n = -1);
  virtual std::string dynamic_type_name() const { return scanline_typeid.type; }

  /// Type description, used for finding names of the mesh class for
  /// dynamic compilation purposes. Some of this should be obsolete
  virtual const TypeDescription *get_type_description() const;
  static const TypeDescription* node_type_description();
  static const TypeDescription* edge_type_description();
  static const TypeDescription* face_type_description();
  static const TypeDescription* cell_type_description();
  static const TypeDescription* elem_type_description()
  { return edge_type_description(); }

  /// This function returns a maker for Pio.
  static Persistent *maker() { return new ScanlineMesh(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return boost::make_shared<ScanlineMesh>(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle scanline_maker(size_type x, const Core::Geometry::Point& min, const Core::Geometry::Point& max)
  {
    return boost::make_shared<ScanlineMesh>(x,min,max);
  }

  /// This function will find the closest element and the location on that
  /// element that is the closest
  template <class INDEX>
  bool
  find_closest_node(double& pdist, Core::Geometry::Point &result,
                    INDEX &node, const Core::Geometry::Point &p, double maxdist) const
  {
    bool ret = find_closest_node(pdist,result,node,p);
    if (!ret)  return (false);
    if (maxdist < 0.0 || pdist < maxdist) return (true);
    return (false);
  }

  /// This function will find the closest element and the location on that
  /// element that is the closest
  template <class INDEX>
  bool
  find_closest_node(double& pdist, Core::Geometry::Point &result,
                    INDEX &node, const Core::Geometry::Point &p) const
  {
    /// If there are no elements, we cannot find the closest
    if (ni_ == 0)  return (false);

    const Core::Geometry::Point r = transform_.unproject(p);

    double rx = floor(r.x() + 0.5);
    const double nii = static_cast<double>(ni_-1);

    if (rx < 0.0) rx = 0.0; if (rx > nii) rx = nii;

    result = transform_.project(Core::Geometry::Point(rx,0.0,0.0));
    index_type irx =static_cast<index_type>(rx);
    node = INDEX(irx);
    pdist = (p-result).length();

    return (true);
  }


  template <class INDEX, class ARRAY>
  bool
  find_closest_elem(double& pdist,
                    Core::Geometry::Point& result,
                    ARRAY &coords,
                    INDEX &elem,
                    const Core::Geometry::Point &p) const
  {
    /// If there are no elements, we cannot find the closest
    if (ni_ == 0) return (false);

    const Core::Geometry::Point r = transform_.unproject(p);

    double ii = r.x();
    const double nii = static_cast<double>(ni_-2);

    if (ii < 0.0) ii = 0.0; if (ii > nii) ii = nii;

    const double fi = floor(ii);

    elem = INDEX(static_cast<index_type>(fi));
    result = transform_.project(Core::Geometry::Point(ii,0,0));
    pdist = (p-result).length();

    coords.resize(1);
    coords[0] = ii-fi;

    return (true);
  }

  template <class INDEX, class ARRAY>
  bool
  find_closest_elem(double& pdist, Core::Geometry::Point& result,
                    ARRAY& coords, INDEX &elem,
                    const Core::Geometry::Point &p, double maxdist) const
  {
    bool ret = find_closest_elem(pdist,result,coords,elem,p);
    if (!ret)  return (false);
    if (maxdist < 0.0 || pdist < maxdist) return (true);
    return (false);
  }

  /// This function will find the closest element and the location on that
  /// element that is the closest
  template <class INDEX>
  bool
  find_closest_elem(double& pdist,
                    Core::Geometry::Point &result,
                    INDEX &elem,
                    const Core::Geometry::Point &p) const
  {
    /// If there are no elements, we cannot find the closest
    if (ni_ == 0) return (false);

    const Core::Geometry::Point r = transform_.unproject(p);

    double ii = r.x();
    const double nii = static_cast<double>(ni_-2);

    if (ii < 0.0) ii = 0.0; if (ii > nii) ii = nii;

    elem = INDEX(static_cast<index_type>(floor(ii)));
    result = transform_.project(Core::Geometry::Point(ii,0,0));
    pdist = (p-result).length();

    return (true);
  }


  /// This function will return multiple elements if the closest point is
  /// located on a node or edge. All bordering elements are returned in that
  /// case.
  template <class ARRAY>
  bool
  find_closest_elems(double& pdist, Core::Geometry::Point &result,
                     ARRAY &elems, const Core::Geometry::Point &p) const
  {
    /// If there are no elements, we cannot find the closest
    if (ni_ == 0) return (false);

    const double epsilon_ = 1e-8;

    const Core::Geometry::Point r = transform_.unproject(p);

    double ii = r.x();
    const double nii = static_cast<double>(ni_-2);

    if (ii < 0.0) ii = 0.0; if (ii > nii) ii = nii;

    const double fii = floor(ii);
    index_type i = static_cast<index_type>(fii);
    elems.push_back(static_cast<typename ARRAY::value_type>(i));

    if ((fabs(fii-ii) < epsilon_) && ((i-1)>0))
    {
      elems.push_back(static_cast<typename ARRAY::value_type>(i-1));
    }

    if ((fabs(fii-(ii+1.0)) < epsilon_) && (i<(ni_-1)))
    {
      elems.push_back(static_cast<typename ARRAY::value_type>(i+1));
    }

    result = transform_.project(Core::Geometry::Point(ii,0,0));
    pdist = (p-result).length();

    return (true);
  }







protected:

  void compute_jacobian();

  /// the min typename Node::index_type ( incase this is a subLattice )
  index_type           min_i_;

  /// the typename Node::index_type space extents of a ScanlineMesh
  /// (min=min_typename Node::index_type, max=min+extents-1)
  size_type            ni_;

  /// the object space extents of a ScanlineMesh
  Core::Geometry::Transform            transform_;

  /// the basis fn
  Basis                basis_;

  boost::shared_ptr<VMesh>        vmesh_;

  // The jacobian is the same for every element
  // hence store them as soon as we know the transfrom_
  // This should speed up FE computations on these regular grids.
  double jacobian_[9];
  double inverse_jacobian_[9];
  double det_jacobian_;
  double scaled_jacobian_;
  double det_inverse_jacobian_;

};


template <class Basis>
PersistentTypeID
ScanlineMesh<Basis>::scanline_typeid(type_name(-1), "Mesh",
                             ScanlineMesh<Basis>::maker);


template <class Basis>
ScanlineMesh<Basis>::ScanlineMesh(size_type ni,
                                  const Core::Geometry::Point &min, const Core::Geometry::Point &max)
  : min_i_(0), ni_(ni)
{
  DEBUG_CONSTRUCTOR("ScanlineMesh")

  Core::Geometry::Vector v0 = max - min;
  Core::Geometry::Vector v1, v2;
  v0.find_orthogonal(v1,v2);

  // The two points define a line, this sets up the transfrom so it projects
  // to the x-axis

  transform_.load_basis(min,v0,v1,v2);
  transform_.post_scale(Core::Geometry::Vector(1.0 / (ni_ - 1.0), 1.0, 1.0));

  transform_.compute_imat();
  compute_jacobian();

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  vmesh_.reset(CreateVScanlineMesh(this));
}


template <class Basis>
Core::Geometry::BBox
ScanlineMesh<Basis>::get_bounding_box() const
{
  Core::Geometry::Point p0(0.0, 0.0, 0.0);
  Core::Geometry::Point p1(ni_ - 1, 0.0, 0.0);

  Core::Geometry::BBox result;
  result.extend(transform_.project(p0));
  result.extend(transform_.project(p1));
  return result;
}


template <class Basis>
Core::Geometry::Vector
ScanlineMesh<Basis>::diagonal() const
{
  return get_bounding_box().diagonal();
}


template <class Basis>
void
ScanlineMesh<Basis>::transform(const Core::Geometry::Transform &t)
{
  transform_.pre_trans(t);
  compute_jacobian();
}


template <class Basis>
void
ScanlineMesh<Basis>::get_canonical_transform(Core::Geometry::Transform &t)
{
  t = transform_;
  t.post_scale(Core::Geometry::Vector(ni_ - 1.0, 1.0, 1.0));
}


template <class Basis>
bool
ScanlineMesh<Basis>::get_min(std::vector<index_type> &array ) const
{
  array.resize(1);
  array.clear();

  array.push_back(min_i_);

  return true;
}


template <class Basis>
bool
ScanlineMesh<Basis>::get_dim(std::vector<size_type> &array) const
{
  array.resize(1);
  array.clear();

  array.push_back(ni_);

  return true;
}


template <class Basis>
void
ScanlineMesh<Basis>::set_min(std::vector<index_type> min)
{
  min_i_ = min[0];
}


template <class Basis>
void
ScanlineMesh<Basis>::set_dim(std::vector<size_type> dim)
{
  ni_ = dim[0];

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  vmesh_.reset(CreateVScanlineMesh(this));
}


template <class Basis>
void
ScanlineMesh<Basis>::get_nodes(typename Node::array_type &array,
                               typename Edge::index_type idx) const
{
  array.resize(2);
  array[0] = typename Node::index_type(idx);
  array[1] = typename Node::index_type(idx + 1);
}


template <class Basis>
void
ScanlineMesh<Basis>::get_elems(typename Elem::array_type &result,
                               typename Node::index_type index) const
{
  result.reserve(2);
  result.clear();
  if (index > 0)
  {
    result.push_back(typename Edge::index_type(index-1));
  }
  if (index < ni_-1)
  {
    result.push_back(typename Edge::index_type(index));
  }
}


template <class Basis>
void
ScanlineMesh<Basis>::get_center(Core::Geometry::Point &result,
                                typename Node::index_type idx) const
{
  Core::Geometry::Point p(idx, 0.0, 0.0);
  result = transform_.project(p);
}


template <class Basis>
void
ScanlineMesh<Basis>::get_center(Core::Geometry::Point &result,
                                typename Edge::index_type idx) const
{
  Core::Geometry::Point p(idx + 0.5, 0.0, 0.0);
  result = transform_.project(p);
}


template <class Basis>
int
ScanlineMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Node::array_type &l,
                                 double *w)
{
  typename Edge::index_type idx;
  if (locate(idx, p))
  {
    get_nodes(l,idx);
    std::vector<double> coords(1);
    if (get_coords(coords, p, idx))
    {
      basis_.get_weights(coords, w);
      return basis_.dofs();
    }
  }
  return 0;
}


template <class Basis>
int
ScanlineMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Edge::array_type &l,
                                 double *w)
{
  typename Edge::index_type idx;
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
ScanlineMesh<Basis>::get_random_point(Core::Geometry::Point &p,
                                      typename Elem::index_type ei,
                                      FieldRNG &rng) const
{
  Core::Geometry::Point p0, p1;
  get_center(p0, typename Node::index_type(ei));
  get_center(p1, typename Node::index_type(under_type(ei)+1));

  p = p0 + (p1 - p0) * rng();
}


#define SCANLINEMESH_VERSION 4

template <class Basis>
void
ScanlineMesh<Basis>::io(Piostream& stream)
{
  int version = stream.begin_class(type_name(-1), SCANLINEMESH_VERSION);

  Mesh::io(stream);

  // IO data members, in order
  Pio(stream, ni_);

  if (version < 4)
  {
    unsigned int ni = static_cast<unsigned int>(ni_);
    Pio(stream, ni);
    ni_ = static_cast<size_type>(ni);
  }
  else
  {
    Pio_size(stream, ni_);
  }

  if (version < 2 && stream.reading() )
  {
    Pio_old(stream, transform_);
  }
  else
  {
    Pio(stream, transform_);
  }
  if (version >= 3)
  {
    basis_.io(stream);
  }
  stream.end_class();

  if (stream.reading())
  {
    compute_jacobian();
    vmesh_.reset(CreateVScanlineMesh(this));
  }
}


template <class Basis>
const std::string
ScanlineMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("ScanlineMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
void
ScanlineMesh<Basis>::begin(typename ScanlineMesh::Node::iterator &itr) const
{
  itr = typename Node::iterator(min_i_);
}


template <class Basis>
void
ScanlineMesh<Basis>::end(typename ScanlineMesh::Node::iterator &itr) const
{
  itr = typename Node::iterator(min_i_ + ni_);
}


template <class Basis>
void
ScanlineMesh<Basis>::size(typename ScanlineMesh::Node::size_type &s) const
{
  s = typename Node::size_type(ni_);
}


template <class Basis>
void
ScanlineMesh<Basis>::begin(typename ScanlineMesh::Edge::iterator &itr) const
{
  itr = typename Edge::iterator(min_i_);
}


template <class Basis>
void
ScanlineMesh<Basis>::end(typename ScanlineMesh::Edge::iterator &itr) const
{
  itr = typename Edge::iterator(min_i_+ni_-1);
}


template <class Basis>
void
ScanlineMesh<Basis>::size(typename ScanlineMesh::Edge::size_type &s) const
{
  s = typename Edge::size_type(ni_ - 1);
}


template <class Basis>
void
ScanlineMesh<Basis>::begin(typename ScanlineMesh::Face::iterator& /*itr*/) const
{
  ASSERTFAIL("This mesh type does not have faces use \"elem\".");
}


template <class Basis>
void
ScanlineMesh<Basis>::end(typename ScanlineMesh::Face::iterator& /*itr*/) const
{
  ASSERTFAIL("This mesh type does not have faces use \"elem\".");
}


template <class Basis>
void
ScanlineMesh<Basis>::size(typename ScanlineMesh::Face::size_type& /*s*/) const
{
  ASSERTFAIL("This mesh type does not have faces use \"elem\".");
}


template <class Basis>
void
ScanlineMesh<Basis>::begin(typename ScanlineMesh::Cell::iterator& /*itr*/) const
{
  ASSERTFAIL("This mesh type does not have cells use \"elem\".");
}


template <class Basis>
void
ScanlineMesh<Basis>::end(typename ScanlineMesh::Cell::iterator& /*itr*/) const
{
  ASSERTFAIL("This mesh type does not have cells use \"elem\".");
}


template <class Basis>
void
ScanlineMesh<Basis>::size(typename ScanlineMesh::Cell::size_type& /*s*/) const
{
  ASSERTFAIL("This mesh type does not have cells use \"elem\".");
}


template <class Basis>
double
ScanlineMesh<Basis>::get_epsilon() const
{
  Core::Geometry::Point p0(static_cast<double>(ni_-1),0.0,0.0);
  Core::Geometry::Point p1(0.0,0.0,0.0);
  Core::Geometry::Point q0 = transform_.project(p0);
  Core::Geometry::Point q1 = transform_.project(p1);
  return ((q0-q1).length()*1e-8);
}

template <class Basis>
const TypeDescription*
get_type_description(ScanlineMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("ScanlineMesh", subs,
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
ScanlineMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((ScanlineMesh *)0);
}


template <class Basis>
const TypeDescription*
ScanlineMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((ScanlineMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Node",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
ScanlineMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((ScanlineMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Edge",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
ScanlineMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((ScanlineMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Face",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
ScanlineMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((ScanlineMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Cell",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
void
ScanlineMesh<Basis>::compute_jacobian()
{
  if (basis_.polynomial_order() < 2)
  {
    Core::Geometry::Vector J1 = transform_.project(Core::Geometry::Vector(1.0,0.0,0.0));
    Core::Geometry::Vector J2, J3;
    J1.find_orthogonal(J2,J3);
    J2.normalize();
    J3.normalize();

    jacobian_[0] = J1.x();
    jacobian_[1] = J1.y();
    jacobian_[2] = J1.z();
    jacobian_[3] = J2.x();
    jacobian_[4] = J2.y();
    jacobian_[5] = J2.z();
    jacobian_[6] = J3.x();
    jacobian_[7] = J3.y();
    jacobian_[8] = J3.z();

    det_jacobian_ = DetMatrix3x3(jacobian_);
    scaled_jacobian_ = ScaledDetMatrix3x3(jacobian_);
    det_inverse_jacobian_ = InverseMatrix3x3(jacobian_,inverse_jacobian_);
  }
}

template <class Basis>
bool
ScanlineMesh<Basis>::locate(typename Node::index_type &node, const Core::Geometry::Point &p) const
{
  /// If there are no nodes, return false, otherwise there will always be
  /// a node that is closest
  if (ni_ == 0) return (false);

  const Core::Geometry::Point r = transform_.unproject(p);

  double rx = floor(r.x() + 0.5);
  const double nii = static_cast<double>(ni_-1);

  if (rx < 0.0) rx = 0.0; if (rx > nii) rx = nii;

  node = static_cast<index_type>(rx);

  return (true);
}

template <class Basis>
bool
ScanlineMesh<Basis>::locate(typename Edge::index_type &elem, const Core::Geometry::Point &p) const
{
  if (basis_.polynomial_order() > 1) return elem_locate(elem, *this, p);

  /// If there are no elements, we are definitely not inside an element
  if (ni_ < 2) return (false);

  const double epsilon_ = 1e-7;

  const Core::Geometry::Point r = transform_.unproject(p);

  double ii = r.x();
  const double nii = static_cast<double>(ni_-1);

  if (ii>=nii && (ii-epsilon_)<nii) ii=nii-epsilon_;

  if (ii<0 && ii>(-epsilon_)) ii=0.0;

  const index_type i = static_cast<index_type>(floor(ii));

  if (i < (ni_-1) && i >= 0 && ii >= 0.0)
  {
    elem = i;
    return (true);
  }

  /// Not inside any elements
  return (false);
}

template <class Basis>
bool
ScanlineMesh<Basis>::locate(typename Elem::index_type &elem,
                            std::vector<double>& coords,
                            const Core::Geometry::Point &p) const
{
  if (basis_.polynomial_order() > 1) return elem_locate(elem, *this, p);

  /// If there are no elements, we are definitely not inside an element
  if (ni_ < 2) return (false);

  coords.resize(1);
  const double epsilon_ = 1e-8;

  const Core::Geometry::Point r = transform_.unproject(p);

  double ii = r.x();
  const double nii = static_cast<double>(ni_-1);

  if (ii>nii && (ii-epsilon_)<nii) ii=nii-epsilon_;

  if (ii<0 && ii>(-epsilon_)) ii=0.0;

  const double fi = floor(ii);
  const index_type i = static_cast<index_type>(fi);

  if (i < (ni_-1) && i >= 0 && ii >= 0.0)
  {
    elem = i;
    return (true);
  }

  coords[0] = ii - fi;

  /// Not inside any elements
  return (false);
}

} // namespace SCIRun

#endif
