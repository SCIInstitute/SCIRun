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


#ifndef CORE_DATATYPES_QUADSURFMESH_H
#define CORE_DATATYPES_QUADSURFMESH_H 1

/// Include what kind of support we want to have
/// Need to fix this and couple it sci-defs
#include <Core/Datatypes/Legacy/Field/MeshSupport.h>

#include <Core/Containers/StackVector.h>

#include <Core/GeometryPrimitives/SearchGridT.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/CompGeom.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/Vector.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/QuadBilinearLgn.h>
#include <Core/Basis/QuadBiquadraticLgn.h>
#include <Core/Basis/QuadBicubicHmt.h>

#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/FieldIterator.h>
#include <Core/Datatypes/Legacy/Field/FieldRNG.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>

#include <Core/Thread/Mutex.h>
#include <Core/Thread/ConditionVariable.h>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <Core/Persistent/PersistentSTL.h>

/// Needed for some specialized functions
#include <set>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

/////////////////////////////////////////////////////
// Declarations for virtual interface

template <class Basis> class QuadSurfMesh;

/// make sure any other mesh other than the preinstantiate ones
/// returns no virtual interface. Altering this behavior will allow
/// for dynamically compiling the interface if needed.
template<class MESH>
VMesh* CreateVQuadSurfMesh(MESH* mesh) { return (0); }

#if (SCIRUN_QUADSURF_SUPPORT > 0)
/// These declarations are needed for a combined dynamic compilation as
/// as well as virtual functions solution.
/// Declare that these can be found in a library that is already
/// precompiled. So dynamic compilation will not instantiate them again.
SCISHARE VMesh* CreateVQuadSurfMesh(QuadSurfMesh<Core::Basis::QuadBilinearLgn<Core::Geometry::Point> >* mesh);
#if (SCIRUN_QUADRATIC_SUPPORT > 0)
SCISHARE VMesh* CreateVQuadSurfMesh(QuadSurfMesh<Core::Basis::QuadBiquadraticLgn<Core::Geometry::Point> >* mesh);
#endif
#if (SCIRUN_CUBIC_SUPPORT > 0)
SCISHARE VMesh* CreateVQuadSurfMesh(QuadSurfMesh<Core::Basis::QuadBicubicHmt<Core::Geometry::Point> >* mesh);
#endif

#endif
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
// Declarations for QuadSurfMesh class


template <class Basis>
class QuadSurfMesh : public Mesh
{

/// Make sure the virtual interface has access
template<class MESH> friend class VQuadSurfMesh;
template<class MESH> friend class VMeshShared;
template<class MESH> friend class VUnstructuredMesh;

public:
  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type                 under_type;
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::size_type                  size_type;
  typedef SCIRun::mask_type                  mask_type;

  typedef boost::shared_ptr<QuadSurfMesh<Basis> > handle_type;
  typedef Basis                         basis_type;

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
    typedef typename QuadSurfMesh<Basis>::index_type index_type;

    ElemData(const QuadSurfMesh<Basis>& msh, const index_type ind) :
      mesh_(msh),
      index_(ind)
    {
      /// Linear and Constant Basis never use edges_
      if (basis_type::polynomial_order() > 1)
      {
        mesh_.get_edges_from_face(edges_, index_);
      }
    }

    // the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const
    {
      return mesh_.faces_[index_ * 4];
    }
    inline
    index_type node1_index() const
    {
      return mesh_.faces_[index_ * 4 + 1];
    }
    inline
    index_type node2_index() const
    {
      return mesh_.faces_[index_ * 4 + 2];
    }
    inline
    index_type node3_index() const
    {
      return mesh_.faces_[index_ * 4 + 3];
    }

    /// changed the indexing as we now use unique indices
    inline
    index_type edge0_index() const
    {
      return edges_[0];
    }
    inline
    index_type edge1_index() const
    {
      return edges_[1];
    }
    inline
    index_type edge2_index() const
    {
      return edges_[2];
    }
    inline
    index_type edge3_index() const
    {
      return edges_[3];
    }

    inline
    const Core::Geometry::Point &node0() const
    {
      return mesh_.points_[node0_index()];
    }
    inline
    const Core::Geometry::Point &node1() const
    {
      return mesh_.points_[node1_index()];
    }
    inline
    const Core::Geometry::Point &node2() const
    {
      return mesh_.points_[node2_index()];
    }
    inline
    const Core::Geometry::Point &node3() const
    {
      return mesh_.points_[node3_index()];
    }

  private:
    /// reference to the mesh
    const QuadSurfMesh<Basis>        &mesh_;
    /// copy of element index
    const index_type                 index_;
    /// need edges for quadratic meshes
    typename Edge::array_type        edges_;
   };


  friend class Synchronize;

  class Synchronize //: public Runnable
  {
    public:
      Synchronize(QuadSurfMesh<Basis>* mesh, mask_type sync) :
        mesh_(mesh), sync_(sync)
      {
      }

      void operator()()
      {
        run();
      }

      void run()
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
        if (sync_ & Mesh::NODE_NEIGHBORS_E) mesh_->compute_node_neighbors();

        if (sync_ & Mesh::EDGES_E) mesh_->compute_edges();
        if (sync_ & Mesh::NORMALS_E) mesh_->compute_normals();

        if (sync_ & Mesh::BOUNDING_BOX_E) mesh_->compute_bounding_box();

        // These depend on the bounding box being synchronized
        if (sync_ & (Mesh::NODE_LOCATE_E|Mesh::ELEM_LOCATE_E))
        {
          {
            Core::Thread::UniqueLock lock(mesh_->synchronize_lock_.get());
            while(!(mesh_->synchronized_ & Mesh::BOUNDING_BOX_E))
            {
              mesh_->synchronize_cond_.wait(lock);
            }
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
        mesh_->synchronize_lock_.unlock();
        /// Tell other threads we are done
        mesh_->synchronize_cond_.conditionBroadcast();
      }

    private:
      QuadSurfMesh<Basis>* mesh_;
      mask_type  sync_;
  };


  //////////////////////////////////////////////////////////////////

  /// Construct a new mesh
  QuadSurfMesh();

  /// Copy a mesh, needed for detaching the mesh from a field
  QuadSurfMesh(const QuadSurfMesh& copy);

  /// Clone function for detaching the mesh and automatically generating
  /// a new version if needed.
  virtual QuadSurfMesh *clone() const { return new QuadSurfMesh(*this); }

  /// Destructor
  virtual ~QuadSurfMesh();

  MeshFacadeHandle getFacade() const
  {
    return boost::make_shared<Core::Datatypes::VirtualMeshFacade<VMesh>>(vmesh_);
  }

  /// Access point to virtual interface
  virtual VMesh* vmesh() {  return vmesh_.get(); }

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
  // Note: normals point inward.
  /// @todo: this is inconsistent with TriSurfMesh - should both surfaces
  // have consistent normal direction?
  virtual bool has_normals() const { return (true); }

  /// Compute tables for doing topology, these need to be synchronized
  ///before doing a lot of operations.
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
  void to_index(typename Cell::index_type&, index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  /// Get the child elements of the given index.
  void get_nodes(typename Node::array_type &array,
                 typename Node::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_nodes(typename Node::array_type &array,
                 typename Edge::index_type idx) const
  { get_nodes_from_edge(array,idx); }
  void get_nodes(typename Node::array_type &array,
                 typename Face::index_type idx) const
  { get_nodes_from_face(array,idx); }
  void get_nodes(typename Node::array_type&,
                 typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  void get_edges(typename Edge::array_type &array,
                 typename Node::index_type idx)
  { get_edges_from_node(array,idx); }
  void get_edges(typename Edge::array_type &array,
                 typename Edge::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_edges(typename Edge::array_type &array,
                 typename Face::index_type idx) const
  { get_edges_from_face(array,idx); }
  void get_edges(typename Edge::array_type&,
                 typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  void get_faces(typename Face::array_type &array,
                 typename Node::index_type idx) const
  { get_faces_from_node(array,idx); }
  void get_faces(typename Face::array_type &array,
                 typename Edge::index_type idx) const
  { get_faces_from_edge(array,idx); }
  void get_faces(typename Face::array_type &array,
                 typename Face::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_faces(typename Face::array_type&,
                 typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

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
  { get_faces_from_node(array,idx); }
  void get_elems(typename Elem::array_type &array,
                 typename Edge::index_type idx) const
  { get_faces_from_edge(array,idx); }
  void get_elems(typename Elem::array_type &array,
                 typename Face::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_elems(typename Face::array_type&,
                 typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  void get_delems(typename DElem::array_type &array,
                  typename Node::index_type idx) const
  { get_edges_from_node(array,idx); }
  void get_delems(typename DElem::array_type &array,
                  typename Edge::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_delems(typename DElem::array_type &array,
                  typename Face::index_type idx) const
  { get_edges_from_face(array,idx); }
  void get_delems(typename DElem::array_type&,
                  typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

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
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  /// Get the size of an element (length, area, volume)
  double get_size(typename Node::index_type /*idx*/) const
  { return 0.0; }
  double get_size(typename Edge::index_type idx) const
  {
    typename Node::array_type arr;
    get_nodes_from_edge(arr, idx);
    return (point(arr[0]) - point(arr[1])).length();
  }
  double get_size(typename Face::index_type idx) const
  {
    typename Node::array_type ra;
    get_nodes_from_face(ra,idx);
    const Core::Geometry::Point &p0 = point(ra[0]);
    const Core::Geometry::Point &p1 = point(ra[1]);
    const Core::Geometry::Point &p2 = point(ra[2]);
    const Core::Geometry::Point &p3 = point(ra[3]);
    return ((Cross(p0-p1,p2-p1)).length()+(Cross(p2-p3,p0-p3)).length()+
	    (Cross(p3-p0,p1-p0)).length()+(Cross(p1-p2,p3-p2)).length())*0.25;
  }
  double get_size(typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  /// More specific names for get_size
  double get_length(typename Edge::index_type idx) const
  { return get_size(idx); }
  double get_area(typename Face::index_type idx) const
  { return get_size(idx); }
  double get_volume(typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }



  /// Get neighbors of an element or a node

  /// THIS ONE IS FLAWED AS IN 3D SPACE MULTIPLE EDGES CAN CONNECTED THROUGH
  /// ONE EDGE
  bool get_neighbor(typename Elem::index_type &neighbor,
                    typename Elem::index_type elem,
                    typename DElem::index_type delem) const
  { return(get_elem_neighbor(neighbor,elem,delem)); }

  /// These are more general implementations
  void get_neighbors(std::vector<typename Node::index_type> &array,
                     typename Node::index_type node)
  { get_node_neighbors(array,node); }
  bool get_neighbors(std::vector<typename Elem::index_type> &array,
                     typename Elem::index_type elem,
                     typename DElem::index_type delem) const
  { return(get_elem_neighbors(array,elem,delem)); }
  void get_neighbors(typename Elem::array_type &array,
                     typename Elem::index_type elem) const
  { get_elem_neighbors(array,elem); }


  /// Locate a point in a mesh, find which is the closest node
  bool locate(typename Node::index_type &node, const Core::Geometry::Point &p) const
  { return (locate_node(node,p)); }
  bool locate(typename Edge::index_type &edge, const Core::Geometry::Point &p) const
  { return (locate_edge(edge,p)); }
  bool locate(typename Face::index_type &face, const Core::Geometry::Point &p) const
  { return (locate_elem(face,p)); }
  bool locate(typename Cell::index_type&, const Core::Geometry::Point &) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  bool locate(typename Elem::index_type &elem,
              std::vector<double>& coords,
              const Core::Geometry::Point &p) const
  { return (locate_elem(elem,coords,p)); }


  /// These should become obsolete soon, they do not follow the concept
  /// of the basis functions....
  int get_weights(const Core::Geometry::Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point & , typename Edge::array_type & , double * )
  { ASSERTFAIL("QuadSurfMesh: get_weights(edges) is not supported."); }
  int get_weights(const Core::Geometry::Point &p, typename Face::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point & , typename Cell::array_type & , double * )
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  /// Access the nodes of the mesh
  void get_point(Core::Geometry::Point &p, typename Node::index_type i) const
    { p = points_[i]; }
  void set_point(const Core::Geometry::Point &p, typename Node::index_type i)
    { points_[i] = p; }

  void get_random_point(Core::Geometry::Point &, typename Elem::index_type, FieldRNG &rng) const;

  /// Normals for visualizations
  void get_normal(Core::Geometry::Vector &n, typename Node::index_type i) const
  {
    ASSERTMSG(synchronized_ & Mesh::NORMALS_E,
	      "Must call synchronize NORMALS_E on QuadSurfMesh first");
    n = normals_[i];
  }

  /// Get the normals at the outside of the element
  template<class VECTOR, class INDEX1, class INDEX2>
  void get_normal(Core::Geometry::Vector &result, VECTOR &coords,
                  INDEX1 eidx,INDEX2 /*fidx*/)
  {
/*
 * THE ISSUE IS WHAT THE NORMAL IS AT THE NODE, THIS FUNCTION TAKES AN ELEMENT
 * INDEX AS WEL, HENCE IT SHOULD RETURN THE NORMAL OF THAT FACE, NOT AN AVERAGED
 * ONE OVER NEIGHBORING FACES

    if (basis_.polynomial_order() < 2)
    {
      ASSERTMSG(synchronized_ & Mesh::NORMALS_E, "Must call synchronize NORMALS_E on TriSurfMesh first");

      typename Node::array_type arr(3);
      get_nodes_from_face(arr, eidx);

      const double c0_0 = fabs(coords[0]);
      const double c1_0 = fabs(coords[1]);
      const double c0_1 = fabs(coords[0] - 1.0L);
      const double c1_1 = fabs(coords[1] - 1.0L);

      if (c0_0 < 1e-7 && c1_0 < 1e-7) {
        // arr[0]
        result = normals_[arr[0]];
        return;
      } else if (c0_1 < 1e-7 && c1_0 < 1e-7) {
        // arr[1]
        result = normals_[arr[1]];
        return;
      } else if (c0_1 < 1e-7 && c1_1 < 1e-7) {
        // arr[2]
        result = normals_[arr[2]];
        return;
      } else if (c0_0 < 1e-7 && c1_1 < 1e-7) {
        // arr[3]
        result = normals_[arr[3]];
        return;
      }
    }
*/

    ElemData ed(*this, eidx);
    std::vector<Core::Geometry::Point> Jv;
    basis_.derivate(coords, ed, Jv);
    result = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1]));
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
    ASSERTMSG(a.size() == 4, "Tried to add non-quad element.");
    ASSERTMSG(order_face_nodes(a[0],a[1],a[2],a[3]), "add_elem: element that is being added is invalid");

    faces_.push_back(static_cast<typename Node::index_type>(a[0]));
    faces_.push_back(static_cast<typename Node::index_type>(a[1]));
    faces_.push_back(static_cast<typename Node::index_type>(a[2]));
    faces_.push_back(static_cast<typename Node::index_type>(a[3]));
    return static_cast<typename Elem::index_type>((static_cast<index_type>(faces_.size()) - 1) >> 2);
  }

  void node_reserve(size_type s) { points_.reserve(static_cast<size_t>(s)); }
  void elem_reserve(size_type s) { faces_.reserve(static_cast<size_t>(s*4)); }
  void resize_nodes(size_type s) { points_.resize(static_cast<size_t>(s)); }
  void resize_elems(size_type s) { faces_.resize(static_cast<size_t>(s*4)); }


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
  /// version of this matrix. This is currently here for completeness of the
  /// interface
  template<class VECTOR, class INDEX>
  void jacobian(const VECTOR& coords, INDEX idx, double* J) const
  {
    StackVector<Core::Geometry::Point,2> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
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

  /// Get the inverse jacobian of the transformation. This one is needed to
  /// translate local gradients into global gradients. Hence it is crucial for
  /// calculating gradients of fields, or constructing finite elements.
  template<class VECTOR, class INDEX>
  double inverse_jacobian(const VECTOR& coords, INDEX idx, double* Ji) const
  {
    StackVector<Core::Geometry::Point,2> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
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
  double scaled_jacobian_metric(INDEX idx) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    basis_.derivate(basis_.unit_center,ed,Jv);
    Jv.resize(3);
    Core::Geometry::Vector v = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1]));
    v.normalize();
    Jv[2] = v.asPoint();
    double min_jacobian = ScaledDetMatrix3P(Jv);

    size_t num_vertices = basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3);
      v = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1])); v.normalize();
      Jv[2] = v.asPoint();
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
    Core::Geometry::Vector v = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1])); v.normalize();
    Jv[2] = v.asPoint();
    double min_jacobian = DetMatrix3P(Jv);

    size_t num_vertices = basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3);
      v = Cross(Core::Geometry::Vector(Jv[0]),Core::Geometry::Vector(Jv[1])); v.normalize();
      Jv[2] = v.asPoint();
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }

    return (min_jacobian);
  }

  template<class INDEX>
  double inscribed_circumscribed_radius_metric(INDEX idx) const
  {
    return (0.0);
  }

  template <class INDEX>
  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         INDEX &node, const Core::Geometry::Point &p) const
  {
    return(find_closest_node(pdist,result,node,p,-1.0));
  }

  template <class INDEX>
  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         INDEX &node, const Core::Geometry::Point &p,double maxdist) const
  {
    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;
    typename Node::size_type sz; size(sz);

    /// If there are no nodes we cannot find the closest one
    if (sz == 0) return (false);

    if (node >= 0 && node < sz)
    {
      Core::Geometry::Point point = points_[node];
      double dist = (point-p).length2();

      if ( dist < epsilon2_ )
      {
        result = point;
        pdist = sqrt(dist);
        return (true);
      }
    }

    ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
              "QuadSurfMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

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
                typename SearchGridT<index_type>::iterator it, eit;
                found = false;
                node_grid_->lookup_ijk(it,eit, i, j, k);

                while (it != eit)
                {
                  const Core::Geometry::Point point = points_[*it];
                  const double dist = (p-point).length2();

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
        "QuadSurfMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

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
  bool find_closest_nodes(ARRAY1 &distances,ARRAY2 &nodes, const Core::Geometry::Point &p, double maxdist) const
  {
    nodes.clear();
    distances.clear();

    ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
        "QuadSurfMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

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
                         ARRAY& coords,
                         INDEX &elem,
                         const Core::Geometry::Point &p) const
  {
    return(find_closest_elem(pdist,result,coords,elem,p,-1.0));
  }

  template <class INDEX, class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         ARRAY& coords,
                         INDEX &elem,
                         const Core::Geometry::Point &p,
                         double maxdist) const
  {
    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;

    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find the closest one
    if (sz == 0) return (false);

    /// Test the one in face that is an initial guess
    if (elem >= 0 && elem < sz)
    {
      const index_type idx = elem * 4;
      est_closest_point_on_quad(result, p,
                           points_[faces_[idx  ]],
                           points_[faces_[idx+1]],
                           points_[faces_[idx+2]],
                           points_[faces_[idx+3]]);

      double dist = (p-result).length2();
      if ( dist < epsilon2_ )
      {
        /// As we computed an estimate, we use the Newton's method in the basis functions
        /// compute a more refined solution. This function may slow down computation.
        /// This piece of code will calculate the coordinates in the local element framework
        /// (the newton's method of finding a minimum), then it will project this back
        /// THIS CODE SHOULD BE FURTHER OPTIMIZED

        ElemData ed(*this,elem);
        basis_.get_coords(coords,result,ed);

        result = basis_.interpolate(coords,ed);
        double dmin = (result-p).length2();
        pdist = sqrt(dmin);
        return (true);
      }
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
        "QuadSurfMesh::find_closest_elem requires synchronize(ELEM_LOCATE_E).")
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
              if (elem_grid_->min_distance_squared(p, i, j, k) < dmin)
              {
                found = false;
                typename SearchGridT<index_type>::iterator it, eit;
                elem_grid_->lookup_ijk(it, eit, i, j, k);

                while (it != eit)
                {
                  Core::Geometry::Point r;
                  const index_type idx = (*it) * 4;
                  est_closest_point_on_quad(r, p,
                                       points_[faces_[idx  ]],
                                       points_[faces_[idx+1]],
                                       points_[faces_[idx+2]],
                                       points_[faces_[idx+3]]);
                  const double dist = (p - r).length2();
                  if (dist < dmin)
                  {
                    found_one = true;
                    result = r;
                    elem = INDEX(*it);
                    dmin = dist;
                    if (dmin < epsilon2_)
                    {
                      /// As we computed an estimate, we use the Newton's method in the basis functions
                      /// compute a more refined solution. This function may slow down computation.
                      /// This piece of code will calculate the coordinates in the local element framework
                      /// (the newton's method of finding a minimum), then it will project this back
                      /// THIS CODE SHOULD BE FURTHER OPTIMIZED

                      ElemData ed(*this,elem);
                      basis_.get_coords(coords,result,ed);

                      result = basis_.interpolate(coords,ed);
                      dmin = (result-p).length2();

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

    ElemData ed(*this,elem);
    basis_.get_coords(coords,result,ed);

    result = basis_.interpolate(coords,ed);
    dmin = (result-p).length2();

    pdist = sqrt(dmin);

    return (true);
  }

  /// This function will find the closest element and the location on that
  /// element that is the closest; it takes an additional repelpos argument
  /// which defines an exclusion zone that's not considered.  A repelnormal
  /// can also be specified in order to restrict the faces to those that are
  /// pointing in a different direction from the repel point.
  template <class INDEX, class ARRAY>
  bool find_closest_elem_far_from(double& pdist,
                         Core::Geometry::Point &result,
                         ARRAY &coords,
                         INDEX &face,
                         const Core::Geometry::Point &p,
			 const Core::Geometry::Point &repelpos,
			 double mindistfromrepel,
			 const Core::Geometry::Vector &repelnormal,
			 double mindotfromrepel,
			 double maxdist) const
  {
    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;
    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find the closest one
    if (sz == 0) return (false);

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
	      "QuadSurfMesh::find_closest_elem requires synchronize(ELEM_LOCATE_E).")

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
    double mindistfromrepelsquared = mindistfromrepel * mindistfromrepel;
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
                  Core::Geometry::Point p1(points_[faces_[(*it)*4]]);
                  Core::Geometry::Point p2(points_[faces_[(*it)*4+1]]);
                  Core::Geometry::Point p3(points_[faces_[(*it)*4+2]]);
                  Core::Geometry::Point p4(points_[faces_[(*it)*4+3]]);
                  Core::Geometry::Vector v0(p2-p1);
                  v0.normalize();
                  Core::Geometry::Vector v1(p3-p2);
                  v1.normalize();
                  Core::Geometry::Vector n=Cross(v0,v1);
                  double dot=Dot(n,repelnormal);
                  if (dot > mindotfromrepel &&
                      (((p1-repelpos).length2() < mindistfromrepelsquared) ||
                       ((p2-repelpos).length2() < mindistfromrepelsquared) ||
                       ((p3-repelpos).length2() < mindistfromrepelsquared) ||
                       ((p4-repelpos).length2() < mindistfromrepelsquared)))
                    { ++it; continue; }
                  Core::Geometry::Point r;
                  est_closest_point_on_quad(r, p, p1, p2, p3, p4);
                  const double dtmp = (p - r).length2();
                  if (dtmp < dmin)
                  {
                    found_one = true;
                    result = r;
                    face = INDEX(*it);
                    dmin = dtmp;

                    if (dmin < epsilon2_)
                    {
                      /// As we computed an estimate, we use the Newton's method in the basis functions
                      /// compute a more refined solution. This function may slow down computation.
                      /// This piece of code will calculate the coordinates in the local element framework
                      /// (the newton's method of finding a minimum), then it will project this back
                      /// THIS CODE SHOULD BE FURTHER OPTIMIZED

                      ElemData ed(*this,face);
                      basis_.get_coords(coords,result,ed);

                      result = basis_.interpolate(coords,ed);
                      dmin = (result-p).length2();

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

    ElemData ed(*this,face);
    basis_.get_coords(coords,result,ed);

    result = basis_.interpolate(coords,ed);
    dmin = (result-p).length2();

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

  template <class ARRAY>
  bool find_closest_elems(double& pdist, Core::Geometry::Point &result,
                          ARRAY &elems, const Core::Geometry::Point &p) const
  {
    elems.clear();

    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find the closest one
    if (sz == 0) return (false);

    // Walking the grid like this works really well if we're near the
    // surface.  It's degenerately bad if for example the point is
    // placed in the center of a sphere (because then we still have to
    // test all the faces, but with the grid overhead and triangle
    // duplication as well).
    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
              "QuadSurfMesh::find_closest_elems requires synchronize(ELEM_LOCATE_E).")

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
                typename SearchGridT<index_type>::iterator it, eit;
                elem_grid_->lookup_ijk(it,eit, i, j, k);

                while (it != eit)
                {
                  Core::Geometry::Point rtmp;
                  const index_type idx = (*it) * 4;
                  est_closest_point_on_quad(rtmp, p,
                                       points_[faces_[idx  ]],
                                       points_[faces_[idx+1]],
                                       points_[faces_[idx+2]],
                                       points_[faces_[idx+3]]);
                  const double dtmp = (p - rtmp).length2();

                  if (dtmp < dmin - epsilon2_)
                  {
                    elems.clear();
                    result = rtmp;
                    elems.push_back(*it);
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


    // As we computed an estimate, we use the Newton's method in the basis functions
    // compute a more refined solution. This function may slow down computation.
    // This piece of code will calculate the coordinates in the local element framework
    // (the newton's method of finding a minimum), then it will project this back
    // THIS CODE SHOULD BE FURTHER OPTIMIZED

    if (elems.size() == 1)
    {
      // if the number of faces is more then one the point we found is located
      // on the node or on the edge, which means the estimate is correct.
      std::vector<double> coords;
      ElemData ed(*this,elems[0]);
      basis_.get_coords(coords,result,ed);
      result = basis_.interpolate(coords,ed);
      dmin = (result-p).length2();
    }

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
  static PersistentTypeID quadsurfmesh_typeid;

  /// Core functionality for getting the name of a templated mesh class
  static  const std::string type_name(int n = -1);
  virtual std::string dynamic_type_name() const { return quadsurfmesh_typeid.type; }

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
  static Persistent *maker() { return new QuadSurfMesh<Basis>(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return boost::make_shared<QuadSurfMesh<Basis>>(); }


  //////////////////////////////////////////////////////////////////
  // Mesh specific functions (these are not implemented in every mesh)

  template <class INDEX>
  bool inside(INDEX idx, const Core::Geometry::Point &p) const
  {

    typename Node::array_type nodes;
    get_nodes_from_elem(nodes,idx);

    Core::Geometry::BBox bbox;
    bbox.extend(points_[nodes[0]]);
    bbox.extend(points_[nodes[1]]);
    bbox.extend(points_[nodes[2]]);
    bbox.extend(points_[nodes[3]]);
    bbox.extend(epsilon_);

    if (bbox.inside(p))
    {
      StackVector<double,2> coords;
      ElemData ed(*this, idx);
      if(basis_.get_coords(coords, p, ed)) return (true);
    }

    return (false);
  }

  // Extra functionality needed by this specific geometry.
  typename Node::index_type add_find_point(const Core::Geometry::Point &p,
                                           double err = 1.0e-3);
  typename Elem::index_type add_quad(typename Node::index_type a,
				     typename Node::index_type b,
				     typename Node::index_type c,
				     typename Node::index_type d);
  typename Elem::index_type add_quad(const Core::Geometry::Point &p0, const Core::Geometry::Point &p1,
				     const Core::Geometry::Point &p2, const Core::Geometry::Point &p3);

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
  inline void get_nodes_from_edge(ARRAY& array, INDEX i) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "QuadSurfMesh: Must call synchronize EDGES_E on QuadSurfMesh first");

    static int table[4][2] = { {0, 1}, {1, 2}, {2, 3}, {3, 0} };

    const index_type idx = edges_[i][0];
    const index_type off = idx % 4;
    const index_type node = idx - off;
    array.resize(2);
    array[0] = static_cast<typename ARRAY::value_type>(faces_[node + table[off][0]]);
    array[1] = static_cast<typename ARRAY::value_type>(faces_[node + table[off][1]]);
  }


  template<class ARRAY, class INDEX>
  inline void get_nodes_from_face(ARRAY& array, INDEX idx) const
  {
    array.resize(4);
    array[0] = static_cast<typename ARRAY::value_type>(faces_[idx * 4 + 0]);
    array[1] = static_cast<typename ARRAY::value_type>(faces_[idx * 4 + 1]);
    array[2] = static_cast<typename ARRAY::value_type>(faces_[idx * 4 + 2]);
    array[3] = static_cast<typename ARRAY::value_type>(faces_[idx * 4 + 3]);
    order_face_nodes(array[0],array[1],array[2],array[3]);
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
            "QuadSurfMesh: Must call synchronize EDGES_E on QuadSurfMesh first");

    array.clear();
    array.resize(4);
    index_type he;
    int i = 0;
    he = static_cast<typename ARRAY::value_type>(halfedge_to_edge_[idx * 4 ]);
    if (he >=0)  array[i++] = he;
    he = static_cast<typename ARRAY::value_type>(halfedge_to_edge_[idx * 4 + 1]);
    if (he >=0)  array[i++] = he;
    he = static_cast<typename ARRAY::value_type>(halfedge_to_edge_[idx * 4 + 2]);
    if (he >=0)  array[i++] = he;
    he = static_cast<typename ARRAY::value_type>(halfedge_to_edge_[idx * 4 + 3]);
    if (he >=0)  array[i++] = he;
  }


  template<class ARRAY, class INDEX>
  inline void get_edges_from_elem(ARRAY& array, INDEX idx) const
  {
    get_edges_from_face(array,idx);
  }


  template <class ARRAY, class INDEX>
  inline void get_faces_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
	      "QuadSurfMesh: Must call synchronize NODE_NEIGHBORS_E on QuadSurfMesh first");

    array.resize(node_neighbors_[idx].size());
    for (size_t i = 0; i < node_neighbors_[idx].size(); ++i)
      array[i] = static_cast<typename ARRAY::value_type>(node_neighbors_[idx][i]);
  }


  template <class ARRAY, class INDEX>
  inline void get_faces_from_edge(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on QuadSurfMesh first");

    const std::vector<index_type>& faces = edges_[idx];

    // clear array
    array.clear();
    // conservative estimate
    array.reserve(faces.size());

    size_type fs = faces.size();
    for (index_type i=0; i<fs; i++)
    {
      array.push_back((faces[i]>>2));
    }
  }


  template<class ARRAY, class INDEX>
  inline void get_edges_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on QuadSurfMesh first");

    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on QuadSurfMesh first");
    // Get the table of faces that are connected to the two nodes
    Core::Thread::Guard nn(synchronize_lock_.get());

    const std::vector<typename Face::index_type>& faces  = node_neighbors_[idx];
    array.clear();

    typename ARRAY::value_type edge;
    for (index_type i=0; i<static_cast<size_type>(faces.size()); i++)
    {
      for (index_type j=0; j<4; j++)
      {
        edge = halfedge_to_edge_[faces[i]*4+j];
        size_t k=0;
        for (; k<array.size(); k++)
          if (array[k] == edge) break;
        if (k == array.size()) array.push_back(edge);
      }
    }
  }

  template <class ARRAY, class INDEX>
  inline void set_nodes_by_elem(ARRAY &array, INDEX idx)
  {
    for (index_type n = 0; n < 4; ++n)
      faces_[idx * 4 + n] = static_cast<index_type>(array[n]);
  }

  /// This function has been rewritten to allow for non manifold surfaces to be
  /// handled ok.
  template <class INDEX1, class INDEX2>
  bool get_elem_neighbor(INDEX1 &neighbor, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on QuadSurfMesh first");

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
  bool get_elem_neighbors(ARRAY &array, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on QuadSurfMesh first");

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
  void get_elem_neighbors(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on QuadSurfMesh first");

    typename Edge::array_type edges;
    get_edges_from_face(edges, idx);

    array.clear();
    array.reserve(edges.size());

    for (index_type i=0; i<static_cast<index_type>(edges.size()); i++)
    {
      typename ARRAY::value_type nbor;
      if (get_elem_neighbor(nbor, idx, edges[i]))
      {
        array.push_back(nbor);
      }
    }
  }


  template <class ARRAY, class INDEX>
  void get_node_neighbors(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on QuadSurfMesh first");
    // clear old contents
    array.clear();

    // Get all the neighboring elements
    typename Node::array_type nodes;
    const std::vector<typename Face::index_type>& faces  = node_neighbors_[idx];
    // Make a conservative estimate of the number of node neighbors
    array.reserve(2*faces.size());

    for (index_type i=0;i<static_cast<index_type>(faces.size());i++)
    {
      index_type base = faces[i]*4;
      for (index_type j=0;j<4;j++)
      {
        if (faces_[base+j] ==  idx) continue;
        size_t k=0;
        for (;k<array.size();k++)
          if (static_cast<typename ARRAY::value_type>(faces_[base+j]) == array[k]) break;
        if (k==array.size())
          array.push_back(static_cast<typename ARRAY::value_type>(faces_[base+j]));
      }
    }
  }

// That would increase the search speed of node localization

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
                typename SearchGridT<index_type>::iterator it, eit;
                node_grid_->lookup_ijk(it,eit, i, j, k);

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

  /// This is currently implemented as an exhaustive search
  template <class INDEX>
  inline bool locate_edge(INDEX &loc, const Core::Geometry::Point &p) const
  {
    ASSERTMSG(synchronized_ & EDGES_E,
              "QuadSurfMesh::locate_edge requires synchronize(EDGES_E).")

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
      if (inside(elem,p)) return (true);
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
              "QuadSurfMesh::locate_node requires synchronize(ELEM_LOCATE_E).")

    typename SearchGridT<index_type>::iterator it, eit;
    if (elem_grid_->lookup(it,eit, p))
    {
      while (it != eit)
      {
        if (inside(typename Elem::index_type(*it), p))
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
              "QuadSurfMesh::locate_elems requires synchronize(ELEM_LOCATE_E).")

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
  inline bool locate_elem(INDEX &elem, ARRAY& coords, const Core::Geometry::Point &p) const
  {
    if (basis_.polynomial_order() > 1) return elem_locate(elem, *this, p);

    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find a closest point
    if (sz == 0) return (false);

    /// Check whether the estimate given in idx is the point we are looking for
    if ((elem > 0)&&(elem < sz))
    {
      if (inside(elem,p))
      {
        ElemData ed(*this, elem);
        basis_.get_coords(coords, p, ed);
        return (true);
      }
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
              "QuadSurfMesh::locate_node requires synchronize(ELEM_LOCATE_E).")

    typename SearchGridT<index_type>::iterator it, eit;
    if (elem_grid_->lookup(it,eit, p))
    {
      while (it != eit)
      {
        if (inside(typename Elem::index_type(*it), p))
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
  inline void get_node_center(Core::Geometry::Point &p, INDEX idx) const
  {
    p = points_[idx];
  }

  template <class INDEX>
  inline void get_edge_center(Core::Geometry::Point &result, INDEX idx) const
  {
    typename Node::array_type arr;
    get_nodes_from_edge(arr, idx);
    result = points_[arr[0]];
    result += points_[arr[1]];
    result *= 0.5;
  }


  template <class INDEX>
  inline void get_face_center(Core::Geometry::Point &p, INDEX idx) const
  {
    // NEED TO OPTIMIZE THIS ONE
    typename Node::array_type nodes;
    get_nodes_from_face(nodes, idx);
    ASSERT(nodes.size() == 4);
    typename Node::array_type::iterator nai = nodes.begin();
    p = points_[*nai];
    ++nai;
    while (nai != nodes.end())
    {
      const Core::Geometry::Point &pp = points_[*nai];
      p += pp;
      ++nai;
    }
    p *= (1.0 / 4.0);
  }


  const Core::Geometry::Point &point(typename Node::index_type i) const { return points_[i]; }

  // These require the synchronize_lock_ to be held before calling.
  void compute_edges();
  void compute_normals();
  void compute_node_neighbors();

  void compute_node_grid();
  void compute_elem_grid();
  void compute_bounding_box();

  /// Used to recompute data for individual cells.
  void insert_elem_into_grid(typename Elem::index_type ci);
  void remove_elem_from_grid(typename Elem::index_type ci);

  void insert_node_into_grid(typename Node::index_type ci);
  void remove_node_from_grid(typename Node::index_type ci);

  template <class NODE>
  bool order_face_nodes(NODE& n1,NODE& n2, NODE& n3, NODE& n4) const
  {
    // Check for degenerate or misformed face
    // Opposite faces cannot be equal
    if ((n1 == n3)||(n2==n4)) return (false);

    // Face must have three unique identifiers otherwise it was condition
    // n1==n3 || n2==n4 would be met.

    if (n1==n2)
    {
      if (n3==n4) return (false); // this is a line not a face
      NODE t;
      // shift one position to left
      t = n1; n1 = n2; n2 = n3; n3 = n4; n4 = t;
      return (true);
    }
    else if (n2 == n3)
    {
      if (n1==n4) return (false); // this is a line not a face
      NODE t;
      // shift two positions to left
      t = n1; n1 = n3; n3 = t; t = n2; n2 = n4; n4 = t;
      return (true);
    }
    else if (n3 == n4)
    {
      NODE t;
      // shift one positions to right
      t = n4; n4 = n3; n3 = n2; n2 = n1; n1 = t;
      return (true);
    }
    else if (n4 == n1)
    {
      // proper order
      return (true);
    }
    else
    {
      if ((n1 < n2)&&(n1 < n3)&&(n1 < n4))
      {
        // proper order
        return (true);
      }
      else if ((n2 < n3)&&(n2 < n4))
      {
        NODE t;
        // shift one position to left
        t = n1; n1 = n2; n2 = n3; n3 = n4; n4 = t;
        return (true);
      }
      else if (n3 < n4)
      {
        NODE t;
        // shift two positions to left
        t = n1; n1 = n3; n3 = t; t = n2; n2 = n4; n4 = t;
        return (true);
      }
      else
      {
        NODE t;
        // shift one positions to right
        t = n4; n4 = n3; n3 = n2; n2 = n1; n1 = t;
        return (true);
      }
    }
  }


  index_type next(index_type i) { return ((i%4)==3) ? (i-3) : (i+1); }
  index_type prev(index_type i) { return ((i%4)==0) ? (i+3) : (i-1); }

  /// array with all the points
  std::vector<Core::Geometry::Point>                         points_;
  /// array with the four nodes that make up a face
  std::vector<index_type>                    faces_;

  /// FOR EDGE -> NODES
  /// array with information from edge number (unique ones) to the node numbers
  /// this one refers to the first node
  std::vector<std::vector<index_type> >            edges_;
  /// FOR FACES -> EDGE NUMBER
  /// array with information from halfedge (computed directly from face) to the edge number
  std::vector<index_type>                    halfedge_to_edge_;  // halfedge->edge map

  typedef std::vector<std::vector<typename Elem::index_type> > NodeNeighborMap;
  NodeNeighborMap                       node_neighbors_;

  std::vector<Core::Geometry::Vector>                           normals_; /// normalized per node
  boost::shared_ptr<SearchGridT<index_type> >  node_grid_; /// Lookup grid for nodes
  boost::shared_ptr<SearchGridT<index_type> >  elem_grid_; /// Lookup grid for elements

  // Lock and Condition Variable for hand shaking
  mutable Core::Thread::Mutex         synchronize_lock_;
  Core::Thread::ConditionVariable     synchronize_cond_;

  // Which tables have been computed
  mask_type             synchronized_;
  // Which tables are currently being computed
  mask_type             synchronizing_;

  Basis     basis_;    /// Basis for interpolation

  Core::Geometry::BBox      bbox_;
  double    epsilon_;  /// epsilon for calculations 1e-8*diagonal bounding box
  double    epsilon2_; /// square of epsilon for squared distance comparisons

  /// Pointer to virtual interface
  boost::shared_ptr<VMesh> vmesh_; /// Virtual function table
};


template <class Basis>
PersistentTypeID
QuadSurfMesh<Basis>::quadsurfmesh_typeid(type_name(-1), "Mesh",
                               QuadSurfMesh<Basis>::maker);

template <class Basis>
const std::string
QuadSurfMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("QuadSurfMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
QuadSurfMesh<Basis>::QuadSurfMesh()
  : points_(0),
    faces_(0),
    edges_(0),
    normals_(0),
    synchronize_lock_("QuadSurfMesh lock"),
    synchronize_cond_("QuadSurfMesh condition variable"),
    synchronized_(Mesh::NODES_E | Mesh::FACES_E | Mesh::CELLS_E),
    synchronizing_(0),
    epsilon_(0.0),
    epsilon2_(0.0)
{
  DEBUG_CONSTRUCTOR("QuadSurfMesh")

  /// Initialize the virtual interface when the mesh is created
  vmesh_.reset(CreateVQuadSurfMesh(this));
}


template <class Basis>
QuadSurfMesh<Basis>::QuadSurfMesh(const QuadSurfMesh &copy)
  : Mesh(copy),
    points_(0),
    faces_(0),
    edges_(0),
    normals_(0),
    synchronize_lock_("QuadSurfMesh lock"),
    synchronize_cond_("QuadSurfMesh condition variable"),
    synchronized_(Mesh::NODES_E | Mesh::FACES_E | Mesh::CELLS_E),
    synchronizing_(0),
    epsilon_(0.0),
    epsilon2_(0.0)
{
  DEBUG_CONSTRUCTOR("QuadSurfMesh")

  /// We need to lock before we can copy these as these
  /// structures are generate dynamically when they are
  /// needed.
  Core::Thread::Guard rlock(copy.synchronize_lock_.get());

  points_ = copy.points_;
  faces_ = copy.faces_;

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  vmesh_.reset(CreateVQuadSurfMesh(this));
}


template <class Basis>
QuadSurfMesh<Basis>::~QuadSurfMesh()
{
  DEBUG_DESTRUCTOR("QuadSurfMesh")
}


template <class Basis>
Core::Geometry::BBox
QuadSurfMesh<Basis>::get_bounding_box() const
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
QuadSurfMesh<Basis>::get_canonical_transform(Core::Geometry::Transform &t) const
{
  t.load_identity();
  Core::Geometry::BBox bbox = get_bounding_box();
  t.pre_scale(bbox.diagonal());
  t.pre_translate(Core::Geometry::Vector(bbox.get_min()));
}

template <class Basis>
void
QuadSurfMesh<Basis>::transform(const Core::Geometry::Transform &t)
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
QuadSurfMesh<Basis>::begin(typename QuadSurfMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on QuadSurfMesh first");
  itr = 0;
}


template <class Basis>
void
QuadSurfMesh<Basis>::end(typename QuadSurfMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on QuadSurfMesh first");
  itr = static_cast<size_type>(points_.size());
}


template <class Basis>
void
QuadSurfMesh<Basis>::begin(typename QuadSurfMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on QuadSurfMesh first");
  itr = 0;
}


template <class Basis>
void
QuadSurfMesh<Basis>::end(typename QuadSurfMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on QuadSurfMesh first");
  itr = static_cast<size_type>(edges_.size());
}


template <class Basis>
void
QuadSurfMesh<Basis>::begin(typename QuadSurfMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on QuadSurfMesh first");
  itr = 0;
}


template <class Basis>
void
QuadSurfMesh<Basis>::end(typename QuadSurfMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on QuadSurfMesh first");
  itr = static_cast<size_type>(faces_.size() / 4);
}


template <class Basis>
void
QuadSurfMesh<Basis>::begin(typename QuadSurfMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::CELLS_E,
            "Must call synchronize CELLS_E on QuadSurfMesh first");
  itr = 0;
}


template <class Basis>
void
QuadSurfMesh<Basis>::end(typename QuadSurfMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::CELLS_E,
            "Must call synchronize CELLS_E on QuadSurfMesh first");
  itr = 0;
}


template <class Basis>
int
QuadSurfMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Face::array_type &l,
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
QuadSurfMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Node::array_type &l,
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
void
QuadSurfMesh<Basis>::get_random_point(Core::Geometry::Point &p,
                                      typename Elem::index_type ei,
                                      FieldRNG &rng) const
{
  const Core::Geometry::Point &a0 = points_[faces_[ei*4+0]];
  const Core::Geometry::Point &a1 = points_[faces_[ei*4+1]];
  const Core::Geometry::Point &a2 = points_[faces_[ei*4+2]];

  const Core::Geometry::Point &b0 = points_[faces_[ei*4+2]];
  const Core::Geometry::Point &b1 = points_[faces_[ei*4+3]];
  const Core::Geometry::Point &b2 = points_[faces_[ei*4+0]];

  const double aarea = Cross(a1 - a0, a2 - a0).length();
  const double barea = Cross(b1 - b0, b2 - b0).length();

  if (rng() * (aarea + barea) < aarea)
  {
    // Fold the quad sample into a triangle.
    double u = rng();
    double v = rng();
    if (u + v > 1.0) { u = 1.0 - u; v = 1.0 - v; }

    // Compute the position of the random point.
    p = a0+((a1-a0)*u)+((a2-a0)*v);
  }
  else
  {
    // Fold the quad sample into a triangle.
    double u = rng();
    double v = rng();
    if (u + v > 1.0) { u = 1.0 - u; v = 1.0 - v; }

    // Compute the position of the random point.
    p = b0+((b1-b0)*u)+((b2-b0)*v);
  }
}

template <class Basis>
bool
QuadSurfMesh<Basis>::synchronize(mask_type sync)
{
  // Conversion table
  if (sync & (Mesh::DELEMS_E))
  { sync |= Mesh::EDGES_E; sync &= ~(Mesh::DELEMS_E); }

  if (sync & Mesh::FIND_CLOSEST_NODE_E)
  { sync |= NODE_LOCATE_E; sync &=  ~(Mesh::FIND_CLOSEST_NODE_E); }

  if (sync & Mesh::FIND_CLOSEST_ELEM_E)
  { sync |= ELEM_LOCATE_E; sync &=  ~(Mesh::FIND_CLOSEST_ELEM_E); }

  if (sync & (Mesh::NODE_LOCATE_E|Mesh::ELEM_LOCATE_E)) sync |= Mesh::BOUNDING_BOX_E;
  if (sync & Mesh::ELEM_NEIGHBORS_E) sync |= Mesh::NODE_NEIGHBORS_E|Mesh::EDGES_E;

  if (sync & Mesh::NODE_NEIGHBORS_E) sync |= Mesh::EDGES_E;

  // Filter out the only tables available
  sync &= (Mesh::EDGES_E|Mesh::NORMALS_E|
           Mesh::NODE_NEIGHBORS_E|Mesh::BOUNDING_BOX_E|
           Mesh::NODE_LOCATE_E|Mesh::ELEM_LOCATE_E);

  {

  Core::Thread::UniqueLock lock(synchronize_lock_.get());

  // Only sync what hasn't been synched
  sync &= (~synchronized_);


  if (sync == 0)
  {
    return (true);
  }

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

  }

  return (true);
}

template <class Basis>
bool
QuadSurfMesh<Basis>::unsynchronize(mask_type /*sync*/)
{
  return (true);
}


template <class Basis>
bool
QuadSurfMesh<Basis>::clear_synchronization()
{
  synchronize_lock_.lock();
  // Undo marking the synchronization
  synchronized_ = Mesh::NODES_E | Mesh::ELEMS_E | Mesh::FACES_E;

  if (synchronizing_)
    ASSERTFAIL("clear_synchronization is in parallel to a synchronization call from another thread.");
  // Free memory where possible

  edges_.clear();
  normals_.clear();
  halfedge_to_edge_.clear();
  node_neighbors_.clear();

  node_grid_.reset();
  elem_grid_.reset();

  synchronize_lock_.unlock();
  return (true);
}

template <class Basis>
void
QuadSurfMesh<Basis>::compute_normals()
{
  normals_.resize(points_.size()); // 1 per node

  // build table of faces that touch each node
  std::vector<std::vector<typename Face::index_type> > node_in_faces(points_.size());
  /// face normals (not normalized) so that magnitude is also the area.
  std::vector<Core::Geometry::Vector> face_normals(faces_.size());
  // Computing normal per face.
  typename Node::array_type nodes(4);
  typename Face::iterator iter, iter_end;
  begin(iter);
  end(iter_end);
  while (iter != iter_end)
  {
    get_nodes(nodes, *iter);

    Core::Geometry::Point p0, p1, p2, p3;
    get_point(p0, nodes[0]);
    get_point(p1, nodes[1]);
    get_point(p2, nodes[2]);
    get_point(p3, nodes[3]);

    // build table of faces that touch each node
    node_in_faces[nodes[0]].push_back(*iter);
    node_in_faces[nodes[1]].push_back(*iter);
    node_in_faces[nodes[2]].push_back(*iter);
    node_in_faces[nodes[3]].push_back(*iter);

    Core::Geometry::Vector v0 = p1 - p0;
    Core::Geometry::Vector v1 = p2 - p1;
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
typename QuadSurfMesh<Basis>::Node::index_type
QuadSurfMesh<Basis>::add_find_point(const Core::Geometry::Point &p, double err)
{
  typename Node::index_type i;
  if (locate(i, p) && (points_[i] - p).length2() < err)
  {
    return i;
  }
  else
  {
    points_.push_back(p);
    return static_cast<typename Node::index_type>
        (static_cast<size_type>(points_.size()) - 1);
  }
}


template <class Basis>
typename QuadSurfMesh<Basis>::Elem::index_type
QuadSurfMesh<Basis>::add_quad(typename Node::index_type a,
                              typename Node::index_type b,
                              typename Node::index_type c,
                              typename Node::index_type d)
{
  ASSERTMSG(order_face_nodes(a,b,c,d), "add_quad: element that is being added is invalid");
  faces_.push_back(a);
  faces_.push_back(b);
  faces_.push_back(c);
  faces_.push_back(d);
  synchronize_lock_.lock();
  synchronized_ &= ~Mesh::ELEM_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  synchronized_ &= ~Mesh::NORMALS_E;
  synchronize_lock_.unlock();
  return static_cast<typename Elem::index_type>((static_cast<index_type>(faces_.size()) - 1) >> 2);
}

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

template <class Basis>
void
QuadSurfMesh<Basis>::compute_edges()
{
  EdgeMapType2 edge_map;

  size_type num_faces = static_cast<index_type>(faces_.size()>>2);
  for (index_type i=0; i < num_faces; i++)
  {
    index_type n0 = faces_[4*i];
    index_type n1 = faces_[4*i+1];
    index_type n2 = faces_[4*i+2];
    index_type n3 = faces_[4*i+3];


    if (n0 > n1) edge_map[std::pair<index_type, index_type>(n1,n0)].push_back(i<<2);
    else if (n1 > n0) edge_map[std::pair<index_type, index_type>(n0,n1)].push_back(i<<2);

    if (n1 > n2) edge_map[std::pair<index_type, index_type>(n2,n1)].push_back((i<<2)+1);
    else if (n2 > n1) edge_map[std::pair<index_type, index_type>(n1,n2)].push_back((i<<2)+1);

    if (n2 > n3) edge_map[std::pair<index_type, index_type>(n3,n2)].push_back((i<<2)+2);
    else if (n3 > n2) edge_map[std::pair<index_type, index_type>(n2,n3)].push_back((i<<2)+2);

    if (n3 > n0) edge_map[std::pair<index_type, index_type>(n0,n3)].push_back((i<<2)+3);
    else if (n0 > n3) edge_map[std::pair<index_type, index_type>(n3,n0)].push_back((i<<2)+3);
  }

  typename EdgeMapType2::iterator itr;
  edges_.clear();
  edges_.resize(edge_map.size());
  halfedge_to_edge_.resize(faces_.size(),-1);

  size_t k=0;
  for (itr = edge_map.begin(); itr != edge_map.end(); ++itr)
  {
    const std::vector<index_type >& hedges = (*itr).second;
    for (size_t j=0; j<hedges.size(); j++)
    {
      index_type h = hedges[j];
      edges_[k].push_back(h);
      halfedge_to_edge_[h] = k;
    }
    k++;
  }

  synchronize_lock_.lock();
  synchronized_ |= Mesh::EDGES_E;
  synchronize_lock_.unlock();
}


template <class Basis>
void
QuadSurfMesh<Basis>::compute_node_neighbors()
{
  node_neighbors_.clear();
  node_neighbors_.resize(points_.size());
  index_type i, num_elems = static_cast<index_type>(faces_.size());
  for (i = 0; i < num_elems; i++)
  {
    node_neighbors_[faces_[i]].push_back(i/4);
  }

  synchronize_lock_.lock();
  synchronized_ |= Mesh::NODE_NEIGHBORS_E;
  synchronize_lock_.unlock();

}


template <class Basis>
void
QuadSurfMesh<Basis>::insert_elem_into_grid(typename Elem::index_type ci)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.
  const index_type idx = ci*4;
  Core::Geometry::BBox box;
  box.extend(points_[faces_[idx]]);
  box.extend(points_[faces_[idx+1]]);
  box.extend(points_[faces_[idx+2]]);
  box.extend(points_[faces_[idx+3]]);
  box.extend(epsilon_);
  elem_grid_->insert(ci, box);
}


template <class Basis>
void
QuadSurfMesh<Basis>::remove_elem_from_grid(typename Elem::index_type ci)
{
  const index_type idx = ci*4;
  Core::Geometry::BBox box;
  box.extend(points_[faces_[idx]]);
  box.extend(points_[faces_[idx+1]]);
  box.extend(points_[faces_[idx+2]]);
  box.extend(points_[faces_[idx+3]]);
  box.extend(epsilon_);
  elem_grid_->remove(ci, box);
}


template <class Basis>
void
QuadSurfMesh<Basis>::insert_node_into_grid(typename Node::index_type ni)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.
  node_grid_->insert(ni,points_[ni]);
}


template <class Basis>
void
QuadSurfMesh<Basis>::remove_node_from_grid(typename Node::index_type ni)
{
  node_grid_->remove(ni,points_[ni]);
}


template <class Basis>
void
QuadSurfMesh<Basis>::compute_node_grid()
{
  if (bbox_.valid())
  {
    // Cubed root of number of elems to get a subdivision ballpark.

    typename Node::size_type esz;  size(esz);

    const size_type s = 3*static_cast<size_type>
                  ((ceil(pow(static_cast<double>(esz) , (1.0/3.0))))/2.0 + 1.0);

    Core::Geometry::Vector diag  = bbox_.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(0.5+diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(0.5+diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(0.5+diag.z()/trace*s));

    Core::Geometry::BBox b = bbox_;
    b.extend(10*epsilon_);
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
QuadSurfMesh<Basis>::compute_elem_grid()
{
  if (bbox_.valid())
  {
    // Cubed root of number of elems to get a subdivision ballpark.

    typename Elem::size_type esz;  size(esz);

    const size_type s = 3*static_cast<size_type>
                  ((ceil(pow(static_cast<double>(esz) , (1.0/3.0))))/2.0 + 1.0);

    Core::Geometry::Vector diag  = bbox_.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(0.5+diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(0.5+diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(0.5+diag.z()/trace*s));

    Core::Geometry::BBox b = bbox_;
    b.extend(10*epsilon_);
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
QuadSurfMesh<Basis>::compute_bounding_box()
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

  synchronize_lock_.lock();
  synchronized_ |= Mesh::BOUNDING_BOX_E;
  synchronize_lock_.unlock();
}

template <class Basis>
typename QuadSurfMesh<Basis>::Node::index_type
QuadSurfMesh<Basis>::add_point(const Core::Geometry::Point &p)
{
  points_.push_back(p);
  return static_cast<typename Node::index_type>(static_cast<index_type>(points_.size() - 1));
}


template <class Basis>
typename QuadSurfMesh<Basis>::Elem::index_type
QuadSurfMesh<Basis>::add_quad(const Core::Geometry::Point &p0, const Core::Geometry::Point &p1,
                              const Core::Geometry::Point &p2, const Core::Geometry::Point &p3)
{
  return add_quad(add_find_point(p0), add_find_point(p1),
                  add_find_point(p2), add_find_point(p3));
}


#define QUADSURFMESH_VERSION 4
template <class Basis>
void
QuadSurfMesh<Basis>::io(Piostream &stream)
{
  const int version = stream.begin_class(type_name(-1), QUADSURFMESH_VERSION);

  Mesh::io(stream);

  Pio(stream, points_);
  Pio_index(stream, faces_);

  // In case the face is degenerate
  // move the degerenaracy to the end
  // this way the visualization works fine
  if (version != 1)
  {
    if (stream.reading())
    {
      for (size_t i=0; i < faces_.size(); i += 4)
      {
        ASSERTMSG(order_face_nodes(faces_[i],faces_[i+1],faces_[i+2],faces_[i+3]),
          "Detected an invalid quadrilateral face");
      }
    }
  }

  if (version == 1)
  {
    // We no longer save out this table, we actually no longer compute it
    // Hence when saving we save an empty table, which may break the old
    // SCIRun version (a version that was not working anyway)
    // When loading an old file we just ignore the table once it is loaded.
    std::vector<under_type> dummy;
    Pio(stream,dummy);
  }

  if (version >= 3)
  {
    basis_.io(stream);
  }

  stream.end_class();

  if (stream.reading())
  {
    synchronized_ = Mesh::NODES_E | Mesh::FACES_E | Mesh::CELLS_E;
  }

  if (stream.reading())
    vmesh_.reset(CreateVQuadSurfMesh(this));
}


template <class Basis>
void
QuadSurfMesh<Basis>::size(typename QuadSurfMesh::Node::size_type &s) const
{
  typename Node::iterator itr; end(itr);
  s = *itr;
}


template <class Basis>
void
QuadSurfMesh<Basis>::size(typename QuadSurfMesh::Edge::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on QuadSurfMesh first");
  s = edges_.size();
}


template <class Basis>
void
QuadSurfMesh<Basis>::size(typename QuadSurfMesh::Face::size_type &s) const
{
  typename Face::iterator itr; end(itr);
  s = *itr;
}


template <class Basis>
void
QuadSurfMesh<Basis>::size(typename QuadSurfMesh::Cell::size_type &s) const
{
  typename Cell::iterator itr; end(itr);
  s = *itr;
}


template <class Basis>
const TypeDescription*
get_type_description(QuadSurfMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("QuadSurfMesh", subs,
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
QuadSurfMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((QuadSurfMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
QuadSurfMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((QuadSurfMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Node",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
QuadSurfMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((QuadSurfMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Edge",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
QuadSurfMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((QuadSurfMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Face",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
QuadSurfMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
   const TypeDescription *me =
      SCIRun::get_type_description((QuadSurfMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Cell",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}




} // namespace SCIRun

#endif
