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


#ifndef CORE_DATATYPES_TRISURFMESH_H
#define CORE_DATATYPES_TRISURFMESH_H 1

/// Include what kind of support we want to have
/// Need to fix this and couple it sci-defs
#include <Core/Datatypes/Legacy/Field/MeshSupport.h>

#include <Core/Containers/StackVector.h>

#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/CompGeom.h>
#include <Core/Containers/StackVector.h>
#include <Core/GeometryPrimitives/SearchGridT.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Basis/TriQuadraticLgn.h>
#include <Core/Basis/TriCubicHmt.h>

#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldIterator.h>
#include <Core/Datatypes/Legacy/Field/FieldRNG.h>
#include <Core/Datatypes/Legacy/Base/Types.h>

#include <Core/Thread/Mutex.h>
#include <Core/Thread/ConditionVariable.h>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <set>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

/////////////////////////////////////////////////////
// Declarations for virtual interface

/// Functions for creating the virtual interface
/// Declare the functions that instantiate the virtual interface
template<class MESH> class TriSurfMesh;

/// make sure any other mesh other than the preinstantiate ones
/// returns no virtual interface. Altering this behavior will allow
/// for dynamically compiling the interface if needed.
template<class MESH>
VMesh* CreateVTriSurfMesh(MESH*) { return (0); }

#if (SCIRUN_TRISURF_SUPPORT > 0)
/// Declare that these can be found in a library that is already
/// precompiled. So dynamic compilation will not instantiate them again.
SCISHARE VMesh* CreateVTriSurfMesh(TriSurfMesh<Core::Basis::TriLinearLgn<Core::Geometry::Point> >* mesh);
#if (SCIRUN_QUADRATIC_SUPPORT > 0)
SCISHARE VMesh* CreateVTriSurfMesh(TriSurfMesh<Core::Basis::TriQuadraticLgn<Core::Geometry::Point> >* mesh);
#endif
#if (SCIRUN_CUBIC_SUPPORT > 0)
SCISHARE VMesh* CreateVTriSurfMesh(TriSurfMesh<Core::Basis::TriCubicHmt<Core::Geometry::Point> >* mesh);
#endif

#endif
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
// Declarations for TriSurfMesh class

template <class Basis>
class TriSurfMesh : public Mesh
{

/// Make sure the virtual interface has access
template<class MESH> friend class VTriSurfMesh;
template<class MESH> friend class VMeshShared;
template<class MESH> friend class VUnstructuredMesh;

public:
  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type                 under_type;
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::size_type                  size_type;
  typedef SCIRun::mask_type                  mask_type;

  typedef boost::shared_ptr<TriSurfMesh<Basis> > handle_type;
  typedef Basis                              basis_type;

  /// Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef NodeIndex<under_type>       index_type;
    typedef NodeIterator<under_type>    iterator;
    typedef NodeIndex<under_type>       size_type;
    typedef StackVector<index_type, 4>  array_type;
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

  /// Elem refers to the most complex topological object
  /// DElem refers to object just below Elem in the topological hierarchy
  typedef Face Elem;
  typedef Edge DElem;

  /// Somehow the information of how to interpolate inside an element
  /// ended up in a separate class, as they need to share information
  /// this construction was created to transfer data.
  /// Hopefully in the future this class will disappear again.
  friend class ElemData;

  class ElemData
  {
  public:
    typedef typename TriSurfMesh<Basis>::index_type index_type;

    ElemData(const TriSurfMesh<Basis>& msh, const index_type ind) :
      mesh_(msh),
      index_(ind)
    {
      /// Linear and Constant Basis never use edges_
      if (basis_type::polynomial_order() > 1) {
        mesh_.get_edges_from_face(edges_, index_);
      }
    }

    // the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const {
      return mesh_.faces_[index_ * 3];
    }
    inline
    index_type node1_index() const {
      return mesh_.faces_[index_ * 3 + 1];
    }
    inline
    index_type node2_index() const {
      return mesh_.faces_[index_ * 3 + 2];
    }

    // the following designed to coordinate with ::get_edges
    inline
    index_type edge0_index() const {
      return edges_[0];
    }
    inline
    index_type edge1_index() const {
      return edges_[1];
    }
    inline
    index_type edge2_index() const {
      return edges_[2];
    }

    inline
    index_type elem_index() const {
      return index_;
    }

    inline
    const Core::Geometry::Point &node0() const {
      return mesh_.points_[node0_index()];
    }
    inline
    const Core::Geometry::Point &node1() const {
      return mesh_.points_[node1_index()];
    }
    inline
    const Core::Geometry::Point &node2() const {
      return mesh_.points_[node2_index()];
    }

  private:
    /// reference to the mesh
    const TriSurfMesh<Basis>          &mesh_;
    /// copy of element index
    const index_type                  index_;
    /// need edges for quadratic meshes
    typename Edge::array_type         edges_;
  };


  friend class Synchronize;

  class Synchronize /*: public Runnable*/
  {
    public:
      Synchronize(TriSurfMesh<Basis>* mesh, mask_type sync) :
        mesh_(mesh), sync_(sync) {}

      void run()
      {
        this->operator()();
      }
      void operator()()
      {
        mesh_->synchronize_lock_.lock();
        // block out all the tables that are already synchronized
        sync_ &= ~(mesh_->synchronized_);
        // block out all the tables that are already being computed
        sync_ &= ~(mesh_->synchronizing_);
        // Now sync_ contains what this thread will synchronize
        // Denote what this thread will synchronize
        mesh_->synchronizing_ |= sync_;
        // Other threads now know what this tread will be doing
        mesh_->synchronize_lock_.unlock();

        // Sync node neighbors
        if (sync_ & Mesh::ELEM_NEIGHBORS_E) mesh_->compute_edge_neighbors();
        if (sync_ & Mesh::NODE_NEIGHBORS_E) mesh_->compute_node_neighbors();
        if (sync_ & Mesh::EDGES_E) mesh_->compute_edges_bugfix();
        if (sync_ & Mesh::NORMALS_E) mesh_->compute_normals();
        if (sync_ & Mesh::BOUNDING_BOX_E) mesh_->compute_bounding_box();

        // These depend on the bounding box being synchronized
        if (sync_ & (Mesh::NODE_LOCATE_E|Mesh::ELEM_LOCATE_E))
        {
          {
            Core::Thread::UniqueLock lock(mesh_->synchronize_lock_.get());
            while(!(mesh_->synchronized_ & Mesh::BOUNDING_BOX_E))
              mesh_->synchronize_cond_.wait(lock);
          }
          if (sync_ & Mesh::NODE_LOCATE_E)
          {
            mesh_->compute_node_grid();
          }
          if (sync_ & Mesh::ELEM_LOCATE_E)
          {
            mesh_->compute_elem_grid();
          }
        }

        mesh_->synchronize_lock_.lock();
        // Mark the ones that were just synchronized
        mesh_->synchronized_ |= sync_;
        // Unmark the the ones that were done
        mesh_->synchronizing_ &= ~(sync_);
        /// Tell other threads we are done
        mesh_->synchronize_cond_.conditionBroadcast();
        mesh_->synchronize_lock_.unlock();
      }

    private:
      TriSurfMesh<Basis>* mesh_;
      mask_type  sync_;
  };


  //////////////////////////////////////////////////////////////////

  TriSurfMesh();

  /// Copy a mesh, needed for detaching the mesh from a field
  TriSurfMesh(const TriSurfMesh &copy);

  /// Clone function for detaching the mesh and automatically generating
  /// a new version if needed.
  virtual TriSurfMesh *clone() const { return new TriSurfMesh(*this); }

  virtual MeshFacadeHandle getFacade() const { return boost::make_shared<Core::Datatypes::VirtualMeshFacade<VMesh>>(vmesh_); }

  /// Destructor
  virtual ~TriSurfMesh();

  /// Access point to virtual interface
  virtual VMesh* vmesh() { return (vmesh_.get()); }

  /// This one should go at some point, should be reroute through the
  /// virtual interface
  virtual int basis_order() { return (basis_.polynomial_order()); }

  /// Topological dimension
  virtual int dimensionality() const { return 2; }

  /// What kind of mesh is this
  /// structured = no connectivity data
  /// regular    = no node location data
  virtual int topology_geometry() const
    { return (Mesh::UNSTRUCTURED | Mesh::IRREGULAR); }

  /// Get the bounding box of the field
  virtual Core::Geometry::BBox get_bounding_box() const;

  /// Return the transformation that takes a 0-1 space bounding box
  /// to the current bounding box of this mesh.
  virtual void get_canonical_transform(Core::Geometry::Transform &t) const;

  /// Core::Geometry::Transform a field (transform all nodes using this transformation matrix)
  virtual void transform(const Core::Geometry::Transform &t);

  /// Check whether mesh can be altered by adding nodes or elements
  virtual bool is_editable() const { return (true); }

  /// Has this mesh normals.
  // Note: normals point outward.
  /// @todo: this is inconsistent with QuadSurfMesh - should both surfaces
  // have consistent normal direction?
  virtual bool has_normals() const { return (true); }

  /// Compute tables for doing topology, these need to be synchronized
  /// before doing a lot of operations.
  virtual bool synchronize(mask_type mask);
  virtual bool unsynchronize(mask_type mask);
  bool clear_synchronization();

  /// Get the basis class.
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

  void size(typename Node::size_type &) const;
  void size(typename Edge::size_type &) const;
  void size(typename Face::size_type &) const;
  void size(typename Cell::size_type &) const;

  /// These are here to convert indices to unsigned int
  /// counters. Some how the decision was made to use multi
  /// dimensional indices in some fields, these functions
  /// should deal with different pointer types.
  /// Use the virtual interface to avoid all this non sense.
  void to_index(typename Node::index_type &index, index_type i) const
    { index = i; }
  void to_index(typename Edge::index_type &index, index_type i) const
    { index = i; }
  void to_index(typename Face::index_type &index, index_type i) const
    { index = i; }
  void to_index(typename Cell::index_type &index, index_type i) const
    { index = i; }


  /// Get the child elements of the given index.
  void get_nodes(typename Node::array_type &array, typename Node::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_nodes(typename Node::array_type &array, typename Edge::index_type idx) const
    { get_nodes_from_edge(array,idx); }
  void get_nodes(typename Node::array_type &array, typename Face::index_type idx) const
    { get_nodes_from_face(array,idx); }
  void get_nodes(typename Node::array_type&, typename Cell::index_type) const
    { ASSERTFAIL("TriSurfMesh: get_nodes has not been implemented for cells"); }

  void get_edges(typename Edge::array_type &array, typename Node::index_type idx) const
    { get_edges_from_node(array,idx); }
  void get_edges(typename Edge::array_type &array, typename Edge::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_edges(typename Edge::array_type &array, typename Face::index_type idx) const
    { get_edges_from_face(array,idx); }
  void get_edges(typename Edge::array_type&, typename Cell::index_type) const
    { ASSERTFAIL("TriSurfMesh: get_edges has not been implemented for cells"); }

  void get_faces(typename Face::array_type &array, typename Node::index_type idx) const
    { get_faces_from_node(array,idx); }
  void get_faces(typename Face::array_type &array, typename Edge::index_type idx) const
    { get_faces_from_edge(array,idx); }
  void get_faces(typename Face::array_type &array, typename Face::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_faces(typename Face::array_type&, typename Cell::index_type) const
    { ASSERTFAIL("TriSurfMesh: get_faces has not been implemented for cells"); }

  void get_cells(typename Cell::array_type&, typename Node::index_type) const
    { ASSERTFAIL("TriSurfMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type&, typename Edge::index_type) const
    { ASSERTFAIL("TriSurfMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type&, typename Face::index_type) const
    { ASSERTFAIL("TriSurfMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type&, typename Cell::index_type) const
    { ASSERTFAIL("TriSurfMesh: get_cells has not been implemented"); }

  void get_elems(typename Elem::array_type &array, typename Node::index_type idx) const
    { get_faces_from_node(array,idx); }
  void get_elems(typename Elem::array_type &array, typename Edge::index_type idx) const
    { get_faces_from_edge(array,idx); }
  void get_elems(typename Elem::array_type &array, typename Face::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_elems(typename Face::array_type&, typename Cell::index_type) const
    { ASSERTFAIL("TriSurfMesh: get_elems has not been implemented for cells"); }

  void get_delems(typename DElem::array_type &array, typename Node::index_type idx) const
    { get_edges_from_node(array,idx); }
  void get_delems(typename DElem::array_type &array, typename Edge::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_delems(typename DElem::array_type &array, typename Face::index_type idx) const
    { get_edges_from_face(array,idx); }
  void get_delems(typename DElem::array_type&, typename Cell::index_type) const
    { ASSERTFAIL("TriSurfMesh: get_delems has not been implemented for cells"); }

  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an edge.
  template<class VECTOR, class INDEX>
  void pwl_approx_edge(std::vector<VECTOR > &coords,
                       INDEX ci,
                       unsigned int which_edge,
                       unsigned int div_per_unit) const
  {
    basis_.approx_edge(which_edge, div_per_unit, coords);
  }

  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an face.
  template<class VECTOR, class INDEX>
  void pwl_approx_face(std::vector<std::vector<VECTOR > > &coords,
                       INDEX ci,
                       unsigned int which_face,
                       unsigned int div_per_unit) const
  {
    basis_.approx_face(which_face, div_per_unit, coords);
  }

  /// get the center point (in object space) of an element
  void get_center(Core::Geometry::Point &result, typename Node::index_type idx) const
    { get_node_center(result, idx); }
  void get_center(Core::Geometry::Point &result, typename Edge::index_type idx) const
    { get_edge_center(result, idx); }
  void get_center(Core::Geometry::Point &result, typename Face::index_type idx) const
    { get_face_center(result, idx); }
  void get_center(Core::Geometry::Point&, typename Cell::index_type) const
    { ASSERTFAIL("TriSurfMesh: get_cneter has not been implemented for cells"); }

  /// Get the size of an elemnt (length, area, volume)
  double get_size(typename Node::index_type /*idx*/) const
    { return 0.0; }

  double get_size(typename Edge::index_type idx) const
  {
    typename Node::array_type arr;
    get_nodes_from_edge(arr, idx);
    return (points_[arr[0]] - points_[arr[1]]).length();
  }

  double get_size(typename Face::index_type idx) const
  {
    typename Node::array_type ra;
    get_nodes(ra,idx);
    const Core::Geometry::Point &p0 = points_[ra[0]];
    const Core::Geometry::Point &p1 = points_[ra[1]];
    const Core::Geometry::Point &p2 = points_[ra[2]];
    return (Cross(p0-p1,p2-p0)).length()*0.5;
  }

  double get_size(typename Cell::index_type /*idx*/) const
    { return 0.0; }

  /// More specific names for get_size
  double get_length(typename Edge::index_type idx) const
    { return get_size(idx); }
  double get_area(typename Face::index_type idx) const
    { return get_size(idx); }
  double get_volume(typename Cell::index_type /*idx*/) const
    { return 0.0; }

  /// Get neighbors of an element or a node

  /// THIS ONE IS FLAWED AS IN 3D SPACE MULTIPLE EDGES CAN CONNECTED THROUGH
  /// ONE EDGE
  bool get_neighbor(typename Elem::index_type &neighbor,
                    typename Elem::index_type elem,
                    typename DElem::index_type delem) const
    { return(get_elem_neighbor(neighbor,elem,delem)); }

  /// These are more general implementations
  void get_neighbors(std::vector<typename Node::index_type> &array,
                     typename Node::index_type node) const
    { get_node_neighbors(array,node); }
  bool get_neighbors(std::vector<typename Elem::index_type> &array,
                     typename Elem::index_type elem,
                     typename DElem::index_type delem) const
    { return(get_elem_neighbors(array,elem,delem)); }
  void get_neighbors(typename Elem::array_type &array,
                     typename Elem::index_type elem) const
    { get_elem_neighbors(array,elem); }

  /// Locate a point in a mesh, find which is the closest node
  bool locate(typename Node::index_type &loc, const Core::Geometry::Point &p) const
    { return (locate_node(loc,p)); }
  bool locate(typename Edge::index_type &loc, const Core::Geometry::Point &p) const
    { return (locate_edge(loc,p)); }
  bool locate(typename Face::index_type &loc, const Core::Geometry::Point &p) const
    { return (locate_elem(loc,p)); }
  bool locate(typename Cell::index_type&, const Core::Geometry::Point&) const
    { return (false); }

  bool locate(typename Elem::index_type& elem,
              std::vector<double>& coords,
              const Core::Geometry::Point& p)
    { return(locate_elem(elem,coords,p)); }

  /// These should become obsolete soon, they do not follow the concept
  /// of the basis functions....
  int get_weights(const Core::Geometry::Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point&, typename Edge::array_type&, double*)
    {ASSERTFAIL("TriSurfMesh::get_weights(Edges) not supported."); }
  int get_weights(const Core::Geometry::Point &p, typename Face::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point&, typename Cell::array_type&, double*)
    {ASSERTFAIL("TriSurfMesh::get_weights(Cells) not supported."); }

  /// Access the nodes of the mesh
  void get_point(Core::Geometry::Point &result, typename Node::index_type index) const
    { result = points_[index]; }
  void set_point(const Core::Geometry::Point &point, typename Node::index_type index)
    { points_[index] = point; }

  void get_random_point(Core::Geometry::Point &, typename Elem::index_type, FieldRNG &rng) const;

  /// Normals for visualizations
  void get_normal(Core::Geometry::Vector &result, typename Node::index_type index) const
    {
      ASSERTMSG(synchronized_ & Mesh::NORMALS_E,
          "Must call synchronize NORMALS_E on TriSurfMesh first");
      result = normals_[index];
    }

  /// Get the normals at the outside of the element
  template<class VECTOR, class INDEX1, class INDEX2>
  void get_normal(Core::Geometry::Vector &result, VECTOR &coords,
                  INDEX1 eidx, INDEX2 /*fidx*/)
  {

    ElemData ed(*this, eidx);
    std::vector<Core::Geometry::Point> Jv;
    basis_.derivate(coords, ed, Jv);
    result = Cross(Core::Geometry::Vector(Jv[0]), Core::Geometry::Vector(Jv[1]));
    result.normalize();
  }

    /// Add a new node to the mesh
  typename Node::index_type add_point(const Core::Geometry::Point &p);
  typename Node::index_type add_node(const Core::Geometry::Point &p)
    { return(add_point(p)); }

    /// Add a new element to the mesh
  template <class ARRAY>
  typename Elem::index_type add_elem(ARRAY a)
  {
    ASSERTMSG(a.size() == 3, "TriSurfMesh: Tried to add non-tri element.");

    faces_.push_back(static_cast<typename Node::index_type>(a[0]));
    faces_.push_back(static_cast<typename Node::index_type>(a[1]));
    faces_.push_back(static_cast<typename Node::index_type>(a[2]));
    return static_cast<typename Elem::index_type>((faces_.size() - 1) / 3);
  }


  void node_reserve(size_type s) { points_.reserve(static_cast<size_t>(s)); }
  void elem_reserve(size_type s) { faces_.reserve(static_cast<size_t>(s*3)); }
  void resize_nodes(size_type s) { points_.resize(static_cast<size_t>(s)); }
  void resize_elems(size_type s) { faces_.resize(static_cast<size_t>(s*3)); }

  /// Get the local coordinates for a certain point within an element
  /// This function uses a couple of newton iterations to find the local
  /// coordinate of a point
  template<class VECTOR, class INDEX>
  bool get_coords(VECTOR &coords, const Core::Geometry::Point &p, INDEX idx) const
  {
    ElemData ed(*this, idx);
    return basis_.get_coords(coords, p, ed);
  }

  /// Find the location in the global coordinate system for a local coordinate
  /// This function is the opposite of get_coords.
  template<class VECTOR, class INDEX>
  void interpolate(Core::Geometry::Point &pt, VECTOR &coords, INDEX idx) const
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
  /// version of this matrix. This is currently here for completeness of the
  /// interface
  template<class VECTOR, class INDEX>
  void jacobian(const VECTOR& coords, INDEX idx, double* J) const
  {
    StackVector<Core::Geometry::Point,2> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
    Core::Geometry::Vector Jv2 = Cross(Core::Geometry::Vector(Jv[0]), Core::Geometry::Vector(Jv[1]));
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

  /// Get the inverse jacobian of the transformation. This one is needed to
  /// translate local gradients into global gradients. Hence it is crucial for
  /// calculating gradients of fields, or constructing finite elements.
  template<class VECTOR, class INDEX>
  double inverse_jacobian(const VECTOR& coords, INDEX idx, double* Ji) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
    Jv.resize(3);
    Core::Geometry::Vector v = Cross(Core::Geometry::Vector(Jv[0]), Core::Geometry::Vector(Jv[1])); v.normalize();
    Jv[2] = v.asPoint();

    return (InverseMatrix3P(Jv,Ji));
  }

  template<class INDEX>
  double scaled_jacobian_metric(INDEX idx) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    Core::Geometry::Point p0, p1, p2;
    typename Node::array_type nodes;
    get_nodes(nodes,idx);
    get_center(p0,nodes[0]);
    get_center(p1,nodes[1]);
    get_center(p2,nodes[2]);

    double l1 = (p0-p1).length();
    double l2 = (p1-p2).length();
    double l3 = (p2-p0).length();

    double max_scale = l1*l2;
    if (l2*l3 > max_scale) max_scale = l2*l3;
    if (l1*l3 > max_scale) max_scale = l1*l3;

    basis_.derivate(basis_.unit_center,ed,Jv);
    Jv.resize(3);
    Core::Geometry::Vector v = Cross(Core::Geometry::Vector(Jv[0]), Core::Geometry::Vector(Jv[1]));
    v.normalize();
    Jv[2] = v.asPoint();
    double min_jacobian = DetMatrix3P(Jv);
    size_t num_vertices = basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3);
      v = Cross(Core::Geometry::Vector(Jv[0]), Core::Geometry::Vector(Jv[1]));
      v.normalize();
      Jv[2] = v.asPoint();
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }

    return (min_jacobian/max_scale);
  }


  template<class INDEX>
  double jacobian_metric(INDEX idx) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    basis_.derivate(basis_.unit_center,ed,Jv);
    Jv.resize(3);
    Core::Geometry::Vector v = Cross(Core::Geometry::Vector(Jv[0]), Core::Geometry::Vector(Jv[1]));
    v.normalize();
    Jv[2] = v.asPoint();
    double min_jacobian = DetMatrix3P(Jv);

    size_t num_vertices = basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3);
      v = Cross(Core::Geometry::Vector(Jv[0]), Core::Geometry::Vector(Jv[1]));
      v.normalize();
      Jv[2] = v.asPoint();
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }

    return (min_jacobian);
  }


  template <class INDEX>
  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         INDEX &node, const Core::Geometry::Point &p) const
  {
    return(find_closest_node(pdist,result,node,p,-1.0));
  }

  template <class INDEX>
  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         INDEX &node, const Core::Geometry::Point &p, double maxdist) const
  {
    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;
    typename Node::size_type sz; size(sz);

    /// If there are no nodes we cannot find the closest one
    if (sz == 0) return (false);

    if (node >= 0 && node < sz)
    {
      Core::Geometry::Point point = points_[node];
      double dist = (p-point).length2();

      if ( dist < epsilon2_ )
      {
        result = point;
        pdist = sqrt(dist);
        return (true);
      }
    }

    ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
        "TriSurfMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

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
                typename SearchGridT<index_type>::iterator  it, eit;
                node_grid_->lookup_ijk(it, eit, i, j, k);

                while (it != eit)
                {
                  const Core::Geometry::Point point = points_[*it];
                  const double dist  = (p-point).length2();

                  if (dist < dmin)
                  {
                    found_one = true;
                    result = point;
                    node = INDEX(*it);
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

  template <class ARRAY>
  bool find_closest_nodes(ARRAY &nodes, const Core::Geometry::Point &p, double maxdist) const
  {
    nodes.clear();

    ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
        "TriSurfMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

    // get grid sizes
    const size_type ni = node_grid_->get_ni()-1;
    const size_type nj = node_grid_->get_nj()-1;
    const size_type nk = node_grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, bj, bk, ei, ej, ek;

    Core::Geometry::Point max = p+Core::Geometry::Vector(maxdist,maxdist,maxdist);
    Core::Geometry::Point min = p+Core::Geometry::Vector(-maxdist,-maxdist,-maxdist);

    node_grid_->unsafe_locate(bi, bj, bk, min);
    node_grid_->unsafe_locate(ei, ej, ek, max);

    // Clamp to closest point on the grid.
    if (bi > ni) bi = ni;
    if (bi < 0) bi = 0;
    if (bj > nj) bj = nj;
    if (bj < 0) bj = 0;
    if (bk > nk) bk = nk;
    if (bk < 0) bk = 0;

    if (ei > ni) ei = ni;
    if (ei < 0) ei = 0;
    if (ej > nj) ej = nj;
    if (ej < 0) ej = 0;
    if (ek > nk) ek = nk;
    if (ek < 0) ek = 0;

    double maxdist2 = maxdist*maxdist;

    for (index_type i = bi; i <= ei; i++)
    {
      for (index_type j = bj; j <= ej; j++)
      {
        for (index_type k = bk; k <= ek; k++)
        {
          if (node_grid_->min_distance_squared(p, i, j, k) < maxdist2)
          {
            typename SearchGridT<index_type>::iterator  it, eit;
            node_grid_->lookup_ijk(it, eit, i, j, k);

            while (it != eit)
            {
              const Core::Geometry::Point point = points_[*it];
              const double dist  = (p-point).length2();

              if (dist < maxdist2)
              {
                nodes.push_back(*it);
              }
              ++it;
            }
          }
        }
      }
    }

    return(nodes.size() > 0);
  }


  template <class ARRAY1, class ARRAY2>
  bool find_closest_nodes(ARRAY1 &distances, ARRAY2 &nodes, const Core::Geometry::Point &p, double maxdist) const
  {
    nodes.clear();
    distances.clear();

    ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
        "TriSurfMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

    // get grid sizes
    const size_type ni = node_grid_->get_ni()-1;
    const size_type nj = node_grid_->get_nj()-1;
    const size_type nk = node_grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, bj, bk, ei, ej, ek;

    Core::Geometry::Point max = p+Core::Geometry::Vector(maxdist,maxdist,maxdist);
    Core::Geometry::Point min = p+Core::Geometry::Vector(-maxdist,-maxdist,-maxdist);

    node_grid_->unsafe_locate(bi, bj, bk, min);
    node_grid_->unsafe_locate(ei, ej, ek, max);

    // Clamp to closest point on the grid.
    if (bi > ni) bi = ni;
    if (bi < 0) bi = 0;
    if (bj > nj) bj = nj;
    if (bj < 0) bj = 0;
    if (bk > nk) bk = nk;
    if (bk < 0) bk = 0;

    if (ei > ni) ei = ni;
    if (ei < 0) ei = 0;
    if (ej > nj) ej = nj;
    if (ej < 0) ej = 0;
    if (ek > nk) ek = nk;
    if (ek < 0) ek = 0;

    double maxdist2 = maxdist*maxdist;

    for (index_type i = bi; i <= ei; i++)
    {
      for (index_type j = bj; j <= ej; j++)
      {
        for (index_type k = bk; k <= ek; k++)
        {
          if (node_grid_->min_distance_squared(p, i, j, k) < maxdist2)
          {
            typename SearchGridT<index_type>::iterator  it, eit;
            node_grid_->lookup_ijk(it, eit, i, j, k);

            while (it != eit)
            {
              const Core::Geometry::Point point = points_[*it];
              const double dist  = (p-point).length2();

              if (dist < maxdist2)
              {
                nodes.push_back(*it);
                distances.push_back(dist);
              }
              ++it;
            }
          }
        }
      }
    }

    return(nodes.size() > 0);
  }


  /// This function will find the closest element and the location on that
  /// element that is the closest
  template <class INDEX, class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         ARRAY &coords,
                         INDEX &face,
                         const Core::Geometry::Point &p) const
  {
    return (find_closest_elem(pdist,result,coords,face,p,-1.0));
  }

  /// This function will find the closest element and the location on that
  /// element that is the closest
  template <class INDEX, class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         ARRAY &coords,
                         INDEX &face,
                         const Core::Geometry::Point &p,
                         double maxdist) const
  {
    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;
    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find the closest one
    if (sz == 0) return (false);

    /// Test the one in face that is an initial guess
    if (face >= 0 && face < sz)
    {
      index_type idx = face*3;
      closest_point_on_tri(result, p, points_[faces_[idx]],
                           points_[faces_[idx+1]], points_[faces_[idx+2]]);
      double dist = (p-result).length2();
      if ( dist < epsilon2_ )
      {
        pdist = sqrt(dist);

        ElemData ed(*this,face);
        basis_.get_coords(coords,result,ed);
        return (true);
      }
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
        "TriSurfMesh::find_closest_elem requires synchronize(ELEM_LOCATE_E).")

    // get grid sizes
    const size_type ni = elem_grid_->get_ni()-1;
    const size_type nj = elem_grid_->get_nj()-1;
    const size_type nk = elem_grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, ei, bj, ej, bk, ek;
    elem_grid_->unsafe_locate(bi, bj, bk, p);

    // Clamp to closest point on the grid.
    if (bi > ni)
      bi = ni;
    if (bi < 0)
      bi = 0;
    if (bj > nj)
      bj = nj;
    if (bj < 0)
      bj = 0;
    if (bk > nk)
      bk = nk;
    if (bk < 0)
      bk = 0;

    ei = bi; ej = bj; ek = bk;

    double dmin = maxdist;
    double dmean = maxdist;
    bool found = true;
    bool found_one = false;
    double perturb= epsilon_*100; //value to move to find new point.

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
                typename SearchGridT<index_type>::iterator it, eit;
                elem_grid_->lookup_ijk(it,eit, i, j, k);

                while (it != eit)
                {
                  Core::Geometry::Point r, r_pert;
                  index_type idx = (*it) * 3;

                  closest_point_on_tri(r, p, points_[faces_[idx]], points_[faces_[idx+1]], points_[faces_[idx+2]]);
                  double dtmp = (p - r).length2();


                  //test triangle size for scaling
                  Core::Geometry::Vector v1= Core::Geometry::Vector(points_[faces_[idx+1]]-points_[faces_[idx  ]]); v1.normalize();
                  Core::Geometry::Vector v2= Core::Geometry::Vector(points_[faces_[idx+2]]-points_[faces_[idx  ]]); v2.normalize();

                  Core::Geometry::Vector n=Cross(v1,v2); n.normalize();
                  Core::Geometry::Vector pr=Core::Geometry::Vector(r-p); pr.normalize();

                  if (std::abs(Dot(pr,n))>1-perturb)
                  {
                    r_pert=r;
                  }
                  else
                  {

                    Core::Geometry::Vector pp=Cross(n,pr); pp.normalize();
                    Core::Geometry::Vector vect=Cross(pp,n); vect.normalize();

                    r_pert=Core::Geometry::Point(r+vect*perturb);
                  }

                  double dtmp2=(p-r_pert).length2();

                  //check for closest face and check within precision
                  if (dtmp-dmin <= epsilon_)
                  {
                    if (dtmp-dmin < - epsilon_)
                    {
                      found_one = true;
                      result = r;
                      face = INDEX(*it);
                      dmin = dtmp;
                      dmean =dtmp2;

                      if (dmin < epsilon2_)
                      {

                        pdist = sqrt(dmin);
                        pdist = sqrt(dmean);

                        ElemData ed(*this,face);
                        basis_.get_coords(coords,result,ed);
                        return (true);
                      }
                    }
                    else if (dtmp2-dmean < - epsilon_ )
                    {
                      found_one = true;
                      result = r;
                      face = INDEX(*it);
                      if (dmin>=dtmp) dmin=dtmp;
                      dmean =dtmp2;
                    }
                    else if (dtmp<dmin  && std::abs(dtmp2-dmean) < epsilon_ )
                    {
                      found_one = true;
                      result = r;
                      face = INDEX(*it);
                      dmin = dtmp;
                      dmean =dtmp2;
                      if (dmin < epsilon2_)
                      {

                        pdist = sqrt(dmin);
                        pdist = sqrt(dmean);

                        ElemData ed(*this,face);
                        basis_.get_coords(coords,result,ed);
                      }
                    }
                    else if (dtmp2 < dmean && dtmp-dmin > - epsilon_)
                    {
                      found_one = true;
                      result = r;
                      face = INDEX(*it);
                      dmean =dtmp2;
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

    ElemData ed(*this,face);
    basis_.get_coords(coords,result,ed);

    if (!found_one) return (false);

    pdist = sqrt(dmin);
    return (true);
  }


  template <class INDEX>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         INDEX &elem,
                         const Core::Geometry::Point &p) const
  {
    StackVector<double,2> coords;
    return(find_closest_elem(pdist,result,coords,elem,p,-1.0));
  }



  template <class INDEX>
  inline bool search_node(INDEX &loc, const Core::Geometry::Point &p) const
  {
    typename Node::iterator ni, nie;
    begin(ni);
    end(nie);


    if (ni == nie)
    {
      return false;
    }

    double min_dist = (p - points_[*ni]).length2();
    loc = static_cast<INDEX>(*ni);
    ++ni;

    while (ni != nie)
    {
      const double dist = (p - points_[*ni]).length2();
      if (dist < min_dist)
      {
        min_dist = dist;
        loc = static_cast<INDEX>(*ni);
      }
      ++ni;
    }
    return true;
  }

  /// This function will return multiple elements if the closest point is
  /// located on a node or edge. All bordering elements are returned in that
  /// case.
  template<class ARRAY>
  bool find_closest_elems(double& pdist, Core::Geometry::Point &result,
                          ARRAY &elems, const Core::Geometry::Point &p) const
  {
    elems.clear();

    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find the closest one
    if (sz == 0) return (false);

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
        "TriSurfMesh::find_closest_elems requires synchronize(ELEM_LOCATE_E).")

    // get grid sizes
    const size_type ni = elem_grid_->get_ni()-1;
    const size_type nj = elem_grid_->get_nj()-1;
    const size_type nk = elem_grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, ei, bj, ej, bk, ek;
    elem_grid_->unsafe_locate(bi, bj, bk, p);

    // Clamp to closest point on the grid.
    if (bi > ni)
      bi = ni;
    if (bi < 0)
      bi = 0;
    if (bj > nj)
      bj = nj;
    if (bj < 0)
      bj = 0;
    if (bk > nk)
      bk = nk;
    if (bk < 0)
      bk = 0;

    ei = bi; ej = bj; ek = bk;

    double dmin = DBL_MAX;

    bool found;
    do
    {
      found = true;
      /// This looks incorrect - but it is correct
      /// We need to do a full shell without any elements that are closer
      /// to make sure there no closer elements
      for (index_type i = bi; i <= ei; i++)
      {
        if (i < 0|| i > ni) continue;
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
                typename SearchGridT<index_type>::iterator it, eit;
                elem_grid_->lookup_ijk(it,eit, i, j, k);

                while (it != eit)
                {
                  Core::Geometry::Point rtmp;
                  index_type idx = (*it) * 3;
                  closest_point_on_tri(rtmp, p,
                                       points_[faces_[idx  ]],
                                       points_[faces_[idx+1]],
                                       points_[faces_[idx+2]]);
                  const double dtmp = (p - rtmp).length2();

                  if (dtmp < dmin - epsilon2_)
                  {
                    elems.clear();
                    result = rtmp;
                    elems.push_back(typename ARRAY::value_type(*it));
                    found = false;
                    dmin = dtmp;
                  }
                  else if (dtmp < dmin + epsilon2_)
                  {
                    elems.push_back(typename ARRAY::value_type(*it));
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

    pdist = sqrt(dmin);
    return (true);
  }


  double get_epsilon() const
    { return (epsilon_); }

  ///////////////////////////////////////////////////
  // STATIC VARIABLES AND FUNCTIONS
  /// Export this class using the old Pio system
  virtual void io(Piostream&);

  /// This ID is created as soon as this class will be instantiated
  static PersistentTypeID trisurf_typeid;
  /// Core functionality for getting the name of a templated mesh class
  static  const std::string type_name(int n = -1);
  virtual std::string dynamic_type_name() const { return trisurf_typeid.type; }
  /// Type description, used for finding names of the mesh class for
  /// dynamic compilation purposes. Some of this should be obsolete
  virtual const TypeDescription *get_type_description() const;
  static const TypeDescription* node_type_description();
  static const TypeDescription* edge_type_description();
  static const TypeDescription* face_type_description();
  static const TypeDescription* cell_type_description();
  static const TypeDescription* elem_type_description()
    { return face_type_description(); }

  /// This function returns a maker for Pio.
  static Persistent *maker() { return new TriSurfMesh<Basis>(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return boost::make_shared<TriSurfMesh<Basis>>(); }


  //////////////////////////////////////////////////////////////////
  // Mesh specific functions (these are not implemented in every mesh)

  // Extra functionality needed by this specific geometry.
  typename Node::index_type add_find_point(const Core::Geometry::Point &p,
					   double err = 1.0e-3);
  typename Elem::index_type add_triangle(typename Node::index_type,
					 typename Node::index_type,
					 typename Node::index_type);
  typename Elem::index_type add_triangle(const Core::Geometry::Point& p0,
					 const Core::Geometry::Point& p1,
					 const Core::Geometry::Point& p2);


  /// swap the shared edge between 2 faces, if they share an edge.
  bool swap_shared_edge(typename Face::index_type, typename Face::index_type);
  bool remove_face(typename Face::index_type);
  bool remove_orphan_nodes();
  /// walk all the faces, enforcing consistent face orientations.
  void orient_faces();
  /// flip the orientaion of all the faces
  /// orient could make all faces face inward...
  void flip_faces();
  void flip_face(typename Face::index_type face);


  /// Subdivision Methods
  bool insert_node(const Core::Geometry::Point &p);
  void insert_node(typename Face::index_type face, const Core::Geometry::Point &p);
  void bisect_element(const typename Face::index_type);

  bool              insert_node_in_edge_aux(typename Face::array_type &tris,
                                          typename Node::index_type &ni,
                                          index_type halfedge,
                                          const Core::Geometry::Point &p);

  bool              insert_node_in_face_aux(typename Face::array_type &tris,
                                            typename Node::index_type &ni,
                                            typename Face::index_type face,
                                            const Core::Geometry::Point &p);

  bool                  insert_node_in_face(typename Face::array_type &tris,
                                            typename Node::index_type &ni,
                                            typename Face::index_type face,
                                            const Core::Geometry::Point &p);



  const Core::Geometry::Point &point(typename Node::index_type i) { return points_[i]; }

  // This one should be made obsolete
  bool get_neighbor(index_type &nbr_half_edge,
                    index_type half_edge) const;

  void collapse_edges(const std::vector<index_type> &nodemap);

  // This function removes any triangles which happen to share one or
  // more exact nodes.  It doesn't do an area test.
  void remove_obvious_degenerate_triangles();

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
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "TriSurfMesh: Must call synchronize EDGES_E on TriSurfMesh first");

    index_type a = edges_[idx][0];
    index_type faceidx = a >> 2;
    index_type offset = a & 0x3;
    array.resize(2);
    if (offset == 0)
    {
      array[0] = static_cast<typename ARRAY::value_type>(faces_[faceidx*3]);
      array[1] = static_cast<typename ARRAY::value_type>(faces_[faceidx*3 + 1]);
    }
    else if (offset == 1)
    {
      array[0] = static_cast<typename ARRAY::value_type>(faces_[faceidx*3 + 1]);
      array[1] = static_cast<typename ARRAY::value_type>(faces_[faceidx*3 + 2]);
    }
    else
    {
      array[0] = static_cast<typename ARRAY::value_type>(faces_[faceidx*3 + 2]);
      array[1] = static_cast<typename ARRAY::value_type>(faces_[faceidx*3 ]);
    }
  }


  template<class ARRAY, class INDEX>
  inline void get_nodes_from_face(ARRAY& array, INDEX idx) const
  {
    array.resize(3);
    array[0] = static_cast<typename ARRAY::value_type>(faces_[idx * 3 + 0]);
    array[1] = static_cast<typename ARRAY::value_type>(faces_[idx * 3 + 1]);
    array[2] = static_cast<typename ARRAY::value_type>(faces_[idx * 3 + 2]);
  }


  template<class ARRAY, class INDEX>
  inline void get_nodes_from_elem(ARRAY& array, INDEX idx) const
  {
    get_nodes_from_face(array,idx);
  }


  template<class ARRAY, class INDEX>
  inline void get_edges_from_face(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "TriSurfMesh: Must call synchronize EDGES_E on TriSurfMesh first");

    array.resize(3);

    array[0] = static_cast<typename ARRAY::value_type>(halfedge_to_edge_[idx * 3 + 0]);
    array[1] = static_cast<typename ARRAY::value_type>(halfedge_to_edge_[idx * 3 + 1]);
    array[2] = static_cast<typename ARRAY::value_type>(halfedge_to_edge_[idx * 3 + 2]);
  }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_elem(ARRAY& array, INDEX idx) const
  {
    get_edges_from_face(array,idx);
  }


  template<class ARRAY, class INDEX>
  inline void get_faces_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
	      "TriSurfMesh: Must call synchronize NODE_NEIGHBORS_E on TriSurfMesh first");

    array.resize(node_neighbors_[idx].size());
    for (size_t i = 0; i < node_neighbors_[idx].size(); ++i)
      array[i] = static_cast<typename ARRAY::value_type>(node_neighbors_[idx][i]);
  }


  template<class ARRAY, class INDEX>
  inline void get_faces_from_edge(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on TriSurfMesh first");

    const std::vector<index_type>& faces = edges_[idx];

    // clear array
    array.clear();
    // conservative estimate
    array.reserve(faces.size());

    size_type fs = faces.size();
    for (index_type i=0; i<fs; i++)
    {
      array.push_back(faces[i]>>2);
    }
  }


  // Fixes bug #887 (gforge)
  template<class ARRAY, class INDEX>
  inline void get_edges_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on TriSurfMesh first");

    // Get the table of faces that are connected to the two nodes
    const std::vector<index_type>& faces  = node_neighbors_[idx];
    array.clear();

    typename ARRAY::value_type edge;
    for (size_t i=0; i<faces.size(); i++)
    {
      for (index_type j=0; j<3; j++)
      {
        edge = halfedge_to_edge_[faces[i]*3+j];
        size_t k=0;
        for (; k<array.size(); k++)
          if (array[k] == edge) break;
        if (k == array.size()) array.push_back(edge);
      }
    }
  }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_node_bugfix(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
      "Must call synchronize NODE_NEIGHBORS_E on TriSurfMesh first");

    array.clear();
    int n=edge_on_node_[idx].size();
    typename ARRAY::value_type edge;
    for(int i=0; i<n; i++)
    {
      edge = edge_on_node_[idx][i];
      size_t k=0;
      for (; k<array.size(); k++)
        if (array[k] == edge) break;
      if (k == array.size()) array.push_back(edge);
    }
  }

  template <class ARRAY, class INDEX>
  inline void set_nodes_by_elem(ARRAY &array, INDEX idx)
  {
    for (index_type n = 0; n < 3; ++n)
      faces_[idx * 3 + n] = static_cast<index_type>(array[n]);
  }


  /// This function has been rewritten to allow for non manifold surfaces to be
  /// handled ok.
  template <class INDEX1, class INDEX2>
  inline bool get_elem_neighbor(INDEX1 &neighbor, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on TriSurfMesh first");

    const std::vector<index_type>& faces = edges_[delem];

    size_type fs = faces.size();
    for (index_type i=0; i<fs; i++)
    {
      index_type face = (faces[i]>>2);
      if (face != elem)
      {
        neighbor = face;
        return (true);
      }
    }
    return (false);
  }

  template <class ARRAY,class INDEX1, class INDEX2>
  inline bool get_elem_neighbors(ARRAY &array, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on TriSurfMesh first");

    // Find the two nodes that make up the edge
    const std::vector<index_type>& faces = edges_[delem];

    // clear array
    array.clear();

    size_type fs = faces.size();
    for (index_type i=0; i<fs; i++)
    {
      index_type face = (faces[i]>>2);
      if (face != elem)
      {
        array.push_back(face);
      }
    }

    return (array.size() > 0);
  }

  template <class ARRAY, class INDEX>
  inline void get_elem_neighbors(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on TriSurfMesh first");

    typename Edge::array_type edges;
    get_edges_from_face(edges, idx);

    array.clear();
    array.reserve(edges.size());

    ARRAY nbor;

    size_type sz =static_cast<size_type>(edges.size());
    for (index_type i=0; i<sz; i++)
    {
      if (get_elem_neighbors(nbor, idx, edges[i]))
      {
        size_type nsz =static_cast<size_type>(nbor.size());
        for (index_type j=0; j<nsz; j++)
          array.push_back(nbor[j]);
      }
    }
  }


  template <class ARRAY, class INDEX>
  void get_node_neighbors(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on TriSurfMesh first");
    // clear old contents
    array.clear();

    // Get all the neighboring elements
    const std::vector<index_type>& faces  = node_neighbors_[idx];
    // Make a conservative estimate of the number of node neighbors
    array.reserve(2*faces.size());

    for (size_t i=0;i<faces.size();i++)
    {
      index_type base = faces[i]*3;
      for (index_type j=0;j<3;j++)
      {
        if (faces_[base+j] == idx) continue;
        size_t k=0;
        for (;k<array.size();k++)
          if (static_cast<typename ARRAY::value_type>(faces_[base+j]) == array[k]) break;
        if (k==array.size())
          array.push_back(static_cast<typename ARRAY::value_type>(faces_[base+j]));
      }
    }
  }

  /// Locate a node inside the mesh using the lookup table
  template <class INDEX>
  inline bool locate_node(INDEX &node, const Core::Geometry::Point &p) const
  {
    typename Node::size_type sz; size(sz);

    /// If there are no nodes we cannot find a closest point
    if (sz == 0) return (false);

    /// Check first guess
    if (node >= 0 && node < sz)
    {
      if ((p - points_[node]).length2() < epsilon2_) return (true);
    }


    ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
              "TriSurfMesh::locate_node requires synchronize(NODE_LOCATE_E).")

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
    bool found = true;
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
                SearchGridT<index_type>::iterator it, eit;
                node_grid_->lookup_ijk(it, eit, i, j, k);

                while (it != eit)
                {
                  const Core::Geometry::Point point = points_[*it];
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


  /// Locate an edge inside the mesh using the lookup table
  /// This currently an exhaustive search
  template <class INDEX>
  inline bool locate_edge(INDEX &loc, const Core::Geometry::Point &p) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on TriSurfMesh first");

    typename Edge::iterator bi, ei;
    typename Node::array_type nodes;
    begin(bi);
    end(ei);

    bool found = false;
    double mindist = 0.0;
    while (bi != ei)
    {
      get_nodes(nodes,*bi);
      const double dist = distance_to_line2(p, points_[nodes[0]],
                                            points_[nodes[1]],epsilon_);
      if (!found || dist < mindist)
      {
        loc = static_cast<INDEX>(*bi);
        mindist = dist;
        found = true;
      }
      ++bi;
    }
    return (found);
  }


  /// Locate an element inside the mesh using the lookup table
  template <class INDEX>
  inline bool locate_elem(INDEX &elem, const Core::Geometry::Point &p) const
  {
    if (basis_.polynomial_order() > 1) return elem_locate(elem, *this, p);

    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find a closest point
    if (sz == 0) return (false);

    /// Check whether the estimate given in idx is the point we are looking for
    if ((elem > 0)&&(elem < sz))
    {
      if (inside3_p(elem*3,p)) return (true);
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
              "TriSurfMesh::locate_elem requires synchronize(ELEM_LOCATE_E).")

    typename SearchGridT<index_type>::iterator it, eit;
    if (elem_grid_->lookup(it, eit, p))
    {
      while (it != eit)
      {
        if (inside3_p((*it) * 3, p))
        {
          elem = static_cast<INDEX>(*it);
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
              "TriSurfMesh::locate_elems requires synchronize(ELEM_LOCATE_E).")

    array.clear();
    index_type is,js,ks;
    index_type ie,je,ke;
    elem_grid_->locate_clamp(is,js,ks,b.get_min());
    elem_grid_->locate_clamp(ie,je,ke,b.get_max());
    for (index_type i=is; i<=ie;i++)
      for (index_type j=js; j<je;j++)
        for (index_type k=ks; k<ke; k++)
        {
          typename SearchGridT<index_type>::iterator it, eit;
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
  inline bool locate_elem(INDEX &elem, ARRAY &coords, const Core::Geometry::Point &p) const
  {
    if (basis_.polynomial_order() > 1) return elem_locate(elem, *this, p);

    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find a closest point
    if (sz == 0) return (false);

    /// Check whether the estimate given in idx is the point we are looking for
    if ((elem > 0)&&(elem < sz))
    {
      if (inside3_p(elem*3,p))
      {
        ElemData ed(*this, elem);
        basis_.get_coords(coords, p, ed);
        return (true);
      }
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
              "TriSurfMesh::locate_node requires synchronize(ELEM_LOCATE_E).")

    typename SearchGridT<index_type>::iterator it, eit;
    if (elem_grid_->lookup(it, eit, p))
    {
      while (it != eit)
      {
        if (inside3_p((*it) * 3, p))
        {
          elem = static_cast<INDEX>(*it);
          ElemData ed(*this, elem);
          basis_.get_coords(coords, p, ed);
          return (true);
        }
        ++it;
      }
    }
    return (false);
  }



  template <class INDEX>
  void get_node_center(Core::Geometry::Point &p, INDEX idx) const
  {
    p = points_[idx];
  }


  template <class INDEX>
  void get_edge_center(Core::Geometry::Point &result, INDEX idx) const
  {
    typename Node::array_type arr;
    get_nodes_from_edge(arr, idx);
    result = points_[arr[0]];
    result += points_[arr[1]];
    result *= 0.5;
  }


  template <class INDEX>
  void get_face_center(Core::Geometry::Point &result, INDEX idx) const
  {
    // NEED TO OPTIMIZE THIS ONE
    typename Node::array_type arr;
    get_nodes_from_face(arr, idx);
    result = points_[arr[0]];
    result += points_[arr[1]];
    result += points_[arr[2]];
    result *= (1.0/3.0);
  }


  void walk_face_orient(typename Face::index_type face,
                        std::vector<bool> &tested, std::vector<bool> &flip);

  // These require the synchronize_lock_ to be held before calling.
  void compute_normals();
  void compute_node_neighbors();
  void compute_edges();
  // Fixes bug #887 (gforge)
  void compute_edges_bugfix();
  void compute_edge_neighbors();

  void compute_node_grid();
  void compute_elem_grid();
  void compute_bounding_box();

  /// Used to recompute data for individual cells.
  void insert_elem_into_grid(typename Elem::index_type ci);
  void remove_elem_from_grid(typename Elem::index_type ci);

  void insert_node_into_grid(typename Node::index_type ci);
  void remove_node_from_grid(typename Node::index_type ci);

  void debug_test_edge_neighbors();

  bool inside3_p(index_type face_times_three, const Core::Geometry::Point &p) const;

  static index_type next(index_type i) { return ((i%3)==2) ? (i-2) : (i+1); }
  static index_type prev(index_type i) { return ((i%3)==0) ? (i+2) : (i-1); }

  /// Actual parameters
  std::vector<Core::Geometry::Point>         points_;              // Location of vertices
  std::vector<std::vector<index_type> >    edges_;               // edges->halfedge map
  std::vector<index_type>    halfedge_to_edge_;    // halfedge->edge map
  std::vector<index_type>    faces_;               // Connectivity of this mesh
  std::vector<index_type>    edge_neighbors_;      // Neighbor connectivity
  std::vector<Core::Geometry::Vector>        normals_;             // normalized per node normal.
  std::vector<std::vector<index_type> > node_neighbors_; // Node neighbor connectivity
  std::vector<std::vector<index_type> > edge_on_node_; // Edges emanating from a node

  boost::shared_ptr<SearchGridT<index_type> > node_grid_; // Lookup table for nodes
  boost::shared_ptr<SearchGridT<index_type> > elem_grid_; // Lookup table for elements

  // Lock and Condition Variable for hand shaking
  mutable Core::Thread::Mutex         synchronize_lock_;
  Core::Thread::ConditionVariable     synchronize_cond_;

  // Which tables have been computed
  mask_type             synchronized_;
  // Which tables are currently being computed
  mask_type             synchronizing_;

  Basis                 basis_;             // Interpolation basis

  Core::Geometry::BBox                  bbox_;
  double                epsilon_;           // Epsilon to use for computation 1e-8 of bbox diagonal
  double                epsilon2_;          // Square of epsilon

  boost::shared_ptr<VMesh>         vmesh_;             // Handle to virtual function table

  struct edgehash
  {
    boost::hash<int> hasher_;
    size_t operator()(const std::pair<index_type, index_type> &a) const
    {
      return hasher_(static_cast<int>(hasher_(a.first) + a.second));
    }
  };

  using EdgeMapType = boost::unordered_map<std::pair<index_type, index_type>, index_type, edgehash>;
  using EdgeMapType2 = boost::unordered_map<std::pair<index_type, index_type>, std::vector<index_type>, edgehash>;
};


struct less_int
{
  bool operator()(const index_type s1, const index_type s2) const
  {
    return (s1 < s2);
  }
};

template <class Basis>
PersistentTypeID
TriSurfMesh<Basis>::trisurf_typeid(TriSurfMesh<Basis>::type_name(-1), "Mesh", maker);

template <class Basis>
const std::string
TriSurfMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("TriSurfMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
TriSurfMesh<Basis>::TriSurfMesh()
  : points_(0),
    faces_(0),
    edge_neighbors_(0),
    node_neighbors_(0),
    synchronize_lock_("TriSurfMesh lock"),
    synchronize_cond_("TriSurfMesh condition variable"),
    synchronized_(Mesh::NODES_E | Mesh::FACES_E | Mesh::CELLS_E),
    synchronizing_(0),
    epsilon_(0.0),
    epsilon2_(0.0)
{
  DEBUG_CONSTRUCTOR("TriSurfMesh")

  /// Initialize the virtual interface when the mesh is created
  vmesh_.reset(CreateVTriSurfMesh(this));
}


template <class Basis>
TriSurfMesh<Basis>::TriSurfMesh(const TriSurfMesh &copy)
  : Mesh(copy),
    points_(0),
    edges_(0),
    halfedge_to_edge_(0),
    faces_(0),
    edge_neighbors_(0),
    normals_(0),
    node_neighbors_(0),
    synchronize_lock_("TriSurfMesh lock"),
    synchronize_cond_("TriSurfMesh condition variable"),
    synchronized_(Mesh::NODES_E | Mesh::FACES_E | Mesh::CELLS_E),
    synchronizing_(0),
    epsilon_(0.0),
    epsilon2_(0.0)
{
  DEBUG_CONSTRUCTOR("TriSurfMesh")

  /// We need to lock before we can copy these as these
  /// structures are generate dynamically when they are
  /// needed.
  copy.synchronize_lock_.lock();

  points_ = copy.points_;

  edges_ = copy.edges_;
  halfedge_to_edge_ = copy.halfedge_to_edge_;
  synchronized_ |= copy.synchronized_ & (Mesh::EDGES_E);

  faces_ = copy.faces_;

  edge_neighbors_ = copy.edge_neighbors_;
  synchronized_ |= copy.synchronized_ & Mesh::ELEM_NEIGHBORS_E;

  normals_ = copy.normals_;
  synchronized_ |= copy.synchronized_ & Mesh::NORMALS_E;

  node_neighbors_ = copy.node_neighbors_;
  synchronized_ |= copy.synchronized_ & Mesh::NODE_NEIGHBORS_E;

  copy.synchronize_lock_.unlock();

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  vmesh_.reset(CreateVTriSurfMesh(this));
}


template <class Basis>
TriSurfMesh<Basis>::~TriSurfMesh()
{
  DEBUG_DESTRUCTOR("TriSurfMesh")
}



template <class Basis>
void
TriSurfMesh<Basis>::get_random_point(Core::Geometry::Point &p,
                                     typename Elem::index_type ei,
                                     FieldRNG &rng) const
{
  // Fold the quad sample into a triangle.
  double u = rng();
  double v = rng();
  if (u + v > 1.0) { u = 1.0 - u; v = 1.0 - v; }

  // Compute the position of the random point.
  const Core::Geometry::Point& p0 = points_[faces_[ei*3+0]];
  const Core::Geometry::Point& p1 = points_[faces_[ei*3+1]];
  const Core::Geometry::Point& p2 = points_[faces_[ei*3+2]];
  p = p0+((p1-p0)*u)+((p2-p0)*v);

}


template <class Basis>
Core::Geometry::BBox
TriSurfMesh<Basis>::get_bounding_box() const
{
  Core::Geometry::BBox result;
  typename Node::iterator ni, nie;
  begin(ni);
  end(nie);
  while (ni != nie)
  {
    result.extend(points_[*ni]);
    ++ni;
  }
  return (result);
}

template <class Basis>
void
TriSurfMesh<Basis>::get_canonical_transform(Core::Geometry::Transform &t) const
{
  t.load_identity();
  Core::Geometry::BBox bbox = get_bounding_box();
  t.pre_scale(bbox.diagonal());
  t.pre_translate(Core::Geometry::Vector(bbox.get_min()));
}

template <class Basis>
void
TriSurfMesh<Basis>::transform(const Core::Geometry::Transform &t)
{
  synchronize_lock_.lock();
  std::vector<Core::Geometry::Point>::iterator itr = points_.begin();
  std::vector<Core::Geometry::Point>::iterator eitr = points_.end();
  while (itr != eitr)
  {
    *itr = t.project(*itr);
    ++itr;
  }

  if (bbox_.valid())
  {
    bbox_.reset();

    // Compute bounding box
    typename Node::iterator ni, nie;
    begin(ni);
    end(nie);
    while (ni != nie)
    {
      bbox_.extend(point(*ni));
      ++ni;
    }

    // Compute epsilons associated with the bounding box
    epsilon_ = bbox_.diagonal().length()*1e-8;
    epsilon2_ = epsilon_*epsilon_;

    synchronized_ |= Mesh::BOUNDING_BOX_E;
  }

  if (node_grid_) { node_grid_->transform(t); }
  if (elem_grid_) { elem_grid_->transform(t); }

  synchronize_lock_.unlock();
}


template <class Basis>
void
TriSurfMesh<Basis>::begin(typename TriSurfMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on TriSurfMesh first");
  itr = 0;
}


template <class Basis>
void
TriSurfMesh<Basis>::end(typename TriSurfMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on TriSurfMesh first");
  itr = static_cast<index_type>(points_.size());
}


template <class Basis>
void
TriSurfMesh<Basis>::begin(typename TriSurfMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on TriSurfMesh first");
  itr = 0;
}


template <class Basis>
void
TriSurfMesh<Basis>::end(typename TriSurfMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on TriSurfMesh first");
  itr = static_cast<index_type>(edges_.size());
}


template <class Basis>
void
TriSurfMesh<Basis>::begin(typename TriSurfMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on TriSurfMesh first");
  itr = 0;
}


template <class Basis>
void
TriSurfMesh<Basis>::end(typename TriSurfMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on TriSurfMesh first");
  itr = static_cast<index_type>(faces_.size() / 3);
}


template <class Basis>
void
TriSurfMesh<Basis>::begin(typename TriSurfMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::CELLS_E,
            "Must call synchronize CELLS_E on TriSurfMesh first");
  itr = 0;
}


template <class Basis>
void
TriSurfMesh<Basis>::end(typename TriSurfMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::CELLS_E,
            "Must call synchronize CELLS_E on TriSurfMesh first");
  itr = 0;
}


template <class Basis>
int
TriSurfMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Face::array_type &l,
                                double *w)
{
  typename Face::index_type idx;
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
int
TriSurfMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Node::array_type &l,
                                double *w)
{
  typename Face::index_type idx;
  if (locate(idx, p))
  {
    get_nodes(l,idx);
    std::vector<double> coords(2);
    if (get_coords(coords, p, idx))
    {
      basis_.get_weights(coords, w);
      return basis_.dofs();
    }
  }
  return 0;
}


template <class Basis>
bool
TriSurfMesh<Basis>::inside3_p(index_type i, const Core::Geometry::Point &p) const
{
  const Core::Geometry::Point &p0 = points_[faces_[i+0]];
  const Core::Geometry::Point &p1 = points_[faces_[i+1]];
  const Core::Geometry::Point &p2 = points_[faces_[i+2]];
  Core::Geometry::Vector v01(p0-p1);
  Core::Geometry::Vector v02(p0-p2);
  Core::Geometry::Vector v0(p0-p);
  Core::Geometry::Vector v1(p1-p);
  Core::Geometry::Vector v2(p2-p);
  const double a = Cross(v01, v02).length(); // area of the whole triangle (2x)
  const double a0 = Cross(v1, v2).length();  // area opposite p0
  const double a1 = Cross(v2, v0).length();  // area opposite p1
  const double a2 = Cross(v0, v1).length();  // area opposite p2
  const double s = a0+a1+a2;

  // For the point to be inside a triangle it must be inside one
  // of the four triangles that can be formed by using three of the
  // triangle vertices and the point in question.
  return fabs(s - a) < epsilon2_ && a > epsilon2_;
}


template <class Basis>
bool
TriSurfMesh<Basis>::synchronize(mask_type sync)
{
  // Conversion table
  if (sync & (Mesh::DELEMS_E))
  { sync |= Mesh::EDGES_E; sync &= ~(Mesh::DELEMS_E); }

  if (sync & Mesh::FIND_CLOSEST_NODE_E)
  { sync |= NODE_LOCATE_E; sync &=  ~(Mesh::FIND_CLOSEST_NODE_E); }

  if (sync & Mesh::FIND_CLOSEST_ELEM_E)
  { sync |= ELEM_LOCATE_E; sync &=  ~(Mesh::FIND_CLOSEST_ELEM_E); }

  if (sync & (Mesh::NODE_LOCATE_E|Mesh::ELEM_LOCATE_E)) sync |= Mesh::BOUNDING_BOX_E;
  if (sync & Mesh::ELEM_NEIGHBORS_E) sync |= Mesh::EDGES_E;

  // Filter out the only tables available
  sync &= (Mesh::EDGES_E|Mesh::NORMALS_E|
           Mesh::NODE_NEIGHBORS_E|Mesh::BOUNDING_BOX_E|
           Mesh::ELEM_NEIGHBORS_E|
           Mesh::NODE_LOCATE_E|Mesh::ELEM_LOCATE_E);

  Core::Thread::UniqueLock lock(synchronize_lock_.get());

  // Only sync was hasn't been synched
  sync &= (~synchronized_);

  if (sync == Mesh::EDGES_E)
  {
    Synchronize Synchronize(this,sync);
    synchronize_lock_.unlock();
    Synchronize.run();
    synchronize_lock_.lock();
  }
  else if (sync & Mesh::EDGES_E)
  {
    mask_type tosync = Mesh::EDGES_E;
    Synchronize syncclass(this,tosync);
    boost::thread syncthread(syncclass);
  }

  if (sync == Mesh::NORMALS_E)
  {
    Synchronize Synchronize(this,sync);
    synchronize_lock_.unlock();
    Synchronize.run();
    synchronize_lock_.lock();
  }
  else if (sync & Mesh::NORMALS_E)
  {
    mask_type tosync = Mesh::NORMALS_E;
    Synchronize syncclass(this,tosync);
    boost::thread syncthread(syncclass);
  }

  if (sync == Mesh::NODE_NEIGHBORS_E)
  {
    Synchronize Synchronize(this,sync);
    synchronize_lock_.unlock();
    Synchronize.run();
    synchronize_lock_.lock();
  }
  else if (sync & Mesh::NODE_NEIGHBORS_E)
  {
    mask_type tosync = Mesh::NODE_NEIGHBORS_E;
    Synchronize syncclass(this,tosync);
    boost::thread syncthread(syncclass);
  }

  if (sync == Mesh::ELEM_NEIGHBORS_E)
  {
    Synchronize Synchronize(this,sync);
    synchronize_lock_.unlock();
    Synchronize.run();
    synchronize_lock_.lock();
  }
  else if (sync & Mesh::ELEM_NEIGHBORS_E)
  {
    mask_type tosync = Mesh::ELEM_NEIGHBORS_E;
    Synchronize syncclass(this,tosync);
    boost::thread syncthread(syncclass);
  }

  if (sync == Mesh::BOUNDING_BOX_E)
  {
    Synchronize Synchronize(this,sync);
    synchronize_lock_.unlock();
    Synchronize.run();
    synchronize_lock_.lock();
  }
  else if (sync & Mesh::BOUNDING_BOX_E)
  {
    mask_type tosync = Mesh::BOUNDING_BOX_E;
    Synchronize syncclass(this,tosync);
    boost::thread syncthread(syncclass);
  }

  if (sync == Mesh::NODE_LOCATE_E)
  {
    Synchronize Synchronize(this,sync);
    synchronize_lock_.unlock();
    Synchronize.run();
    synchronize_lock_.lock();
  }
  else if (sync & Mesh::NODE_LOCATE_E)
  {
    mask_type tosync = Mesh::NODE_LOCATE_E;
    Synchronize syncclass(this,tosync);
    boost::thread syncthread(syncclass);
  }

  if (sync == Mesh::ELEM_LOCATE_E)
  {
    Synchronize Synchronize(this,sync);
    synchronize_lock_.unlock();
    Synchronize.run();
    synchronize_lock_.lock();
  }
  else if (sync & Mesh::ELEM_LOCATE_E)
  {
    mask_type tosync = Mesh::ELEM_LOCATE_E;
    Synchronize syncclass(this,tosync);
    boost::thread syncthread(syncclass);
  }

  // Wait until threads are done
  while ((synchronized_ & sync) != sync)
  {
    synchronize_cond_.wait(lock);
  }

  return (true);
}

template <class Basis>
bool
TriSurfMesh<Basis>::unsynchronize(mask_type /*sync*/)
{
  return (true);
}

template <class Basis>
bool
TriSurfMesh<Basis>::clear_synchronization()
{
  synchronize_lock_.lock();
  // Undo marking the synchronization
  synchronized_ = Mesh::NODES_E | Mesh::ELEMS_E | Mesh::FACES_E | Mesh::CELLS_E;

  // Free memory where possible

  halfedge_to_edge_.clear();
  edge_neighbors_.clear();
  normals_.clear();
  edges_.clear();
  node_grid_.reset();
  elem_grid_.reset();

  synchronize_lock_.unlock();
  return (true);
}



template <class Basis>
void
TriSurfMesh<Basis>::compute_normals()
{
  normals_.resize(points_.size()); // 1 per node

  // build table of faces that touch each node
  std::vector<std::vector<typename Face::index_type> > node_in_faces(points_.size());
  /// face normals (not normalized) so that magnitude is also the area.
  std::vector<Core::Geometry::Vector> face_normals(faces_.size());
  // Computing normal per face.
  typename Node::array_type nodes(3);
  typename Face::iterator iter, iter_end;
  begin(iter);
  end(iter_end);
  while (iter != iter_end)
  {
    get_nodes(nodes, *iter);

    Core::Geometry::Point p1, p2, p3;
    get_point(p1, nodes[0]);
    get_point(p2, nodes[1]);
    get_point(p3, nodes[2]);
    // build table of faces that touch each node
    node_in_faces[nodes[0]].push_back(*iter);
    node_in_faces[nodes[1]].push_back(*iter);
    node_in_faces[nodes[2]].push_back(*iter);

    Core::Geometry::Vector v0 = p2 - p1;
    Core::Geometry::Vector v1 = p3 - p2;
    Core::Geometry::Vector n = Cross(v0, v1);
    face_normals[*iter] = n;
    ++iter;
  }

  //Averaging the normals.
  typename std::vector<std::vector<typename Face::index_type> >::iterator nif_iter =
    node_in_faces.begin();
  index_type i = 0;
  while (nif_iter != node_in_faces.end())
  {
    const std::vector<typename Face::index_type> &v = *nif_iter;
    typename std::vector<typename Face::index_type>::const_iterator fiter =
      v.begin();
    Core::Geometry::Vector ave(0.L,0.L,0.L);
    while(fiter != v.end())
    {
      ave += face_normals[*fiter];
      ++fiter;
    }
    ave.safe_normalize();
    normals_[i] = ave; ++i;
    ++nif_iter;
  }

  synchronize_lock_.lock();
  synchronized_ |= Mesh::NORMALS_E;
  synchronize_lock_.unlock();
}


template <class Basis>
void
TriSurfMesh<Basis>::insert_node(typename Face::index_type face, const Core::Geometry::Point &p)
{
  const bool do_neighbors = synchronized_ & Mesh::ELEM_NEIGHBORS_E;
  const bool do_normals = false; // synchronized_ & NORMALS_E;

  typename Node::index_type pi = add_point(p);
  const index_type f0 = face*3;
  const index_type f1 = faces_.size();
  const index_type f2 = f1+3;

  synchronize_lock_.lock();

  faces_.push_back(faces_[f0+1]);
  faces_.push_back(faces_[f0+2]);
  faces_.push_back(pi);

  faces_.push_back(faces_[f0+2]);
  faces_.push_back(faces_[f0+0]);
  faces_.push_back(pi);

  // must do last
  faces_[f0+2] = pi;

  if (do_neighbors)
  {
    edge_neighbors_.push_back(edge_neighbors_[f0+1]);
    if (edge_neighbors_.back() != MESH_NO_NEIGHBOR)
      edge_neighbors_[edge_neighbors_.back()] = edge_neighbors_.size()-1;
    edge_neighbors_.push_back(f2+2);
    edge_neighbors_.push_back(f0+1);

    edge_neighbors_.push_back(edge_neighbors_[f0+2]);
    if (edge_neighbors_.back() != MESH_NO_NEIGHBOR)
      edge_neighbors_[edge_neighbors_.back()] = edge_neighbors_.size()-1;
    edge_neighbors_.push_back(f0+2);
    edge_neighbors_.push_back(f1+1);

    edge_neighbors_[f0+1] = f1+2;
    edge_neighbors_[f0+2] = f2+1;
  }

  if (do_normals)
  {
    Core::Geometry::Vector normal = Core::Geometry::Vector(p +
                             normals_[faces_[f0]] +
                             normals_[faces_[f1]] +
                             normals_[faces_[f2]]);
    normal.safe_normalize();
    normals_.push_back(normals_[faces_[f1]]);
    normals_.push_back(normals_[faces_[f2]]);
    normals_.push_back(normal);

    normals_.push_back(normals_[faces_[f2]]);
    normals_.push_back(normals_[faces_[f0]]);
    normals_.push_back(normal);

    normals_[faces_[f0+2]] = normal;

  }

  if (!do_neighbors) synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  synchronized_ &= ~(Mesh::EDGES_E);
  if (!do_normals) synchronized_ &= ~Mesh::NORMALS_E;

  synchronize_lock_.unlock();
}


template <class Basis>
bool
TriSurfMesh<Basis>::insert_node(const Core::Geometry::Point &p)
{
  typename Face::index_type face;
  if (!locate(face,p)) return false;
  insert_node(face,p);
  return true;
}


template <class Basis>
void
TriSurfMesh<Basis>::debug_test_edge_neighbors()
{
  for (index_type i = 0; i < static_cast<index_type>(edge_neighbors_.size()); i++)
  {
    if (edge_neighbors_[i] != MESH_NO_NEIGHBOR &&
        edge_neighbors_[edge_neighbors_[i]] != i)
    {
//      cout << "bad nbr[" << i << "] = " << edge_neighbors_[i] << ", nbr[" << edge_neighbors_[i] << "] = " << edge_neighbors_[edge_neighbors_[i]] << "\n";
    }
  }
}

template <class Basis>
bool
TriSurfMesh<Basis>::insert_node_in_edge_aux(typename Face::array_type &tris,
                                            typename Node::index_type &ni,
                                            index_type halfedge,
                                            const Core::Geometry::Point &p)
{
  ni = add_point(p);

  synchronize_lock_.lock();

  remove_elem_from_grid(halfedge/3);

  tris.clear();

  const index_type nbr = edge_neighbors_[halfedge];

  // f1
  const index_type f1 = static_cast<index_type>(faces_.size());

  tris.push_back(f1 / 3);
  faces_.push_back(ni);
  faces_.push_back(faces_[next(halfedge)]);
  faces_.push_back(faces_[prev(halfedge)]);
  edge_neighbors_.push_back(nbr);
  edge_neighbors_.push_back(edge_neighbors_[next(halfedge)]);
  edge_neighbors_.push_back(next(halfedge));

  if (edge_neighbors_[next(halfedge)] != MESH_NO_NEIGHBOR)
  {
    edge_neighbors_[edge_neighbors_[next(halfedge)]] = next(f1);
  }

  const index_type f3 = static_cast<index_type>(faces_.size()); // Only created if there's a neighbor.

  // f0
  tris.push_back(halfedge / 3);
  faces_[next(halfedge)] = ni;
  edge_neighbors_[halfedge] = (nbr!=MESH_NO_NEIGHBOR)?f3:MESH_NO_NEIGHBOR;
  edge_neighbors_[next(halfedge)] = prev(f1);
  edge_neighbors_[prev(halfedge)] = edge_neighbors_[prev(halfedge)];

  if (nbr != MESH_NO_NEIGHBOR)
  {
    remove_elem_from_grid(nbr / 3);

    // f3
    tris.push_back(f3 / 3);
    faces_.push_back(ni);
    faces_.push_back(faces_[next(nbr)]);
    faces_.push_back(faces_[prev(nbr)]);
    edge_neighbors_.push_back(halfedge);
    edge_neighbors_.push_back(edge_neighbors_[next(nbr)]);
    edge_neighbors_.push_back(next(nbr));

    if (edge_neighbors_[next(nbr)] != MESH_NO_NEIGHBOR)
    {
      edge_neighbors_[edge_neighbors_[next(nbr)]] = next(f3);
    }

    // f2
    tris.push_back(nbr / 3);
    faces_[next(nbr)] = ni;
    edge_neighbors_[nbr] = f1;
    edge_neighbors_[next(nbr)] = f3+2;
  }

  debug_test_edge_neighbors();

  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  synchronized_ &= ~(Mesh::EDGES_E);
  synchronized_ &= ~Mesh::NORMALS_E;

  for (size_t i = 0; i < tris.size(); i++)
  {
    insert_elem_into_grid(tris[i]);
  }

  synchronize_lock_.unlock();

  return true;
}


template <class Basis>
bool
TriSurfMesh<Basis>::insert_node_in_face_aux(typename Face::array_type &tris,
                                            typename Node::index_type &ni,
                                            typename Face::index_type face,
                                            const Core::Geometry::Point &p)
{
  ni = add_point(p);

  synchronize_lock_.lock();

  remove_elem_from_grid(face);

  const index_type f0 = face*3;
  const index_type f1 = static_cast<index_type>(faces_.size());
  const index_type f2 = f1+3;

  tris.clear();

  if (edge_neighbors_[f0+1] != MESH_NO_NEIGHBOR)
  {
    edge_neighbors_[edge_neighbors_[f0+1]] = f1+0;
  }
  if (edge_neighbors_[f0+2] != MESH_NO_NEIGHBOR)
  {
    edge_neighbors_[edge_neighbors_[f0+2]] = f2+0;
  }

  tris.push_back(faces_.size() / 3);
  faces_.push_back(faces_[f0+1]);
  faces_.push_back(faces_[f0+2]);
  faces_.push_back(ni);
  edge_neighbors_.push_back(edge_neighbors_[f0+1]);
  edge_neighbors_.push_back(f2+2);
  edge_neighbors_.push_back(f0+1);

  tris.push_back(faces_.size() / 3);
  faces_.push_back(faces_[f0+2]);
  faces_.push_back(faces_[f0+0]);
  faces_.push_back(ni);
  edge_neighbors_.push_back(edge_neighbors_[f0+2]);
  edge_neighbors_.push_back(f0+2);
  edge_neighbors_.push_back(f1+1);

  // Must do last
  tris.push_back(face);
  faces_[f0+2] = ni;
  edge_neighbors_[f0+1] = f1+2;
  edge_neighbors_[f0+2] = f2+1;

  debug_test_edge_neighbors();

  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  synchronized_ &= ~(Mesh::EDGES_E);
  synchronized_ &= ~Mesh::NORMALS_E;

  for (size_t i = 0; i < tris.size(); i++)
  {
    insert_elem_into_grid(tris[i]);
  }

  synchronize_lock_.unlock();

  return true;
}


template <class Basis>
bool
TriSurfMesh<Basis>::insert_node_in_face(typename Face::array_type &tris,
                                        typename Node::index_type &ni,
                                        typename Face::index_type face,
                                        const Core::Geometry::Point &p)
{
  const Core::Geometry::Point &p0 = point(faces_[face * 3 + 0]);
  const Core::Geometry::Point &p1 = point(faces_[face * 3 + 1]);
  const Core::Geometry::Point &p2 = point(faces_[face * 3 + 2]);

  const double a0 = Cross(p - p1, p - p2).length2();
  const double a1 = Cross(p - p2, p - p0).length2();
  const double a2 = Cross(p - p0, p - p1).length2();

  mask_type mask = 0;
  if (a0 >= epsilon2_*epsilon2_) { mask |= 1; }
  if (a1 >= epsilon2_*epsilon2_) { mask |= 2; }
  if (a2 >= epsilon2_*epsilon2_) { mask |= 4; }

  if (mask == 7)
  {
    // Core::Geometry::Point is inside the face, do a three split.
    return insert_node_in_face_aux(tris, ni, face, p);
  }
  else if (mask == 0)
  {
    // Tri is degenerate, just return first point.
    tris.clear();
    tris.push_back(face);
    ni = faces_[face * 3 + 0];
    return true;
  }
  // The point is on a corner, return that corner.
  else if (mask == 1)
  {
    tris.clear();
    tris.push_back(face);
    ni = faces_[face * 3 + 0];
    return true;
  }
  else if (mask == 2)
  {
    tris.clear();
    tris.push_back(face);
    ni = faces_[face * 3 + 1];
    return true;
  }
  else if (mask == 4)
  {
    tris.clear();
    tris.push_back(face);
    ni = faces_[face * 3 + 2];
    return true;
  }
  // The point is on an edge, split that edge and neighboring triangle.
  else if (mask == 3)
  {
    return insert_node_in_edge_aux(tris, ni, face*3+0, p);
  }
  else if (mask == 5)
  {
    return insert_node_in_edge_aux(tris, ni, face*3+2, p);
  }
  else if (mask == 6)
  {
    return insert_node_in_edge_aux(tris, ni, face*3+1, p);
  }
  return false;
}


template <class Basis>
void
TriSurfMesh<Basis>::collapse_edges(const std::vector<index_type> &nodemap)
{
  for (size_t i = 0; i < faces_.size(); i++)
  {
    faces_[i] = nodemap[faces_[i]];
  }
}


template <class Basis>
void
TriSurfMesh<Basis>::remove_obvious_degenerate_triangles()
{
  std::vector<index_type> oldfaces = faces_;
  faces_.clear();
  for (size_t i = 0; i< oldfaces.size(); i+=3)
  {
    index_type a = oldfaces[i+0];
    index_type b = oldfaces[i+1];
    index_type c = oldfaces[i+2];
    if (a != b && a != c && b != c)
    {
      faces_.push_back(a);
      faces_.push_back(b);
      faces_.push_back(c);
    }
  }
  synchronized_ = NODES_E | FACES_E | CELLS_E;
}


/*             2
//             ^
//            / \
//           /f3 \
//        5 /-----\ 4
//         / \fac/ \
//        /f1 \ /f2 \
//       /     V     \
//      <------------->
//     0       3       1
*/

#define DEBUGINFO(f) cerr << "Face #" << f/3 << " N1: " << faces_[f+0] << " N2: " << faces_[f+1] << " N3: " << faces_[f+2] << "  B1: " << edge_neighbors_[f] << " B2: " << edge_neighbors_[f+1] << "  B3: " << edge_neighbors_[f+2] << endl;
template <class Basis>

void
TriSurfMesh<Basis>::bisect_element(const typename Face::index_type face)
{
  const bool do_neighbors = synchronized_ & Mesh::ELEM_NEIGHBORS_E;
  const bool do_normals = false; //synchronized_ & NORMALS_E;

  const index_type f0 = face*3;
  typename Node::array_type nodes;
  get_nodes(nodes,face);
  std::vector<Core::Geometry::Vector> normals(3);
  for (index_type edge = 0; edge < 3; ++edge)
  {
    Core::Geometry::Point p = ((points_[faces_[f0+edge]] +
                points_[faces_[next(f0+edge)]]) / 2.0).asPoint();
    nodes[edge] = add_point(p);

    if (do_normals)
    {
      normals[edge] = Core::Geometry::Vector(normals_[faces_[f0+edge]] +
                             normals_[faces_[next(f0+edge)]]);
      normals[edge].safe_normalize();
    }
  }

  synchronize_lock_.lock();

  const index_type f1 = static_cast<index_type>(faces_.size());
  faces_.push_back(nodes[0]);
  faces_.push_back(nodes[3]);
  faces_.push_back(nodes[5]);

  const index_type f2 = static_cast<index_type>(faces_.size());
  faces_.push_back(nodes[1]);
  faces_.push_back(nodes[4]);
  faces_.push_back(nodes[3]);

  const index_type f3 = static_cast<index_type>(faces_.size());
  faces_.push_back(nodes[2]);
  faces_.push_back(nodes[5]);
  faces_.push_back(nodes[4]);

  faces_[f0+0] = nodes[3];
  faces_[f0+1] = nodes[4];
  faces_[f0+2] = nodes[5];


  if (do_neighbors)
  {
    edge_neighbors_.push_back(edge_neighbors_[f0+0]);
    edge_neighbors_.push_back(f0+2);
    edge_neighbors_.push_back(MESH_NO_NEIGHBOR);

    edge_neighbors_.push_back(edge_neighbors_[f0+1]);
    edge_neighbors_.push_back(f0+0);
    edge_neighbors_.push_back(MESH_NO_NEIGHBOR);

    edge_neighbors_.push_back(edge_neighbors_[f0+2]);
    edge_neighbors_.push_back(f0+1);
    edge_neighbors_.push_back(MESH_NO_NEIGHBOR);

    // must do last
    edge_neighbors_[f0+0] = f2+1;
    edge_neighbors_[f0+1] = f3+1;
    edge_neighbors_[f0+2] = f1+1;
  }

  if (do_normals)
  {
    normals_.push_back(normals_[f0+0]);
    normals_.push_back(normals[0]);
    normals_.push_back(normals[2]);

    normals_.push_back(normals_[f0+1]);
    normals_.push_back(normals[1]);
    normals_.push_back(normals[0]);

    normals_.push_back(normals_[f0+2]);
    normals_.push_back(normals[2]);
    normals_.push_back(normals[1]);

    normals_[f0+0] = normals[0];
    normals_[f0+1] = normals[1];
    normals_[f0+2] = normals[2];
  }

  if (do_neighbors && edge_neighbors_[f1] != MESH_NO_NEIGHBOR)
  {
    const index_type nbr = edge_neighbors_[f1];
    const index_type pnbr = prev(nbr);
    const index_type f4 = static_cast<index_type>(faces_.size());
    faces_.push_back(nodes[1]);
    faces_.push_back(nodes[3]);
    faces_.push_back(faces_[pnbr]);
    edge_neighbors_[f2+2] = f4;
    edge_neighbors_.push_back(f2+2);
    edge_neighbors_.push_back(pnbr);
    edge_neighbors_.push_back(edge_neighbors_[pnbr]);
    edge_neighbors_[edge_neighbors_.back()] = f4+2;
    faces_[nbr] = nodes[3];
    edge_neighbors_[pnbr] = f4+1;
    if (do_normals)
    {
      normals_[nbr] = normals[0];
      normals_.push_back(normals_[f0+1]);
      normals_.push_back(normals[0]);
      normals_.push_back(normals_[pnbr]);
    }
  }

  if (do_neighbors && edge_neighbors_[f2] != MESH_NO_NEIGHBOR)
  {
    const index_type nbr = edge_neighbors_[f2];
    const index_type pnbr = prev(nbr);
    const index_type f5 = static_cast<index_type>(faces_.size());
    faces_.push_back(nodes[2]);
    faces_.push_back(nodes[4]);
    faces_.push_back(faces_[pnbr]);
    edge_neighbors_[f3+2] = f5;
    edge_neighbors_.push_back(f3+2);
    edge_neighbors_.push_back(pnbr);
    edge_neighbors_.push_back(edge_neighbors_[pnbr]);
    edge_neighbors_[edge_neighbors_.back()] = f5+2;
    faces_[nbr] = nodes[4];
    edge_neighbors_[pnbr] = f5+1;
    if (do_normals)
    {
      normals_[nbr] = normals[1];
      normals_.push_back(normals_[f0+2]);
      normals_.push_back(normals[1]);
      normals_.push_back(normals_[pnbr]);
    }
  }

  if (do_neighbors && edge_neighbors_[f3] != MESH_NO_NEIGHBOR)
  {
    const index_type nbr = edge_neighbors_[f3];
    const index_type pnbr = prev(nbr);
    const index_type f6 = static_cast<index_type>(faces_.size());
    faces_.push_back(nodes[0]);
    faces_.push_back(nodes[5]);
    faces_.push_back(faces_[pnbr]);
    edge_neighbors_[f1+2] = f6;
    edge_neighbors_.push_back(f1+2);
    edge_neighbors_.push_back(pnbr);
    edge_neighbors_.push_back(edge_neighbors_[pnbr]);
    edge_neighbors_[edge_neighbors_.back()] = f6+2;
    faces_[nbr] = nodes[5];
    edge_neighbors_[pnbr] = f6+1;
    if (do_normals)
    {
      normals_[nbr] = normals[2];
      normals_.push_back(normals_[f0+0]);
      normals_.push_back(normals[2]);
      normals_.push_back(normals_[pnbr]);
    }
  }

  if (!do_neighbors) synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  synchronized_ &= ~(Mesh::EDGES_E);
  if (!do_normals) synchronized_ &= ~Mesh::NORMALS_E;

  synchronize_lock_.unlock();
}


template <class Basis>
void
TriSurfMesh<Basis>::compute_node_neighbors()
{
  node_neighbors_.clear();
  node_neighbors_.resize(points_.size());
  size_type nfaces = static_cast<size_type>(faces_.size());
  for (index_type f = 0; f < nfaces; ++f)
  {
    node_neighbors_[faces_[f]].push_back(f/3);
  }
  synchronize_lock_.lock();
  synchronized_ |= Mesh::NODE_NEIGHBORS_E;
  synchronize_lock_.unlock();
}


template <class Basis>
void
TriSurfMesh<Basis>::compute_edges()
{
  EdgeMapType2 edge_map;

  size_type num_faces = static_cast<index_type>(faces_.size())/3;
  for (index_type i=0; i < num_faces; i++)
  {
    index_type n0 = faces_[3*i];
    index_type n1 = faces_[3*i+1];
    index_type n2 = faces_[3*i+2];

    if (n0 > n1) edge_map[std::pair<index_type, index_type>(n1,n0)].push_back(i<<2);
    else  edge_map[std::pair<index_type, index_type>(n0,n1)].push_back(i<<2);

    if (n1 > n2) edge_map[std::pair<index_type, index_type>(n2,n1)].push_back((i<<2)+1);
    else  edge_map[std::pair<index_type, index_type>(n1,n2)].push_back((i<<2)+1);

    if (n2 > n0) edge_map[std::pair<index_type, index_type>(n0,n2)].push_back((i<<2)+2);
    else  edge_map[std::pair<index_type, index_type>(n2,n0)].push_back((i<<2)+2);
  }

  typename EdgeMapType2::iterator itr;
  edges_.clear();
  edges_.resize(edge_map.size());
  halfedge_to_edge_.resize(faces_.size());

  size_t k=0;
  for (itr = edge_map.begin(); itr != edge_map.end(); ++itr)
  {
    const std::vector<index_type >& hedges = (*itr).second;
    for (size_t j=0; j<hedges.size(); j++)
    {
      index_type h = hedges[j];
      edges_[k].push_back(h);
      halfedge_to_edge_[(h>>2)*3 + (h&0x3)] = k;

    }
    k++;
  }

  synchronize_lock_.lock();
  synchronized_ |= (Mesh::EDGES_E);
  synchronize_lock_.unlock();
}

// Fixes bug #887 (gforge)
template <class Basis>
void
TriSurfMesh<Basis>::compute_edges_bugfix()
{
  EdgeMapType2 edge_map;

  size_type num_faces = static_cast<index_type>(faces_.size())/3;
  for (index_type i=0; i < num_faces; i++)
  {
    index_type n0 = faces_[3*i];
    index_type n1 = faces_[3*i+1];
    index_type n2 = faces_[3*i+2];

    if (n0 > n1) edge_map[std::pair<index_type, index_type>(n1,n0)].push_back(i<<2);
    else  edge_map[std::pair<index_type, index_type>(n0,n1)].push_back(i<<2);

    if (n1 > n2) edge_map[std::pair<index_type, index_type>(n2,n1)].push_back((i<<2)+1);
    else  edge_map[std::pair<index_type, index_type>(n1,n2)].push_back((i<<2)+1);

    if (n2 > n0) edge_map[std::pair<index_type, index_type>(n0,n2)].push_back((i<<2)+2);
    else  edge_map[std::pair<index_type, index_type>(n2,n0)].push_back((i<<2)+2);
  }

  typename EdgeMapType2::iterator itr;
  edges_.clear();
  edges_.resize(edge_map.size());
  halfedge_to_edge_.resize(faces_.size());
  edge_on_node_.clear();
  edge_on_node_.resize(points_.size());

  size_t k=0;
  for (itr = edge_map.begin(); itr != edge_map.end(); ++itr)
  {
    const std::vector<index_type >& hedges = (*itr).second;
    for (size_t j=0; j<hedges.size(); j++)
    {
      index_type h = hedges[j];
      edges_[k].push_back(h);
      halfedge_to_edge_[(h>>2)*3 + (h&0x3)] = k;
    }
    edge_on_node_[(*itr).first.first].push_back(k);
    edge_on_node_[(*itr).first.second].push_back(k);
    k++;
  }

  synchronize_lock_.lock();
  synchronized_ |= (Mesh::EDGES_E);
  synchronize_lock_.unlock();
}

template <class Basis>
typename TriSurfMesh<Basis>::Node::index_type
TriSurfMesh<Basis>::add_find_point(const Core::Geometry::Point &p, double err)
{
  typename Node::index_type i;
  if (search_node(i, p) && (p - points_[i]).length2() < err)
  {
    return i;
  }
  else
  {
    synchronize_lock_.lock();
    points_.push_back(p);
    node_neighbors_.push_back(std::vector<under_type>());
    synchronize_lock_.unlock();
    return static_cast<typename Node::index_type>(points_.size() - 1);
  }
}


// swap the shared edge between 2 faces. If faces don't share an edge,
// do nothing.
template <class Basis>
bool
TriSurfMesh<Basis>::swap_shared_edge(typename Face::index_type f1,
                                     typename Face::index_type f2)
{
  const index_type face1 = f1 * 3;
  std::set<index_type, less_int> shared;
  shared.insert(faces_[face1]);
  shared.insert(faces_[face1 + 1]);
  shared.insert(faces_[face1 + 2]);

  index_type not_shar[2];
  index_type *ns = not_shar;
  const index_type face2 = f2 * 3;
  std::pair<std::set<index_type, less_int>::iterator, bool> p = shared.insert(faces_[face2]);
  if (!p.second) { *ns = faces_[face2]; ++ns;}
  p = shared.insert(faces_[face2 + 1]);
  if (!p.second) { *ns = faces_[face2 + 1]; ++ns;}
  p = shared.insert(faces_[face2 + 2]);
  if (!p.second) { *ns = faces_[face2 + 2]; }

  // no shared nodes means no shared edge.
  if (shared.size() > 4) return false;

  std::set<index_type, less_int>::iterator iter = shared.find(not_shar[0]);
  shared.erase(iter);

  iter = shared.find(not_shar[1]);
  shared.erase(iter);

  iter = shared.begin();
  index_type s1 = *iter++;
  index_type s2 = *iter;

  synchronize_lock_.lock();
  faces_[face1] = s1;
  faces_[face1 + 1] = not_shar[0];
  faces_[face1 + 2] = s2;

  faces_[face2] = s2;
  faces_[face2 + 1] = not_shar[1];
  faces_[face2 + 2] = s1;

  synchronized_ &= ~Mesh::ELEM_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NORMALS_E;
  synchronize_lock_.unlock();

  return true;
}

template <class Basis>
bool
TriSurfMesh<Basis>::remove_orphan_nodes()
{
  bool rval = false;

  /// find the orphan nodes.
  std::vector<index_type> onodes;
  /// check each point against the face list.
  for (index_type i = 0; i < static_cast<index_type>(points_.size()); i++) {
    if (find(faces_.begin(), faces_.end(), i) == faces_.end()) {
      /// node does not belong to a face
      onodes.push_back(i);
    }
  }

  if (onodes.size()) rval = true;

  /// check each point against the face list.
  std::vector<index_type>::reverse_iterator orph_iter = onodes.rbegin();
  while (orph_iter != onodes.rend())
  {
    index_type i = *orph_iter++;
    std::vector<index_type>::iterator iter = faces_.begin();
    while (iter != faces_.end())
    {
      index_type &node = *iter++;
      if (node > i)
      {
        node--;
      }
    }
    std::vector<Core::Geometry::Point>::iterator niter = points_.begin();
    niter += i;
    points_.erase(niter);
  }

  synchronized_ &= ~Mesh::ELEM_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NORMALS_E;
  return rval;
}

template <class Basis>
bool
TriSurfMesh<Basis>::remove_face(typename Face::index_type f)
{
  bool rval = true;

  synchronize_lock_.lock();
  std::vector<under_type>::iterator fb = faces_.begin() + f*3;
  std::vector<under_type>::iterator fe = fb + 3;

  if (fe <= faces_.end())
    faces_.erase(fb, fe);
  else {
    rval = false;
  }
  synchronized_ &= ~Mesh::ELEM_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NORMALS_E;
  synchronize_lock_.unlock();

  return rval;
}


template <class Basis>
typename TriSurfMesh<Basis>::Elem::index_type
TriSurfMesh<Basis>::add_triangle(typename Node::index_type a,
                                 typename Node::index_type b,
                                 typename Node::index_type c)
{
  synchronize_lock_.lock();
  faces_.push_back(a);
  faces_.push_back(b);
  faces_.push_back(c);
  synchronized_ &= ~Mesh::ELEM_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NORMALS_E;
  synchronize_lock_.unlock();
  return static_cast<typename Elem::index_type>((static_cast<index_type>(faces_.size()) / 3) - 1);
}


template <class Basis>
void
TriSurfMesh<Basis>::flip_faces()
{
  synchronize_lock_.lock();
  typename Face::iterator fiter, fend;
  begin(fiter);
  end(fend);
  while (fiter != fend)
  {
    flip_face(*fiter);
    ++fiter;
  }
  synchronized_ &= ~(Mesh::EDGES_E);
  synchronized_ &= ~Mesh::ELEM_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NORMALS_E;
  synchronize_lock_.unlock();
}


template <class Basis>
void
TriSurfMesh<Basis>::flip_face(typename Face::index_type face)
{
  const index_type base = face * 3;
  index_type tmp = faces_[base + 1];
  faces_[base + 1] = faces_[base + 2];
  faces_[base + 2] = tmp;

  synchronized_ &= ~(Mesh::EDGES_E);
  synchronized_ &= ~Mesh::ELEM_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NORMALS_E;
}


template <class Basis>
void
TriSurfMesh<Basis>::walk_face_orient(typename Face::index_type face,
                                     std::vector<bool> &tested,
                                     std::vector<bool> &flip)
{
  tested[face] = true;
  for (index_type i = 0; i < 3; i++)
  {
    const index_type edge = face * 3 + i;
    const index_type nbr = edge_neighbors_[edge];
    if (nbr != MESH_NO_NEIGHBOR && !tested[nbr/3])
    {
      if ((!flip[face] && (faces_[edge] == faces_[nbr])) ||
          (flip[face] && (faces_[next(edge)] == faces_[nbr])))
      {
        flip[nbr/3] = true;
      }
      walk_face_orient(nbr/3, tested, flip);
    }
  }
}

template <class Basis>
void
TriSurfMesh<Basis>::orient_faces()
{
  synchronize(Mesh::EDGES_E | Mesh::ELEM_NEIGHBORS_E);
  synchronize_lock_.lock();

  index_type nfaces = (int)faces_.size() / 3;
  std::vector<bool> tested(nfaces, false);
  std::vector<bool> flip(nfaces, false);

  typename Face::iterator fiter, fend;
  begin(fiter);
  end(fend);
  while (fiter != fend)
  {
    if (! tested[*fiter])
    {
      walk_face_orient(*fiter, tested, flip);
    }
    ++fiter;
  }

  begin(fiter);
  while (fiter != fend)
  {
    if (flip[*fiter])
    {
      flip_face(*fiter);
    }
    ++fiter;
  }

  synchronized_ &= ~(Mesh::EDGES_E);
  synchronized_ &= ~Mesh::ELEM_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NORMALS_E;
  synchronize_lock_.unlock();
}


template <class Basis>
void
TriSurfMesh<Basis>::compute_edge_neighbors()
{
  EdgeMapType edge_map;

  edge_neighbors_.resize(faces_.size());
  for (size_t j = 0; j < edge_neighbors_.size(); j++)
  {
    edge_neighbors_[j] = MESH_NO_NEIGHBOR;
  }

  index_type i;
  for (i=static_cast<index_type>(faces_.size()-1); i >= 0; i--)
  {
    const index_type a = i;
    const index_type b = a - a % 3 + (a+1) % 3;

    index_type n0 = faces_[a];
    index_type n1 = faces_[b];
    index_type tmp;
    if (n0 > n1) { tmp = n0; n0 = n1; n1 = tmp; }

    std::pair<index_type, index_type> nodes(n0, n1);

    typename EdgeMapType::iterator maploc;

    maploc = edge_map.find(nodes);
    if (maploc != edge_map.end())
    {
      edge_neighbors_[(*maploc).second] = i;
      edge_neighbors_[i] = (*maploc).second;
    }
    edge_map[nodes] = i;
  }

  debug_test_edge_neighbors();

  synchronize_lock_.lock();
  synchronized_ |= Mesh::ELEM_NEIGHBORS_E;
  synchronize_lock_.unlock();
}


template <class Basis>
void
TriSurfMesh<Basis>::insert_elem_into_grid(typename Elem::index_type ci)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.
  const index_type idx = ci*3;
  Core::Geometry::BBox box;
  box.extend(points_[faces_[idx]]);
  box.extend(points_[faces_[idx+1]]);
  box.extend(points_[faces_[idx+2]]);
  box.extend(epsilon_);
  elem_grid_->insert(ci, box);
}


template <class Basis>
void
TriSurfMesh<Basis>::remove_elem_from_grid(typename Elem::index_type ci)
{
  const index_type idx = ci*3;
  Core::Geometry::BBox box;
  box.extend(points_[faces_[idx]]);
  box.extend(points_[faces_[idx+1]]);
  box.extend(points_[faces_[idx+2]]);
  box.extend(epsilon_);
  elem_grid_->remove(ci, box);
}


template <class Basis>
void
TriSurfMesh<Basis>::insert_node_into_grid(typename Node::index_type ni)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.
  node_grid_->insert(ni,points_[ni]);
}


template <class Basis>
void
TriSurfMesh<Basis>::remove_node_from_grid(typename Node::index_type ni)
{
  node_grid_->remove(ni,points_[ni]);
}


template <class Basis>
void
TriSurfMesh<Basis>::compute_elem_grid()
{
  if (bbox_.valid())
  {
    // Cubed root of number of cells to get a subdivision ballpark.

    typename Elem::size_type esz;  size(esz);

    const size_type s =
      3*static_cast<size_type>((ceil(pow(static_cast<double>(esz) , (1.0/3.0))))/2.0 + 1.0);

    Core::Geometry::Vector diag  = bbox_.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(0.5+diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(0.5+diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(0.5+diag.z()/trace*s));

    Core::Geometry::BBox b = bbox_; b.extend(10*epsilon_);
    elem_grid_.reset(new SearchGridT<index_type>(sx, sy, sz, b.get_min(), b.get_max()));

    typename Elem::iterator ci, cie;
    begin(ci); end(cie);
    while(ci != cie)
    {
      insert_elem_into_grid(*ci);
      ++ci;
    }
  }

  synchronize_lock_.lock();
  synchronized_ |= Mesh::ELEM_LOCATE_E;
  synchronize_lock_.unlock();
}


template <class Basis>
void
TriSurfMesh<Basis>::compute_node_grid()
{
  if (bbox_.valid())
  {
    // Cubed root of number of cells to get a subdivision ballpark.

    typename Elem::size_type esz;  size(esz);

    const size_type s =  3*static_cast<size_type>
                  ((ceil(pow(static_cast<double>(esz) , (1.0/3.0))))/2.0 + 1.0);

    Core::Geometry::Vector diag  = bbox_.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(0.5+diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(0.5+diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(0.5+diag.z()/trace*s));

    Core::Geometry::BBox b = bbox_; b.extend(10*epsilon_);
    node_grid_.reset(new SearchGridT<index_type>(sx, sy, sz, b.get_min(), b.get_max()));

    typename Node::iterator ni, nie;
    begin(ni); end(nie);

    while(ni != nie)
    {
      insert_node_into_grid(*ni);
      ++ni;
    }
  }

  synchronize_lock_.lock();
  synchronized_ |= Mesh::NODE_LOCATE_E;
  synchronize_lock_.unlock();
}


template <class Basis>
void
TriSurfMesh<Basis>::compute_bounding_box()
{
  bbox_.reset();

  // Compute bounding box
  typename Node::iterator ni, nie;
  begin(ni);
  end(nie);
  while (ni != nie)
  {
    bbox_.extend(point(*ni));
    ++ni;
  }

  // Compute epsilons associated with the bounding box
  epsilon_ = bbox_.diagonal().length()*1e-8;
  epsilon2_ = epsilon_*epsilon_;

  synchronize_lock_.lock();
  synchronized_ |= Mesh::BOUNDING_BOX_E;
  synchronize_lock_.unlock();
}


template <class Basis>
typename TriSurfMesh<Basis>::Node::index_type
TriSurfMesh<Basis>::add_point(const Core::Geometry::Point &p)
{
  points_.push_back(p);
  return static_cast<typename Node::index_type>(points_.size() - 1);
}



template <class Basis>
typename TriSurfMesh<Basis>::Elem::index_type
TriSurfMesh<Basis>::add_triangle(const Core::Geometry::Point &p0,
                                 const Core::Geometry::Point &p1,
                                 const Core::Geometry::Point &p2)
{
  return add_triangle(add_find_point(p0), add_find_point(p1), add_find_point(p2));
}

#define TRISURFMESH_VERSION 4

template <class Basis>
void
TriSurfMesh<Basis>::io(Piostream &stream)
{
  int version = stream.begin_class(type_name(-1), TRISURFMESH_VERSION);

  Mesh::io(stream);

  Pio(stream, points_);
  Pio_index(stream, faces_);

  if (version < 4)
  {
// Reading data we do not use anymore
    std::vector<unsigned int> old;
    Pio(stream, old);
  }

  if (version >= 2)
  {
    basis_.io(stream);
  }

  stream.end_class();

  if (stream.reading() && edge_neighbors_.size())
  {
    synchronized_ |= Mesh::ELEM_NEIGHBORS_E;
  }

  if (stream.reading())
    vmesh_.reset(CreateVTriSurfMesh(this));
}

template <class Basis>
void
TriSurfMesh<Basis>::size(typename TriSurfMesh::Node::size_type &s) const
{
  typename Node::iterator itr; end(itr);
  s = *itr;
}


template <class Basis>
void
TriSurfMesh<Basis>::size(typename TriSurfMesh::Edge::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on TriSurfMesh first");

  typename Edge::iterator itr; end(itr);
  s = *itr;
}


template <class Basis>
void
TriSurfMesh<Basis>::size(typename TriSurfMesh::Face::size_type &s) const
{
  typename Face::iterator itr; end(itr);
  s = *itr;
}


template <class Basis>
void
TriSurfMesh<Basis>::size(typename TriSurfMesh::Cell::size_type &s) const
{
  typename Cell::iterator itr; end(itr);
  s = *itr;
}


template <class Basis>
const TypeDescription*
get_type_description(TriSurfMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("TriSurfMesh", subs,
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
TriSurfMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((TriSurfMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
TriSurfMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((TriSurfMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Node",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
TriSurfMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((TriSurfMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Edge",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
TriSurfMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((TriSurfMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Face",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
TriSurfMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((TriSurfMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Cell",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

template <class Basis>
bool
TriSurfMesh<Basis>::get_neighbor(index_type &nbr_half_edge,
                                 index_type half_edge) const
{
  ASSERTMSG(synchronized_ & ELEM_NEIGHBORS_E,
            "Must call synchronize ELEM_NEIGHBORS_E on TriSurfMesh first");
  nbr_half_edge = edge_neighbors_[half_edge];
  return nbr_half_edge != MESH_NO_NEIGHBOR;
}

} // namespace SCIRun


#endif
