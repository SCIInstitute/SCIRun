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


#ifndef CORE_DATATYPES_PRISMVOLMESH_H
#define CORE_DATATYPES_PRISMVOLMESH_H 1

/// Include what kind of support we want to have
/// Need to fix this and couple it sci-defs
#include <Core/Datatypes/Legacy/Field/MeshSupport.h>

#include <Core/Containers/StackVector.h>
#include <Core/Persistent/PersistentSTL.h>

#include <Core/GeometryPrimitives/SearchGridT.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/CompGeom.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/Vector.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/PrismLinearLgn.h>
#include <Core/Basis/PrismQuadraticLgn.h>
#include <Core/Basis/PrismCubicHmt.h>

#include <Core/Datatypes/Legacy/Field/FieldIterator.h>
#include <Core/Datatypes/Legacy/Field/FieldRNG.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <Core/Utils/Legacy/CheckSum.h>

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <Core/Thread/Mutex.h>
#include <Core/Thread/ConditionVariable.h>

#include <set>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

/////////////////////////////////////////////////////
// Declarations for virtual interface

/// Functions for creating the virtual interface
/// Declare the functions that instantiate the virtual interface
template <class Basis> class PrismVolMesh;

/// make sure any other mesh other than the preinstantiate ones
/// returns no virtual interface. Altering this behavior will allow
/// for dynamically compiling the interface if needed.
template<class MESH>
VMesh* CreateVPrismVolMesh(MESH*) { return (0); }

/// These declarations are needed for a combined dynamic compilation as
/// as well as virtual functions solution.
/// Declare that these can be found in a library that is already
/// precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_PRISMVOL_SUPPORT > 0)

SCISHARE VMesh* CreateVPrismVolMesh(PrismVolMesh<Core::Basis::PrismLinearLgn<Core::Geometry::Point> >* mesh);
#if (SCIRUN_QUADRATIC_SUPPORT > 0)
SCISHARE VMesh* CreateVPrismVolMesh(PrismVolMesh<Core::Basis::PrismQuadraticLgn<Core::Geometry::Point> >* mesh);
#endif
#if (SCIRUN_CUBIC_SUPPORT > 0)
SCISHARE VMesh* CreateVPrismVolMesh(PrismVolMesh<Core::Basis::PrismCubicHmt<Core::Geometry::Point> >* mesh);
#endif

#endif
/////////////////////////////////////////////////////

#define PRISM_DUMMY_NODE_INDEX (typename Node::index_type) 999999999

/////////////////////////////////////////////////////
// Declarations for PrismVolMesh class

template <class Basis>
class PrismVolMesh : public Mesh
{
  /// Make sure the virtual interface has access
  template<class MESH> friend class VPrismVolMesh;
  template<class MESH> friend class VMeshShared;
  template<class MESH> friend class VUnstructuredMesh;

public:
  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type                under_type;
  typedef SCIRun::index_type                index_type;
  typedef SCIRun::size_type                 size_type;
  typedef SCIRun::mask_type                 mask_type;

  typedef boost::shared_ptr<PrismVolMesh<Basis> > handle_type;
  typedef Basis                               basis_type;

  /// Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef NodeIndex<under_type>       index_type;
    typedef NodeIterator<under_type>    iterator;
    typedef NodeIndex<under_type>       size_type;
    typedef StackVector<index_type, 6>  array_type;
  };

  struct Edge {
    typedef EdgeIndex<under_type>       index_type;
    typedef EdgeIterator<under_type>    iterator;
    typedef EdgeIndex<under_type>       size_type;
    typedef std::vector<index_type>     array_type;
  };

  struct Face {
    typedef FaceIndex<under_type>       index_type;
    typedef FaceIterator<under_type>    iterator;
    typedef FaceIndex<under_type>       size_type;
    typedef std::vector<index_type>     array_type;
  };

  struct Cell {
    typedef CellIndex<under_type>       index_type;
    typedef CellIterator<under_type>    iterator;
    typedef CellIndex<under_type>       size_type;
    typedef std::vector<index_type>     array_type;
  };

  /// Elem refers to the most complex topological object
  /// DElem refers to object just below Elem in the topological hierarchy

  typedef Cell Elem;
  typedef Face DElem;

  /// Somehow the information of how to interpolate inside an element
  /// ended up in a separate class, as they need to share information
  /// this construction was created to transfer data.
  /// Hopefully in the future this class will disappear again.
  friend class ElemData;

  class ElemData
  {
  public:
    typedef typename PrismVolMesh<Basis>::index_type  index_type;

    ElemData(const PrismVolMesh<Basis>& msh, const index_type ind) :
      mesh_(msh),
      index_(ind)
    {
      //Linear and Constant Basis never use edges_
      if (basis_type::polynomial_order() > 1)
      {
        mesh_.get_edges_from_cell(edges_, index_);
      }
    }

    // the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const {
      return mesh_.cells_[index_ * 6];
    }
    inline
    index_type node1_index() const {
      return mesh_.cells_[index_ * 6 + 1];
    }
    inline
    index_type node2_index() const {
      return mesh_.cells_[index_ * 6 + 2];
    }
    inline
    index_type node3_index() const {
      return mesh_.cells_[index_ * 6 + 3];
    }
    inline
    index_type node4_index() const {
      return mesh_.cells_[index_ * 6 + 4];
    }
    inline
    index_type node5_index() const {
      return mesh_.cells_[index_ * 6 + 5];
    }

    /// the following designed to coordinate with ::get_edges
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
    index_type edge3_index() const {
      return edges_[3];
    }
    inline
    index_type edge4_index() const {
      return edges_[4];
    }
    inline
    index_type edge5_index() const {
      return edges_[5];
    }
    inline
    index_type edge6_index() const {
      return edges_[6];
    }
    inline
    index_type edge7_index() const {
      return edges_[7];
    }
    inline
    index_type edge8_index() const {
      return edges_[8];
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
    inline
    const Core::Geometry::Point &node3() const {
      return mesh_.points_[node3_index()];
    }
    inline
    const Core::Geometry::Point &node4() const {
      return mesh_.points_[node4_index()];
    }
    inline
    const Core::Geometry::Point &node5() const {
      return mesh_.points_[node5_index()];
    }

  private:
    /// reference of the mesh
    const PrismVolMesh<Basis>        &mesh_;
    /// copy of index
    const index_type                 index_;
    /// need edges for quadratic meshes
    typename Edge::array_type        edges_;
  };

  friend class Synchronize;

  class Synchronize //: public Runnable
  {
    public:
      Synchronize(PrismVolMesh<Basis>* mesh, mask_type sync) :
        mesh_(mesh), sync_(sync) {}

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
        if (sync_ & (Mesh::NODE_NEIGHBORS_E))
        {
          {
            Core::Thread::UniqueLock lock(mesh_->synchronize_lock_.get());
            while(!(mesh_->synchronized_&Mesh::EDGES_E))
              mesh_->synchronize_cond_.wait(lock);
          }
          if (sync_ & Mesh::NODE_NEIGHBORS_E) mesh_->compute_node_neighbors();
        }

        if (sync_ & Mesh::EDGES_E) mesh_->compute_edges();
        if (sync_ & Mesh::FACES_E) mesh_->compute_faces();
        if (sync_ & Mesh::BOUNDING_BOX_E) mesh_->compute_bounding_box();

        // These depend on the bounding box being synchronized
        if (sync_ & (Mesh::NODE_LOCATE_E|Mesh::ELEM_LOCATE_E))
        {
          {
            Core::Thread::UniqueLock lock(mesh_->synchronize_lock_.get());
            while(!(mesh_->synchronized_&Mesh::BOUNDING_BOX_E))
              mesh_->synchronize_cond_.wait(lock);
          }
          if (sync_ & Mesh::NODE_LOCATE_E) mesh_->compute_node_grid();
          if (sync_ & Mesh::ELEM_LOCATE_E) mesh_->compute_elem_grid();
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
      PrismVolMesh<Basis>* mesh_;
      mask_type  sync_;
  };

  //////////////////////////////////////////////////////////////////

  /// Construct a new mesh
  PrismVolMesh();

  /// Copy a mesh, needed for detaching the mesh from a field
  PrismVolMesh(const PrismVolMesh &copy);

  /// Clone function for detaching the mesh and automatically generating
  /// a new version if needed.
  virtual PrismVolMesh *clone() const { return new PrismVolMesh(*this); }

  /// Destructor
  virtual ~PrismVolMesh();

  /// Access point to virtual interface
  virtual VMesh* vmesh() { return (vmesh_.get()); }

  MeshFacadeHandle getFacade() const
  {
    return boost::shared_ptr<Core::Datatypes::MeshFacade<VMesh>>();
  }

  /// This one should go at some point, should be reroute throught the
  /// virtual interface
  virtual int basis_order() { return (basis_.polynomial_order()); }

  /// Topological dimension
  virtual int dimensionality() const { return 3; }

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
  virtual bool is_editable() const { return true; }

  /// Has this mesh normals.
  virtual bool has_normals() const { return (false); }

  /// Has this mesh face normals
  virtual bool has_face_normals() const { return (true); }

  double get_epsilon() const { return (epsilon_); }

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
  void to_index(typename Node::index_type &index, index_type i) const
  { index = i; }
  void to_index(typename Edge::index_type &index, index_type i) const
  { index = i; }
  void to_index(typename Face::index_type &index, index_type i) const
  { index = i; }
  void to_index(typename Cell::index_type &index, index_type i) const
  { index = i; }

  /// Get the child topology elements of the given topology
  void get_nodes(typename Node::array_type &array,
                 typename Node::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_nodes(typename Node::array_type &array,
                 typename Edge::index_type idx) const
  { get_nodes_from_edge(array,idx); }
  void get_nodes(typename Node::array_type &array,
                 typename Face::index_type idx) const
  { get_nodes_from_face(array,idx); }
  void get_nodes(typename Node::array_type &array,
                 typename Cell::index_type idx) const
  { get_nodes_from_cell(array,idx); }

  void get_edges(typename Edge::array_type&,
                 typename Node::index_type) const
  { ASSERTFAIL("PrismVolMesh: get_edges has not been implemented for nodes"); }
  void get_edges(typename Edge::array_type &array,
                 typename Edge::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_edges(typename Edge::array_type &array,
                 typename Face::index_type idx) const
  { get_edges_from_face(array,idx); }
  void get_edges(typename Edge::array_type &array,
                 typename Cell::index_type idx) const
  { get_edges_from_cell(array,idx); }

  void get_faces(typename Face::array_type&,
                 typename Node::index_type) const
  { ASSERTFAIL("PrismVolMesh: get_faces has not been implemented for nodes"); }
  void get_faces(typename Face::array_type&,
                 typename Edge::index_type) const
  { ASSERTFAIL("PrismVolMesh: get_faces has not been implemented for edges"); }

  void get_faces(typename Face::array_type &array,
                 typename Face::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_faces(typename Face::array_type &array,
                 typename Cell::index_type idx) const
  { get_faces_from_cell(array,idx); }

  void get_cells(typename Cell::array_type &array,
                 typename Node::index_type idx) const
  { get_cells_from_node(array,idx); }
  void get_cells(typename Cell::array_type &array,
                 typename Edge::index_type idx) const
  { get_cells_from_edge(array,idx); }
  void get_cells(typename Cell::array_type &array,
                 typename Face::index_type idx) const
  { get_cells_from_face(array,idx); }
  void get_cells(typename Cell::array_type &array,
                 typename Cell::index_type idx) const
  { array.resize(1); array[0]= idx; }

  void get_elems(typename Elem::array_type &array,
                 typename Node::index_type idx) const
  { get_cells_from_node(array,idx); }
  void get_elems(typename Elem::array_type &array,
                 typename Edge::index_type idx) const
  { get_cells_from_edge(array,idx); }
  void get_elems(typename Elem::array_type &array,
                 typename Face::index_type idx) const
  { get_cells_from_face(array,idx); }
  void get_elems(typename Elem::array_type &array,
                 typename Cell::index_type idx) const
  { array.resize(1); array[0]= idx; }

  void get_delems(typename DElem::array_type&,
                  typename Node::index_type) const
  { ASSERTFAIL("PrismVolMesh: get_faces has not been implemented for nodes"); }
  void get_delems(typename DElem::array_type&,
                  typename Edge::index_type) const
  { ASSERTFAIL("PrismVolMesh: get_faces has not been implemented for edges"); }
  void get_delems(typename DElem::array_type &array,
                  typename Face::index_type idx) const
  { array.resize(1); array[0]= idx; }
  void get_delems(typename DElem::array_type &array,
                  typename Cell::index_type idx) const
  { get_faces_from_cell(array,idx); }

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
  void get_center(Core::Geometry::Point &result, typename Cell::index_type idx) const
  { get_cell_center(result, idx); }

  /// Get the size of an elemnt (length, area, volume)
  double get_size(typename Node::index_type) const
  { return 0.0; }

  double get_size(typename Edge::index_type idx) const
  {
    typename Node::array_type arr;
    get_nodes(arr, idx);
    Core::Geometry::Point p0, p1;
    get_center(p0, arr[0]);
    get_center(p1, arr[1]);
    return ((p1 - p0).length());
  }

  double get_size(typename Face::index_type idx) const
  {
    typename Node::array_type ra;
    get_nodes(ra,idx);
    Core::Geometry::Point p0,p1,p2,p3;
    get_point(p0,ra[0]);
    get_point(p1,ra[1]);
    get_point(p2,ra[2]);
    if( ra.size() == 3 )
      return (Cross(p0-p1,p2-p0)).length()*0.5;
    else //if( ra.size() == 4 )
    {
      get_point(p3,ra[3]);
      return ((Cross(p0-p1,p2-p0)).length()+(Cross(p0-p3,p2-p0)).length())*0.5;
    }
  }

  double get_size(typename Cell::index_type idx) const
  {
    ElemData ed(*this,idx);
    return (basis_.get_volume(ed));
  }

  /// More specific names for get_size
  double get_length(typename Edge::index_type idx) const
  { return get_size(idx); };
  double get_area  (typename Face::index_type idx) const
  { return get_size(idx); };
  double get_volume(typename Cell::index_type idx) const
  { return get_size(idx); };

  /// Get neighbors of an element or a node

  /// THIS ONE IS FLAWED AS IN 3D SPACE FOR AND ELEMENT TYPE THAT
  /// IS NOT A VOLUME. HENCE IT WORKS HERE, BUT GENERALLY IT IS FLAWED
  /// AS IT ASSUMES ONLY ONE NEIGHBOR, WHEREAS FOR ANYTHING ELSE THAN
  /// A FACE THERE CAN BE MULTIPLE
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

  /// return false if point is out of range.
  bool locate(typename Node::index_type &node, const Core::Geometry::Point &p) const
  { return (locate_node(node,p)); }
  bool locate(typename Edge::index_type &edge, const Core::Geometry::Point &p) const
  { return (locate_edge(edge,p)); }
  bool locate(typename Face::index_type &face, const Core::Geometry::Point &p) const
  { return (locate_face(face,p)); }
  bool locate(typename Cell::index_type &cell, const Core::Geometry::Point &p) const
  { return (locate_elem(cell,p)); }

  bool locate(typename Elem::index_type &elem,
              std::vector<double>& coords,
              const Core::Geometry::Point &p) const
  { return (locate_elem(elem,coords,p)); }

  /// These should become obsolete soon, they do not follow the concept
  /// of the basis functions....
  int get_weights(const Core::Geometry::Point &p, typename Node::array_type &l, double *w) const;
  int get_weights(const Core::Geometry::Point& , typename Edge::array_type& , double*) const
    { ASSERTFAIL("PrismVolMesh::get_weights for edges isn't supported"); }
  int get_weights(const Core::Geometry::Point& , typename Face::array_type& , double*) const
    { ASSERTFAIL("PrismVolMesh::get_weights for faces isn't supported"); }
  int get_weights(const Core::Geometry::Point &p, typename Cell::array_type &l, double *w) const;

   /// Access the nodes of the mesh
  void get_point(Core::Geometry::Point &result, typename Node::index_type index) const
    { result = points_[index]; }
  void set_point(const Core::Geometry::Point &point, typename Node::index_type index)
    { points_[index] = point; }
  void get_random_point(Core::Geometry::Point &p, typename Elem::index_type i, FieldRNG &r) const;

  /// Function for getting node normals
  void get_normal(Core::Geometry::Vector &, typename Node::index_type) const
    { ASSERTFAIL("This mesh type does not have node normals."); }

  /// Get the normals at the outside of the element
  template<class VECTOR, class INDEX1, class INDEX2>
  void get_normal(Core::Geometry::Vector &result, VECTOR& coords,
		  INDEX1 eidx, INDEX2 fidx) const
  {
    // Improved algorithm, which should be faster as it is fully
    // on the stack.

    // Obtain the inverse jacobian
    double Ji[9];
    inverse_jacobian(coords,eidx,Ji);

    // Get the normal in local coordinates
    const double un0 = basis_.unit_face_normals[fidx][0];
    const double un1 = basis_.unit_face_normals[fidx][1];
    const double un2 = basis_.unit_face_normals[fidx][2];
    // Do the matrix multiplication: should result in a vector
    // in the global coordinate space
    result.x(Ji[0]*un0+Ji[1]*un1+Ji[2]*un2);
    result.y(Ji[3]*un0+Ji[4]*un1+Ji[5]*un2);
    result.z(Ji[6]*un0+Ji[7]*un1+Ji[8]*un2);

    // normalize vector
    result.normalize();
  }

  /// Add a new node to the mesh
  typename Node::index_type add_point(const Core::Geometry::Point &p);
  typename Node::index_type add_node(const Core::Geometry::Point &p)
  { return(add_point(p)); }

  /// Add a new element to the mesh
  template<class ARRAY>
  typename Elem::index_type add_elem(ARRAY a)
  {
    ASSERTMSG(a.size() == 6, "Tried to add non-prism element.");

    return add_prism( static_cast<typename Node::index_type>(a[0]),
		      static_cast<typename Node::index_type>(a[1]),
		      static_cast<typename Node::index_type>(a[2]),
		      static_cast<typename Node::index_type>(a[3]),
		      static_cast<typename Node::index_type>(a[4]),
		      static_cast<typename Node::index_type>(a[5]) );
  }

  /// Functions to improve memory management. Often one knows how many
  /// nodes/elements one needs, prereserving memory is often possible.
  void node_reserve(size_type s) { points_.reserve(static_cast<std::vector<Core::Geometry::Point>::size_type>(s)); }
  void elem_reserve(size_type s) { cells_.reserve(static_cast<std::vector<index_type>::size_type>(s*6)); }
  void resize_nodes(size_type s) { points_.resize(static_cast<std::vector<Core::Geometry::Point>::size_type>(s)); }
  void resize_elems(size_type s) { cells_.resize(static_cast<std::vector<index_type>::size_type>(s*6)); }

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
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
    return (DetMatrix3P(Jv));
  }

  /// Get the jacobian of the transformation. In case one wants the non inverted
  /// version of this matrix. This is currentl here for completeness of the
  /// interface
  template<class VECTOR, class INDEX>
  void jacobian(const VECTOR& coords, INDEX idx, double* J) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
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

  /// Get the inverse jacobian of the transformation. This one is needed to
  /// translate local gradients into global gradients. Hence it is crucial for
  /// calculating gradients of fields, or constructing finite elements.
  template<class VECTOR, class INDEX>
  double inverse_jacobian(const VECTOR& coords, INDEX idx, double* Ji) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
    return (InverseMatrix3P(Jv,Ji));
  }

  template<class INDEX>
  double scaled_jacobian_metric(INDEX idx) const
  {
    StackVector<Core::Geometry::Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    basis_.derivate(basis_.unit_center,ed,Jv);
    double min_jacobian = ScaledDetMatrix3P(Jv);

    size_t num_vertices = basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
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
    double min_jacobian = DetMatrix3P(Jv);

    size_t num_vertices = basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }

    return (min_jacobian);
  }

  template <class INDEX>
  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         INDEX &node, const Core::Geometry::Point &p) const
  {
    return find_closest_node(pdist,result,node,p,-1.0);
  }

  template <class INDEX>
  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         INDEX &node, const Core::Geometry::Point &p,
                         double maxdist) const
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
	      "PrismVolMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

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
        "PrismVolMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

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

    ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
        "PrismVolMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

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


  /// Find the closest element to a point
  template <class INDEX, class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         ARRAY &coords,
                         INDEX &elem,
                         const Core::Geometry::Point &p) const
  {
    return(find_closest_elem(pdist,result,coords,elem,p,-1.0));
  }

  /// Find the closest element to a point
  template <class INDEX, class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         ARRAY &coords,
                         INDEX &elem,
                         const Core::Geometry::Point &p,
                         double maxdist) const
  {
    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;
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
        result = p;
        pdist = 0.0;
        return (true);
      }
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
              "PrismVolMesh: need to synchronize ELEM_LOCATE_E first");
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "PrismVolMesh: need to synchronize FACES_E first");

    // First check are we inside an element
    typename SearchGridT<index_type>::iterator it, eit;
    if (elem_grid_->lookup(it, eit, p))
    {
      while (it != eit)
      {
        if (inside(typename Elem::index_type(*it), p))
        {
          pdist = 0.0;
          result = p;
          elem = static_cast<INDEX>(*it);
          ElemData ed(*this, elem);
          basis_.get_coords(coords, p, ed);
          return (true);
        }
        ++it;
      }
    }

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
                typename SearchGridT<index_type>::iterator it, eit;
                elem_grid_->lookup_ijk(it,eit, i, j, k);

                while (it != eit)
                {
                  Core::Geometry::Point r;
                  index_type cidx = (*it);
                  index_type idx = cidx*8;
                  unsigned char b = boundary_faces_[cidx];

                  if (b)
                  {
                    if (b & 0x1)
                    {
                      closest_point_on_tri(r, p,
                                           points_[cells_[idx  ]],
                                           points_[cells_[idx+1]],
                                           points_[cells_[idx+2]]);
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
                          basis_.get_coords(coords,result,ed);

                          result = basis_.interpolate(coords,ed);
                          double dmin = (result-p).length2();
                          pdist = sqrt(dmin);
                          return (true);
                        }
                      }
                    }
                    if (b & 0x2)
                    {
                      closest_point_on_tri(r, p,
                                           points_[cells_[idx+5]],
                                           points_[cells_[idx+4]],
                                           points_[cells_[idx+3]]);
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
                          basis_.get_coords(coords,result,ed);

                          result = basis_.interpolate(coords,ed);
                          double dmin = (result-p).length2();
                          pdist = sqrt(dmin);
                          return (true);
                        }
                      }
                    }
                    if (b & 0x4)
                    {
                      est_closest_point_on_quad(r, p,
                                           points_[cells_[idx+1]],
                                           points_[cells_[idx+4]],
                                           points_[cells_[idx+5]],
                                           points_[cells_[idx+2]]);
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
                          basis_.get_coords(coords,result,ed);

                          result = basis_.interpolate(coords,ed);
                          double dmin = (result-p).length2();
                          pdist = sqrt(dmin);
                          return (true);
                        }
                      }
                    }
                    if (b & 0x8)
                    {
                      est_closest_point_on_quad(r, p,
                                           points_[cells_[idx+2]],
                                           points_[cells_[idx+5]],
                                           points_[cells_[idx+3]],
                                           points_[cells_[idx]]);
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
                          basis_.get_coords(coords,result,ed);

                          result = basis_.interpolate(coords,ed);
                          double dmin = (result-p).length2();
                          pdist = sqrt(dmin);
                          return (true);
                        }
                      }
                    }
                    if (b & 0x10)
                    {
                      est_closest_point_on_quad(r, p,
                                           points_[cells_[idx]],
                                           points_[cells_[idx+3]],
                                           points_[cells_[idx+4]],
                                           points_[cells_[idx+1]]);
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
                          basis_.get_coords(coords,result,ed);

                          result = basis_.interpolate(coords,ed);
                          double dmin = (result-p).length2();
                          pdist = sqrt(dmin);
                          return (true);
                        }
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
    while (!found);

    if (!found_one) return (false);

    ElemData ed(*this,elem);
    basis_.get_coords(coords,result,ed);

    result = basis_.interpolate(coords,ed);
    dmin = (result-p).length2();
    pdist = sqrt(dmin);
    return (true);
  }

  template <class INDEX>
  bool find_closest_elem(double& pdist, Core::Geometry::Point &result,
                         INDEX &elem, const Core::Geometry::Point &p) const
  {
    StackVector<double,3> coords;
    return(find_closest_elem(pdist,result,coords,elem,p,-1.0));
  }

  /// Find the closest elements to a point
  template<class ARRAY>
  bool find_closest_elems(double& /*pdist*/, Core::Geometry::Point& /*result*/,
                          ARRAY& /*faces*/, const Core::Geometry::Point& /*p*/) const
  {
    ASSERTFAIL("PrismVolMesh: Find closest element has not yet been implemented.");
    return (false);
  }

  /// Export this class using the old Pio system
  virtual void io(Piostream&);

  ///////////////////////////////////////////////////
  // STATIC VARIABLES AND FUNCTIONS

  /// This ID is created as soon as this class will be instantiated
  static PersistentTypeID prismvol_typeid;

  /// Core functionality for getting the name of a templated mesh class
  static  const std::string type_name(int n = -1);
  virtual std::string dynamic_type_name() const { return prismvol_typeid.type; }

  /// Type description, used for finding names of the mesh class for
  /// dynamic compilation purposes. Some of this should be obsolete
  virtual const TypeDescription *get_type_description() const;
  static const TypeDescription* node_type_description();
  static const TypeDescription* edge_type_description();
  static const TypeDescription* face_type_description();
  static const TypeDescription* cell_type_description();
  static const TypeDescription* elem_type_description()
  { return cell_type_description(); }

  /// This function returns a maker for Pio.
  static Persistent* maker() { return new PrismVolMesh<Basis>; }
  /// This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return boost::make_shared<PrismVolMesh<Basis>>(); }

  //////////////////////////////////////////////////////////////////
  // Mesh specific functions (these are not implemented in every mesh)
  /// Trying figure out which face is made out of 3/4 nodes (?)
  bool get_face(typename Face::index_type &array,
                typename Node::index_type n1,
                typename Node::index_type n2,
                typename Node::index_type n3,
                typename Node::index_type n4=PRISM_DUMMY_NODE_INDEX) const;

  /// Functions for Cubit
  /// WE SHOULD MAKE THESE GENERAL AND IN EVERY MESHTYPE
  template <class Iter, class Functor>
  void fill_points(Iter begin, Iter end, Functor fill_ftor);
  template <class Iter, class Functor>
  void fill_cells(Iter begin, Iter end, Functor fill_ftor);

  // THIS FUNCTION NEEDS TO BE REVISED AS IT DOES NOT SCALE PROPERLY
  // THE TOLERANCE IS NOT RELATIVE, WHICH IS A PROBLEM.........
  typename Node::index_type add_find_point(const Core::Geometry::Point &p,
					   double err = 1.0e-3);

  // Short cut for generating an element....
  typename Elem::index_type add_prism(typename Node::index_type a, typename Node::index_type b,
				      typename Node::index_type c, typename Node::index_type d,
				      typename Node::index_type e, typename Node::index_type f);

  typename Elem::index_type add_prism(const Core::Geometry::Point &p0, const Core::Geometry::Point &p1,
				      const Core::Geometry::Point &p2, const Core::Geometry::Point &p3,
				      const Core::Geometry::Point &p4, const Core::Geometry::Point &p5);

  /// must detach, if altering points!
  std::vector<Core::Geometry::Point>& get_points() { return points_; }

  int compute_checksum();

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
  inline void get_nodes_from_edge(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
      "PrismVolMesh: Must call synchronize EDGES_E first");

    if (edges_[idx].cells_.size() == 0)
      { array.clear(); return; }

    array.resize(2);

    PEdge e = edges_[idx];
    array[0] = static_cast<typename ARRAY::value_type>(e.nodes_[0]);
    array[1] = static_cast<typename ARRAY::value_type>(e.nodes_[1]);
  }

  // Always returns nodes in counter-clockwise order
  template <class ARRAY, class INDEX>
  inline void get_nodes_from_face(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
      "PrismVolMesh: Must call synchronize FACES_E first");

    const PFace &f = faces_[idx];

    if( static_cast<typename ARRAY::value_type>(f.nodes_[3]) ==
        PRISM_DUMMY_NODE_INDEX)
      array.resize(3);
    else
      array.resize(4);

    array[0] = static_cast<typename ARRAY::value_type>(f.nodes_[0]);
    array[1] = static_cast<typename ARRAY::value_type>(f.nodes_[1]);
    array[2] = static_cast<typename ARRAY::value_type>(f.nodes_[2]);

    if( static_cast<typename ARRAY::value_type>(f.nodes_[3]) !=
        PRISM_DUMMY_NODE_INDEX)
      array[3] = static_cast<typename ARRAY::value_type>(f.nodes_[3]);
  }

  template <class ARRAY, class INDEX>
  inline void get_nodes_from_cell(ARRAY &array, INDEX idx) const
  {
    array.resize(6);
    const auto off = idx * 6;
    array[0] = static_cast<typename ARRAY::value_type>(cells_[off    ]);
    array[1] = static_cast<typename ARRAY::value_type>(cells_[off + 1]);
    array[2] = static_cast<typename ARRAY::value_type>(cells_[off + 2]);
    array[3] = static_cast<typename ARRAY::value_type>(cells_[off + 3]);
    array[4] = static_cast<typename ARRAY::value_type>(cells_[off + 4]);
    array[5] = static_cast<typename ARRAY::value_type>(cells_[off + 5]);
  }

  template <class ARRAY, class INDEX>
  inline void get_nodes_from_elem(ARRAY &array, INDEX idx) const
  {
    get_nodes_from_cell(array,idx);
  }

  template <class ARRAY, class INDEX>
  inline void get_edges_from_face(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & (Mesh::EDGES_E),
      "PrismVolMesh: Must call synchronize EDGES_E first");
    ASSERTMSG(synchronized_ & (Mesh::FACES_E),
      "PrismVolMesh: Must call synchronize FACES_E first");

    array.clear();
    array.reserve(3);
    const PFace &f = faces_[idx];

    if (f.nodes_[0] != f.nodes_[1])
    {
      PEdge e(f.nodes_[0], f.nodes_[1]);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(edge_table_.find(e))).second));
    }
    if (f.nodes_[1] != f.nodes_[2])
    {
      PEdge e(f.nodes_[1], f.nodes_[2]);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(edge_table_.find(e))).second));
    }

    if( static_cast<typename ARRAY::value_type>(f.nodes_[3]) ==
        PRISM_DUMMY_NODE_INDEX)
    {
      if (f.nodes_[2] != f.nodes_[0])
      {
        PEdge e(f.nodes_[2], f.nodes_[0]);
        array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(edge_table_.find(e))).second));
      }
    }
    else
    {
      if (f.nodes_[2] != f.nodes_[3])
      {
        PEdge e(f.nodes_[2], f.nodes_[3]);
        array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(edge_table_.find(e))).second));
      }
      if (f.nodes_[3] != f.nodes_[0])
      {
        PEdge e(f.nodes_[3], f.nodes_[0]);
        array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(edge_table_.find(e))).second));
      }
    }
  }

  template <class ARRAY, class INDEX>
  inline void get_edges_from_cell(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
      "PrismVolMesh: Must call synchronize EDGES_E first");

    const index_type off = idx * 6;
    typename Node::index_type n1,n2;

    size_t i = 0;
    typedef typename ARRAY::value_type T;

    n1 = cells_[off    ]; n2 = cells_[off + 1];
    if (n1 != n2)
    {
      PEdge e(n1,n2);
      array[i++] = (static_cast<T>((*(edge_table_.find(e))).second));
    }
    n1 = cells_[off + 1]; n2 = cells_[off + 2];
    if (n1 != n2)
    {
      PEdge e(n1,n2);
      array[i++] = (static_cast<T>((*(edge_table_.find(e))).second));
    }
    n1 = cells_[off + 2]; n2 = cells_[off    ];
    if (n1 != n2)
    {
      PEdge e(n1,n2);
      array[i++] = (static_cast<T>((*(edge_table_.find(e))).second));
    }

    n1 = cells_[off + 3]; n2 = cells_[off + 4];
    if (n1 != n2)
    {
      PEdge e(n1,n2);
      array[i++] = (static_cast<T>((*(edge_table_.find(e))).second));
    }
    n1 = cells_[off + 4]; n2 = cells_[off + 5];
    if (n1 != n2)
    {
      PEdge e(n1,n2);
      array[i++] = (static_cast<T>((*(edge_table_.find(e))).second));
    }
    n1 = cells_[off + 5]; n2 = cells_[off + 3];
    if (n1 != n2)
    {
      PEdge e(n1,n2);
      array[i++] = (static_cast<T>((*(edge_table_.find(e))).second));
    }

    n1 = cells_[off    ]; n2 = cells_[off + 3];
    if (n1 != n2)
    {
      PEdge e(n1,n2);
      array[i++] = (static_cast<T>((*(edge_table_.find(e))).second));
    }
    n1 = cells_[off + 4]; n2 = cells_[off + 1];
    if (n1 != n2)
    {
      PEdge e(n1,n2);
      array[i++] = (static_cast<T>((*(edge_table_.find(e))).second));
    }
    n1 = cells_[off + 2]; n2 = cells_[off + 5];
    if (n1 != n2)
    {
      PEdge e(n1,n2);
      array[i++] = (static_cast<T>((*(edge_table_.find(e))).second));
    }
  }

  template <class ARRAY, class INDEX>
  inline void get_edges_from_elem(ARRAY &array, INDEX idx) const
  {
    get_edges_from_cell(array,idx);
  }

  template <class ARRAY, class INDEX>
  inline void get_faces_from_cell(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
                "PrismVolMesh: Must call synchronize FACES_E first");

    array.clear();
    array.reserve(5);

    const index_type off = idx * 6;
    typename Node::index_type n1,n2,n3,n4=PRISM_DUMMY_NODE_INDEX;

    // Put faces in node ordering from smallest node and then following CW or CCW
    // ordering. Test for degenerate elements. Degenerate faces are only added if they
    // are valid (only two neighboring nodes are equal)
    n1 = cells_[off    ]; n2 = cells_[off + 1];
    n3 = cells_[off + 2];
    if (order_face_nodes(n1,n2,n3,n4))
    {
      PFace f(n1,n2,n3,n4);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(face_table_.find(f))).second));
    }
    n1 = cells_[off + 5]; n2 = cells_[off + 4];
    n3 = cells_[off + 3];
    if (order_face_nodes(n1,n2,n3,n4))
    {
      PFace f(n1,n2,n3,n4);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(face_table_.find(f))).second));
    }
    n1 = cells_[off    ]; n2 = cells_[off + 3];
    n3 = cells_[off + 4]; n4 = cells_[off + 1];
    if (order_face_nodes(n1,n2,n3,n4))
    {
      PFace f(n1,n2,n3,n4);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(face_table_.find(f))).second));
    }
    n1 = cells_[off + 1]; n2 = cells_[off + 4];
    n3 = cells_[off + 5]; n4 = cells_[off + 2];
    if (order_face_nodes(n1,n2,n3,n4))
    {
      PFace f(n1,n2,n3,n4);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(face_table_.find(f))).second));
    }
    n1 = cells_[off + 2]; n2 = cells_[off + 5];
    n3 = cells_[off + 3]; n4 = cells_[off    ];
    if (order_face_nodes(n1,n2,n3,n4))
    {
      PFace f(n1,n2,n3,n4);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(face_table_.find(f))).second));
    }
  }

  template <class ARRAY, class INDEX>
  inline void get_cells_from_node(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & (Mesh::NODE_NEIGHBORS_E),
      "PrismVolMesh: Must call synchronize NODE_NEIGHBORS_E first");
    ASSERTMSG(synchronized_ & (Mesh::EDGES_E),
      "PrismVolMesh: Must call synchronize EDGES_E first");

    std::vector<typename Node::index_type> neighbors;
    std::set<typename ARRAY::value_type> unique_cells;
    // Get all the nodes that share an edge with this node
    get_node_neighbors(neighbors, idx);
    // Iterate through all those edges
    for (size_t n = 0; n < neighbors.size(); n++)
    {
      // Get the edge information for the current edge
      typename edge_ht::const_iterator iter =
                  edge_table_.find(PEdge(
                    static_cast<typename Node::index_type>(idx),neighbors[n]));
      ASSERTMSG(iter != edge_table_.end(),
                "Edge not found in PrismVolMesh::edge_table_");
      // Insert all cells that share this edge into
      // the unique set of cell indices
      for (size_t c = 0; c < (iter->first).cells_.size(); c++)
        unique_cells.insert(static_cast<typename ARRAY::value_type>(
                                                    (iter->first).cells_[c]));
    }

    // Copy the unique set of cells to our Cells array return argument
    array.resize(unique_cells.size());
    copy(unique_cells.begin(), unique_cells.end(), array.begin());
  }

  template <class ARRAY, class INDEX>
  inline void get_cells_from_edge(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
	      "PrismVolMesh: Must call synchronize EDGES_E first");
    for (size_t i=0; i<edges_[idx].cells_.size();i++)
      array[i] = static_cast<typename ARRAY::value_type>(edges_[idx].cells_[i]);
  }

  template <class ARRAY, class INDEX>
  inline void get_cells_from_face(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
	      "PrismVolMesh: Must call synchronize FACES_E first");
    if (faces_[idx].cells_[1] == MESH_NO_NEIGHBOR)
    {
      array.resize(1);
      array[0] = static_cast<typename ARRAY::value_type>((faces_[idx].cells_[0])>>3);
    }
    else
    {
      array.resize(2);
      array[0] = static_cast<typename ARRAY::value_type>((faces_[idx].cells_[0])>>3);
      array[1] = static_cast<typename ARRAY::value_type>((faces_[idx].cells_[1])>>3);
    }
  }

  template <class ARRAY, class INDEX>
  inline void set_nodes_by_elem(ARRAY &array, INDEX idx)
  {
    for (index_type n = 0; n < 6; ++n)
      cells_[idx * 6 + n] = static_cast<index_type>(array[n]);
  }

  template <class INDEX1, class INDEX2>
  inline bool get_elem_neighbor(INDEX1 &neighbor, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "Must call synchronize FACES_E on PrismVolMesh first");

    const PFace &f = faces_[delem];

    if (static_cast<typename Cell::index_type>(elem) == (f.cells_[0]>>3))
    {
      neighbor = static_cast<INDEX1>((f.cells_[1]>>3));
    }
    else
    {
      neighbor = static_cast<INDEX1>((f.cells_[0]>>3));
    }
    if (neighbor == MESH_NO_NEIGHBOR) return (false);
    return (true);
  }

  template <class ARRAY,class INDEX1, class INDEX2>
  inline bool get_elem_neighbors(ARRAY &array, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "Must call synchronize FACES_E on PrismVolMesh first");

    const PFace &f = faces_[delem];

    if (elem == static_cast<INDEX1>(f.cells_[0]))
    {
      array.resize(1);
      array[0] = static_cast<typename ARRAY::value_type>((f.cells_[1]>>3));
    }
    else
    {
      array.resize(1);
      array[0] = static_cast<typename ARRAY::value_type>((f.cells_[0]>>3));
    }
    if (array[0] == MESH_NO_NEIGHBOR)
    {
      array.clear();
      return (false);
    }
    return (true);
  }

  template <class ARRAY, class INDEX>
  inline void get_elem_neighbors(ARRAY &array, INDEX elem) const
  {
    typename Face::array_type faces;
    get_faces_from_cell(faces, elem);

    array.clear();
    typename Face::array_type::iterator iter = faces.begin();
    while(iter != faces.end())
    {
      typename ARRAY::value_type nbor;
      if (get_elem_neighbor(nbor, elem, *iter))
      {
        array.push_back(nbor);
      }
      ++iter;
    }
  }

  /// We should optimize this function more
  template <class ARRAY, class INDEX>
  inline void get_node_neighbors(ARRAY &array, INDEX node) const
  {
    ASSERTMSG(synchronized_ & NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on PrismVolMesh first.");
    size_t sz = node_neighbors_[node].size();
    array.resize(sz);
    for (size_t i=0; i< sz; i++)
    {
      array[i] = static_cast<typename ARRAY::value_type>(node_neighbors_[node][i]);
    }
  }

  template <class INDEX>
  bool locate_node(INDEX &node, const Core::Geometry::Point &p) const
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
              "PrismVolMesh::locate_node requires synchronize(NODE_LOCATE_E).")

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
    bool found;
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

  /// @todo: Fix this function, needs to use search grid
  template <class INDEX>
  bool locate_edge(INDEX &edge, const Core::Geometry::Point &p) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on PrismVolMesh first");

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
        edge = static_cast<INDEX>(*bi);
        mindist = dist;
        found = true;
      }
      ++bi;
    }
    return (found);
  }

  /// @todo: Fix this function, needs to use search grid
  template <class INDEX>
  bool locate_face(INDEX &face, const Core::Geometry::Point &p) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "Must call synchronize FACES_E on PrismVolMesh first");

    bool found = false;
    double mindist = DBL_MAX;
    typename Face::iterator bi; begin(bi);
    typename Face::iterator ei; end(ei);
    while (bi != ei)
    {
      Core::Geometry::Point c;
      get_center(c, *bi);
      const double dist = (p - c).length2();
      if (!found || dist < mindist)
      {
        mindist = dist;
        face = static_cast<INDEX>(*bi);
        found = true;
      }
      ++bi;
    }
    return (found);
  }

  template <class INDEX>
  bool locate_elem(INDEX &elem, const Core::Geometry::Point &p) const
  {
    /// @todo: Generate bounding boxes for elements and integrate this into the
    // basis function code.
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
	      "PrismVolMesh: need to synchronize ELEM_LOCATE_E first");

    typename SearchGridT<index_type>::iterator it, eit;
    if (elem_grid_->lookup(it, eit, p))
    {
      while (it != eit)
      {
        if (inside(*it, p))
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
              "PrismVolMesh::locate_elems requires synchronize(ELEM_LOCATE_E).")

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
  bool locate_elem(INDEX &elem, ARRAY& coords, const Core::Geometry::Point &p) const
  {
    /// @todo: Generate bounding boxes for elements and integrate this into the
    // basis function code.
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
	      "PrismVolMesh: need to synchronize ELEM_LOCATE_E first");

    typename SearchGridT<index_type>::iterator it, eit;
    if (elem_grid_->lookup(it, eit, p))
    {
      while (it != eit)
      {
        if (inside(*it, p))
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
  inline void get_edge_center(Core::Geometry::Point &p, INDEX idx) const
  {
    StackVector<typename Node::index_type,2> arr;
    get_nodes_from_edge(arr, idx);
    p = points_[arr[0]];
    const Core::Geometry::Point &p1 = points_[arr[1]];
    p += p1;
    p *= 0.5;
  }

  template <class INDEX>
  inline void get_face_center(Core::Geometry::Point &p, INDEX idx) const
  {
    /// NEED TO CLEAN UP THIS CODE
    /// NEED TO FILTER OUT DEGENERATE FACES
    typename Node::array_type nodes;
    get_nodes_from_face(nodes, idx);
    ASSERT(nodes.size() == 3 || nodes.size() == 4);
    typename Node::array_type::iterator nai = nodes.begin();
    get_point(p, *nai);
    ++nai;
    Core::Geometry::Point pp;
    while (nai != nodes.end())
    {
      get_point(pp, *nai);
      p += pp;
      ++nai;
    }
    p /= nodes.size();
  }


  template <class INDEX>
  inline void get_cell_center(Core::Geometry::Point &p, INDEX idx) const
  {
    /// NEED TO CLEAN UP THIS CODE
    /// NEED TO FILTER OUT DEGENERATE ELEMENTS
    typename Node::array_type nodes;
    get_nodes_from_cell(nodes, idx);
    ASSERT(nodes.size() == 6);
    typename Node::array_type::iterator nai = nodes.begin();
    get_point(p, *nai);
    ++nai;
    Core::Geometry::Point pp;
    while (nai != nodes.end())
    {
      get_point(pp, *nai);
      p += pp;
      ++nai;
    }
    p /= 6;
  }

  //////////////////////////////////////////////////////////////
  // Internal functions that the mesh depends on

protected:

  void compute_node_neighbors();
  void compute_edges();
  void compute_faces();
  void compute_node_grid();
  void compute_elem_grid();
  void compute_bounding_box();

  void insert_elem_into_grid(typename Elem::index_type ci);
  void remove_elem_from_grid(typename Elem::index_type ci);
  void insert_node_into_grid(typename Node::index_type ci);
  void remove_node_from_grid(typename Node::index_type ci);

  const Core::Geometry::Point &point(typename Node::index_type i) { return points_[i]; }

  template<class INDEX>
  bool inside(INDEX idx, const Core::Geometry::Point &p) const
  {
    // rewrote this function to more accurately deal with hexes that do not have
    // face aligned with the axes of the coordinate system

    // First a fast test to see whether we are inside the bounding box
    // (this code could be faster, by testing axis by axis)
    // Then if it is inside a tight bounding box compute the local coordinates
    // using the Newton search, this way we account for not planar surfaces of
    // the prisms.

    typename Node::array_type nodes;
    get_nodes_from_elem(nodes,idx);

    Core::Geometry::BBox bbox;
    bbox.extend(points_[nodes[0]]);
    bbox.extend(points_[nodes[1]]);
    bbox.extend(points_[nodes[2]]);
    bbox.extend(points_[nodes[3]]);
    bbox.extend(points_[nodes[4]]);
    bbox.extend(points_[nodes[5]]);
    bbox.extend(epsilon_);

    if (bbox.inside(p))
    {
      StackVector<double,3> coords;
      ElemData ed(*this, idx);
      if(basis_.get_coords(coords, p, ed)) return (true);
    }

    return (false);
  }

  /// all the nodes.
  std::vector<Core::Geometry::Point>        points_;
  /// each 6 indecies make up a Prism
  std::vector<under_type>   cells_;

  /// Face information.
  struct PFace {
    typename Node::index_type         nodes_[4];   /// 4 nodes makes a face.
    typename Cell::index_type         cells_[2];   /// 2 cells may have this face is in common.

    PFace() {
      nodes_[0] = MESH_NO_NEIGHBOR;
      nodes_[1] = MESH_NO_NEIGHBOR;
      nodes_[2] = MESH_NO_NEIGHBOR;
      nodes_[3] = MESH_NO_NEIGHBOR;
      cells_[0] = MESH_NO_NEIGHBOR;
      cells_[1] = MESH_NO_NEIGHBOR;
    }

    // snodes_ must be sorted. See Hash Function below.
    PFace(typename Node::index_type n1, typename Node::index_type n2,
          typename Node::index_type n3, typename Node::index_type n4) {
      cells_[0] = MESH_NO_NEIGHBOR;
      cells_[1] = MESH_NO_NEIGHBOR;
      nodes_[0] = n1;
      nodes_[1] = n2;
      nodes_[2] = n3;
      nodes_[3] = n4;
    }

    bool shared() const { return ((cells_[0] != MESH_NO_NEIGHBOR) &&
                                  (cells_[1] != MESH_NO_NEIGHBOR)); }

    /// true if both have the same nodes (order does not matter)
    bool operator==(const PFace &f) const {
      if (nodes_[2] == nodes_[3])
      {
        return ((nodes_[0] == f.nodes_[0]) &&
                (((nodes_[1]==f.nodes_[1])&&(nodes_[2] == f.nodes_[2]))||
		 ((nodes_[1]==f.nodes_[2])&&(nodes_[2] == f.nodes_[1]))));
      }
      else
      {
        return (((nodes_[0] == f.nodes_[0])&&(nodes_[2] == f.nodes_[2])) &&
                (((nodes_[1]==f.nodes_[1])&&(nodes_[3] == f.nodes_[3]))||
		 ((nodes_[1]==f.nodes_[3])&&(nodes_[3] == f.nodes_[1]))));
      }
    }

    /// Compares each node.  When a non equal node is found the <
    /// operator is applied.
    bool operator<(const PFace &f) const {
      if (nodes_[2] == nodes_[3])
      {
        if ((nodes_[1] < nodes_[2]) && (f.nodes_[1] < f.nodes_[2]))
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[1] == f.nodes_[1])
              return (nodes_[2] < f.nodes_[2]);
            else
              return (nodes_[1] < f.nodes_[1]);
          else
            return (nodes_[0] < f.nodes_[0]);
        }
        else if ((nodes_[1] < nodes_[2]) && (f.nodes_[1] >= f.nodes_[2]))
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[1] == f.nodes_[2])
              return (nodes_[2] < f.nodes_[1]);
            else
              return (nodes_[1] < f.nodes_[2]);
          else
            return (nodes_[0] < f.nodes_[0]);
        }
        else if ((nodes_[1] >= nodes_[2]) && (f.nodes_[1] < f.nodes_[2]))
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[2] == f.nodes_[1])
              return (nodes_[1] < f.nodes_[2]);
            else
              return (nodes_[2] < f.nodes_[1]);
          else
            return (nodes_[0] < f.nodes_[0]);
        }
        else
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[2] == f.nodes_[2])
              return (nodes_[1] < f.nodes_[1]);
            else
              return (nodes_[2] < f.nodes_[2]);
          else
            return (nodes_[0] < f.nodes_[0]);
        }
      }
      else
      {
        if ((nodes_[1] < nodes_[3]) && (f.nodes_[1] < f.nodes_[3]))
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[1] == f.nodes_[1])
              if (nodes_[2] == f.nodes_[2])
                return (nodes_[3] < f.nodes_[3]);
              else
                return (nodes_[2] < f.nodes_[2]);
            else
              return (nodes_[1] < f.nodes_[1]);
          else
            return (nodes_[0] < f.nodes_[0]);
        }
        else if ((nodes_[1] < nodes_[3]) && (f.nodes_[1] >= f.nodes_[3]))
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[1] == f.nodes_[3])
              if (nodes_[2] == f.nodes_[2])
                return (nodes_[3] < f.nodes_[1]);
              else
                return (nodes_[2] < f.nodes_[2]);
            else
              return (nodes_[1] < f.nodes_[3]);
          else
            return (nodes_[0] < f.nodes_[0]);
        }
        else if ((nodes_[1] >= nodes_[3]) && (f.nodes_[1] < f.nodes_[3]))
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[3] == f.nodes_[1])
              if (nodes_[2] == f.nodes_[2])
                return (nodes_[1] < f.nodes_[3]);
              else
                return (nodes_[2] < f.nodes_[2]);
            else
              return (nodes_[3] < f.nodes_[1]);
          else
            return (nodes_[0] < f.nodes_[0]);
        }
        else
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[3] == f.nodes_[3])
              if (nodes_[2] == f.nodes_[2])
                return (nodes_[1] < f.nodes_[1]);
              else
                return (nodes_[2] < f.nodes_[2]);
            else
              return (nodes_[3] < f.nodes_[3]);
          else
            return (nodes_[0] < f.nodes_[0]);
        }
      }
    }

  };

  /// Edge information.
  struct PEdge
  {
    typename Node::index_type         nodes_[2];   /// 2 nodes makes an edge.
    /// list of all the cells this edge is in.
    std::vector<typename Cell::index_type> cells_;

    PEdge() : cells_(0) {
      nodes_[0] = MESH_NO_NEIGHBOR;
      nodes_[1] = MESH_NO_NEIGHBOR;
    }
    // node_[0] must be smaller than node_[1]. See Hash Function below.
    PEdge(typename Node::index_type n1,
          typename Node::index_type n2) : cells_(0)
    {
      if (n1 < n2)
      {
        nodes_[0] = n1;
        nodes_[1] = n2;
      }
      else
      {
        nodes_[0] = n2;
        nodes_[1] = n1;
      }
    }

    bool shared() const { return cells_.size() > 1; }

    /// true if both have the same nodes (order does not matter)
    bool operator==(const PEdge &e) const
    {
      return ((nodes_[0] == e.nodes_[0]) && (nodes_[1] == e.nodes_[1]));
    }

    /// Compares each node.  When a non equal node is found the <
    /// operator is applied.
    bool operator<(const PEdge &e) const
    {
      if (nodes_[0] == e.nodes_[0])
        return (nodes_[1] < e.nodes_[1]);
      else
        return (nodes_[0] < e.nodes_[0]);
    }
  };

  /// hash the egde's node_indecies such that edges with the same nodes
  ///  hash to the same value. nodes are sorted on edge construction.
  static const int sz_int = sizeof(int) * 8; // in bits
  struct FaceHash
  {
    /// These are needed by the hash_map particularly
    // ANSI C++ allows us to initialize these variables in the
    // declaration.  However there may be compilers which will complain
    // about it.
    static const size_t bucket_size = 4;
    static const size_t min_buckets = 8;

    /// These are for our own use (making the hash function.
    static const int sz_quarter_int = (int)(sz_int / 4);
    static const int top4_mask = -(1 << sz_quarter_int << sz_quarter_int << sz_quarter_int);
    static const int up4_mask = top4_mask ^ -(1 << sz_quarter_int << sz_quarter_int);
    static const int mid4_mask =  top4_mask ^ -(1 << sz_quarter_int);
    static const int low4_mask = ~(top4_mask | mid4_mask);

    /// This is the hash function
    size_t operator()(const PFace &f) const {
      if (f.nodes_[1] < f.nodes_[3] )
      {
        return ((f.nodes_[0] << sz_quarter_int << sz_quarter_int <<sz_quarter_int) |
              (up4_mask & (f.nodes_[1] << sz_quarter_int << sz_quarter_int)) |
              (mid4_mask & (f.nodes_[2] << sz_quarter_int)) |
              (low4_mask & f.nodes_[3]));
      }
      else
      {
        return ((f.nodes_[0] << sz_quarter_int << sz_quarter_int <<sz_quarter_int) |
              (up4_mask & (f.nodes_[3] << sz_quarter_int << sz_quarter_int)) |
              (mid4_mask & (f.nodes_[2] << sz_quarter_int)) |
              (low4_mask & f.nodes_[1]));
      }
    }
    /// This should return less than rather than equal to.
    bool operator()(const PFace &f1, const PFace& f2) const {
      return f1 < f2;
    }
  };

  friend struct FaceHash; // needed by the gcc-2.95.3 compiler

  /// hash the egde's node_indecies such that edges with the same nodes
  ///  hash to the same value. nodes are sorted on edge construction.
  struct EdgeHash {
    /// These are needed by the hash_map particularly
    // ANSI C++ allows us to initialize these variables in the
    // declaration.  However there may be compilers which will complain
    // about it.
    static const size_t bucket_size = 4;
    static const size_t min_buckets = 8;

    /// These are for our own use (making the hash function.
    static const int sz_int = sizeof(int) * 8; // in bits
    static const int sz_half_int = sizeof(int) << 2; // in bits
    static const int up_mask = -(1 << sz_half_int);
    static const int low_mask = (~((int)0) ^ up_mask);

    /// This is the hash function
    size_t operator()(const PEdge &e) const
    {
      return (e.nodes_[0] << sz_half_int) |
	(low_mask & e.nodes_[1]);
    }

    ///  This should return less than rather than equal to.
    bool operator()(const PEdge &e1, const PEdge& e2) const
    {
      return e1 < e2;
    }
  };

  using face_ht = boost::unordered_map<PFace, typename Face::index_type, FaceHash>;
  using edge_ht = boost::unordered_map<PEdge, typename Edge::index_type, EdgeHash>;

  /// container for face storage. Must be computed each time
  ///  nodes or cells change.
  std::vector<PFace>            faces_;
  face_ht                  face_table_;
  /// container for edge storage. Must be computed each time
  ///  nodes or cells change.
  std::vector<PEdge>            edges_;
  edge_ht                  edge_table_;

  inline
  void hash_edge(typename Node::index_type n1, typename Node::index_type n2,
                 typename Cell::index_type ci, edge_ht &table) const;

  inline
  void hash_face(typename Node::index_type n1, typename Node::index_type n2,
                 typename Node::index_type n3, typename Node::index_type n4,
                 index_type combined_index, face_ht &table) const;

  template <class INDEX>
  bool order_face_nodes(INDEX& n1, INDEX& n2, INDEX& n3, INDEX& n4) const
  {
    if( n4 == PRISM_DUMMY_NODE_INDEX )
      return (true);

    // Check for degenerate or misformed face
    // Opposite faces cannot be equal
    if ((n1 == n3)||(n2==n4)) return (false);

    // Face must have three unique identifiers otherwise it was condition
    // n1==n3 || n2==n4 would be met.

    if ((n1 < n2)&&(n1 < n3)&&(n1 < n4))
    {
    }
    else if ((n2 < n3)&&(n2 < n4))
    {
      INDEX t;
      // shift one position to left
      t = n1; n1 = n2; n2 = n3; n3 = n4; n4 = t;
    }
    else if (n3 < n4)
    {
      INDEX t;
      // shift two positions to left
      t = n1; n1 = n3; n3 = t; t = n2; n2 = n4; n4 = t;
    }
    else
    {
      INDEX t;
      // shift one positions to right
      t = n4; n4 = n3; n3 = n2; n2 = n1; n1 = t;
    }

    if (n1==n2)
    {
      if (n3==n4) return (false); // this is a line not a face
       n2 = n3; n3 = n4;
    }
    else if (n2 == n3)
    {
      if (n1==n4) return (false); // this is a line not a face
      n3 = n4;
    }
    else if (n4 == n1)
    {
      n4 = n3;
    }
    return (true);
  }

  /// useful functors
  struct FillNodeNeighbors {
    FillNodeNeighbors(std::vector<std::vector<typename Node::index_type> > &n,
                      const PrismVolMesh &m) :
      nbor_vec_(n),
      mesh_(m)
    {}

    void operator()(typename Edge::index_type e) {
      mesh_.get_nodes(nodes_, e);
      nbor_vec_[nodes_[0]].push_back(nodes_[1]);
      nbor_vec_[nodes_[1]].push_back(nodes_[0]);
    }

    std::vector<std::vector<typename Node::index_type> > &nbor_vec_;
    const PrismVolMesh            &mesh_;
    typename Node::array_type   nodes_;
  };

  /// This grid is used as an acceleration structure to expedite calls
  ///  to locate.  For each cell in the grid, we store a list of which
  ///  tets overlap that grid cell -- to find the tet which contains a
  ///  point, we simply find which grid cell contains that point, and
  ///  then search just those tets that overlap that grid cell.
  std::vector<std::vector<typename Node::index_type> > node_neighbors_;

  std::vector<unsigned char> boundary_faces_;
  boost::shared_ptr<SearchGridT<index_type> >  node_grid_;
  boost::shared_ptr<SearchGridT<index_type> >  elem_grid_;

  // Lock and Condition Variable for hand shaking
  mutable Core::Thread::Mutex                 synchronize_lock_;
  Core::Thread::ConditionVariable             synchronize_cond_;

  // Which tables have been computed
  mask_type                     synchronized_;
  // Which tables are currently being computed
  mask_type                     synchronizing_;

  Basis                         basis_;
  Core::Geometry::BBox                          bbox_;
  double                        epsilon_;
  double                        epsilon2_;
  double                        epsilon3_;

  /// Pointer to virtual interface
  boost::shared_ptr<VMesh>                 vmesh_;
};

template <class Basis>
int
PrismVolMesh<Basis>::compute_checksum()
{
  int sum = 0;
  sum += SCIRun::compute_checksum(&points_[0],points_.size());
  sum += SCIRun::compute_checksum(&cells_[0],cells_.size());
  return (sum);
}

template <class Basis>
PrismVolMesh<Basis>::PrismVolMesh() :
  points_(0),
  cells_(0),
  faces_(0),
  face_table_(),
  edges_(0),
  edge_table_(),
  synchronize_lock_("PrismVolMesh Lock"),
  synchronize_cond_("PrismVolMesh condition variable"),
  synchronized_(Mesh::NODES_E | Mesh::CELLS_E),
  synchronizing_(0),
  epsilon_(0.0),
  epsilon2_(0.0),
  epsilon3_(0.0)
{
  DEBUG_CONSTRUCTOR("PrismVolMesh")

  /// Initialize the virtual interface when the mesh is created
  vmesh_.reset(CreateVPrismVolMesh(this));
}

template <class Basis>
PrismVolMesh<Basis>::PrismVolMesh(const PrismVolMesh &copy):
  Mesh(copy),
  points_(0),
  cells_(0),
  faces_(0),
  face_table_(),
  edges_(0),
  edge_table_(),
  synchronize_lock_("PrismVolMesh Lock"),
  synchronize_cond_("PrismVolMesh condition variable"),
  synchronized_(Mesh::NODES_E | Mesh::CELLS_E),
  synchronizing_(0),
  epsilon_(0.0),
  epsilon2_(0.0),
  epsilon3_(0.0)
{
  DEBUG_CONSTRUCTOR("PrismVolMesh")

  /// We need to lock before we can copy these as these
  /// structures are generate dynamically when they are
  /// needed.
  copy.synchronize_lock_.lock();

  points_ = copy.points_;
  cells_ = copy.cells_;

  // Epsilon does not require much space, hence copy those
  synchronized_ |= copy.synchronized_ & Mesh::BOUNDING_BOX_E;
  bbox_     = copy.bbox_;
  epsilon_  = copy.epsilon_;
  epsilon2_ = copy.epsilon2_;
  epsilon3_ = copy.epsilon3_;

  copy.synchronize_lock_.unlock();

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  vmesh_.reset(CreateVPrismVolMesh(this));
}

template <class Basis>
PrismVolMesh<Basis>::~PrismVolMesh()
{
  DEBUG_DESTRUCTOR("PrismVolMesh")
}

template <class Basis>
template <class Iter, class Functor>
void
PrismVolMesh<Basis>::fill_points(Iter begin, Iter end, Functor fill_ftor)
{
  synchronize_lock_.lock();
  Iter iter = begin;
  points_.resize(end - begin); // resize to the new size
  std::vector<Core::Geometry::Point>::iterator piter = points_.begin();
  while (iter != end)
  {
    *piter = fill_ftor(*iter);
    ++piter; ++iter;
  }
  synchronize_lock_.unlock();
}

template <class Basis>
template <class Iter, class Functor>
void
PrismVolMesh<Basis>::fill_cells(Iter begin, Iter end, Functor fill_ftor)
{
  synchronize_lock_.lock();
  Iter iter = begin;
  cells_.resize((end - begin) * 6); // resize to the new size
  std::vector<under_type>::iterator citer = cells_.begin();
  while (iter != end)
  {
    int *nodes = fill_ftor(*iter); // returns an array of length NNODES

    for( int i=0; i<6; i++ )
    {
      *citer = nodes[i];
      ++citer;
    }
    ++iter;
  }

  synchronize_lock_.unlock();
}

template <class Basis>
PersistentTypeID
PrismVolMesh<Basis>::prismvol_typeid(PrismVolMesh<Basis>::type_name(-1), "Mesh",
				     PrismVolMesh<Basis>::maker);

template <class Basis>
const std::string
PrismVolMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("PrismVolMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}

/* To generate a random point inside of a prism, we generate random
   barrycentric coordinates (independent random variables between 0 and
   1 that sum to 1) for the point. */
template <class Basis>
void
PrismVolMesh<Basis>::get_random_point(Core::Geometry::Point &p,
                                      const typename Elem::index_type ei,
                                      FieldRNG &rng) const
{
  const Core::Geometry::Point &p0 = points_[cells_[ei*6+0]];
  const Core::Geometry::Point &p1 = points_[cells_[ei*6+1]];
  const Core::Geometry::Point &p2 = points_[cells_[ei*6+2]];
  const Core::Geometry::Point &p3 = points_[cells_[ei*6+3]];
  const Core::Geometry::Point &p4 = points_[cells_[ei*6+4]];
  const Core::Geometry::Point &p5 = points_[cells_[ei*6+5]];

  const double a0 = tetrahedra_volume(p0, p1, p2, p4);
  const double a1 = tetrahedra_volume(p3, p4, p5, p0);
  const double a2 = tetrahedra_volume(p0, p2, p4, p5);

  const double w = rng() * (a0 + a1 + a2);

  double t = rng();
  double u = rng();
  double v = rng();

  // Fold cube into prism.
  if (t + u > 1.0)
  {
    t = 1.0 - t;
    u = 1.0 - u;
  }

  // Fold prism into tet.
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

  // Convert to Barycentric and compute new point.
  const double a = 1.0 - t - u - v;

  if (w > (a0 + a1))
  {
    p = Core::Geometry::Point(p0*a + p2*t + p4*u + p5*v);
  }
  else if (w > a0)
  {
    p = Core::Geometry::Point(p3*a + p4*t + p5*u + p0*v);
  }
  else
  {
    p = Core::Geometry::Point(p0*a + p1*t + p2*u + p4*v);
  }
}

template <class Basis>
Core::Geometry::BBox
PrismVolMesh<Basis>::get_bounding_box() const
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
  return result;
}

template <class Basis>
void
PrismVolMesh<Basis>::get_canonical_transform(Core::Geometry::Transform &t) const
{
  t.load_identity();
  Core::Geometry::BBox bbox = get_bounding_box();
  t.pre_scale(bbox.diagonal());
  t.pre_translate(Core::Geometry::Vector(bbox.get_min()));
}

template <class Basis>
void
PrismVolMesh<Basis>::transform(const Core::Geometry::Transform &t)
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
    epsilon3_ = epsilon_*epsilon_*epsilon_;

    synchronized_ |= Mesh::BOUNDING_BOX_E;
  }

  if (node_grid_) { node_grid_->transform(t); }
  if (elem_grid_) { elem_grid_->transform(t); }
  synchronize_lock_.unlock();
}

template <class Basis>
void
PrismVolMesh<Basis>::hash_face(typename Node::index_type n1,
			       typename Node::index_type n2,
			       typename Node::index_type n3,
			       typename Node::index_type n4,
			       index_type combined_index,
			       face_ht &table) const
{
  // Reorder nodes while maintaining CCW or CW orientation
  // Check for degenerate faces, if faces has degeneracy it
  // will be ignored (e.g. nodes on opposite corners are equal,
  // or more then two nodes are equal)

  if (!(order_face_nodes(n1,n2,n3,n4))) return;
  PFace f(n1, n2, n3, n4);

  typename face_ht::iterator iter = table.find(f);
  if (iter == table.end())
  {
    f.cells_[0] = combined_index;
    table[f] = 0; // insert for the first time
  }
  else
  {
    PFace f = (*iter).first;
    if (f.cells_[1] != MESH_NO_NEIGHBOR)
    {
      //TODO: inject new logger
      /*
      std::cerr << "PrismVolMesh - This Mesh has problems: Cells #"
           << (f.cells_[0]>>3) << ", #" << (f.cells_[1]>>3) << ", and #" << (combined_index >> 3)
           << " are illegally adjacent." << std::endl;*/
    }
    else if ((f.cells_[0]>>3) == (combined_index>>3))
    {
      //TODO: inject new logger
      /*
      std::cerr << "PrismVolMesh - This Mesh has problems: Cells #"
           << (f.cells_[0]>>3) << " and #" << (combined_index>>3)
           << " are the same." << std::endl;
           */
    }
    else
    {
      f.cells_[1] = combined_index; // add this cell
      table.erase(iter);
      table[f] = 0;
    }
  }
}

template <class Basis>
void
PrismVolMesh<Basis>::compute_faces()
{
  face_table_.clear();

  typename Cell::iterator ci, cie;
  begin(ci); end(cie);
  typename Node::array_type arr(6);
  while (ci != cie)
  {
    get_nodes(arr, *ci);
    // 5 faces -- each is entered CCW from outside looking in
    index_type cell_index = (*ci)<<3;
    hash_face(arr[0], arr[1], arr[2], PRISM_DUMMY_NODE_INDEX, cell_index, face_table_);
    hash_face(arr[5], arr[4], arr[3], PRISM_DUMMY_NODE_INDEX, cell_index + 1, face_table_);
    hash_face(arr[1], arr[4], arr[5], arr[2], cell_index + 2, face_table_);
    hash_face(arr[2], arr[5], arr[3], arr[0], cell_index + 3, face_table_);
    hash_face(arr[0], arr[3], arr[4], arr[1], cell_index + 4, face_table_);

    ++ci;
  }

  // dump edges into the edges_ container.
  faces_.resize(face_table_.size());
  typename std::vector<PFace>::iterator f_iter = faces_.begin();
  typename face_ht::iterator ht_iter = face_table_.begin();

  boundary_faces_.resize(cells_.size() /6);

  index_type i = 0;
  while (ht_iter != face_table_.end())
  {
    *f_iter = (*ht_iter).first;
    (*ht_iter).second = i;

    if ((*f_iter).cells_[1] == -1)
    {
      index_type cell = ((*f_iter).cells_[0]) >> 3;
      index_type face = ((*f_iter).cells_[0]) & 0x7;
      boundary_faces_[cell] |= 1 << face;
    }
    ++f_iter; ++ht_iter; i++;
  }

  synchronize_lock_.lock();
  synchronized_ |= Mesh::FACES_E;
  synchronize_lock_.unlock();
}

template <class Basis>
void
PrismVolMesh<Basis>::hash_edge(typename Node::index_type n1,
			       typename Node::index_type n2,
			       typename Cell::index_type ci,
			       edge_ht &table) const
{
  if (n1 == n2) return;
  PEdge e(n1, n2);
  typename edge_ht::iterator iter = table.find(e);
  if (iter == table.end())
  {
    e.cells_.push_back(ci); // add this cell
    table[e] = 0; // insert for the first time
  }
  else
  {
    PEdge e = (*iter).first;
    e.cells_.push_back(ci); // add this cell
    table.erase(iter);
    table[e] = 0;
  }
}

template <class Basis>
void
PrismVolMesh<Basis>::compute_edges()
{
  typename Cell::iterator ci, cie;
  begin(ci); end(cie);
  typename Node::array_type arr;
  while (ci != cie)
  {
    get_nodes(arr, *ci);
    hash_edge(arr[0], arr[1], *ci, edge_table_);
    hash_edge(arr[1], arr[2], *ci, edge_table_);
    hash_edge(arr[2], arr[0], *ci, edge_table_);

    hash_edge(arr[3], arr[4], *ci, edge_table_);
    hash_edge(arr[4], arr[5], *ci, edge_table_);
    hash_edge(arr[5], arr[3], *ci, edge_table_);

    hash_edge(arr[0], arr[3], *ci, edge_table_);
    hash_edge(arr[4], arr[1], *ci, edge_table_);
    hash_edge(arr[2], arr[5], *ci, edge_table_);
    ++ci;
  }

  // dump edges into the edges_ container.

  edges_.resize(edge_table_.size());

  typename std::vector<PEdge>::iterator e_iter = edges_.begin();
  typename edge_ht::iterator ht_iter = edge_table_.begin();
  while (ht_iter != edge_table_.end()) {
    *e_iter = (*ht_iter).first;
    (*ht_iter).second = static_cast<typename Edge::index_type>(
                                                      e_iter - edges_.begin());
    ++e_iter; ++ht_iter;
  }

  synchronize_lock_.lock();
  synchronized_ |= Mesh::EDGES_E;
  synchronize_lock_.unlock();
}

template <class Basis>
bool
PrismVolMesh<Basis>::synchronize(mask_type sync)
{
  // Conversion table
  if (sync & (Mesh::ELEM_NEIGHBORS_E|Mesh::DELEMS_E))
  { sync |= Mesh::FACES_E; sync &= ~(Mesh::ELEM_NEIGHBORS_E|Mesh::DELEMS_E); }

  if (sync & Mesh::FIND_CLOSEST_NODE_E)
  { sync |= NODE_LOCATE_E; sync &=  ~(Mesh::FIND_CLOSEST_NODE_E); }

  if (sync & Mesh::FIND_CLOSEST_ELEM_E)
  { sync |= ELEM_LOCATE_E|FACES_E; sync &=  ~(Mesh::FIND_CLOSEST_ELEM_E); }

  if (sync & Mesh::NODE_NEIGHBORS_E) sync |= Mesh::EDGES_E;
  if (sync & (Mesh::NODE_LOCATE_E|Mesh::ELEM_LOCATE_E)) sync |= Mesh::BOUNDING_BOX_E;

  // Filter out the only tables available
  sync &= (Mesh::EDGES_E|Mesh::FACES_E|
           Mesh::NODE_NEIGHBORS_E|Mesh::BOUNDING_BOX_E|
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

  if (sync == Mesh::FACES_E)
  {
    Synchronize Synchronize(this,sync);
    synchronize_lock_.unlock();
    Synchronize.run();
    synchronize_lock_.lock();
  }
  else if (sync & Mesh::FACES_E)
  {
    mask_type tosync = Mesh::FACES_E;
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

  return (true);
}

template <class Basis>
bool
PrismVolMesh<Basis>::unsynchronize(mask_type /*tosync*/)
{
  return (true);
}

template <class Basis>
bool
PrismVolMesh<Basis>::clear_synchronization()
{
  synchronize_lock_.lock();

  // Undo marking the synchronization
  synchronized_ = Mesh::NODES_E | Mesh::ELEMS_E | Mesh::CELLS_E;

  // Free memory where possible
  node_neighbors_.clear();
  edges_.clear();
  edge_table_.clear();
  faces_.clear();
  face_table_.clear();
  boundary_faces_.clear();

  node_grid_.reset();
  elem_grid_.reset();

  synchronize_lock_.unlock();

  return (true);
}


template <class Basis>
void
PrismVolMesh<Basis>::begin(typename PrismVolMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on PrismVolMesh first");
  itr = 0;
}

template <class Basis>
void
PrismVolMesh<Basis>::end(typename PrismVolMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on PrismVolMesh first");
  itr = static_cast<typename Node::iterator>(points_.size());
}

template <class Basis>
void
PrismVolMesh<Basis>::size(typename PrismVolMesh::Node::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on PrismVolMesh first");
  s = static_cast<typename Node::size_type>(points_.size());
}

template <class Basis>
void
PrismVolMesh<Basis>::begin(typename PrismVolMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on PrismVolMesh first");
  itr = 0;
}

template <class Basis>
void
PrismVolMesh<Basis>::end(typename PrismVolMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on PrismVolMesh first");
  itr = static_cast<typename Edge::iterator>(edges_.size());
}

template <class Basis>
void
PrismVolMesh<Basis>::size(typename PrismVolMesh::Edge::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on PrismVolMesh first");
  s = static_cast<typename Edge::size_type>(edges_.size());
}

template <class Basis>
void
PrismVolMesh<Basis>::begin(typename PrismVolMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on PrismVolMesh first");
  itr = 0;
}

template <class Basis>
void
PrismVolMesh<Basis>::end(typename PrismVolMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on PrismVolMesh first");
  itr = static_cast<typename Face::iterator>(faces_.size());
}

template <class Basis>
void
PrismVolMesh<Basis>::size(typename PrismVolMesh::Face::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on PrismVolMesh first");
  s = static_cast<typename Face::size_type>(faces_.size());
}

template <class Basis>
void
PrismVolMesh<Basis>::begin(typename PrismVolMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::CELLS_E,
            "Must call synchronize CELLS_E on PrismVolMesh first");
  itr = 0;
}

template <class Basis>
void
PrismVolMesh<Basis>::end(typename PrismVolMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::CELLS_E,
            "Must call synchronize CELLS_E on PrismVolMesh first");
  itr = static_cast<typename Cell::iterator>(cells_.size() / 6);
}

template <class Basis>
void
PrismVolMesh<Basis>::size(typename PrismVolMesh::Cell::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::CELLS_E,
            "Must call synchronize CELLS_E on PrismVolMesh first");
  s = static_cast<typename Cell::size_type>(cells_.size() / 6);
}

template <class Basis>
bool
PrismVolMesh<Basis>::get_face(typename Face::index_type &face,
			      typename Node::index_type n1,
			      typename Node::index_type n2,
			      typename Node::index_type n3,
			      typename Node::index_type n4) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on PrismVolMesh first");
  if(!(order_face_nodes(n1,n2,n3,n4))) return (false);
  PFace f(n1, n2, n3, n4);
  typename face_ht::const_iterator fiter = face_table_.find(f);
  if (fiter == face_table_.end()) {
    return false;
  }
  face = (*fiter).second;
  return true;
}

template <class Basis>
void
PrismVolMesh<Basis>::compute_node_neighbors()
{
  node_neighbors_.clear();
  node_neighbors_.resize(points_.size());
  typename Edge::iterator ei, eie;
  begin(ei); end(eie);
  std::for_each(ei, eie, FillNodeNeighbors(node_neighbors_, *this));

  synchronize_lock_.lock();
  synchronized_ |= Mesh::NODE_NEIGHBORS_E;
  synchronize_lock_.unlock();
}

template <class Basis>
int
PrismVolMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Node::array_type &l,
                               double *w) const
{
  typename Cell::index_type idx;
  if (locate_elem(idx, p))
  {
    get_nodes(l,idx);
    std::vector<double> coords(3);
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
PrismVolMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Cell::array_type &l,
                              double *w) const
{
  typename Cell::index_type idx;
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
void
PrismVolMesh<Basis>::insert_elem_into_grid(typename Elem::index_type ci)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.

  const index_type idx = ci*6;
  Core::Geometry::BBox box;
  box.extend(points_[cells_[idx]]);
  box.extend(points_[cells_[idx+1]]);
  box.extend(points_[cells_[idx+2]]);
  box.extend(points_[cells_[idx+3]]);
  box.extend(points_[cells_[idx+4]]);
  box.extend(points_[cells_[idx+5]]);
  box.extend(epsilon_);
  elem_grid_->insert(ci, box);
}

template <class Basis>
void
PrismVolMesh<Basis>::remove_elem_from_grid(typename Elem::index_type ci)
{
  const index_type idx = ci*6;
  Core::Geometry::BBox box;
  box.extend(points_[cells_[idx]]);
  box.extend(points_[cells_[idx+1]]);
  box.extend(points_[cells_[idx+2]]);
  box.extend(points_[cells_[idx+3]]);
  box.extend(points_[cells_[idx+4]]);
  box.extend(points_[cells_[idx+5]]);
  box.extend(epsilon_);
  elem_grid_->remove(ci, box);
}

template <class Basis>
void
PrismVolMesh<Basis>::insert_node_into_grid(typename Node::index_type ni)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.
  node_grid_->insert(ni,points_[ni]);
}

template <class Basis>
void
PrismVolMesh<Basis>::remove_node_from_grid(typename Node::index_type ni)
{
  node_grid_->remove(ni,points_[ni]);
}

template <class Basis>
void
PrismVolMesh<Basis>::compute_elem_grid()
{
  if (bbox_.valid())
  {
    // Cubed root of number of cells to get a subdivision ballpark.

    typename Elem::size_type esz;  size(esz);

    const size_type s =
      3*static_cast<size_type>((ceil(pow(static_cast<double>(esz) , (1.0/3.0))))/2.0 + 1.0);

    Core::Geometry::Vector diag  = bbox_.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(diag.z()/trace*s));

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
PrismVolMesh<Basis>::compute_node_grid()
{
  ASSERTMSG(bbox_.valid(),"PrismVolMesh BBox not valid");
  if (bbox_.valid())
  {
    // Cubed root of number of cells to get a subdivision ballpark.

    typename Elem::size_type esz;  size(esz);

    const size_type s =  3*static_cast<size_type>
                  ((ceil(pow(static_cast<double>(esz) , (1.0/3.0))))/2.0 + 1.0);

    Core::Geometry::Vector diag  = bbox_.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(diag.z()/trace*s));

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
PrismVolMesh<Basis>::compute_bounding_box()
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
  epsilon3_ = epsilon_*epsilon_*epsilon_;

  synchronize_lock_.lock();
  synchronized_ |= Mesh::BOUNDING_BOX_E;
  synchronize_lock_.unlock();
}

template <class Basis>
typename PrismVolMesh<Basis>::Node::index_type
PrismVolMesh<Basis>::add_find_point(const Core::Geometry::Point &p, double err)
{
  typename Node::index_type i;
  if (locate(i, p) && (points_[i] - p).length2() < err)
  {
    return i;
  }
  else
  {
    points_.push_back(p);
    return static_cast<typename Node::index_type>(points_.size() - 1);
  }
}

template <class Basis>
typename PrismVolMesh<Basis>::Elem::index_type
PrismVolMesh<Basis>::add_prism(typename Node::index_type a,
                               typename Node::index_type b,
                               typename Node::index_type c,
                               typename Node::index_type d,
                               typename Node::index_type e,
                               typename Node::index_type f)
{
  const index_type prism = static_cast<index_type>(cells_.size()) / 6;
  cells_.push_back(a);
  cells_.push_back(b);
  cells_.push_back(c);
  cells_.push_back(d);
  cells_.push_back(e);
  cells_.push_back(f);
  return prism;
}

template <class Basis>
typename PrismVolMesh<Basis>::Node::index_type
PrismVolMesh<Basis>::add_point(const Core::Geometry::Point &p)
{
  points_.push_back(p);
  return static_cast<typename Node::index_type>(points_.size() - 1);
}

template <class Basis>
typename PrismVolMesh<Basis>::Elem::index_type
PrismVolMesh<Basis>::add_prism(const Core::Geometry::Point &p0, const Core::Geometry::Point &p1,
                               const Core::Geometry::Point &p2, const Core::Geometry::Point &p3,
                               const Core::Geometry::Point &p4, const Core::Geometry::Point &p5)
{
  return add_prism(add_find_point(p0), add_find_point(p1),
		   add_find_point(p2), add_find_point(p3),
		   add_find_point(p4), add_find_point(p5));
}

#define PRISM_VOL_MESH_VERSION 3

template <class Basis>
void
PrismVolMesh<Basis>::io(Piostream &stream)
{
  const int version = stream.begin_class(type_name(-1),
                                         PRISM_VOL_MESH_VERSION);
  Mesh::io(stream);

  SCIRun::Pio(stream, points_);
  SCIRun::Pio_index(stream, cells_);
  if (version == 1)
  {
    std::vector<int> neighbors;
    SCIRun::Pio(stream, neighbors);
  }

  if (version >= 2)
  {
    basis_.io(stream);
  }

  stream.end_class();

  if (stream.reading())
  {
    synchronized_ = NODES_E | CELLS_E;

    vmesh_.reset(CreateVPrismVolMesh(this));
  }
}

template <class Basis>
const TypeDescription* get_type_description(PrismVolMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("PrismVolMesh", subs,
			     std::string(__FILE__),
			     "SCIRun",
			     TypeDescription::MESH_E);
  }
  return td;
}

template <class Basis>
const TypeDescription*
PrismVolMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((PrismVolMesh<Basis> *)0);
}

template <class Basis>
const TypeDescription*
PrismVolMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PrismVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Node",
			     std::string(__FILE__),
			     "SCIRun",
			     TypeDescription::MESH_E);
  }
  return td;
}

template <class Basis>
const TypeDescription*
PrismVolMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PrismVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Edge",
			     std::string(__FILE__),
			     "SCIRun",
			     TypeDescription::MESH_E);
  }
  return td;
}

template <class Basis>
const TypeDescription*
PrismVolMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PrismVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Face",
			     std::string(__FILE__),
			     "SCIRun",
			     TypeDescription::MESH_E);
  }
  return td;
}

template <class Basis>
const TypeDescription*
PrismVolMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PrismVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Cell",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

} // namespace SCIRun

#endif
