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


#ifndef CORE_DATATYPES_TETVOLMESH_H
#define CORE_DATATYPES_TETVOLMESH_H 1

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
#include <Core/Basis/TetLinearLgn.h>
#include <Core/Basis/TetQuadraticLgn.h>
#include <Core/Basis/TetCubicHmt.h>

#include <Core/Datatypes/Legacy/Field/FieldIterator.h>
#include <Core/Datatypes/Legacy/Field/FieldRNG.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Math/MiscMath.h>

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
template <class Basis> class TetVolMesh;

/// make sure any other mesh other than the preinstantiate ones
/// returns no virtual interface. Altering this behavior will allow
/// for dynamically compiling the interface if needed.
template<class MESH>
VMesh* CreateVTetVolMesh(MESH*) { return nullptr; }

/// These declarations are needed for a combined dynamic compilation as
/// as well as virtual functions solution.
/// Declare that these can be found in a library that is already
/// precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_TETVOL_SUPPORT > 0)

SCISHARE VMesh* CreateVTetVolMesh(TetVolMesh<Core::Basis::TetLinearLgn<Core::Geometry::Point> >* mesh);
#if (SCIRUN_QUADRATIC_SUPPORT > 0)
SCISHARE VMesh* CreateVTetVolMesh(TetVolMesh<Core::Basis::TetQuadraticLgn<Core::Geometry::Point> >* mesh);
#endif
#if (SCIRUN_CUBIC_SUPPORT > 0)
SCISHARE VMesh* CreateVTetVolMesh(TetVolMesh<Core::Basis::TetCubicHmt<Core::Geometry::Point> >* mesh);
#endif

#endif
/////////////////////////////////////////////////////

static const int
TetVolEdgePerNodeTable[4][6] = {{0,1, 2,0, 3,0},
                                {0,1, 1,2, 3,1},
                                {1,2, 2,0, 3,2},
                                {3,0, 3,1, 3,2}};

static const int
TetVolFacePerNodeTable[4][9] = {{0,1,2, 0,1,3, 0,3,2},
                               {0,1,2, 1,2,3, 0,1,3},
                               {0,1,2, 1,2,3, 0,3,2},
                               {1,2,3, 0,1,3, 0,3,2}};

static const int
TetVolFacePerEdgeTable[6][6] = {{0,1,2, 0,1,3}, {0,1,2, 1,2,3},
                                {0,1,2, 0,3,2}, {0,1,3, 0,3,2},
                                {1,2,3, 0,1,3}, {1,2,3, 0,3,2}};

static const int
TetVolEdgePerFaceTable[4][6] = {{0,1, 1,2, 2,0 },
                                {1,2, 3,1, 3,2 },
                                {0,1, 3,0, 3,1 },
                                {2,0, 3,0, 3,2}};

static const int
TetVolEdgeTable[6][2] = {{0,1},{1,2},{2,0},{3,0},{3,1},{3,2}};

static const int
TetVolFaceTable[4][3] = { {0,2,1},{1,2,3},{0,1,3},{0,3,2}};

/////////////////////////////////////////////////////
// Declarations for TetVolMesh class

template <class Basis>
class TetVolMesh : public Mesh
{
  /// Make sure the virtual interface has access
  template<class MESH> friend class VTetVolMesh;
  template<class MESH> friend class VMeshShared;
  template<class MESH> friend class VUnstructuredMesh;

public:
  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type                under_type;
  typedef SCIRun::index_type                index_type;
  typedef SCIRun::size_type                 size_type;
  typedef SCIRun::mask_type                 mask_type;

  typedef boost::shared_ptr<TetVolMesh<Basis> > handle_type;
  typedef Basis                             basis_type;

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
    typedef typename TetVolMesh<Basis>::index_type  index_type;

    ElemData(const TetVolMesh<Basis>& msh, const index_type ind) :
      mesh_(msh),
      index_(ind)
    {
      /// Linear and Constant Basis never use edges_
      if (basis_type::polynomial_order() > 1)
      {
        mesh_.get_edges_from_cell(edges_, index_);
      }
    }

    /// the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const
    {
      return mesh_.cells_[index_ * 4];
    }
    inline
    index_type node1_index() const
    {
      return mesh_.cells_[index_ * 4 + 1];
    }
    inline
    index_type node2_index() const
    {
      return mesh_.cells_[index_ * 4 + 2];
    }
    inline
    index_type node3_index() const
    {
      return mesh_.cells_[index_ * 4 + 3];
    }

    /// the following designed to coordinate with ::get_edges
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
    index_type edge4_index() const
    {
      return edges_[4];
    }
    inline
    index_type edge5_index() const
    {
      return edges_[5];
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
    const TetVolMesh<Basis>          &mesh_;
    /// copy of element index
    const index_type                 index_;
    /// need edges for quadratic meshes
    typename Edge::array_type        edges_;
  };

  friend class Synchronize;

  class Synchronize //: public Runnable
  {
    public:
      Synchronize(TetVolMesh<Basis>* mesh, mask_type sync) :
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

        if (sync_ & Mesh::NODE_NEIGHBORS_E) mesh_->compute_node_neighbors();
        if (sync_ & Mesh::EDGES_E) mesh_->compute_edges();
        if (sync_ & Mesh::FACES_E) mesh_->compute_faces();
        if (sync_ & Mesh::BOUNDING_BOX_E) mesh_->compute_bounding_box();

        // These depend on the bounding box being synchronized
        if (sync_ & (Mesh::NODE_LOCATE_E|Mesh::ELEM_LOCATE_E))
        {
          {
            Core::Thread::UniqueLock lock(mesh_->synchronize_lock_.get());
            while(!(mesh_->synchronized_ & Mesh::BOUNDING_BOX_E))
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
      TetVolMesh<Basis>* mesh_;
      mask_type  sync_;
  };

  //////////////////////////////////////////////////////////////////

  /// Construct a new mesh
  TetVolMesh();

  /// Copy a mesh, needed for detaching the mesh from a field
  TetVolMesh(const TetVolMesh &copy);

  /// Clone function for detaching the mesh and automatically generating
  /// a new version if needed.
  virtual TetVolMesh *clone() const override { return new TetVolMesh(*this); }

  /// Destructor
  virtual ~TetVolMesh();

  /// Access point to virtual interface
  virtual VMesh* vmesh() override { return vmesh_.get(); }

  MeshFacadeHandle getFacade() const override
  {
    return boost::make_shared<Core::Datatypes::VirtualMeshFacade<VMesh>>(vmesh_);
  }

  /// This one should go at some point, should be reroute through the
  /// virtual interface
  virtual int basis_order() override { return (basis_.polynomial_order()); }

  /// Topological dimension
  virtual int  dimensionality() const { return 3; }

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

  /// Core::Geometry::Transform a field (transform all nodes using this transformation matrix)
  virtual void transform(const Core::Geometry::Transform &t);

  /// Check whether mesh can be altered by adding nodes or elements
  virtual bool is_editable() const { return (true); }

  /// Has this mesh normals.
  virtual bool has_normals() const { return (false); }

  /// Has this mesh face normals
  virtual bool has_face_normals() const { return (true); }

  double get_epsilon() const { return (epsilon_); }

  /// Compute tables for doing topology, these need to be synchronized
  /// before doing a lot of operations.
  virtual bool synchronize(mask_type mask) override;
  virtual bool unsynchronize(mask_type mask) override;
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

  void get_edges(typename Edge::array_type &array,
                 typename Node::index_type idx) const
    { get_edges_from_node(array,idx); }
  void get_edges(typename Edge::array_type &array,
                 typename Edge::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_edges(typename Edge::array_type &array,
                 typename Face::index_type idx) const
    { get_edges_from_face(array,idx); }
  void get_edges(typename Edge::array_type &array,
                 typename Cell::index_type idx) const
    { get_edges_from_cell(array,idx); }

  void get_faces(typename Face::array_type &array,
                 typename Node::index_type idx) const
    { get_faces_from_node(array,idx);  }
  void get_faces(typename Face::array_type &array,
                 typename Edge::index_type idx) const
    { get_faces_from_edge(array,idx);  }

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

  void get_delems(typename DElem::array_type &array,
                  typename Node::index_type idx) const
    { get_faces_from_node(array,idx); }
  void get_delems(typename DElem::array_type &array,
                  typename Edge::index_type idx) const
    { get_faces_from_edge(array,idx); }
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
                       unsigned which_edge,
                       unsigned div_per_unit) const
  {
    basis_.approx_edge(which_edge, div_per_unit, coords);
  }

  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an face.
  template<class VECTOR, class INDEX>
  void pwl_approx_face(std::vector<std::vector<VECTOR > > &coords,
                       INDEX ci,
                       unsigned which_face,
                       unsigned div_per_unit) const
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

  /// Get the size of an element (length, area, volume)
  double get_size(typename Node::index_type /*idx*/) const
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
    Core::Geometry::Point p0,p1,p2;
    get_point(p0,ra[0]);
    get_point(p1,ra[1]);
    get_point(p2,ra[2]);
    return (Cross(p0-p1,p2-p0)).length()*0.5;
  }

  double get_size(typename Elem::index_type idx) const
  {
    ElemData ed(*this,idx);
    return (basis_.get_volume(ed));
  }

  /// More specific names for get_size
  double get_length(typename Edge::index_type idx) const
  { return get_size(idx); };
  double get_area(typename Face::index_type idx) const
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
  int get_weights(const Core::Geometry::Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point&, typename Edge::array_type&, double*)
  {ASSERTFAIL("TetVolMesh::get_weights for edges isn't supported"); }
  int get_weights(const Core::Geometry::Point&, typename Face::array_type&, double*)
  {ASSERTFAIL("TetVolMesh::get_weights for faces isn't supported"); }
  int get_weights(const Core::Geometry::Point &p, typename Cell::array_type &l, double *w);

  /// Access the nodes of the mesh
  void get_point(Core::Geometry::Point &result, typename Node::index_type index) const
  { result = points_[index]; }
  void set_point(const Core::Geometry::Point &point, typename Node::index_type index)
  { points_[index] = point; }
  void get_random_point(Core::Geometry::Point &p, typename Elem::index_type i, FieldRNG &r) const;

  /// Normals for visualizations
  void get_normal(Core::Geometry::Vector& /*result*/, typename Node::index_type /*index*/) const
  { ASSERTFAIL("TetVolMesh: This mesh type does not have node normals."); }

  /// Get the normals at the outside of the element
  template<class VECTOR, class INDEX1, class INDEX2>
  void get_normal(Core::Geometry::Vector &result, VECTOR& coords,
		  INDEX1 eidx, INDEX2 fidx)
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
    ASSERTMSG(a.size() == 4, "Tried to add non-tet element.");

    return add_tet( static_cast<typename Node::index_type>(a[0]),
		    static_cast<typename Node::index_type>(a[1]),
		    static_cast<typename Node::index_type>(a[2]),
		    static_cast<typename Node::index_type>(a[3]) );
  }

  /// Functions to improve memory management. Often one knows how many
  /// nodes/elements one needs, prereserving memory is often possible.
  void node_reserve(size_type s) { points_.reserve(static_cast<std::vector<Core::Geometry::Point>::size_type>(s)); }
  void elem_reserve(size_type s) { cells_.reserve(static_cast<std::vector<index_type>::size_type>(s*4)); }
  void resize_nodes(size_type s) { points_.resize(static_cast<std::vector<Core::Geometry::Point>::size_type>(s)); }
  void resize_elems(size_type s) { cells_.resize(static_cast<std::vector<index_type>::size_type>(s*4)); }

  /// Get the local coordinates for a certain point within an element
  /// This function uses a couple of newton iterations to find the local
  /// coordinate of a point
  template<class VECTOR, class INDEX>
  bool  get_coords(VECTOR &coords, const Core::Geometry::Point &p, INDEX idx) const
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
  void jacobian(const VECTOR& coords,INDEX idx, double* J) const
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
    double min_jacobian = DetMatrix3P(Jv);
    INDEX idx2 = idx*4;
    Core::Geometry::Point p0 = points_[cells_[idx2]];
    Core::Geometry::Point p1 = points_[cells_[idx2+1]];
    Core::Geometry::Point p2 = points_[cells_[idx2+2]];
    Core::Geometry::Point p3 = points_[cells_[idx2+3]];

    double l0 = (p1-p0).length();
    double l1 = (p2-p1).length();
    double l2 = (p0-p2).length();
    double l3 = (p3-p0).length();
    double l4 = (p3-p1).length();
    double l5 = (p3-p2).length();

    double min_scale = l0*l2*l3;
    temp = l0*l1*l4;
    if (temp > min_scale) min_scale = temp;
    temp = l1*l2*l5;
    if (temp > min_scale) min_scale = temp;
    temp = l3*l4*l5;
    if (temp > min_scale) min_scale = temp;
    if (min_jacobian > min_scale) min_scale = min_jacobian;

    return (sqrt(2.0)*min_jacobian/(min_scale));
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


  template<class INDEX>
  double inscribed_circumscribed_radius_metric(INDEX idx) const
  {
    StackVector<Core::Geometry::Point,3> Vec;
    ElemData ed(*this,idx);

    INDEX idx2 = idx*4;
    Core::Geometry::Point p0 = points_[cells_[idx2]];
    Core::Geometry::Point p1 = points_[cells_[idx2+1]];
    Core::Geometry::Point p2 = points_[cells_[idx2+2]];
    Core::Geometry::Point p3 = points_[cells_[idx2+3]];

    double l0 = (p1-p0).length();
    double l1 = (p2-p1).length();
    double l2 = (p0-p2).length();
    double l3 = (p3-p0).length();
    double l4 = (p3-p1).length();
    double l5 = (p3-p2).length();

  // area of a triangle using heron's formula
  // heron's area = sqrt(s(s-a)(s-b)(s-c)) where s=0.5*(a+b+c)
  double tri_AREA = (l0*l5 + l1*l3 + l2*l4) / 2.0;

  // volume of the element
  double tet_VOL = basis_.get_volume(ed);

  // surface area of element
  double s_A = 0.5*(l0+l1+l2);
  double s_B = 0.5*(l1+l4+l5);
  double s_C = 0.5*(l2+l3+l5);
  double s_D = 0.5*(l0+l3+l4);

  double tri_A = sqrt(s_A*(s_A - l0)*(s_A - l1)*(s_A - l2));
  double tri_B = sqrt(s_B*(s_B - l1)*(s_B - l4)*(s_B - l5));
  double tri_C = sqrt(s_C*(s_C - l2)*(s_C - l3)*(s_C - l5));
  double tri_D = sqrt(s_D*(s_D - l0)*(s_D - l3)*(s_D - l4));

  double tet_SA = tri_A + tri_B + tri_C + tri_D;

  // calculating inscribed and circumscribed radii
  double r_in = 3.0 * tet_VOL / tet_SA;
  double r_cir = sqrt(tri_AREA*(tri_AREA - l0*l5)*(tri_AREA - l1*l3)*(tri_AREA - l2*l4)) / (6.0*tet_VOL);

  // for an equilateral tetrahedron, the circ radius is 3 times larger than the inscr
  double ideal_ratio = 0.333333;
  double normalized_actual_ratio = (r_in / r_cir) / ideal_ratio;
  return (normalized_actual_ratio);
  }

  template <class INDEX>
  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         INDEX &node, const Core::Geometry::Point &p) const
  {
    return(find_closest_node(pdist,result,node,p,-1.0));
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
	      "TetVolMesh::find_closest_node requires synchronize(NODE_LOCATE_E).");

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
        "TetVolMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

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
        "TetVolMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

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
    return (find_closest_elem(pdist,result,coords,elem,p,-1.0));
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

    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "TetVolMesh: need to synchronize FACES_E first");
    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
              "TetVolMesh: need to synchronize ELEM_LOCATE_E first");

    // First check are we inside an element
    SearchGridT<index_type>::iterator it, eit;
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
                  index_type idx = cidx*4;
                  unsigned char b = boundary_faces_[cidx];
                  if (b)
                  {
                    if (b & 0x1)
                    {
                      closest_point_on_tri(r, p,
                                           points_[cells_[idx  ]],
                                           points_[cells_[idx+2]],
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
                          pdist = sqrt(dmin);
                          ElemData ed(*this,elem);
                          basis_.get_coords(coords,result,ed);
                          return (true);
                        }
                      }
                    }
                    if (b & 0x2)
                    {
                      closest_point_on_tri(r, p,
                                           points_[cells_[idx+1]],
                                           points_[cells_[idx+2]],
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
                          pdist = sqrt(dmin);
                          ElemData ed(*this,elem);
                          basis_.get_coords(coords,result,ed);
                          return (true);
                        }
                      }
                    }
                    if (b & 0x4)
                    {
                      closest_point_on_tri(r, p,
                                           points_[cells_[idx]],
                                           points_[cells_[idx+1]],
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
                          pdist = sqrt(dmin);
                          ElemData ed(*this,elem);
                          basis_.get_coords(coords,result,ed);
                          return (true);
                        }
                      }
                    }
                    if (b & 0x8)
                    {
                      closest_point_on_tri(r, p,
                                           points_[cells_[idx]],
                                           points_[cells_[idx+3]],
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
                          pdist = sqrt(dmin);
                          ElemData ed(*this,elem);
                          basis_.get_coords(coords,result,ed);
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
    while (!found) ;

    if (!found_one) return (false);

    ElemData ed(*this,elem);
    basis_.get_coords(coords,result,ed);

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
                          ARRAY& /*elems*/, const Core::Geometry::Point& /*p*/) const
  {
    ASSERTFAIL("TetVolMesh: Find closest element has not yet been implemented.");
    return (false);
  }

  /// Export this class using the old Pio system
  virtual void io(Piostream&) override;

  ///////////////////////////////////////////////////
  // STATIC VARIABLES AND FUNCTIONS

  /// This ID is created as soon as this class will be instantiated
  static PersistentTypeID tetvolmesh_typeid;

  /// Core functionality for getting the name of a templated mesh class
  static  const std::string type_name(int n = -1);
  virtual std::string dynamic_type_name() const override { return tetvolmesh_typeid.type; }

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
  static Persistent* maker() { return new TetVolMesh(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return boost::make_shared<TetVolMesh>(); }

  //////////////////////////////////////////////////////////////////
  // Mesh specific functions (these are not implemented in every mesh)

  // Extra functionality needed by this specific geometry.
  void set_nodes(typename Node::array_type &,
		 typename Cell::index_type);

  /// Deleting cells, make sure mesh is detached
  void delete_cells(std::set<index_type> &to_delete);
  void delete_nodes(std::set<index_type> &to_delete);

  /// Orient a cell properly
  void orient(typename Cell::index_type ci);

  /// MORE UNSORTED FUNCTIONS
  /// Used to recompute data for individual cells.  Don't use these, they
  // are not synchronous.  Use create_cell_syncinfo instead.
  void create_cell_edges(typename Cell::index_type);
  void delete_cell_edges(typename Cell::index_type, bool table_only = false);
  void create_cell_faces(typename Cell::index_type);
  void delete_cell_faces(typename Cell::index_type, bool table_only = false);
  void create_cell_node_neighbors(typename Cell::index_type);
  void delete_cell_node_neighbors(typename Cell::index_type);

  void create_cell_syncinfo(typename Cell::index_type ci);
  void delete_cell_syncinfo(typename Cell::index_type ci);
  // Create the partial sync info needed by insert_node_in_elem
  void create_cell_syncinfo_special(typename Cell::index_type ci);
  void delete_cell_syncinfo_special(typename Cell::index_type ci);
  // May reorient the tet to make the signed volume positive.
  typename Elem::index_type add_tet_pos(typename Node::index_type a,
                                        typename Node::index_type b,
                                        typename Node::index_type c,
                                        typename Node::index_type d);
  void mod_tet_pos(typename Elem::index_type tet,
                   typename Node::index_type a,
                   typename Node::index_type b,
                   typename Node::index_type c,
                   typename Node::index_type d);

  /// Functions needed for cubit interface
  /// WE SHOULD MAKE THESE GENERAL AND IN EVERY MESHTYPE
  template <class Iter, class Functor>
  void fill_points(Iter begin, Iter end, Functor fill_ftor);
  template <class Iter, class Functor>
  void fill_cells(Iter begin, Iter end, Functor fill_ftor);
  template <class Iter, class Functor>
  void fill_neighbors(Iter begin, Iter end, Functor fill_ftor);
  template <class Iter, class Functor>
  void fill_data(Iter begin, Iter end, Functor fill_ftor);

  // THIS FUNCTION NEEDS TO BE REVISED AS IT DOES NOT SCALE PROPERLY
  // THE TOLERANCE IS NOT RELATIVE, WHICH IS A PROBLEM.........
  typename Node::index_type     add_find_point(const Core::Geometry::Point &p,
                                               double err = 1.0e-6);

  // Short cut for generating an element....
  typename Elem::index_type add_tet(typename Node::index_type a,
				    typename Node::index_type b,
				    typename Node::index_type c,
				    typename Node::index_type d);

  typename Elem::index_type add_tet(const Core::Geometry::Point &p0,
				    const Core::Geometry::Point &p1,
				    const Core::Geometry::Point &p2,
				    const Core::Geometry::Point &p3);

  bool insert_node_in_elem(typename Elem::array_type &tets,
			   typename Node::index_type &ni,
			   typename Elem::index_type ci,
			   const Core::Geometry::Point &p);

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
  inline void get_nodes_from_edge(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "TetVolMesh: Must call synchronize EDGES_E first");

    if (edges_[idx].cells_.size() == 0)
      { array.clear(); return; }

    array.resize(2);

    index_type cell_edge_index = edges_[idx].cells_[0];
    index_type cell_index = (cell_edge_index>>3) << 2;
    index_type edge_index = (cell_edge_index)&0x7;

    const int *offset = TetVolEdgeTable[edge_index];
    array[0] = static_cast<typename ARRAY::value_type>(cells_[cell_index+offset[0]]);
    array[1] = static_cast<typename ARRAY::value_type>(cells_[cell_index+offset[1]]);
  }

  // Always returns nodes in counter-clockwise order
  template<class ARRAY, class INDEX>
  inline void get_nodes_from_face(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "TetVolMesh: Must call synchronize FACES_E first");

    if (faces_[idx].cells_[0] == MESH_NO_NEIGHBOR)
      { array.clear(); return; }

    index_type cell_face_index = faces_[idx].cells_[0];
    index_type cell_index = cell_face_index&(~0x3);
    index_type face_index = cell_face_index&0x3;

    array.resize(3);

    const int *offset = TetVolFaceTable[face_index];
    array[0] = static_cast<typename ARRAY::value_type>(cells_[cell_index+offset[0]]);
    array[1] = static_cast<typename ARRAY::value_type>(cells_[cell_index+offset[1]]);
    array[2] = static_cast<typename ARRAY::value_type>(cells_[cell_index+offset[2]]);
  }

  template<class ARRAY, class INDEX>
  inline void get_nodes_from_cell(ARRAY& array, INDEX idx) const
  {
    array.resize(4);
    const index_type off = idx * 4;
    array[0] = static_cast<typename ARRAY::value_type>(cells_[off]);
    array[1] = static_cast<typename ARRAY::value_type>(cells_[off + 1]);
    array[2] = static_cast<typename ARRAY::value_type>(cells_[off + 2]);
    array[3] = static_cast<typename ARRAY::value_type>(cells_[off + 3]);
  }

  template<class ARRAY, class INDEX>
  inline void get_nodes_from_elem(ARRAY& array, INDEX idx) const
  {
    get_nodes_from_cell(array,idx);
  }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_face(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & (Mesh::EDGES_E),
              "TetVolMesh: Must call synchronize EDGES_E first");
    ASSERTMSG(synchronized_ & (Mesh::FACES_E),
              "TetVolMesh: Must call synchronize FACES_E first");

    array.clear();
    array.reserve(3);
    index_type cell_index = (faces_[idx].cells_[0])&(~0x3);
    index_type face_index = (faces_[idx].cells_[0])&(0x3);

    const int* offset = TetVolFaceTable[face_index];

    typename Node::index_type n0 = cells_[cell_index+offset[0]];
    typename Node::index_type n1 = cells_[cell_index+offset[1]];
    typename Node::index_type n2 = cells_[cell_index+offset[2]];

    if (n0 != n1)
    {
      PEdgeNode e(n0, n1);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(edge_table_.find(e))).second));
    }
    if (n1 != n2)
    {
      PEdgeNode e(n1, n2);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(edge_table_.find(e))).second));
    }
    if (n2 != n0)
    {
      PEdgeNode e(n2, n0);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(edge_table_.find(e))).second));
    }
 }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_cell(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "TetVolMesh: Must call synchronize EDGES_E first");

    const index_type off = idx * 4;
    PEdgeNode e00(cells_[off + 0], cells_[off + 1]);
    PEdgeNode e01(cells_[off + 1], cells_[off + 2]);
    PEdgeNode e02(cells_[off + 2], cells_[off + 0]);
    PEdgeNode e03(cells_[off + 0], cells_[off + 3]);
    PEdgeNode e04(cells_[off + 1], cells_[off + 3]);
    PEdgeNode e05(cells_[off + 2], cells_[off + 3]);
    typename Node::index_type n1,n2;

    n1 = cells_[off    ]; n2 = cells_[off + 1];
    size_t i = 0;
    typedef typename ARRAY::value_type T;
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
    n1 = cells_[off    ]; n2 = cells_[off + 3];
    if (n1 != n2)
    {
      PEdge e(n1,n2);
      array[i++] = (static_cast<T>((*(edge_table_.find(e))).second));
    }
    n1 = cells_[off + 1]; n2 = cells_[off + 3];
    if (n1 != n2)
    {
      PEdge e(n1,n2);
      array[i++] = (static_cast<T>((*(edge_table_.find(e))).second));
    }
    n1 = cells_[off + 2]; n2 = cells_[off + 3];
    if (n1 != n2)
    {
      PEdge e(n1,n2);
      array[i++] = (static_cast<T>((*(edge_table_.find(e))).second));
    }
  }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_elem(ARRAY& array, INDEX idx) const
  {
    get_edges_from_cell(array,idx);
  }

  template<class ARRAY, class INDEX>
  inline void get_faces_from_cell(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "TetVolMesh: Must call synchronize FACES_E first");

    array.clear();
    array.resize(4);

    const index_type off = idx << 2;
    const typename Node::index_type n0 = cells_[off    ];
    const typename Node::index_type n1 = cells_[off + 1];
    const typename Node::index_type n2 = cells_[off + 2];
    const typename Node::index_type n3 = cells_[off + 3];

    PFaceNode f0(n3, n2, n1);
    PFaceNode f1(n0, n2, n3);
    PFaceNode f2(n3, n1, n0);
    PFaceNode f3(n0, n1, n2);

    array[0] = static_cast<typename ARRAY::value_type>(
                                          (*(face_table_.find(f0))).second);
    array[1] = static_cast<typename ARRAY::value_type>(
                                          (*(face_table_.find(f1))).second);
    array[2] = static_cast<typename ARRAY::value_type>(
                                          (*(face_table_.find(f2))).second);
    array[3] = static_cast<typename ARRAY::value_type>(
                                          (*(face_table_.find(f3))).second);
  }

  template<class ARRAY, class INDEX>
  inline void get_cells_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
            "TetVolMesh: Must call synchronize NODE_NEIGHBORS_E first.");

    array.resize(node_neighbors_[idx].size());
    for (size_t i = 0; i < node_neighbors_[idx].size(); ++i)
      array[i] = static_cast<typename ARRAY::value_type>(
                                                  (node_neighbors_[idx][i])>>2);
  }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & (Mesh::NODE_NEIGHBORS_E),
      "HexVolMesh: Must call synchronize NODE_NEIGHBORS_E first");
    ASSERTMSG(synchronized_ & (Mesh::EDGES_E),
      "HexVolMesh: Must call synchronize EDGES_E first");

    // Get all the nodes that share an edge with this node
    const std::vector<typename Cell::index_type>& neighbors = node_neighbors_[idx];

    array.clear();
    array.reserve(neighbors.size());
    // Iterate through all those edges
    for (size_t n = 0; n < neighbors.size(); n++)
    {
      index_type cell_index = neighbors[n]&(~0x3);
      index_type node_index = neighbors[n]&0x3;

      const int *offset = TetVolEdgePerNodeTable[node_index];

      PEdgeNode e(cells_[cell_index+offset[0]],cells_[cell_index+offset[1]]);
      typename edge_nt::const_iterator iter = edge_table_.find(e);
      if (((edges_[iter->second].cells_[0])&(~0x7))==(cell_index<<1) )
        array.push_back(typename ARRAY::value_type(iter->second));

      PEdgeNode e1(cells_[cell_index+offset[2]],cells_[cell_index+offset[3]]);
      iter = edge_table_.find(e1);
      if (((edges_[iter->second].cells_[0])&(~0x7))==(cell_index<<1) )
        array.push_back(typename ARRAY::value_type(iter->second));

      PEdgeNode e2(cells_[cell_index+offset[4]],cells_[cell_index+offset[5]]);
      iter = edge_table_.find(e2);
      if (((edges_[iter->second].cells_[0])&(~0x7))==(cell_index<<1) )
        array.push_back(typename ARRAY::value_type(iter->second));
    }
  }

  template<class ARRAY, class INDEX>
  inline void get_faces_from_edge(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & (Mesh::EDGES_E),
      "TetVolMesh: Must call synchronize EDGES_E first");
    ASSERTMSG(synchronized_ & (Mesh::FACES_E),
      "TetVolMesh: Must call synchronize FACES_E first");

    array.clear();

    for (size_t c=0; c<edges_[idx].cells_.size();c++)
    {
      index_type cell_index = ((edges_[idx].cells_[c])>>3)<<2;
      index_type face_index = (edges_[idx].cells_[c])&0x7;

      const int* off = TetVolFacePerEdgeTable[face_index];

      typename Node::index_type n1, n2, n3;
      typename face_nt::const_iterator fiter;

      n1 = cells_[cell_index+off[0]]; n2 = cells_[cell_index+off[1]];
      n3 = cells_[cell_index+off[2]];

      fiter = face_table_.find(PFaceNode(n1,n2,n3));
      if (((faces_[fiter->second].cells_[0])&(~0x3)) == cell_index)
        array.push_back(typename ARRAY::value_type(fiter->second));

      n1 = cells_[cell_index+off[3]]; n2 = cells_[cell_index+off[4]];
      n3 = cells_[cell_index+off[5]];

      fiter = face_table_.find(PFaceNode(n1,n2,n3));
      if (((faces_[fiter->second].cells_[0])&(~0x3)) == cell_index)
        array.push_back(typename ARRAY::value_type(fiter->second));
    }
  }

  template<class ARRAY, class INDEX>
  inline void get_faces_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & (Mesh::NODE_NEIGHBORS_E),
      "TetVolMesh: Must call synchronize NODE_NEIGHBORS_E first");
    ASSERTMSG(synchronized_ & (Mesh::EDGES_E),
      "TetVolMesh: Must call synchronize EDGES_E first");
    ASSERTMSG(synchronized_ & (Mesh::FACES_E),
      "TetVolMesh: Must call synchronize FACES_E first");

    // Get all the nodes that share an edge with this node
    const std::vector<typename Cell::index_type>& neighbors = node_neighbors_[idx];

    array.clear();
    array.reserve(neighbors.size());
    // Iterate through all those edges
    for (size_t n = 0; n < neighbors.size(); n++)
    {
      index_type cell_index = neighbors[n]&(~0x3);
      index_type node_index = neighbors[n]&0x3;

      const int *offset = TetVolFacePerNodeTable[node_index];

      PFaceNode e(cells_[cell_index+offset[0]],
                  cells_[cell_index+offset[1]],cells_[cell_index+offset[2]]);

      typename face_nt::const_iterator iter = face_table_.find(e);
      if (((faces_[iter->second].cells_[0])&(~0x3))==cell_index)
        array.push_back(typename ARRAY::value_type(iter->second));

      PFaceNode e1(cells_[cell_index+offset[3]],cells_[cell_index+offset[4]],
        cells_[cell_index+offset[5]]);
      iter = face_table_.find(e1);
      if (((faces_[iter->second].cells_[0])&(~0x3))==cell_index )
        array.push_back(typename ARRAY::value_type(iter->second));

      PFaceNode e2(cells_[cell_index+offset[6]],cells_[cell_index+offset[7]],
        cells_[cell_index+offset[8]]);
      iter = face_table_.find(e2);
      if (((faces_[iter->second].cells_[0])&(~0x3))==cell_index )
        array.push_back(typename ARRAY::value_type(iter->second));
    }
  }

  template<class ARRAY, class INDEX>
  inline void get_cells_from_edge(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "TetVolMesh: Must call synchronize EDGES_E first");
    for (size_t i=0; i< edges_[idx].cells_.size(); i++)
      array[i] = static_cast<typename ARRAY::value_type>(edges_[idx].cells_[i]);
  }

  template<class ARRAY, class INDEX>
  inline void get_cells_from_face(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "TetVolMesh: Must call synchronize FACES_E first");
    if (faces_[idx].cells_[1] == MESH_NO_NEIGHBOR)
    {
      array.resize(1);
      array[0] = static_cast<typename ARRAY::value_type>((faces_[idx].cells_[0])>>2);
    }
    else
    {
      array.resize(2);
      array[0] = static_cast<typename ARRAY::value_type>((faces_[idx].cells_[0])>>2);
      array[1] = static_cast<typename ARRAY::value_type>((faces_[idx].cells_[1])>>2);
    }
  }

  template <class ARRAY, class INDEX>
  inline void set_nodes_by_elem(ARRAY &array, INDEX idx)
  {
    for (index_type n = 0; n < 4; ++n)
      cells_[idx * 4 + n] = static_cast<index_type>(array[n]);
  }

  template <class INDEX1, class INDEX2>
  inline bool get_elem_neighbor(INDEX1 &neighbor, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "TetVolMesh: Must call synchronize FACES_E on TetVolMesh first");
    const PFaceCell &f = faces_[delem];

    if (elem == static_cast<INDEX1>((f.cells_[0])>>2))
    {
      if (f.cells_[1] == MESH_NO_NEIGHBOR) return (false);
      neighbor = static_cast<INDEX1>((f.cells_[1])>>2);
    }
    else
    {
      if (f.cells_[0] == MESH_NO_NEIGHBOR) return (false);
      neighbor = static_cast<INDEX1>((f.cells_[0])>>2);
    }
    return (true);
  }

  template <class ARRAY, class INDEX1, class INDEX2>
  inline bool get_elem_neighbors(ARRAY &array, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "Must call synchronize FACES_E on TetVolMesh first");

    const PFaceCell &f = faces_[delem];

    if (elem == static_cast<INDEX1>((f.cells_[0])>>2))
    {
      if (f.cells_[1] == MESH_NO_NEIGHBOR) return (false);
      array.resize(1);
      array[0] = static_cast<typename ARRAY::value_type>((f.cells_[1])>>2);
    }
    else
    {
      if (f.cells_[0] == MESH_NO_NEIGHBOR) return (false);
      array.resize(1);
      array[0] = static_cast<typename ARRAY::value_type>((f.cells_[0])>>2);
    }
    return (true);
  }

  template <class ARRAY, class INDEX>
  inline void get_elem_neighbors(ARRAY &array, INDEX idx) const
  {
    typename Face::array_type faces;
    get_faces_from_cell(faces, idx);

    array.clear();
    array.reserve(faces.size());
    typename Face::array_type::iterator iter = faces.begin();
    while(iter != faces.end())
    {
      typename ARRAY::value_type nbor;
      if (get_elem_neighbor(nbor, idx, *iter))
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
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on TetVolMesh first.");
    size_t sz = node_neighbors_[node].size();

    std::set<index_type> inserted;
    for (size_t i = 0; i < sz; i++)
    {
      const index_type base = ((node_neighbors_[node][i])&(~0x3));
      for (index_type c = base; c < base+4; ++c)
      {
        if (cells_[c] != node) inserted.insert(cells_[c]);
      }
    }

    array.clear();
    array.reserve(inserted.size());
    array.insert(array.begin(), inserted.begin(), inserted.end());
  }

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
              "TetVolMesh::locate_node requires synchronize(NODE_LOCATE_E).")

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
  inline bool locate_edge(INDEX &edge, const Core::Geometry::Point &p) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on TetVolMesh first");

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
  inline bool locate_face(INDEX &face, const Core::Geometry::Point &p) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "Must call synchronize FACES_E on TetVolMesh first");

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
  inline bool locate_elem(INDEX &elem, const Core::Geometry::Point &p) const
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
                "TetVolMesh: need to synchronize ELEM_LOCATE_E first");

    typename SearchGridT<index_type>::iterator it, eit;
    if (elem_grid_->lookup(it, eit, p))
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
              "TetVolMesh::locate_elems requires synchronize(ELEM_LOCATE_E).")

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
                "TetVolMesh: need to synchronize ELEM_LOCATE_E first");

    typename SearchGridT<index_type>::iterator it, eit;
    if (elem_grid_->lookup(it, eit, p))
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
  inline void get_edge_center(Core::Geometry::Point &p, INDEX idx) const
  {
    typename Node::array_type arr;
    get_nodes_from_edge(arr, idx);
    Core::Geometry::Point p1;
    get_node_center(p, arr[0]);
    get_node_center(p1, arr[1]);
    p += p1;
    p *= 0.5;
  }

  template <class INDEX>
  inline void get_face_center(Core::Geometry::Point &p, INDEX idx) const
  {
    const double s = 1.0/3.0;
    typename Node::array_type arr;
    get_nodes_from_face(arr, idx);
    p = points_[arr[0]];
    const Core::Geometry::Point &p1 = points_[arr[1]];
    const Core::Geometry::Point &p2 = points_[arr[2]];

    p += p1;
    p += p2;
    p *= s;
  }

  template <class INDEX>
  inline void get_cell_center(Core::Geometry::Point &p, INDEX idx) const
  {
    const double s = 0.25;
    const Core::Geometry::Point &p0 = points_[cells_[idx * 4 + 0]];
    const Core::Geometry::Point &p1 = points_[cells_[idx * 4 + 1]];
    const Core::Geometry::Point &p2 = points_[cells_[idx * 4 + 2]];
    const Core::Geometry::Point &p3 = points_[cells_[idx * 4 + 3]];

    p = Core::Geometry::Point((p0 + p1 + p2 + p3) * s);
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
    const Core::Geometry::Point &p0 = points_[cells_[idx*4+0]];
    const Core::Geometry::Point &p1 = points_[cells_[idx*4+1]];
    const Core::Geometry::Point &p2 = points_[cells_[idx*4+2]];
    const Core::Geometry::Point &p3 = points_[cells_[idx*4+3]];

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
    if (s0 < -1e-7) return (false);

    const double b1 = + (y3*z0-y0*z3) + (y0*z2-y2*z0) + (y2*z3-y3*z2);
    const double c1 = - (x3*z0-x0*z3) - (x0*z2-x2*z0) - (x2*z3-x3*z2);
    const double d1 = + (x3*y0-x0*y3) + (x0*y2-x2*y0) + (x2*y3-x3*y2);
    const double s1 = iV6 * (a1 + b1*p.x() + c1*p.y() + d1*p.z());
    if (s1 < -1e-7) return (false);

    const double b2 = - (y0*z1-y1*z0) - (y1*z3-y3*z1) - (y3*z0-y0*z3);
    const double c2 = + (x0*z1-x1*z0) + (x1*z3-x3*z1) + (x3*z0-x0*z3);
    const double d2 = - (x0*y1-x1*y0) - (x1*y3-x3*y1) - (x3*y0-x0*y3);
    const double s2 = iV6 * (a2 + b2*p.x() + c2*p.y() + d2*p.z());
    if (s2 < -1e-7) return (false);

    const double b3 = +(y1*z2-y2*z1) + (y2*z0-y0*z2) + (y0*z1-y1*z0);
    const double c3 = -(x1*z2-x2*z1) - (x2*z0-x0*z2) - (x0*z1-x1*z0);
    const double d3 = +(x1*y2-x2*y1) + (x2*y0-x0*y2) + (x0*y1-x1*y0);
    const double s3 = iV6 * (a3 + b3*p.x() + c3*p.y() + d3*p.z());
    if (s3 < -1e-7) return (false);

    return (true);
  }

  /// all the nodes.
  std::vector<Core::Geometry::Point>         points_;

  /// each 4 indicies make up a tet
  std::vector<under_type>    cells_;

  /// Face information.
  class PFaceCell {
  public:
    PFaceCell()
    {
      cells_[0] = MESH_NO_NEIGHBOR;
      cells_[1] = MESH_NO_NEIGHBOR;
    }

    bool shared() const { return ((cells_[0] != MESH_NO_NEIGHBOR) &&
                                  (cells_[1] != MESH_NO_NEIGHBOR)); }
    index_type  cells_[2];
  };

  class PFaceNode {
    public:
    // The order of nodes_ corresponds with cells_[0] for CW/CCW purposes.
    typename Node::index_type         nodes_[3];  /// 3 nodes makes a face.

    PFaceNode() {
      nodes_[0] = MESH_NO_NEIGHBOR;
      nodes_[1] = MESH_NO_NEIGHBOR;
      nodes_[2] = MESH_NO_NEIGHBOR;
    }

    // snodes_ must be sorted. See Hash Function below.
    PFaceNode(typename Node::index_type n1, typename Node::index_type n2,
          typename Node::index_type n3)
    {
      if (n2 < n1 && n2 < n3)
      {
        typename Node::index_type t;
        if (n3 < n1)
        {
          t = n1; n1 = n2; n2 = n3; n3 = t;
        }
        else
        {
          t = n1; n1 = n2; n2 = t;
        }
      }
      else if (n3 < n1 && n3 < n2)
      {
        typename Node::index_type t;
        if (n1 < n2)
        {
          t = n1; n1 = n3; n3 = n2; n2 = t;
        }
        else
        {
          t = n1; n1 = n3; n3 = t;
        }
      }
      else if (n3 < n2)
      {
        typename Node::index_type t;
        t = n2; n2 = n3; n3 = t;
      }
      nodes_[0] = n1;
      nodes_[1] = n2;
      nodes_[2] = n3;
    }

    /// true if both have the same nodes (order does not matter)
    bool operator==(const PFaceNode &f) const
    {
      return ((nodes_[0] == f.nodes_[0]) && (nodes_[1] == f.nodes_[1]) &&
              (nodes_[2] == f.nodes_[2]));
    }

    /// Compares each node.  When a non equal node is found the <
    /// operator is applied.
    bool operator<(const PFaceNode &f) const
    {
      if (nodes_[0] == f.nodes_[0])
        if (nodes_[1] == f.nodes_[1])
            return (nodes_[2] < f.nodes_[2]);
        else
          return (nodes_[1] < f.nodes_[1]);
      else
        return (nodes_[0] < f.nodes_[0]);
    }
  };

  class PFace : public PFaceNode, public PFaceCell
  {
    public:
      PFace(typename Node::index_type n1, typename Node::index_type n2,
          typename Node::index_type n3) :
        PFaceNode(n1,n2,n3) {}
  };

  /// Edge information.
  class PEdgeNode {
    public:
    typename Node::index_type nodes_[2];

    PEdgeNode()
    {
      nodes_[0] = MESH_NO_NEIGHBOR;
      nodes_[1] = MESH_NO_NEIGHBOR;
    }
    // node_[0] must be smaller than node_[1]. See Hash Function below.
    PEdgeNode(typename Node::index_type n1,
              typename Node::index_type n2)
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

    /// true if both have the same nodes (order does not matter)
    bool operator==(const PEdgeNode &e) const
    {
      return ((nodes_[0] == e.nodes_[0]) && (nodes_[1] == e.nodes_[1]));
    }

    /// Compares each node.  When a non equal node is found the <
    /// operator is applied.
    bool operator<(const PEdgeNode &e) const
    {
      if (nodes_[0] == e.nodes_[0])
        return (nodes_[1] < e.nodes_[1]);
      else
        return (nodes_[0] < e.nodes_[0]);
    }
  };

  class PEdgeCell {
    public:
      /// list of all the cells this edge is in.
      std::vector<index_type> cells_;

      bool shared() const { return cells_.size() > 1; }
  };

  /// Edge information.
  class PEdge : public PEdgeNode, public PEdgeCell
  {
    public:
      PEdge(typename Node::index_type n1,typename Node::index_type n2) :
        PEdgeNode(n1,n2) {}
  };

  /// hash the egde's node_indecies such that edges with the same nodes
  ///  hash to the same value. nodes are sorted on edge construction.
  static const int sz_int = sizeof(int) * 8; // in bits
  struct FaceHash
  {
    // These are needed by the hash_map particularly
    // ANSI C++ allows us to initialize these variables in the
    // declaration.  However there may be compilers which will complain
    // about it.
    static const size_t bucket_size = 4;
    static const size_t min_buckets = 8;

    /// These are for our own use (making the hash function).
    static const int sz_third_int = (int)(sz_int / 3);
    static const int up_mask = -(1 << sz_third_int << sz_third_int);
    static const int mid_mask =  up_mask ^ -(1 << sz_third_int);
    static const int low_mask = ~(up_mask | mid_mask);

    /// This is the hash function
    template <class PFACE>
    size_t operator()(const PFACE &f) const
    {
      return ((up_mask & (static_cast<int>(f.nodes_[0]) << sz_third_int << sz_third_int)) |
              (mid_mask & (static_cast<int>(f.nodes_[1]) << sz_third_int)) |
              (low_mask & static_cast<int>(f.nodes_[2])));
    }
    /// This should return less than rather than equal to.

    template <class PFACE>
    bool operator()(const PFACE &f1, const PFACE& f2) const
    {
      return f1 < f2;
    }
  };

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
    template <class PEDGE>
    size_t operator()(const PEDGE &e) const
    {
      return (static_cast<int>(e.nodes_[0]) << sz_half_int) |
              (low_mask & static_cast<int>(e.nodes_[1]));
    }

    ///  This should return less than rather than equal to.
    template <class PEDGE>
    bool operator()(const PEDGE &e1, const PEDGE& e2) const
    {
      return e1 < e2;
    }
  };

  using face_ht = boost::unordered_map<PFace, typename Face::index_type, FaceHash>;
  using face_nt = boost::unordered_map<PFaceNode, typename Face::index_type, FaceHash>;
  using edge_ht = boost::unordered_map<PEdge, typename Edge::index_type, EdgeHash>;
  using edge_nt = boost::unordered_map<PEdgeNode, typename Edge::index_type, EdgeHash>;

  typedef std::vector<PFaceCell> face_ct;
  typedef std::vector<PEdgeCell> edge_ct;

  // These should not be called outside of the synchronize_lock_.

  /// container for face storage. Must be computed each time
  ///  nodes or cells change.
  face_ct faces_;
  face_nt face_table_;
  /// container for edge storage. Must be computed each time
  ///  nodes or cells change.
  edge_ct edges_;
  edge_nt edge_table_;

  inline void remove_edge(typename Node::index_type n1,
			  typename Node::index_type n2,
			  typename Cell::index_type ci,
			  bool table_only = false);

  inline void hash_edge(typename Node::index_type n1,
                        typename Node::index_type n2,
                        index_type combined_index,
                        edge_ht& table);

  inline void add_edge(typename Node::index_type n1,
                        typename Node::index_type n2,
                        index_type combined_index);

  inline void remove_face(typename Node::index_type n1,
                          typename Node::index_type n2,
                          typename Node::index_type n3,
                          typename Cell::index_type ci,
                          bool table_only = false);
  inline void hash_face(typename Node::index_type n1,
                        typename Node::index_type n2,
                        typename Node::index_type n3,
                        index_type ci, face_ht& table);
  inline void add_face(typename Node::index_type n1,
                       typename Node::index_type n2,
                       typename Node::index_type n3,
                       index_type combined_index);

  std::vector<std::vector<typename Cell::index_type> > node_neighbors_;
  std::vector<unsigned char> boundary_faces_;

  /// This grid is used as an acceleration structure to expedite calls
  ///  to locate.  For each cell in the grid, we store a list of which
  ///  tets overlap that grid cell -- to find the tet which contains a
  ///  point, we simply find which grid cell contains that point, and
  ///  then search just those tets that overlap that grid cell.
  boost::shared_ptr<SearchGridT<index_type> >  node_grid_;
  boost::shared_ptr<SearchGridT<index_type> >  elem_grid_;

  // Lock and Condition Variable for hand shaking
  mutable Core::Thread::Mutex                 synchronize_lock_;
  Core::Thread::ConditionVariable             synchronize_cond_;

  // Which tables have been computed
  mask_type                     synchronized_;
  // Which tables are currently being computed
  mask_type                     synchronizing_;

  Core::Geometry::BBox                  bbox_;
  Basis                 basis_;
  double                epsilon_;
  double                epsilon2_;
  double                epsilon3_;

  /// Pointer to virtual interface
  boost::shared_ptr<VMesh>         vmesh_;

public:

  /// Always creates 4 tets, does not handle element boundaries properly.
  bool insert_node_in_cell(typename Cell::array_type &tets,
			   typename Cell::index_type ci,
			   typename Node::index_type &ni,
			   const Core::Geometry::Point &p);

  void insert_node_in_face(typename Cell::array_type &tets,
			   typename Node::index_type ni,
			   typename Cell::index_type ci,
			   const PFaceNode &pf);

  void insert_node_in_edge(typename Cell::array_type &tets,
			   typename Node::index_type ni,
			   typename Cell::index_type ci,
			   const PEdgeNode &e);

}; // end class TetVolMesh

template <class Basis>
int
TetVolMesh<Basis>::compute_checksum()
{
  int sum = 0;
  sum += SCIRun::compute_checksum(&points_[0],points_.size());
  sum += SCIRun::compute_checksum(&cells_[0],cells_.size());
  return (sum);
}

template <class Basis>
TetVolMesh<Basis>::TetVolMesh() :
  points_(0),
  cells_(0),
  faces_(0),
  face_table_(),
  edges_(0),
  edge_table_(),
  synchronize_lock_("TetVolMesh lock"),
  synchronize_cond_("TetVolMesh condition variable"),
  synchronized_(Mesh::NODES_E | Mesh::CELLS_E),
  synchronizing_(0),
  epsilon_(0.0),
  epsilon2_(0.0),
  epsilon3_(0.0)
{
  DEBUG_CONSTRUCTOR("TetVolMesh")

  vmesh_.reset(CreateVTetVolMesh(this));
}

template <class Basis>
TetVolMesh<Basis>::TetVolMesh(const TetVolMesh &copy) :
  Mesh(copy),
  points_(0),
  cells_(0),
  faces_(0),
  face_table_(),
  edges_(0),
  edge_table_(),
  synchronize_lock_("TetVolMesh lock"),
  synchronize_cond_("TetVolMesh condition variable"),
  synchronized_(Mesh::NODES_E | Mesh::CELLS_E),
  synchronizing_(0),
  epsilon_(0.0),
  epsilon2_(0.0),
  epsilon3_(0.0)
{
  DEBUG_CONSTRUCTOR("TetVolMesh")

  /// We need to lock before we can copy these as these
  /// structures are generate dynamically when they are
  /// needed.
  copy.synchronize_lock_.lock();

  points_ = copy.points_;
  cells_ = copy.cells_;

  // Epsilon does not require much space, hence copy those
  synchronized_ |= copy.synchronized_ & Mesh::BOUNDING_BOX_E;
  bbox_ = copy.bbox_;
  epsilon_ = copy.epsilon_;
  epsilon2_ = copy.epsilon2_;
  epsilon3_ = copy.epsilon3_;

  copy.synchronize_lock_.unlock();

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  vmesh_.reset(CreateVTetVolMesh(this));
}

template <class Basis>
TetVolMesh<Basis>::~TetVolMesh()
{
  DEBUG_DESTRUCTOR("TetVolMesh")
}

template <class Basis>
template <class Iter, class Functor>
void
TetVolMesh<Basis>::fill_points(Iter begin, Iter end, Functor fill_ftor)
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
TetVolMesh<Basis>::fill_cells(Iter begin, Iter end, Functor fill_ftor)
{
  synchronize_lock_.lock();
  Iter iter = begin;
  cells_.resize((end - begin) * 4); // resize to the new size
  std::vector<under_type>::iterator citer = cells_.begin();
  while (iter != end)
  {
    index_type *nodes = fill_ftor(*iter); // returns an array of length 4
    *citer = nodes[0];
    ++citer;
    *citer = nodes[1];
    ++citer;
    *citer = nodes[2];
    ++citer;
    *citer = nodes[3];
    ++citer; ++iter;
  }
  synchronize_lock_.unlock();
}

template <class Basis>
PersistentTypeID
TetVolMesh<Basis>::tetvolmesh_typeid(TetVolMesh<Basis>::type_name(-1), "Mesh",
				     TetVolMesh<Basis>::maker);

template <class Basis>
const std::string
TetVolMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("TetVolMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}

/* To generate a random point inside of a tetrahedron, we generate random
   barrycentric coordinates (independent random variables between 0 and
   1 that sum to 1) for the point. */
template <class Basis>
void
TetVolMesh<Basis>::get_random_point(Core::Geometry::Point &p,
				    typename Elem::index_type ei,
                                    FieldRNG &rng) const
{
  // get positions of the vertices
  const Core::Geometry::Point &p0 = points_[cells_[ei*4+0]];
  const Core::Geometry::Point &p1 = points_[cells_[ei*4+1]];
  const Core::Geometry::Point &p2 = points_[cells_[ei*4+2]];
  const Core::Geometry::Point &p3 = points_[cells_[ei*4+3]];

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
  p = Core::Geometry::Point(p0*a + p1*t + p2*u + p3*v);
}

template <class Basis>
Core::Geometry::BBox
TetVolMesh<Basis>::get_bounding_box() const
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
TetVolMesh<Basis>::get_canonical_transform(Core::Geometry::Transform &t) const
{
  t.load_identity();
  Core::Geometry::BBox bbox = get_bounding_box();
  t.pre_scale(bbox.diagonal());
  t.pre_translate(Core::Geometry::Vector(bbox.get_min()));
}

template <class Basis>
void
TetVolMesh<Basis>::transform(const Core::Geometry::Transform &t)
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

    synchronized_ |= BOUNDING_BOX_E;
  }

  if (node_grid_) { node_grid_->transform(t); }
  if (elem_grid_) { elem_grid_->transform(t); }

  synchronize_lock_.unlock();
}

template <class Basis>
void
TetVolMesh<Basis>::remove_face(typename Node::index_type n1,
			       typename Node::index_type n2,
			       typename Node::index_type n3,
			       typename Cell::index_type ci,
			       bool /*table_only*/)
{
  PFaceNode f(n1, n2, n3);
  typename face_nt::iterator iter = face_table_.find(f);

  if (iter == face_table_.end())
  {
    ASSERTFAIL("this face did not exist in the table");
  }
  index_type found_idx = (*iter).second;

  index_type* cells = faces_[found_idx].cells_;

  if (cells[1] == MESH_NO_NEIGHBOR)
  {
    // this face belongs to only one cell
    cells[0] = MESH_NO_NEIGHBOR;
    cells[1] = MESH_NO_NEIGHBOR;
    face_table_.erase(iter);
  }
  else
  {
    if (((faces_[found_idx].cells_[0])>>2) == ci)
    {
      cells[0] = cells[1];
      cells[1] = MESH_NO_NEIGHBOR;
    }
    else if (((faces_[found_idx].cells_[1])>>2) == ci)
    {
      cells[1] = MESH_NO_NEIGHBOR;
    }
    else
    {
      ASSERTFAIL("remove face: face does exist but is ");
    }
  }
}

template <class Basis>
void
TetVolMesh<Basis>::hash_face(typename Node::index_type n1,
                             typename Node::index_type n2,
                             typename Node::index_type n3,
                             index_type combined_index,
                             face_ht& table)
{
  PFace f(n1, n2, n3);

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
      std::cerr << "TetVolMesh - This Mesh has problems: Cells #"
           << (f.cells_[0]>>2) << ", #" << (f.cells_[1]>>2) << ", and #"
           << (combined_index>>2) << " are illegally adjacent." << std::endl;
           */
    }
    else if ((f.cells_[0]>>2) == (combined_index>>2))
    {
      //TODO: inject new logger
      /*
      std::cerr << "TetVolMesh - This Mesh has problems: Cells #"
           << (f.cells_[0]>>2) << " and #" << (combined_index>>2)
           << " are the same." << std::endl;*/
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
TetVolMesh<Basis>::compute_faces()
{
  typename Cell::iterator ci, cie;
  begin(ci); end(cie);
  typename Node::array_type arr(4);

  face_ht table;

  while (ci != cie)
  {
    get_nodes(arr, *ci);
    // 4 faces -- each is entered CCW from outside looking in

    index_type cell_index = (*ci)<<2;
    hash_face(arr[0], arr[2], arr[1], cell_index, table);
    hash_face(arr[1], arr[2], arr[3], cell_index + 1, table);
    hash_face(arr[0], arr[1], arr[3], cell_index + 2, table);
    hash_face(arr[0], arr[3], arr[2], cell_index + 3, table);
    ++ci;
  }

  faces_.resize(table.size());

  typename face_ht::iterator ht_iter = table.begin();
  typename face_ht::iterator ht_iter_end = table.end();

  boundary_faces_.resize(cells_.size() >> 2);

  index_type uidx = 0;
  while (ht_iter != ht_iter_end)
  {
    const PFace& pface = (*ht_iter).first;
    faces_[uidx].cells_[0] = pface.cells_[0];
    faces_[uidx].cells_[1] = pface.cells_[1];
    face_table_[PFaceNode(pface.nodes_[0],pface.nodes_[1],pface.nodes_[2])] = uidx;

    if (pface.cells_[1] == -1)
    {
      index_type cell = (pface.cells_[0]) >> 2;
      index_type face = (pface.cells_[0]) & 0x3;
      boundary_faces_[cell] |= 1 << face;
    }
    ++ht_iter; uidx++;
  }

  synchronize_lock_.lock();
  synchronized_ |= Mesh::FACES_E;
  synchronize_lock_.unlock();


}


template <class Basis>
void
TetVolMesh<Basis>::add_face(typename Node::index_type n1,
                            typename Node::index_type n2,
                            typename Node::index_type n3,
                            index_type combined_index)
{

  PFaceNode e(n1,n2,n3);
  typename face_nt::iterator nt_iter = face_table_.find(e);

  if (nt_iter == face_table_.end())
  {
    index_type uidx = static_cast<index_type>(faces_.size());
    face_table_[e] = uidx;
    PFaceCell c;
    faces_.push_back(c);
    faces_[uidx].cells_[0] = combined_index;
  }
  else
  {
    if (faces_[nt_iter->second].cells_[0] == MESH_NO_NEIGHBOR)
    {
      ASSERTFAIL("Face is in face_table_, but not in faces_ table");
    }
    if (faces_[nt_iter->second].cells_[1] != MESH_NO_NEIGHBOR)
    {
      ASSERTFAIL("Adding a face that is already connected twice");
    }

    faces_[nt_iter->second].cells_[1] = combined_index;
  }
}

template <class Basis>
void
TetVolMesh<Basis>::hash_edge(typename Node::index_type n1,
                             typename Node::index_type n2,
                             index_type combined_index,
                             edge_ht& table)
{
  if (n1 == n2) return;
  PEdge e(n1, n2);
  typename edge_ht::iterator iter = table.find(e);

  if (iter == table.end())
  {
    e.cells_.push_back(combined_index); // add this cell
    table[e] = 0; // insert for the first time
  }
  else
  {
    PEdge e = (*iter).first;
    e.cells_.push_back(combined_index); // add this cell
    table.erase(iter);
    table[e] = 0;
  }
}

template <class Basis>
void
TetVolMesh<Basis>::compute_edges()
{
  typename Cell::iterator ci, cie;
  begin(ci); end(cie);
  edge_ht table;

  typename Node::array_type arr;
  while (ci != cie)
  {
    get_nodes(arr, *ci);
    index_type cell_index = (*ci)<<3;
    hash_edge(arr[0], arr[1], cell_index  ,table);
    hash_edge(arr[1], arr[2], cell_index+1,table);
    hash_edge(arr[2], arr[0], cell_index+2,table);
    hash_edge(arr[3], arr[0], cell_index+3,table);
    hash_edge(arr[3], arr[1], cell_index+4,table);
    hash_edge(arr[3], arr[2], cell_index+5,table);
    ++ci;
  }

  edges_.resize(table.size());

  typename edge_ht::iterator ht_iter = table.begin();
  typename edge_ht::iterator ht_iter_end = table.end();

  index_type uidx = 0;
  while (ht_iter != ht_iter_end)
  {
    const PEdge& pedge = (*ht_iter).first;
    edges_[uidx].cells_ = pedge.cells_;
    edge_table_[PEdgeNode(pedge.nodes_[0],pedge.nodes_[1])] = uidx;
    ++ht_iter; uidx++;
  }

  synchronize_lock_.lock();
  synchronized_ |= Mesh::EDGES_E;
  synchronize_lock_.unlock();
}

template <class Basis>
void
TetVolMesh<Basis>::add_edge(typename Node::index_type n1,
                            typename Node::index_type n2, index_type combined_index)
{
  PEdgeNode e(n1,n2);
  typename edge_nt::iterator ht_iter = edge_table_.find(e);
  if (ht_iter == edge_table_.end())
  {
    index_type uidx = static_cast<index_type>(edges_.size());
    edge_table_[e] = uidx;
    PEdgeCell c;
    edges_.push_back(c);
    edges_[uidx].cells_.push_back(combined_index);
  }
  else
  {
    edges_[ht_iter->second].cells_.push_back(combined_index);
  }
}

template <class Basis>
bool
TetVolMesh<Basis>::synchronize(mask_type sync)
{
  // Conversion table
  if (sync & (Mesh::ELEM_NEIGHBORS_E|Mesh::DELEMS_E))
  { sync |= Mesh::FACES_E; sync &= ~(Mesh::ELEM_NEIGHBORS_E|Mesh::DELEMS_E); }

  if (sync & Mesh::FIND_CLOSEST_NODE_E)
  { sync |= NODE_LOCATE_E; sync &=  ~(Mesh::FIND_CLOSEST_NODE_E); }

  if (sync & Mesh::FIND_CLOSEST_ELEM_E)
  { sync |= ELEM_LOCATE_E|FACES_E; sync &=  ~(Mesh::FIND_CLOSEST_ELEM_E); }

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
TetVolMesh<Basis>::unsynchronize(mask_type /*sync*/)
{
  return (true);
}

template <class Basis>
bool
TetVolMesh<Basis>::clear_synchronization()
{
  // Undo marking the synchronization
  synchronize_lock_.lock();

  synchronized_ = Mesh::NODES_E | Mesh::ELEMS_E | Mesh::CELLS_E;

  // Free memory where possible

  faces_.clear();
  face_table_.clear();
  edges_.clear();
  edge_table_.clear();
  node_neighbors_.clear();
  boundary_faces_.clear();

  node_grid_.reset();
  elem_grid_.reset();

  synchronize_lock_.unlock();

  return (true);
}

template <class Basis>
void
TetVolMesh<Basis>::begin(typename TetVolMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on TetVolMesh first");
  itr = 0;
}

template <class Basis>
void
TetVolMesh<Basis>::end(typename TetVolMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on TetVolMesh first");
  itr = static_cast<typename Node::iterator>(points_.size());
}

template <class Basis>
void
TetVolMesh<Basis>::size(typename TetVolMesh::Node::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on TetVolMesh first");
  s = points_.size();
}

template <class Basis>
void
TetVolMesh<Basis>::begin(typename TetVolMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on TetVolMesh first");
  itr = 0;
}

template <class Basis>
void
TetVolMesh<Basis>::end(typename TetVolMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on TetVolMesh first");
  itr = static_cast<index_type>(edges_.size());
}

template <class Basis>
void
TetVolMesh<Basis>::size(typename TetVolMesh::Edge::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on TetVolMesh first");
  s = static_cast<index_type>(edges_.size());
}

template <class Basis>
void
TetVolMesh<Basis>::begin(typename TetVolMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on TetVolMesh first");
  itr = 0;
}

template <class Basis>
void
TetVolMesh<Basis>::end(typename TetVolMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on TetVolMesh first");
  itr = static_cast<index_type>(faces_.size());
}

template <class Basis>
void
TetVolMesh<Basis>::size(typename TetVolMesh::Face::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on TetVolMesh first");
  s = static_cast<index_type>(faces_.size());
}

template <class Basis>
void
TetVolMesh<Basis>::begin(typename TetVolMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & CELLS_E,
            "Must call synchronize CELLS_E on TetVolMesh first");
  itr = 0;
}

template <class Basis>
void
TetVolMesh<Basis>::end(typename TetVolMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & CELLS_E,
            "Must call synchronize CELLS_E on TetVolMesh first");
  itr = cells_.size() >> 2;
}

template <class Basis>
void
TetVolMesh<Basis>::size(typename TetVolMesh::Cell::size_type &s) const
{
  ASSERTMSG(synchronized_ & CELLS_E,
            "Must call synchronize CELLS_E on TetVolMesh first");
  s = static_cast<size_type>(cells_.size() >> 2);
}

template <class Basis>
void
TetVolMesh<Basis>::create_cell_edges(typename Cell::index_type c)
{
  typename Node::array_type arr;
  get_nodes(arr, c);
  index_type cell_index = (c<<3);
  add_edge(arr[0], arr[1], cell_index);
  add_edge(arr[1], arr[2], cell_index+1);
  add_edge(arr[2], arr[0], cell_index+2);
  add_edge(arr[3], arr[0], cell_index+3);
  add_edge(arr[3], arr[1], cell_index+4);
  add_edge(arr[3], arr[2], cell_index+5);
}

template <class Basis>
void
TetVolMesh<Basis>::remove_edge(typename Node::index_type n1,
			       typename Node::index_type n2,
			       typename Cell::index_type ci,
             bool table_only)
{
  PEdgeNode e(n1, n2);
  typename edge_nt::iterator iter = edge_table_.find(e);

  if (iter == edge_table_.end())
  {
    ASSERTFAIL("this edge did not exist in the table");
  }

  index_type found_idx = (*iter).second;

  const std::vector<index_type>& cells = edges_[found_idx].cells_;
  if (cells.size() < 2)
  {
    if ((cells[0] >>3) !=  ci)
    {
      ASSERTFAIL("this edge does exist in the table but is not connected to this cell");
    }
    edge_table_.erase(iter);
    if (!table_only) edges_[found_idx].cells_.clear();
  }
  else
  {
    typename std::vector<index_type>::iterator citer = edges_[found_idx].cells_.begin();
    typename std::vector<index_type>::iterator citer_end = edges_[found_idx].cells_.end();

    index_type cell_idx = ci;
    while(citer != citer_end)
    {
      if (((*citer)>>3)==cell_idx) {
         // temporary fix
         // should be replaced with algorithm
        edges_[found_idx].cells_.erase(citer);
        citer = edges_[found_idx].cells_.begin();
        citer_end = edges_[found_idx].cells_.end();
      }
      else {
        ++citer;
      }
    }
  }
}

template <class Basis>
void
TetVolMesh<Basis>::delete_cell_edges(typename Cell::index_type c,
                                     bool table_only)
{
  typename Node::array_type arr;
  get_nodes(arr, c);
  remove_edge(arr[0], arr[1], c, table_only);
  remove_edge(arr[1], arr[2], c, table_only);
  remove_edge(arr[2], arr[0], c, table_only);
  remove_edge(arr[3], arr[0], c, table_only);
  remove_edge(arr[3], arr[1], c, table_only);
  remove_edge(arr[3], arr[2], c, table_only);
}

template <class Basis>
void
TetVolMesh<Basis>::create_cell_faces(typename Cell::index_type c)
{
  typename Node::array_type arr;
  get_nodes(arr, c);
  index_type cell_index = c<<2;
  add_face(arr[0], arr[2], arr[1], cell_index);
  add_face(arr[1], arr[2], arr[3], cell_index+1);
  add_face(arr[0], arr[1], arr[3], cell_index+2);
  add_face(arr[0], arr[3], arr[2], cell_index+3);
}


template <class Basis>
void
TetVolMesh<Basis>::delete_cell_faces(typename Cell::index_type c,
                                     bool table_only)
{
  typename Node::array_type arr;
  get_nodes(arr, c);
  remove_face(arr[0], arr[2], arr[1], c, table_only);
  remove_face(arr[1], arr[2], arr[3], c, table_only);
  remove_face(arr[0], arr[1], arr[3], c, table_only);
  remove_face(arr[0], arr[3], arr[2], c, table_only);
}


template <class Basis>
void
TetVolMesh<Basis>::create_cell_node_neighbors(typename Cell::index_type c)
{
  for (index_type i = c*4; i < c*4+4; ++i)
  {
    node_neighbors_[cells_[i]].push_back(i);
  }
}


template <class Basis>
void
TetVolMesh<Basis>::delete_cell_node_neighbors(typename Cell::index_type c)
{
  for (index_type i = c*4; i < c*4+4; ++i)
  {
    const index_type n = cells_[i];
    typename std::vector<typename Cell::index_type>::iterator node_cells_end =
      node_neighbors_[n].end();
    typename std::vector<typename Cell::index_type>::iterator cell =
      node_neighbors_[n].begin();
    while (cell != node_cells_end && (*cell) != i) ++cell;

    /// ASSERT that the node_neighbors_ structure contains this cell
    ASSERT(cell != node_cells_end);

    node_neighbors_[n].erase(cell);
  }
}

template <class Basis>
void
TetVolMesh<Basis>::create_cell_syncinfo(typename Cell::index_type ci)
{
  synchronize_lock_.lock();
  if (synchronized_ & Mesh::NODE_NEIGHBORS_E)
    create_cell_node_neighbors(ci);
  if (synchronized_&Mesh::EDGES_E)
    create_cell_edges(ci);
  if (synchronized_&Mesh::FACES_E)
    create_cell_faces(ci);
  if (synchronized_ & Mesh::LOCATE_E)
    insert_elem_into_grid(ci);
  synchronize_lock_.unlock();
}

template <class Basis>
void
TetVolMesh<Basis>::delete_cell_syncinfo(typename Cell::index_type ci)
{
  synchronize_lock_.lock();
  if (synchronized_ & Mesh::NODE_NEIGHBORS_E)
    delete_cell_node_neighbors(ci);
  if (synchronized_&Mesh::EDGES_E)
    delete_cell_edges(ci);
  if (synchronized_&Mesh::FACES_E)
    delete_cell_faces(ci);
  if (synchronized_ & Mesh::LOCATE_E)
    remove_elem_from_grid(ci);
  synchronize_lock_.unlock();
}

template <class Basis>
void
TetVolMesh<Basis>::create_cell_syncinfo_special(typename Cell::index_type ci)
{
  synchronize_lock_.lock();
  if (synchronized_ & Mesh::NODE_NEIGHBORS_E)
    create_cell_node_neighbors(ci);

  if (synchronized_&Mesh::EDGES_E)
  {
    typename Node::array_type arr;
    get_nodes(arr, ci);
    index_type cell_index = ci<<3;
    add_edge(arr[0], arr[1], cell_index);
    add_edge(arr[1], arr[2], cell_index+1);
    add_edge(arr[2], arr[0], cell_index+2);
    add_edge(arr[3], arr[0], cell_index+3);
    add_edge(arr[3], arr[1], cell_index+4);
    add_edge(arr[3], arr[2], cell_index+5);
  }

  if (synchronized_&Mesh::FACES_E)
  {
    typename Node::array_type arr;
    get_nodes(arr, ci);
    index_type cell_index = ci<<2;
    add_face(arr[0], arr[2], arr[1], cell_index);
    add_face(arr[1], arr[2], arr[3], cell_index+1);
    add_face(arr[0], arr[1], arr[3], cell_index+2);
    add_face(arr[0], arr[3], arr[2], cell_index+3);
  }
  if (synchronized_ & Mesh::LOCATE_E)
    insert_elem_into_grid(ci);
  synchronize_lock_.unlock();
}

template <class Basis>
void
TetVolMesh<Basis>::delete_cell_syncinfo_special(typename Cell::index_type ci)
{
  synchronize_lock_.lock();
  if (synchronized_ & Mesh::NODE_NEIGHBORS_E)
    delete_cell_node_neighbors(ci);
  if (synchronized_&Mesh::EDGES_E)
    delete_cell_edges(ci, true);
  if (synchronized_&Mesh::FACES_E)
    delete_cell_faces(ci, true);
  if (synchronized_ & Mesh::LOCATE_E)
    remove_elem_from_grid(ci);
  synchronize_lock_.unlock();
}


template <class Basis>
void
TetVolMesh<Basis>::set_nodes(typename Node::array_type &array,
                             typename Cell::index_type idx)
{
  ASSERT(array.size() == 4);

  delete_cell_syncinfo(idx);

  for (index_type n = 0; n < 4; ++n)
    cells_[idx * 4 + n] = array[n];

  create_cell_syncinfo(idx);
}

template <class Basis>
void
TetVolMesh<Basis>::compute_node_neighbors()
{
  node_neighbors_.clear();
  node_neighbors_.resize(points_.size());
  index_type i, num_cells = cells_.size();
  for (i = 0; i < num_cells; i++)
  {
    node_neighbors_[cells_[i]].push_back(i);
  }

  synchronize_lock_.lock();
  synchronized_ |= Mesh::NODE_NEIGHBORS_E;
  synchronize_lock_.unlock();
}

template <class Basis>
int
TetVolMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Cell::array_type &l,
                               double *w)
{
  typename Cell::index_type idx;
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
TetVolMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Node::array_type &l,
                               double *w)
{
  typename Cell::index_type idx;
  if (locate(idx, p))
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
void
TetVolMesh<Basis>::insert_elem_into_grid(typename Elem::index_type ci)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.

  const index_type idx = ci*4;
  Core::Geometry::BBox box;
  box.extend(points_[cells_[idx]]);
  box.extend(points_[cells_[idx+1]]);
  box.extend(points_[cells_[idx+2]]);
  box.extend(points_[cells_[idx+3]]);
  box.extend(epsilon_);
  elem_grid_->insert(ci, box);
}


template <class Basis>
void
TetVolMesh<Basis>::remove_elem_from_grid(typename Elem::index_type ci)
{
  const index_type idx = ci*4;
  Core::Geometry::BBox box;
  box.extend(points_[cells_[idx]]);
  box.extend(points_[cells_[idx+1]]);
  box.extend(points_[cells_[idx+2]]);
  box.extend(points_[cells_[idx+3]]);
  box.extend(epsilon_);
  elem_grid_->remove(ci, box);
}

template <class Basis>
void
TetVolMesh<Basis>::insert_node_into_grid(typename Node::index_type ni)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.
  node_grid_->insert(ni,points_[ni]);
}

template <class Basis>
void
TetVolMesh<Basis>::remove_node_from_grid(typename Node::index_type ni)
{
  node_grid_->remove(ni,points_[ni]);
}

template <class Basis>
void
TetVolMesh<Basis>::compute_elem_grid()
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
TetVolMesh<Basis>::compute_node_grid()
{
  ASSERTMSG(bbox_.valid(),"TetVolMesh BBox not valid");
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
TetVolMesh<Basis>::compute_bounding_box()
{
  bbox_.reset();

  // Compute bounding box
  typename Node::iterator ni, nie;
  begin(ni);
  end(nie);

  // make sure it does not fail on empty meshes
  if (ni == nie)
  {
    bbox_.extend(Core::Geometry::Point(0.0,0.0,0.0));
    bbox_.extend(Core::Geometry::Point(1.0,1.0,1.0));
  }

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
  synchronized_ |= BOUNDING_BOX_E;
  synchronize_lock_.unlock();
}

template <class Basis>
typename TetVolMesh<Basis>::Node::index_type
TetVolMesh<Basis>::add_find_point(const Core::Geometry::Point &p, double /*err*/)
{
  typename Node::index_type i;
  if (locate(i, p) && (points_[i] - p).length2() < epsilon2_)
  {
    return i;
  }
  else
  {
    points_.push_back(p);
    if (synchronized_ & Mesh::NODE_NEIGHBORS_E)
    {
      synchronize_lock_.lock();
      node_neighbors_.push_back(std::vector<typename Cell::index_type>());
      synchronize_lock_.unlock();
    }
    return static_cast<typename Node::index_type>(points_.size() - 1);
  }
}

template <class Basis>
typename TetVolMesh<Basis>::Elem::index_type
TetVolMesh<Basis>::add_tet(typename Node::index_type a,
			   typename Node::index_type b,
			   typename Node::index_type c,
			   typename Node::index_type d)
{
  const index_type tet = static_cast<index_type>(cells_.size()) / 4;
  cells_.push_back(a);
  cells_.push_back(b);
  cells_.push_back(c);
  cells_.push_back(d);
  return tet;
}

template <class Basis>
typename TetVolMesh<Basis>::Node::index_type
TetVolMesh<Basis>::add_point(const Core::Geometry::Point &p)
{
  points_.push_back(p);
  return static_cast<typename Node::index_type>(points_.size() - 1);
}


template <class Basis>
typename TetVolMesh<Basis>::Elem::index_type
TetVolMesh<Basis>::add_tet(const Core::Geometry::Point &p0, const Core::Geometry::Point &p1,
			   const Core::Geometry::Point &p2, const Core::Geometry::Point &p3)
{
  return add_tet(add_find_point(p0), add_find_point(p1),
                 add_find_point(p2), add_find_point(p3));
}

template <class Basis>
typename TetVolMesh<Basis>::Elem::index_type
TetVolMesh<Basis>::add_tet_pos(typename Node::index_type a,
                               typename Node::index_type b,
                               typename Node::index_type c,
                               typename Node::index_type d)
{
  const index_type tet = static_cast<index_type>(cells_.size()) / 4;
  const Core::Geometry::Point &p0 = point(a);
  const Core::Geometry::Point &p1 = point(b);
  const Core::Geometry::Point &p2 = point(c);
  const Core::Geometry::Point &p3 = point(d);

  if (Dot(Cross(p1-p0,p2-p0),p3-p0) >= 0.0)
  {
    cells_.push_back(a);
    cells_.push_back(b);
  }
  else
  {
    cells_.push_back(b);
    cells_.push_back(a);
  }
  cells_.push_back(c);
  cells_.push_back(d);
  return tet;
}

template <class Basis>
void
TetVolMesh<Basis>::mod_tet_pos(typename TetVolMesh<Basis>::Elem::index_type ci,
                               typename Node::index_type a,
                               typename Node::index_type b,
                               typename Node::index_type c,
                               typename Node::index_type d)
{
  const Core::Geometry::Point &p0 = point(a);
  const Core::Geometry::Point &p1 = point(b);
  const Core::Geometry::Point &p2 = point(c);
  const Core::Geometry::Point &p3 = point(d);

  if (Dot(Cross(p1-p0,p2-p0),p3-p0) >= 0.0)
  {
    cells_[ci*4+0] = a;
    cells_[ci*4+1] = b;
  }
  else
  {
    cells_[ci*4+0] = b;
    cells_[ci*4+1] = a;
  }
  cells_[ci*4+2] = c;
  cells_[ci*4+3] = d;
}

template <class Basis>
void
TetVolMesh<Basis>::delete_cells(std::set<index_type> &to_delete)
{
  synchronize_lock_.lock();
  std::set<index_type>::reverse_iterator iter = to_delete.rbegin();
  while (iter != to_delete.rend())
  {
    // erase the correct cell
    typename TetVolMesh<Basis>::Cell::index_type ci = *iter++;
    index_type ind = ci * 4;
    std::vector<index_type>::iterator cb = cells_.begin() + ind;
    std::vector<index_type>::iterator ce = cb;
    ce+=4;
    cells_.erase(cb, ce);
  }

  synchronized_ &= ~Mesh::LOCATE_E;
  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  if (synchronized_ & Mesh::FACES_E)
  {
    synchronized_ &= ~Mesh::FACES_E;
    compute_faces();
  }
  if (synchronized_ & Mesh::EDGES_E)
  {
    synchronized_ &= ~Mesh::EDGES_E;
    compute_edges();
  }
  synchronize_lock_.unlock();
}

template <class Basis>
void
TetVolMesh<Basis>::delete_nodes(std::set<index_type> &to_delete)
{
  synchronize_lock_.lock();
  std::set<index_type>::reverse_iterator iter = to_delete.rbegin();
  while (iter != to_delete.rend())
  {
    typename TetVolMesh::Node::index_type n = *iter++;
    std::vector<Core::Geometry::Point>::iterator pit = points_.begin() + n;
    points_.erase(pit);
  }
  synchronized_ &= ~Mesh::LOCATE_E;
  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  if (synchronized_ & Mesh::FACES_E)
  {
    synchronized_ &= ~Mesh::FACES_E;
    compute_faces();
  }
  if (synchronized_ & Mesh::EDGES_E)
  {
    synchronized_ &= ~Mesh::EDGES_E;
    compute_edges();
  }
  synchronize_lock_.unlock();
}

template <class Basis>
bool
TetVolMesh<Basis>::insert_node_in_cell(typename Cell::array_type &tets,
                                       typename Cell::index_type ci,
                                       typename Node::index_type &pi,
                                       const Core::Geometry::Point &p)
{
  if (!inside(ci, p)) return false;

  pi = add_point(p);

  delete_cell_syncinfo_special(ci);

  tets.resize(4, ci);
  const index_type index = ci*4;
  tets[1] = add_tet_pos(cells_[index+0], cells_[index+3], cells_[index+1], pi);
  tets[2] = add_tet_pos(cells_[index+1], cells_[index+3], cells_[index+2], pi);
  tets[3] = add_tet_pos(cells_[index+0], cells_[index+2], cells_[index+3], pi);

  mod_tet_pos(ci, cells_[index+0], cells_[index+1], cells_[index+2], pi);

  create_cell_syncinfo_special(ci);
  create_cell_syncinfo_special(tets[1]);
  create_cell_syncinfo_special(tets[2]);
  create_cell_syncinfo_special(tets[3]);

  return true;
}

template <class Basis>
void
TetVolMesh<Basis>::insert_node_in_face(typename Cell::array_type &tets,
                                       typename Node::index_type pi,
                                       typename Cell::index_type ci,
                                       const PFaceNode &f)
{

  // NOTE: This function assumes that the same operation has or will happen
  // to any neighbor cell across the face.

  index_type skip;
  for (skip = 0; skip < 4; skip++)
  {
    typename Node::index_type ni = cells_[ci*4+skip];
    if (ni != f.nodes_[0] && ni != f.nodes_[1] && ni != f.nodes_[2]) break;
  }

  delete_cell_syncinfo_special(ci);

  const index_type i = ci*4;
  tets.push_back(ci);
  if (skip == 0)
  {
    tets.push_back(add_tet_pos(cells_[i+0], cells_[i+3], cells_[i+1], pi));
    tets.push_back(add_tet_pos(cells_[i+0], cells_[i+2], cells_[i+3], pi));
    mod_tet_pos(ci, cells_[i+0], cells_[i+1], cells_[i+2], pi);
  }
  else if (skip == 1)
  {
    tets.push_back(add_tet_pos(cells_[i+0], cells_[i+3], cells_[i+1], pi));
    tets.push_back(add_tet_pos(cells_[i+1], cells_[i+3], cells_[i+2], pi));
    mod_tet_pos(ci, cells_[i+0], cells_[i+1], cells_[i+2], pi);
  }
  else if (skip == 2)
  {
    tets.push_back(add_tet_pos(cells_[i+1], cells_[i+3], cells_[i+2], pi));
    tets.push_back(add_tet_pos(cells_[i+0], cells_[i+2], cells_[i+3], pi));
    mod_tet_pos(ci, cells_[i+0], cells_[i+1], cells_[i+2], pi);
  }
  else if (skip == 3)
  {
    tets.push_back(add_tet_pos(cells_[i+0], cells_[i+3], cells_[i+1], pi));
    tets.push_back(add_tet_pos(cells_[i+1], cells_[i+3], cells_[i+2], pi));
    mod_tet_pos(ci, cells_[i+0], cells_[i+2], cells_[i+3], pi);
  }

  create_cell_syncinfo_special(ci);
  create_cell_syncinfo_special(tets[tets.size()-2]);
  create_cell_syncinfo_special(tets[tets.size()-1]);

}

template <class Basis>
void
TetVolMesh<Basis>::insert_node_in_edge(typename Cell::array_type &tets,
                                       typename Node::index_type pi,
                                       typename Cell::index_type ci,
                                       const PEdgeNode &e)
{
  index_type skip1=-1, skip2=-1;

  for (index_type j = 0; j < 4; j++)
  {
    typename Node::index_type ni = cells_[ci*4+j];
    if (ni != e.nodes_[0] && ni != e.nodes_[1])
    {
      if (skip1 == -1) skip1 = j;
      else skip2 = j;
    }
  }

  delete_cell_syncinfo_special(ci);

  bool pushed = false;
  tets.push_back(ci);
  const index_type i = ci*4;
  if (skip1 != 0 && skip2 != 0)
  {
    // add 1 3 2 pi
    tets.push_back(add_tet_pos(cells_[i+1], cells_[i+3], cells_[i+2], pi));
    pushed = true;
  }
  if (skip1 != 1 && skip2 != 1)
  {
    // add 0 2 3 pi
    if (pushed)
    {
      mod_tet_pos(ci, cells_[i+0], cells_[i+2], cells_[i+3], pi);
    }
    else
    {
      tets.push_back(add_tet_pos(cells_[i+0], cells_[i+2], cells_[i+3], pi));
    }
    pushed = true;
  }
  if (skip1 != 2 && skip2 != 2)
  {
    // add 0 3 1 pi
    if (pushed)
    {
      mod_tet_pos(ci, cells_[i+0], cells_[i+3], cells_[i+1], pi);
    }
    else
    {
      tets.push_back(add_tet_pos(cells_[i+0], cells_[i+3], cells_[i+1], pi));
    }
    pushed = true;
  }
  if (skip1 != 3 && skip2 != 3)
  {
    // add 0 1 2 pi
    ASSERTMSG(pushed,
              "insert_node_in_cell_edge::skip1 or skip2 were invalid.");
    mod_tet_pos(ci, cells_[i+0], cells_[i+1], cells_[i+2], pi);
  }

  create_cell_syncinfo_special(ci);
  create_cell_syncinfo_special(tets[tets.size()-1]);
}

template <class Basis>
bool
TetVolMesh<Basis>::insert_node_in_elem(typename Elem::array_type &tets,
                                       typename Node::index_type &pi,
                                       typename Elem::index_type ci,
                                       const Core::Geometry::Point &p)
{

  const Core::Geometry::Point &p0 = points_[cells_[ci*4 + 0]];
  const Core::Geometry::Point &p1 = points_[cells_[ci*4 + 1]];
  const Core::Geometry::Point &p2 = points_[cells_[ci*4 + 2]];
  const Core::Geometry::Point &p3 = points_[cells_[ci*4 + 3]];

  // Compute all the new tet areas.
  const double aerr = std::fabs(Dot(Cross(p1 - p0, p2 - p0), p3 - p0)) * 0.01;
  const double a0 = std::fabs(Dot(Cross(p1 - p, p2 - p), p3 - p));
  const double a1 = std::fabs(Dot(Cross(p - p0, p2 - p0), p3 - p0));
  const double a2 = std::fabs(Dot(Cross(p1 - p0, p - p0), p3 - p0));
  const double a3 = std::fabs(Dot(Cross(p1 - p0, p2 - p0), p - p0));

  mask_type mask = 0;
  if (a0 >= aerr && a0 >= epsilon3_) { mask |= 1; }
  if (a1 >= aerr && a1 >= epsilon3_) { mask |= 2; }
  if (a2 >= aerr && a2 >= epsilon3_) { mask |= 4; }
  if (a3 >= aerr && a3 >= epsilon3_) { mask |= 8; }

  // If we're completely inside then we do a normal 4 tet insert.
  // Test this first because it's most common.
  if (mask == 15) { return insert_node_in_cell(tets, ci, pi, p); }

  // If the tet is degenerate then we just return any corner and are done.
  else if (mask == 0)
  {
    tets.clear();
    tets.push_back(ci);
    pi = cells_[ci*4 + 0];
    return true;
  }

  // If we're on a corner, we're done.  The corner is the point.
  else if (mask == 1)
  {
    tets.clear();
    tets.push_back(ci);
    pi = cells_[ci*4 + 0];
    return true;
  }
  else if (mask == 2)
  {
    tets.clear();
    tets.push_back(ci);
    pi = cells_[ci*4 + 1];
    return true;
  }
  else if (mask == 4)
  {
    tets.clear();
    tets.push_back(ci);
    pi = cells_[ci*4 + 2];
    return true;
  }
  else if (mask == 8)
  {
    tets.clear();
    tets.push_back(ci);
    pi = cells_[ci*4 + 3];
    return true;
  }

  // If we're on an edge, we do an edge insert.
  else if (mask == 3 || mask == 5 || mask == 6 ||
           mask == 9 || mask == 10 || mask == 12)
  {
    PEdgeNode etmp;
    if      (mask == 3)
    {
      /* 0-1 edge */
      etmp = PEdgeNode(cells_[ci*4 + 0], cells_[ci*4 + 1]);
    }
    else if (mask == 5)
    {
      /* 0-2 edge */
      etmp = PEdgeNode(cells_[ci*4 + 0], cells_[ci*4 + 2]);
    }
    else if (mask == 6)
    {
      /* 1-2 edge */
      etmp = PEdgeNode(cells_[ci*4 + 1], cells_[ci*4 + 2]);
    }
    else if (mask == 9)
    {
      /* 0-3 edge */
      etmp = PEdgeNode(cells_[ci*4 + 0], cells_[ci*4 + 3]);
    }
    else if (mask == 10)
    {
      /* 1-3 edge */
      etmp = PEdgeNode(cells_[ci*4 + 1], cells_[ci*4 + 3]);
    }
    else if (mask == 12)
    {
      /* 2-3 edge */
      etmp = PEdgeNode(cells_[ci*4 + 2], cells_[ci*4 + 3]);
    }

    typename edge_nt::iterator iter = edge_table_.find(etmp);
    PEdgeNode e = iter->first;
    const std::vector<index_type>& cells = edges_[iter->second].cells_;

    pi = add_point(p);
    tets.clear();
    for (size_t i = 0; i < cells.size(); i++)
    {
      insert_node_in_edge(tets, pi, (cells[i]>>3), e);
    }
  }

  // If we're on a face, we do a face insert.
  else if (mask == 7 || mask == 11 || mask == 13 || mask == 14)
  {
    typename Face::index_type fi;
    PFaceNode ftmp;
    if        (mask == 7)
    {
      /* on 0 1 2 face */
      ftmp = PFaceNode(cells_[ci*4 + 0], cells_[ci*4 + 1], cells_[ci*4 + 2]);
    }
    else if (mask == 11)
    {
      /* on 0 1 3 face */
      ftmp = PFaceNode(cells_[ci*4 + 0], cells_[ci*4 + 1], cells_[ci*4 + 3]);
    }
    else if (mask == 13)
    {
      /* on 0 2 3 face */
      ftmp = PFaceNode(cells_[ci*4 + 0], cells_[ci*4 + 2], cells_[ci*4 + 3]);
    }
    else if (mask == 14)
    {
      /* on 1 2 3 face */
      ftmp = PFaceNode(cells_[ci*4 + 1], cells_[ci*4 + 2], cells_[ci*4 + 3]);
    }

    typename face_nt::iterator iter = face_table_.find(ftmp);
    const PFaceNode& n = iter->first;
    const PFaceCell& f = faces_[iter->second];
    typename Cell::index_type nbr_tet =
      (ci == (f.cells_[0])>>2) ? ((f.cells_[1])>>2) : ((f.cells_[0])>>2);

    pi = add_point(p);
    tets.clear();
    insert_node_in_face(tets, pi, ci, n);
    if (nbr_tet != MESH_NO_NEIGHBOR)
    {
      insert_node_in_face(tets, pi, nbr_tet, n);
    }
  }

  return true;
}

template <class Basis>
void
TetVolMesh<Basis>::orient(typename Cell::index_type ci)
{
  const Core::Geometry::Point &p0 = point(cells_[ci*4+0]);
  const Core::Geometry::Point &p1 = point(cells_[ci*4+1]);
  const Core::Geometry::Point &p2 = point(cells_[ci*4+2]);
  const Core::Geometry::Point &p3 = point(cells_[ci*4+3]);

  // Unsigned volumex6 of the tet.
  const double sgn = Dot(Cross(p1-p0,p2-p0),p3-p0);
  if (sgn < 0.0)
  {
    typename Node::index_type tmp = cells_[ci*4+0];
    cells_[ci*4+0] = cells_[ci*4+1];
    cells_[ci*4+1] = tmp;
  }
}

#define TETVOLMESH_VERSION 4

template <class Basis>
void
TetVolMesh<Basis>::io(Piostream &stream)
{
  const int version = stream.begin_class(type_name(-1),
					 TETVOLMESH_VERSION);
  Mesh::io(stream);

  SCIRun::Pio(stream, points_);
  SCIRun::Pio_index(stream, cells_);
  if (version == 1)
  {
    std::vector<unsigned int> neighbors;
    SCIRun::Pio(stream, neighbors);
  }

  if (version >= 3)
  {
    basis_.io(stream);
  }

  stream.end_class();

  if (stream.reading())
  {
    synchronized_ = NODES_E | CELLS_E;

    vmesh_.reset(CreateVTetVolMesh(this));
  }
}

template <class Basis>
const TypeDescription* get_type_description(TetVolMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("TetVolMesh", subs,
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

template <class Basis>
const TypeDescription*
TetVolMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((TetVolMesh<Basis> *)0);
}

template <class Basis>
const TypeDescription*
TetVolMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((TetVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Node",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

template <class Basis>
const TypeDescription*
TetVolMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((TetVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Edge",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

template <class Basis>
const TypeDescription*
TetVolMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((TetVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Face",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

template <class Basis>
const TypeDescription*
TetVolMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((TetVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Cell",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

} // namespace SCIRun

#endif
