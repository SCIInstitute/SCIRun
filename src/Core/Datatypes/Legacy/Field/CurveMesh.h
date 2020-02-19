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


#ifndef CORE_DATATYPES_CURVEMESH_H
#define CORE_DATATYPES_CURVEMESH_H 1

/// Include what kind of support we want to have
/// Need to fix this and couple it sci-defs
#include <Core/Datatypes/Legacy/Field/MeshSupport.h>

#include <Core/Containers/StackVector.h>
#include <Core/Persistent/PersistentSTL.h>

#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/Vector.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Basis/CrvQuadraticLgn.h>
#include <Core/Basis/CrvCubicHmt.h>

#include <Core/Thread/Mutex.h>

#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Datatypes/Legacy/Field/FieldIterator.h>
#include <Core/Datatypes/Legacy/Field/FieldRNG.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

/////////////////////////////////////////////////////
/// Declarations for virtual interface

template <class Basis> class CurveMesh;

/// make sure any other mesh other than the preinstantiate ones
/// returns no virtual interface. Altering this behavior will allow
/// for dynamically compiling the interface if needed.
template<class MESH>
VMesh* CreateVCurveMesh(MESH*) { return (0); }

#if (SCIRUN_CURVE_SUPPORT > 0)
/// Declare that these can be found in a library that is already
/// precompiled. So dynamic compilation will not instantiate them again.
SCISHARE VMesh* CreateVCurveMesh(CurveMesh<Core::Basis::CrvLinearLgn<Core::Geometry::Point> >* mesh);
#if (SCIRUN_QUADRATIC_SUPPORT > 0)
SCISHARE VMesh* CreateVCurveMesh(CurveMesh<Core::Basis::CrvQuadraticLgn<Core::Geometry::Point> >* mesh);
#endif
#if (SCIRUN_CUBIC_SUPPORT > 0)
SCISHARE VMesh* CreateVCurveMesh(CurveMesh<Core::Basis::CrvCubicHmt<Core::Geometry::Point> >* mesh);
#endif

#endif
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
/// Declarations for CurveMesh class

template <class Basis>
class CurveMesh : public Mesh
{

/// Make sure the virtual interface has access
template<class MESH> friend class VCurveMesh;
template<class MESH> friend class VMeshShared;
template<class MESH> friend class VUnstructuredMesh;

public:

  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type            under_type;
  typedef SCIRun::index_type            index_type;
  typedef SCIRun::size_type             size_type;
  typedef SCIRun::mask_type             mask_type;

  typedef boost::shared_ptr<CurveMesh<Basis> > handle_type;
  typedef Basis                            basis_type;

  /// Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef NodeIndex<under_type>       index_type;
    typedef NodeIterator<under_type>    iterator;
    typedef NodeIndex<under_type>       size_type;
    typedef StackVector<index_type, 2>  array_type;
  };

  struct Edge {
    typedef EdgeIndex<under_type>       index_type;
    typedef EdgeIterator<under_type>    iterator;
    typedef EdgeIndex<under_type>       size_type;
    typedef std::vector<index_type>     array_type;
  };

  /// For dynamic compilation to be compatible with
  /// other mesh types
  struct Face {
    typedef FaceIndex<under_type>       index_type;
    typedef FaceIterator<under_type>    iterator;
    typedef FaceIndex<under_type>       size_type;
    typedef std::vector<index_type>     array_type;
  };

  /// For dynamic compilation to be compatible with
  /// other mesh types
  struct Cell {
    typedef CellIndex<under_type>       index_type;
    typedef CellIterator<under_type>    iterator;
    typedef CellIndex<under_type>       size_type;
    typedef std::vector<index_type>     array_type;
  };

  /// Elem refers to the most complex topological object
  /// DElem refers to object just below Elem in the topological hierarchy

  typedef Edge Elem;
  typedef Node DElem;

  /// Definition specific to this class (should at some point jsut
  /// remove this and make it similar to the other classes)
  typedef std::pair<typename Node::index_type,
               typename Node::index_type> index_pair_type;


  /// Somehow the information of how to interpolate inside an element
  /// ended up in a separate class, as they need to share information
  /// this construction was created to transfer data.
  /// Hopefully in the future this class will disappear again.
  friend class ElemData;

  class ElemData
  {
  public:
    typedef typename CurveMesh<Basis>::index_type  index_type;

    ElemData(const CurveMesh<Basis>& msh, const index_type idx) :
      mesh_(msh),
      index_(idx)
    {}

    inline
    index_type node0_index() const {
      return mesh_.edges_[2*index_];
    }
    inline
    index_type node1_index() const {
      return mesh_.edges_[2*index_+1];
    }

    inline
    const Core::Geometry::Point &node0() const {
      return mesh_.points_[mesh_.edges_[2*index_]];
    }
    inline
    const Core::Geometry::Point &node1() const {
      return mesh_.points_[mesh_.edges_[2*index_+1]];
    }

    inline
    index_type edge0_index() const
    {
      return index_;
    }

  private:
    /// reference of the mesh
    const CurveMesh<Basis>   &mesh_;
    /// copy of index
    const index_type         index_;
  };

  //////////////////////////////////////////////////////////////////

  CurveMesh();

  /// Copy a mesh, needed for detaching the mesh from a field
  CurveMesh(const CurveMesh &copy);

  /// Clone function for detaching the mesh and automatically generating
  /// a new version if needed.
  virtual CurveMesh *clone() const { return new CurveMesh(*this); }

  MeshFacadeHandle getFacade() const
  {
    return boost::make_shared<Core::Datatypes::VirtualMeshFacade<VMesh>>(vmesh_);
  }

  virtual ~CurveMesh();

  /// Obtain the virtual interface pointer
  virtual VMesh* vmesh() { return (vmesh_.get()); }

  /// This one should go at some point, should be reroute through the
  /// virtual interface
  virtual int basis_order() { return (basis_.polynomial_order()); }

  /// Topological dimension
  virtual int dimensionality() const { return 1; }

  /// What kind of mesh is this
  /// structured = no connectivity data
  /// regular    = no node location data
  virtual int  topology_geometry() const
    { return (Mesh::UNSTRUCTURED | Mesh::IRREGULAR); }

  /// Get the bounding box of the field
  virtual Core::Geometry::BBox get_bounding_box() const;

  /// Return the transformation that takes a 0-1 space bounding box
  /// to the current bounding box of this mesh.
  virtual void get_canonical_transform(Core::Geometry::Transform &t) const;

  virtual void compute_bounding_box();

  /// Transform a field
  virtual void transform(const Core::Geometry::Transform &t);

  /// Check whether mesh can be altered by adding nodes or elements
  virtual bool is_editable() const { return (true); }

  /// Has this mesh normals.
  virtual bool has_normals() const { return (false); }

  /// Compute tables for doing topology, these need to be synchronized
  /// before doing a lot of operations.
  virtual bool synchronize(mask_type sync);
  virtual bool unsynchronize(mask_type sync);
  bool clear_synchronization();

  /// Get the basis class
  Basis& get_basis() { return basis_; }

  /// begin/end iterators
  void begin(typename Node::iterator &) const;
  void begin(typename Edge::iterator &) const;
  void begin(typename Face::iterator &) const;
  void begin(typename Cell::iterator &) const;

  void end(typename Node::iterator &) const;
  void end(typename Edge::iterator &) const;
  void end(typename Face::iterator &) const;
  void end(typename Cell::iterator &) const;

  /// Get the iteration sizes
  void size(typename Node::size_type &) const;
  void size(typename Edge::size_type &) const;
  void size(typename Face::size_type &) const;
  void size(typename Cell::size_type &) const;

  /// These are here to convert indices to unsigned int
  /// counters. Some how the decision was made to use multi
  /// dimensional indices in some fields, these functions
  /// should deal with different pointer types.
  /// Use the virtual interface to avoid all this non sense.
  inline void to_index(typename Node::index_type &index, index_type i) const
    { index = i; }
  inline void to_index(typename Edge::index_type &index, index_type i) const
    { index = i; }
  inline void to_index(typename Face::index_type&, index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  inline void to_index(typename Cell::index_type&, index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }


  /// Get the child elements of the given index.
  void get_nodes(typename Node::array_type &array,
                 typename Node::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_nodes(typename Node::array_type &array,
                 typename Edge::index_type idx) const
  { get_nodes_from_edge(array,idx); }
  void get_nodes(typename Node::array_type&,
                 typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  void get_nodes(typename Node::array_type&,
                 typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  void get_edges(typename Edge::array_type &array,
                 typename Node::index_type idx) const
  { get_edges_from_node(array,idx); }
  void get_edges(typename Edge::array_type &array,
                 typename Edge::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_edges(typename Edge::array_type&,
                 typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  void get_edges(typename Edge::array_type&,
                 typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  void get_faces(typename Face::array_type&,
                 typename Node::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  void get_faces(typename Face::array_type&,
                 typename Edge::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  void get_faces(typename Face::array_type&,
                 typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  void get_faces(typename Face::array_type&,
                 typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }

  void get_cells(typename Cell::array_type&,
                 typename Node::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }
  void get_cells(typename Cell::array_type&,
                 typename Edge::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }
  void get_cells(typename Cell::array_type&,
                 typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }
  void get_cells(typename Cell::array_type&,
                 typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  void get_elems(typename Elem::array_type &array,
                 typename Node::index_type idx) const
  { get_edges_from_node(array,idx); }
  void get_elems(typename Elem::array_type &array,
                 typename Edge::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_elems(typename Elem::array_type&,
                 typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  void get_elems(typename Face::array_type&,
                 typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  void get_delems(typename DElem::array_type &array,
                  typename Node::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_delems(typename DElem::array_type &array,
                  typename Edge::index_type idx) const
  { get_nodes_from_edge(array,idx); }
  void get_delems(typename DElem::array_type&,
                  typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  void get_delems(typename DElem::array_type&,
                  typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }



  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an edge.
  template<class VECTOR>
  void pwl_approx_edge(VECTOR &coords,
                       typename Elem::index_type,
                       unsigned int which_edge,
                       unsigned int div_per_unit) const
  {
    // only one edge in the unit edge.
    basis_.approx_edge(which_edge, div_per_unit, coords);
  }

  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an face.
  template<class VECTOR>
  void pwl_approx_face(VECTOR& /*coords*/,
                       typename Elem::index_type /*ci*/,
                       typename Face::index_type /*fi*/,
                       unsigned int /*div_per_unit*/) const
  {
    ASSERTFAIL("CurveMesh: cannot approximate faces");
  }

  /// get the center point (in object space) of an element
  void get_center(Core::Geometry::Point &result, typename Node::index_type idx) const
  { result = points_[idx]; }
  void get_center(Core::Geometry::Point &, typename Edge::index_type) const;
  void get_center(Core::Geometry::Point &, typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  void get_center(Core::Geometry::Point &, typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  const Core::Geometry::Point &point(typename Node::index_type i) const { return points_[i]; }

  /// Get the size of an element (length, area, volume)
  double get_size(typename Node::index_type /*idx*/) const
    { return 0.0; }
  double get_size(typename Edge::index_type idx) const;
  double get_size(typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  double get_size(typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  /// More specific names for get_size
  double get_length(typename Edge::index_type idx) const
    { return get_size(idx); }
  double get_area(typename Face::index_type) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  double get_volume(typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  /// Get neighbors of an element or a node
  /// THIS ONE IS FLAWED AS IN 3D SPACE  A NODE CAN BE CONNECTED TO
  /// MANY ELEMENTS
  bool get_neighbor(typename Elem::index_type &neighbor,
                    typename Elem::index_type edge,
                    typename DElem::index_type node) const
  {
    return(get_elem_neighbor(neighbor,edge,node));
  }

  /// These are more general neighbor functions
  void get_neighbors(std::vector<typename Node::index_type> &array,
                     typename Node::index_type idx) const
  {
    get_node_neighbors(array,idx);
  }

  bool get_neighbors(std::vector<typename Elem::index_type> &array,
                     typename Elem::index_type edge,
                     typename DElem::index_type idx) const
  {
    return(get_elem_neighbors(array,edge,idx));
  }

  void get_neighbors(std::vector<typename Elem::index_type> &array,
                     typename Elem::index_type idx) const
  {
    get_elem_neighbors(array,idx);
  }

  /// Locate a point in a mesh, find which is the closest node
  bool locate(typename Node::index_type &i, const Core::Geometry::Point &p) const
  { return(locate_node(i,p)); }
  bool locate(typename Edge::index_type &i, const Core::Geometry::Point &p) const
  { return(locate_elem(i,p)); }
  bool locate(typename Face::index_type &, const Core::Geometry::Point &) const
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  bool locate(typename Cell::index_type &, const Core::Geometry::Point &) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }


  /// These should become obsolete soon, they do not follow the concept
  /// of the basis functions....
  int get_weights(const Core::Geometry::Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point &p, typename Edge::array_type &l, double *w);

  int get_weights(const Core::Geometry::Point &, typename Face::array_type &, double *)
  { ASSERTFAIL("This mesh type does not have faces use \"elem\"."); }
  int get_weights(const Core::Geometry::Point &, typename Cell::array_type &, double *)
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  /// Access the nodes of the mesh
  void get_point(Core::Geometry::Point &result, typename Node::index_type idx) const
    { get_center(result,idx); }
  void set_point(const Core::Geometry::Point &point, typename Node::index_type index)
    { points_[index] = point; }
  void get_random_point(Core::Geometry::Point &p, typename Elem::index_type i, FieldRNG &r) const;

  /// Normals for visualizations
  void get_normal(Core::Geometry::Vector&, typename Node::index_type) const
    { ASSERTFAIL("CurveMesh: This mesh type does not have node normals."); }

  template<class VECTOR, class INDEX1, class INDEX2>
  void get_normal(Core::Geometry::Vector&, VECTOR&, INDEX1, INDEX2) const
    { ASSERTFAIL("CurveMesh: This mesh type does not have element normals."); }

  /// use these to build up a new contour mesh
  typename Node::index_type add_node(const Core::Geometry::Point &p)
  {
    points_.push_back(p);
    return static_cast<under_type>(points_.size() - 1);
  }

  typename Node::index_type add_point(const Core::Geometry::Point &point)
    { return add_node(point); }

  typename Edge::index_type add_edge(typename Node::index_type i1,
                                     typename Node::index_type i2)
  {
    edges_.push_back(i1);
    edges_.push_back(i2);
    return static_cast<index_type>((edges_.size()>>1)-1);
  }

  template <class ARRAY>
  typename Elem::index_type add_elem(ARRAY a)
  {
    ASSERTMSG(a.size() == 2, "Tried to add non-line element.");
    edges_.push_back(static_cast<typename Node::index_type>(a[0]));
    edges_.push_back(static_cast<typename Node::index_type>(a[1]));
    return static_cast<index_type>((edges_.size()>>1)-1);
  }

  /// Functions to improve memory management. Often one knows how many
  /// nodes/elements one needs, prereserving memory is often possible.
  void node_reserve(size_type s) { points_.reserve(static_cast<size_t>(s)); }
  void elem_reserve(size_type s) { edges_.reserve(static_cast<size_t>(2*s)); }
  void resize_nodes(size_type s) { points_.resize(static_cast<size_t>(s)); }
  void resize_elems(size_type s) { edges_.resize(static_cast<size_t>(2*s)); }

  /// Get the local coordinates for a certain point within an element
  /// This function uses a couple of newton iterations to find the local
  /// coordinate of a point
  template<class VECTOR, class INDEX>
  bool get_coords(VECTOR &coords,const Core::Geometry::Point &p,INDEX idx) const
  {
    ElemData ed(*this, idx);
    return basis_.get_coords(coords, p, ed);
  }

  /// Find the location in the global coordinate system for a local coordinate
  /// This function is the opposite of get_coords.
  template<class VECTOR, class INDEX>
  void interpolate(Core::Geometry::Point &pt, const VECTOR &coords, INDEX idx) const
  {
    ElemData ed(*this, idx);
    pt = basis_.interpolate(coords, ed);
  }

  /// Interpolate the derivate of the function, This infact will return the
  /// jacobian of the local to global coordinate transformation. This function
  /// is mainly intended for the non linear elements
  template<class VECTOR1, class INDEX, class VECTOR2>
  void derivate(const VECTOR1 &coords, INDEX idx, VECTOR2 &J) const
  {
    ElemData ed(*this, idx);
    basis_.derivate(coords, ed, J);
  }

  /// Get the determinant of the jacobian, which is the local volume of an element
  /// and is intended to help with the integration of functions over an element.
  template<class VECTOR, class INDEX>
  double det_jacobian(const VECTOR& coords, INDEX idx) const
  {
    double J[9];
    jacobian(coords,idx,J);
    return (DetMatrix3x3(J));
  }

  /// Get the jacobian of the transformation. In case one wants the non inverted
  /// version of this matrix. This is currentl here for completeness of the
  /// interface
  template<class VECTOR, class INDEX>
  void jacobian(const VECTOR& coords, INDEX idx, double* J) const
  {
    StackVector<Core::Geometry::Point,1> Jv;
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
  }

  /// Get the inverse jacobian of the transformation. This one is needed to
  /// translate local gradients into global gradients. Hence it is crucial for
  /// calculating gradients of fields, or constructing finite elements.
  template<class VECTOR, class INDEX>
  double inverse_jacobian(const VECTOR& coords, INDEX idx, double* Ji) const
  {
    StackVector<Core::Geometry::Point,1> Jv;
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

  template<class INDEX>
  double scaled_jacobian_metric(INDEX idx) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    basis_.derivate(basis_.unit_center,ed,Jv);
    Jv.resize(3);
    Core::Geometry::Vector v,w;
    Core::Geometry::Vector(Jv[0]).find_orthogonal(v,w);
    Jv[1] = v.asPoint();
    Jv[2] = w.asPoint();
    double min_jacobian = ScaledDetMatrix3P(Jv);

    size_type num_vertices = static_cast<size_type>(basis_.number_of_vertices());
    for (size_type j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
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

    basis_.derivate(basis_.unit_center,ed,Jv);
    Jv.resize(3);
    Core::Geometry::Vector v,w;
    Core::Geometry::Vector(Jv[0]).find_orthogonal(v,w);
    Jv[1] = v.asPoint();
    Jv[2] = w.asPoint();
    double min_jacobian = DetMatrix3P(Jv);

    size_type num_vertices = static_cast<size_type>(basis_.number_of_vertices());
    for (size_type j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
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
    typename Node::size_type sz; size(sz);

    typename Node::iterator ni; begin(ni);
    typename Node::iterator nie; end(nie);

    /// If there are no nodes we cannot find a closest point
    if (sz == 0) return (false);

    /// Check first guess
    if (idx >= 0 && idx < sz)
    {
      if ((p - points_[idx]).length2() < epsilon2_) return (true);
    }

    double mindist = DBL_MAX;

    while(ni != nie)
    {
      double dist = (p-points_[*ni]).length2();
      if ( dist < mindist )
      {
        mindist = dist;
        idx = static_cast<INDEX>(*ni);

        /// Exit if we cannot find one that is closer
        if (mindist < epsilon2_ ) return (true);
      }
      ++ni;
    }

    return (true);
  }


  template <class INDEX>
  bool locate_elem(INDEX &idx, const Core::Geometry::Point &p) const
  {
    if (basis_.polynomial_order() > 1) return elem_locate(idx, *this, p);

    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find a closest point
    if (sz == 0) return (false);

    double alpha;

    /// Check whether the estimate given in idx is the point we are looking for
    if (idx >= 0 && idx < sz)
    {
      if (inside2_p(idx,p,alpha)) return (true);
    }

    /// Loop over all nodes to find one that is located inside
    typename Elem::iterator ei; begin(ei);
    typename Elem::iterator eie; end(eie);

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

  template <class ARRAY>
  bool locate_elems(ARRAY &array, const Core::Geometry::BBox &b) const
  {
    array.clear();

    /// Loop over all nodes to find one
    typename Elem::iterator ei; begin(ei);
    typename Elem::iterator eie; end(eie);
    typename Node::array_type nodes;

    while (ei != eie)
    {
      get_nodes(nodes,*ei);
      Core::Geometry::BBox be(points_[nodes[0]],points_[nodes[1]]);
      if (b.intersect(be) != Core::Geometry::BBox::OUTSIDE)
      {
        size_t p=0;
        for (;p<array.size();p++) if (array[p] == typename ARRAY::value_type(*ei)) break;
        if (p == array.size()) array.push_back(typename ARRAY::value_type(*ei));
      }
      ++ei;
    }

    return (array.size() > 0);
  }



  template <class INDEX, class ARRAY>
  bool locate_elem(INDEX &idx, ARRAY& coords, const Core::Geometry::Point &p) const
  {
    if (basis_.polynomial_order() > 1) return elem_locate(idx, *this, p);

    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find a closest point
    if (sz == 0) return (false);

    double alpha;
    coords.resize(1);

    /// Check whether the estimate given in idx is the point we are looking for
    if (idx >= 0 && idx < sz)
    {
      if (inside2_p(idx,p,coords[0])) return (true);
    }

    /// Loop over all nodes to find one that finds
    typename Elem::iterator ei; begin(ei);
    typename Elem::iterator eie; end(eie);

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


  /// Find the closest element to a point
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
    typename Node::size_type sz; size(sz);

    /// If there are no nodes we cannot find the closest one
    if (sz == 0) return (false);

    Core::Geometry::Point r;
    double dist;

    if (idx >= 0 && idx < sz)
    {
      r = points_[idx];
      dist = (point-r).length2();

      if ( dist < epsilon2_ )
      {
        result = point;
        pdist = sqrt(dist);
        return (true);
      }
    }

    typename Node::iterator ni; begin(ni);
    typename Node::iterator nie; end(nie);

    double mindist = maxdist;

    while(ni != nie)
    {
      r = points_[*ni];
      dist = (point-r).length2();

      if ( dist < mindist )
      {
        mindist = dist;
        idx = static_cast<INDEX>(*ni);
        result = r;
        if (mindist < epsilon2_)
        {
          pdist = sqrt(mindist);
          return (true);
        }
      }
      ++ni;
    }

    if (mindist >= maxdist) return (false);

    pdist = sqrt(mindist);
    return (true);
  }

  template <class ARRAY>
  bool find_closest_nodes(ARRAY &nodes, const Core::Geometry::Point &point, double maxdist) const
  {
    nodes.clear();
    double maxdist2 = maxdist*maxdist;

    typename Node::iterator ni; begin(ni);
    typename Node::iterator nie; end(nie);

    while(ni != nie)
    {
      double dist = (point-points_[*ni]).length2();

      if ( dist < maxdist2 )
      {
        nodes.push_back(static_cast<typename ARRAY::value_type>(*ni));
      }
      ++ni;
    }

    return (nodes.size() > 0);
  }


  template <class ARRAY1, class ARRAY2>
  bool find_closest_nodes(ARRAY1 &distances,
                          ARRAY2 &nodes,
                          const Core::Geometry::Point &point, double maxdist) const
  {
    distances.clear();
    nodes.clear();
    double maxdist2 = maxdist*maxdist;

    typename Node::iterator ni; begin(ni);
    typename Node::iterator nie; end(nie);

    while(ni != nie)
    {
      double dist = (point-points_[*ni]).length2();

      if ( dist < maxdist2 )
      {
        nodes.push_back(static_cast<typename ARRAY2::value_type>(*ni));
        distances.push_back(static_cast<typename ARRAY1::value_type>(dist));
      }
      ++ni;
    }

    return (nodes.size() > 0);
  }

  /// Find the closest element to a point
  template <class INDEX, class ARRAY>
  bool find_closest_elem(double &pdist,
                         Core::Geometry::Point &result,
                         ARRAY &coords,
                         INDEX &idx,
                         const Core::Geometry::Point &p) const
  {
    return (find_closest_elem(pdist,result,coords,idx,p,-1.0));
  }

  /// Find the closest element to a point
  template <class INDEX, class ARRAY>
  bool find_closest_elem(double &pdist,
                         Core::Geometry::Point &result,
                         ARRAY &coords,
                         INDEX &idx,
                         const Core::Geometry::Point &p,
                         double maxdist) const
  {
    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;

    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find the closest one
    if (sz == 0) return (false);

    double dist;

    coords.resize(1);
    /// Test the one in idx
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

    double mindist = maxdist;
    Core::Geometry::Point res;

    typename Elem::iterator ni; begin(ni);
    typename Elem::iterator nie; end(nie);

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

    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find the closest one
    if (sz == 0) return (false);

    typename Elem::iterator ni; begin(ni);
    typename Elem::iterator nie; end(nie);

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

  double get_epsilon() const
  { return (epsilon_); }

  /// Export this class using the old Pio system
  virtual void io(Piostream&);

  ///////////////////////////////////////////////////
  // STATIC VARIABLES AND FUNCTIONS

  /// These IDs are created as soon as this class will be instantiated
  /// The first one is for Pio and the second for the virtual interface
  /// These are currently different as they serve different needs.
  static PersistentTypeID curvemesh_typeid;

  /// Core functionality for getting the name of a templated mesh class
  static const std::string type_name(int n = -1);
  virtual std::string dynamic_type_name() const { return curvemesh_typeid.type; }

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
  static Persistent *maker() { return new CurveMesh<Basis>(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return boost::make_shared<CurveMesh<Basis>>(); }


  /// Functions local to CurveMesh, the latter are not thread safe
  /// THESE ARE NOT WELL INTEGRATED YET
  typename Node::index_type delete_node(typename Node::index_type i1)
  {
    CHECKARRAYBOUNDS(static_cast<index_type>(i1),
		     static_cast<index_type>(0),
		     static_cast<index_type>(points_.size()));

    std::vector<Core::Geometry::Point>::iterator niter;
    niter = points_.begin() + i1;
    points_.erase(niter);
    return static_cast<typename Node::index_type>(points_.size() - 1);
  }

  typename Node::index_type delete_nodes(typename Node::index_type i1,
					 typename Node::index_type i2)
  {
    CHECKARRAYBOUNDS(static_cast<index_type>(i1),
		     static_cast<index_type>(0),
		     static_cast<index_type>(points_.size()));

    CHECKARRAYBOUNDS(static_cast<index_type>(i2),
		     static_cast<index_type>(0),
		     static_cast<index_type>(points_.size()+1));

    std::vector<Core::Geometry::Point>::iterator niter1;
    niter1 = points_.begin() + i1;

    std::vector<Core::Geometry::Point>::iterator niter2;
    niter2 = points_.begin() + i2;

    points_.erase(niter1, niter2);
    return static_cast<typename Node::index_type>(points_.size() - 1);
  }

  typename Edge::index_type delete_edge(typename Edge::index_type i1)
  {
    CHECKARRAYBOUNDS(static_cast<index_type>(i1),
		     static_cast<index_type>(0),
		     static_cast<index_type>(edges_.size()>>1));

    typename std::vector<index_type>::iterator niter1;
    niter1 = edges_.begin() + 2*i1;

    typename std::vector<index_type>::iterator niter2;
    niter2 = edges_.begin() + 2*i1+2;

    edges_.erase(niter1, niter2);
    return static_cast<typename Edge::index_type>((edges_.size()>>1) - 1);
  }

  typename Edge::index_type delete_edges(typename Edge::index_type i1,
					 typename Edge::index_type i2)
  {
    CHECKARRAYBOUNDS(static_cast<index_type>(i1),
		     static_cast<index_type>(0),
		     static_cast<index_type>(edges_.size()>>1));

    CHECKARRAYBOUNDS(static_cast<index_type>(i2),
		     static_cast<index_type>(0),
		     static_cast<index_type>((edges_.size()>>1)+1));

    typename std::vector<index_type>::iterator niter1;
    niter1 = edges_.begin() + 2*i1;

    typename std::vector<index_type>::iterator niter2;
    niter2 = edges_.begin() + 2*i2;

    edges_.erase(niter1, niter2);

    return static_cast<typename Edge::index_type>((edges_.size()>>1) - 1);
  }

protected:
  //////////////////////////////////////////////////////////////
  // These functions are templates and are used to define the
  // dynamic compilation interface and the virtual interface
  // as they both use different datatypes as indices and arrays
  // the following functions have been templated and are inlined
  // at the places where they are needed.
  //
  // Secondly these templates allow for the use of the stack vector
  // as well as the STL vector. When an algorithm supports non linear
  // functions an STL vector is a better choice, in the other cases
  // often a StackVector is enough (The latter improves performance).

  template<class ARRAY, class INDEX>
  inline void get_nodes_from_edge(ARRAY& array, INDEX idx) const
  {
    array.resize(2);
    array[0] = static_cast<typename ARRAY::value_type>(edges_[2*idx]);
    array[1] = static_cast<typename ARRAY::value_type>(edges_[2*idx+1]);
  }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
      "CurveMesh: Must call synchronize NODE_NEIGHBORS_E on CurveMesh first");

    array.resize(node_neighbors_[idx].size());
    for (typename NodeNeighborMap::size_type i = 0; i < node_neighbors_[idx].size(); ++i)
      array[i] =
        static_cast<typename ARRAY::value_type>(node_neighbors_[idx][i]);
  }

  template<class ARRAY, class INDEX>
  inline void get_nodes_from_elem(ARRAY& array, INDEX idx) const
  {
    get_nodes_from_edge(array,idx);
  }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_elem(ARRAY& array, INDEX idx) const
  {
    array.resize(1); array[0] = typename ARRAY::value_type(idx);
  }


  template <class ARRAY, class INDEX>
  inline void set_nodes_by_elem(ARRAY &array, INDEX idx)
  {
    for (index_type n = 0; n < 2; ++n)
      edges_[idx * 2 + n] = static_cast<index_type>(array[n]);
  }

  template <class INDEX1, class INDEX2>
  bool
  get_elem_neighbor(INDEX1 &neighbor, INDEX1 edge,INDEX2 node) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
                  "Must call synchronize NODE_NEIGHBORS_E on CurveMesh first");

    if (node_neighbors_[node].size() > 1)
    {
      if (node_neighbors_[node][0] == edge)
        neighbor = static_cast<INDEX1>(node_neighbors_[node][1]);
      else neighbor = static_cast<INDEX1>(node_neighbors_[node][0]);
      return (true);
    }
    return (false);
  }


  template <class ARRAY, class INDEX>
  void
  get_node_neighbors(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
                  "Must call synchronize NODE_NEIGHBORS_E on CurveMesh first");
    array.resize(node_neighbors_[idx].size());
    for (typename NodeNeighborMap::size_type p=0;p<node_neighbors_[idx].size();p++)
    {
      if (edges_[2*(node_neighbors_[idx][p])] == idx)
        array[p] = static_cast<typename ARRAY::value_type>(edges_[2*(node_neighbors_[idx][p])+1]);
      else
        array[p] = static_cast<typename ARRAY::value_type>(edges_[2*(node_neighbors_[idx][p])]);
    }
  }


  template <class ARRAY, class INDEX1, class INDEX2>
  bool
  get_elem_neighbors(ARRAY &array, INDEX1 /*edge*/, INDEX2 idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
                  "Must call synchronize NODE_NEIGHBORS_E on CurveMesh first");

    typename NodeNeighborMap::size_type sz = node_neighbors_[idx].size();
    if (sz < 1) return (false);

    array.clear();
    array.reserve(sz-1);
    for (typename NodeNeighborMap::size_type i=0; i<sz;i++)
    {
      if (node_neighbors_[idx][i] != static_cast<typename ARRAY::value_type>(idx))
          array.push_back(typename ARRAY::value_type(node_neighbors_[idx][i]));
    }
    return (true);
  }

  template <class ARRAY, class INDEX>
  void
  get_elem_neighbors(ARRAY &array, INDEX idx) const

  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
                  "Must call synchronize NODE_NEIGHBORS_E on CurveMesh first");
    typename Node::index_type n1 = edges_[2*idx];
    typename Node::index_type n2 = edges_[2*idx+1];

    typename NodeNeighborMap::size_type s1 = node_neighbors_[n1].size();
    typename NodeNeighborMap::size_type s2 = node_neighbors_[n2].size();

    array.clear();
    array.reserve(s1+s2-2);
    for (typename NodeNeighborMap::size_type i=0; i<s1;i++)
    {
      if (node_neighbors_[n1][i] != idx)
        array.push_back(typename ARRAY::value_type(node_neighbors_[n1][i]));
    }
    for (typename NodeNeighborMap::size_type i=0; i<s2;i++)
    {
      if (node_neighbors_[n2][i] != idx)
        array.push_back(typename ARRAY::value_type(node_neighbors_[n2][i]));
    }
  }

  bool inside2_p(index_type idx, const Core::Geometry::Point &p, double& coord) const;
  double distance2_p(index_type idx, const Core::Geometry::Point &p,
                     Core::Geometry::Point& projection, double& coord) const;

  void compute_node_neighbors();

  //////////////////////////////////////////////////////////////
  // Actual data stored in the mesh

  /// Vector with the node locations
  std::vector<Core::Geometry::Point>           points_;
  /// Vector with connectivity data
  std::vector<index_type>      edges_;
  /// The basis function, contains additional information on elements
  Basis                   basis_;

  /// Record which parts of the mesh are synchronized
  mask_type               synchronized_;
  /// Lock to synchronize between threads
  mutable Core::Thread::Mutex           synchronize_lock_;

  /// Vector indicating which edges are conected to which
  /// node. This is the reverse of the connectivity data
  /// stored in the edges_ array.
  typedef std::vector<std::vector<typename Edge::index_type> > NodeNeighborMap;
  NodeNeighborMap         node_neighbors_;
  Core::Geometry::BBox                    bbox_;
  double                  epsilon_;
  double                  epsilon2_;
  double                  epsilon3_;

  /// Pointer to virtual interface
  /// This one is created as soon as the mesh is generated
  /// Put this one in a handle as we have a virtual destructor
  boost::shared_ptr<VMesh>           vmesh_;
};


template<class Basis>
CurveMesh<Basis>::CurveMesh() :
  synchronized_(ALL_ELEMENTS_E),
  synchronize_lock_("CurveMesh Lock"),
  epsilon_(0.0),
  epsilon2_(0.0)
{
  DEBUG_CONSTRUCTOR("CurveMesh")
  /// Initialize the virtual interface when the mesh is created
  vmesh_.reset(CreateVCurveMesh(this));
}

template<class Basis>
CurveMesh<Basis>::CurveMesh(const CurveMesh &copy) :
  Mesh(copy),
  points_(copy.points_),
  edges_(copy.edges_),
  basis_(copy.basis_),
  synchronized_(copy.synchronized_),
  synchronize_lock_("CurveMesh Lock")
{
  DEBUG_CONSTRUCTOR("CurveMesh")

  /// We need to lock before we can copy these as these
  /// structures are generate dynamically when they are
  /// needed.
  copy.synchronize_lock_.lock();
  node_neighbors_ = copy.node_neighbors_;
  synchronized_ |= copy.synchronized_ & NODE_NEIGHBORS_E;

  // Make sure we got the synchronized version
  epsilon_ = copy.epsilon_;
  epsilon2_ = copy.epsilon2_;

  copy.synchronize_lock_.unlock();

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  vmesh_.reset(CreateVCurveMesh(this));
}

template<class Basis>
CurveMesh<Basis>::~CurveMesh()
{
  DEBUG_DESTRUCTOR("CurveMesh")
}



template <class Basis>
const TypeDescription* get_type_description(CurveMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("CurveMesh", subs,
      std::string(__FILE__),
      "SCIRun",
      TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
CurveMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((CurveMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
CurveMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((CurveMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Node",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
CurveMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((CurveMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Edge",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
CurveMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((CurveMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Face",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
CurveMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((CurveMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Cell",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


/// Add an entry into the database for Pio
template <class Basis>
PersistentTypeID
CurveMesh<Basis>::curvemesh_typeid(type_name(-1), "Mesh", CurveMesh<Basis>::maker);

//////////////////////////////////////////////////////////////////////
// Functions in the mesh

template <class Basis>
Core::Geometry::BBox
CurveMesh<Basis>::get_bounding_box() const
{
  Core::Geometry::BBox result;

  // Compute bounding box
  typename Node::iterator ni, nie;
  begin(ni);
  end(nie);
  while (ni != nie)
  {
    result.extend(points_[*ni]);
    ++ni;
  }

  return result;
}

template <class Basis>
void
CurveMesh<Basis>::get_canonical_transform(Core::Geometry::Transform &t) const
{
  t.load_identity();
  Core::Geometry::BBox bbox = get_bounding_box();
  t.pre_scale(bbox.diagonal());
  t.pre_translate(Core::Geometry::Vector(bbox.get_min()));
}

template <class Basis>
void
CurveMesh<Basis>::compute_bounding_box()
{
  bbox_.reset();

  // Compute bounding box
  typename Node::iterator ni, nie;
  begin(ni);
  end(nie);
  while (ni != nie)
  {
    bbox_.extend(points_[*ni]);
    ++ni;
  }

  // Compute epsilons associated with the bounding box
  epsilon_ = bbox_.diagonal().length()*1e-8;
  epsilon2_ = epsilon_*epsilon_;
  epsilon3_ = epsilon_*epsilon_*epsilon_;

  synchronize_lock_.lock();
  synchronized_ |= Mesh::BOUNDING_BOX_E;
  synchronize_lock_.unlock();
}

template <class Basis>
void
CurveMesh<Basis>::transform(const Core::Geometry::Transform &t)
{
  auto itr = points_.begin();
  auto eitr = points_.end();
  while (itr != eitr)
  {
    *itr = t.project(*itr);
    ++itr;
  }

  /// If we have nodes on the edges they should be transformed in
  /// the same way
  size_type num_enodes = static_cast<size_type>(basis_.size_node_values());
  for (size_type i=0; i<num_enodes; i++)
  {
    Core::Geometry::Point p;
    basis_.get_node_value(p,i);
    p =t.project(p);
    basis_.set_node_value(p,i);
  }

  /// Projecting derivates is more difficult
}



template <class Basis>
double
CurveMesh<Basis>::get_size(typename Edge::index_type idx) const
{
  ElemData ed(*this, idx);
  std::vector<Core::Geometry::Point> pledge;
  std::vector<std::vector<double> > coords;
  // Perhaps there is a better choice for the number of divisions.
  pwl_approx_edge(coords, idx, 0, 5);

  double total = 0.0L;
  std::vector<std::vector<double> >::iterator iter = coords.begin();
  std::vector<std::vector<double> >::iterator last = coords.begin() + 1;
  while (last != coords.end())
  {
    std::vector<double> &c0 = *iter++;
    std::vector<double> &c1 = *last++;
    Core::Geometry::Point p0 = basis_.interpolate(c0, ed);
    Core::Geometry::Point p1 = basis_.interpolate(c1, ed);
    total += (p1 - p0).length();
  }
  return total;
}


template <class Basis>
void
CurveMesh<Basis>::get_center(Core::Geometry::Point &result,
                             typename Edge::index_type idx) const
{
  ElemData cmcd(*this, idx);
  std::vector<double> coord(1,0.5L);
  result =  basis_.interpolate(coord, cmcd);
}


template <class Basis>
int
CurveMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Node::array_type &l,
                              double *w)
{
  typename Edge::index_type idx;
  if (locate(idx, p))
  {
    get_nodes(l,idx);
    StackVector<double,1> coords(1);
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
CurveMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Edge::array_type &l,
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
CurveMesh<Basis>::get_random_point(Core::Geometry::Point &p,
                                   typename Elem::index_type ei,
                                   FieldRNG &rng) const
{
  const Core::Geometry::Point &p0 = points_[edges_[2*ei]];
  const Core::Geometry::Point &p1 = points_[edges_[2*ei+1]];

  p = p0 + (p1 - p0) * rng();
}


template <class Basis>
bool
CurveMesh<Basis>::synchronize(mask_type sync)
{
  synchronize_lock_.lock();

  /// Test whether we need to synchronize for any of the neighor function
  /// calls. If so compute the node_neighbor_ array, which we can use
  /// for the element neighbors as well.

  if (sync & (Mesh::NODE_NEIGHBORS_E|Mesh::ELEM_NEIGHBORS_E)
      && !(synchronized_ & Mesh::NODE_NEIGHBORS_E))
  {
    compute_node_neighbors();
  }

  if (sync & (Mesh::EPSILON_E|Mesh::LOCATE_E) && !(synchronized_ & Mesh::EPSILON_E))
  {
    if( points_.size() )
      epsilon_ = get_bounding_box().diagonal().length()*1e-8;

    epsilon2_ = epsilon_ * epsilon_;
    synchronized_ |= Mesh::EPSILON_E;
    synchronized_ |= Mesh::LOCATE_E;
  }

  synchronize_lock_.unlock();
  return (true);
}


template <class Basis>
bool
CurveMesh<Basis>::unsynchronize(mask_type /*sync*/)
{
  return (true);
}

template <class Basis>
bool
CurveMesh<Basis>::clear_synchronization()
{
  // Undo marking the synchronization
  synchronize_lock_.lock();
  synchronized_ = Mesh::NODES_E | Mesh::EDGES_E | Mesh::ELEMS_E;
  // Free memory where possible
  node_neighbors_.clear();
  synchronize_lock_.unlock();
  return (true);
}

/// Compute the inverse route of connectivity: from node to edge
template <class Basis>
void
CurveMesh<Basis>::compute_node_neighbors()
{
  node_neighbors_.clear();
  node_neighbors_.resize(points_.size());
  index_type i;
  size_type  num_elems = (edges_.size()>>1);
  for (i = 0; i < num_elems; i++)
  {
    node_neighbors_[edges_[2*i]].push_back(i);
    node_neighbors_[edges_[2*i+1]].push_back(i);
  }
  synchronized_ |= Mesh::NODE_NEIGHBORS_E;
}

#define CURVE_MESH_VERSION 3

template <class Basis>
void
CurveMesh<Basis>::io(Piostream& stream)
{
  int version = stream.begin_class(type_name(-1), CURVE_MESH_VERSION);

  Mesh::io(stream);

  // IO data members, in order
  Pio(stream, points_);
  if (version < 3)
  {
    // NOTE: This one is unsigned int for backwards compatibility from before we
    // used index_type
    std::vector<std::pair<unsigned int,unsigned int> > tmp;
    Pio(stream,tmp);
    edges_.resize(tmp.size()*2);
    for (std::vector<std::pair<unsigned int,unsigned int> >::size_type j=0;j<tmp.size();j++)
    {
      edges_[2*j] = tmp[j].first;
      edges_[2*j+1] = tmp[j].second;
    }
  }
  else
  {
    Pio_index(stream, edges_);
  }
  if (version >= 2)
  {
    basis_.io(stream);
  }
  stream.end_class();

  if (stream.reading())
    vmesh_.reset(CreateVCurveMesh(this));
}


template <class Basis>
const std::string
CurveMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("CurveMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
void
CurveMesh<Basis>::begin(typename CurveMesh<Basis>::Node::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
CurveMesh<Basis>::end(typename CurveMesh<Basis>::Node::iterator &itr) const
{
  itr = static_cast<index_type>(points_.size());
}


template <class Basis>
void
CurveMesh<Basis>::begin(typename CurveMesh<Basis>::Edge::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
CurveMesh<Basis>::end(typename CurveMesh<Basis>::Edge::iterator &itr) const
{
  itr = static_cast<index_type>(edges_.size()>>1);
}


template <class Basis>
void
CurveMesh<Basis>::begin(typename CurveMesh<Basis>::Face::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
CurveMesh<Basis>::end(typename CurveMesh<Basis>::Face::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
CurveMesh<Basis>::begin(typename CurveMesh<Basis>::Cell::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
CurveMesh<Basis>::end(typename CurveMesh<Basis>::Cell::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
CurveMesh<Basis>::size(typename CurveMesh<Basis>::Node::size_type &s) const
{
  s = static_cast<size_type>(points_.size());
}


template <class Basis>
void
CurveMesh<Basis>::size(typename CurveMesh<Basis>::Edge::size_type &s) const
{
  s = static_cast<size_type>(edges_.size()>>1);
}


template <class Basis>
void
CurveMesh<Basis>::size(typename CurveMesh<Basis>::Face::size_type &s) const
{
  s = 0;
}


template <class Basis>
void
CurveMesh<Basis>::size(typename CurveMesh<Basis>::Cell::size_type &s) const
{
  s = 0;
}


template <class Basis>
bool
CurveMesh<Basis>::inside2_p(index_type i, const Core::Geometry::Point &p, double& alpha) const
{
  const index_type j = 2*i;
  const Core::Geometry::Point &p0 = points_[edges_[j]];
  const Core::Geometry::Point &p1 = points_[edges_[j+1]];

  const Core::Geometry::Vector v = p0-p1;
  alpha = Dot(p0-p,v)/v.length2();

  Core::Geometry::Point point;
  if (alpha < 0.0) { point = p0; alpha = 0.0; }
  else if (alpha > 1.0) { point = p1; alpha = 1.0; }
  else { point = (alpha*p1 + (1.0-alpha)*p0).asPoint(); }

  if ((point - p).length2() < epsilon2_) return (true);

  return (false);
}


template <class Basis>
double
CurveMesh<Basis>::distance2_p(index_type i, const Core::Geometry::Point& p,
                              Core::Geometry::Point& result, double& alpha) const
{
  const index_type j = 2*i;
  const Core::Geometry::Point &p0 = points_[edges_[j]];
  const Core::Geometry::Point &p1 = points_[edges_[j+1]];

  const Core::Geometry::Vector v = p0-p1;
  alpha = Dot(p0-p,v)/v.length2();

  if (alpha < 0.0) { result = p0; alpha = 0.0;}
  else if (alpha > 1.0) { result = p1; alpha = 1.0; }
  else { result = (alpha*p1 + (1.0-alpha)*p0).asPoint(); }

  double dist = (result - p).length2();
  return (dist);
}

} // namespace SCIRun

#endif
