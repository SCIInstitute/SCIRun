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


#ifndef CORE_DATATYPES_POINTCLOUDMESH_H
#define CORE_DATATYPES_POINTCLOUDMESH_H 1

/// Include what kind of support we want to have
/// Need to fix this and couple it sci-defs
#include <Core/Datatypes/Legacy/Field/MeshSupport.h>

#include <Core/Persistent/PersistentSTL.h>
#include <Core/GeometryPrimitives/SearchGridT.h>
#include <Core/Containers/StackVector.h>

#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/Point.h>

#include <Core/Thread/Mutex.h>
#include <Core/Basis/Locate.h>
#include <Core/Basis/Constant.h>

#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldIterator.h>
#include <Core/Datatypes/Legacy/Field/FieldRNG.h>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

/////////////////////////////////////////////////////
// Declarations for virtual interface

/// Functions for creating the virtual interface
/// Declare the functions that instantiate the virtual interface
template <class Basis> class PointCloudMesh;

/// make sure any other mesh other than the preinstantiate ones
/// returns no virtual interface. Altering this behavior will allow
/// for dynamically compiling the interface if needed.

template<class MESH>
VMesh* CreateVPointCloudMesh(MESH*) { return (0); }

/// Declare that these can be found in a library that is already
/// precompiled. So dynamic compilation will not instantiate them again.
#if (SCIRUN_POINTCLOUD_SUPPORT > 0)
SCISHARE VMesh* CreateVPointCloudMesh(PointCloudMesh<Core::Basis::ConstantBasis<Core::Geometry::Point> >* mesh);

#endif
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
// Declarations for PointCloudMesh class

template <class Basis>
class PointCloudMesh : public Mesh
{

/// Make sure the virtual interface has access
template<class MESH> friend class VPointCloudMesh;
template<class MESH> friend class VMeshShared;

public:
  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type            under_type;
  typedef SCIRun::index_type            index_type;
  typedef SCIRun::size_type             size_type;
  typedef SCIRun::mask_type             mask_type;

  typedef boost::shared_ptr<PointCloudMesh<Basis> > handle_type;
  typedef Basis                                 basis_type;

  /// Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef NodeIndex<under_type>       index_type;
    typedef NodeIterator<under_type>    iterator;
    typedef NodeIndex<under_type>       size_type;
    typedef StackVector<index_type, 1>  array_type;
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

  typedef Node Elem;
  typedef Node DElem;

  /// Somehow the information of how to interpolate inside an element
  /// ended up in a separate class, as they need to share information
  /// this construction was created to transfer data.
  /// Hopefully in the future this class will disappear again.
  friend class ElemData;
  class ElemData
  {
  public:
    typedef typename PointCloudMesh<Basis>::index_type  index_type;

    ElemData(const PointCloudMesh<Basis>& msh, const index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    // the following designed to coordinate with ::get_nodes
    inline index_type node0_index() const
    {
      return index_;
    }

    inline
    const Core::Geometry::Point &node0() const
    {
      return mesh_.points_[index_];
    }

  private:
    const PointCloudMesh<Basis>          &mesh_;
    const index_type                     index_;
  };


  //////////////////////////////////////////////////////////////////

  /// Construct a new mesh
  PointCloudMesh();

  /// Copy a mesh, needed for detaching the mesh from a field
  PointCloudMesh(const PointCloudMesh &copy);

  /// Clone function for detaching the mesh and automatically generating
  /// a new version if needed.
  virtual PointCloudMesh *clone() const { return new PointCloudMesh(*this); }

  /// Destructor
  virtual ~PointCloudMesh();

  /// Access point to virtual interface
  virtual VMesh* vmesh() { return (vmesh_.get()); }

  virtual MeshFacadeHandle getFacade() const
  {
    return boost::make_shared<Core::Datatypes::VirtualMeshFacade<VMesh>>(vmesh_);
  }

  /// This one should go at some point, should be reroute throught the
  /// virtual interface
  virtual int basis_order()
  {
    return (basis_.polynomial_order());
  }

  /// Topological dimension
  virtual int dimensionality() const { return 0; }

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
  inline void to_index(typename Face::index_type &index, index_type i) const
    { index = i; }
  inline void to_index(typename Cell::index_type &index, index_type i) const
    { index = i; }


  // This is actually get_nodes(typename Node::array_type &, Elem::index_type)
  // for compilation purposes.  IE It is redundant unless we are
  // templated by Elem type and we don't know that Elem is Node.
  // This is needed in ClipField, for example.
  void get_nodes(typename Node::array_type &a,
                 typename Node::index_type i) const
    { a.resize(1); a[0] = i; }
  void get_nodes(typename Node::array_type &,
                 typename Edge::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_nodes has not been implemented for edges"); }
  void get_nodes(typename Node::array_type &,
                 typename Face::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_nodes has not been implemented for faces"); }
  void get_nodes(typename Node::array_type &,
                 typename Cell::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_nodes has not been implemented for cells"); }


  void get_edges(typename Edge::array_type&,
                 typename Node::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_edges has not been implemented for nodes"); }
  void get_edges(typename Edge::array_type&,
                 typename Edge::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_edgees has not been implemented for edges"); }
  void get_edges(typename Edge::array_type&,
                 typename Face::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_edgees has not been implemented for faces"); }
  void get_edges(typename Edge::array_type&,
                 typename Cell::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_edges has not been implemented for cells"); }

  void get_faces(typename Face::array_type&,
                 typename Node::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_faces has not been implemented"); }
  void get_faces(typename Face::array_type&,
                 typename Edge::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_faces has not been implemented"); }
  void get_faces(typename Face::array_type&,
                 typename Face::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_faces has not been implemented"); }
  void get_faces(typename Face::array_type&,
                 typename Cell::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_faces has not been implemented"); }

  void get_cells(typename Cell::array_type&,
                 typename Node::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type&,
                 typename Edge::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type&,
                 typename Face::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type&,
                 typename Cell::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_cells has not been implemented"); }


  /// get the parent element(s) of the given index
  void get_elems(typename Elem::array_type &result,
                 typename Node::index_type idx) const
  {
  //    result.clear();
  //    result.push_back(idx);
    result[0] = idx;
  }
  void get_elems(typename Elem::array_type&,
                 typename Edge::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_elems has not been implemented for edges"); }
  void get_elems(typename Elem::array_type&,
                 typename Face::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_elems has not been implemented for faces"); }
  void get_elems(typename Elem::array_type&,
                 typename Cell::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_elems has not been implemented for cells"); }

  void get_delems(typename DElem::array_type&,
                  typename Node::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_delems has not been implemented for nodes"); }
  void get_delems(typename DElem::array_type&,
                  typename Edge::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_delems has not been implemented for edges"); }
  void get_delems(typename DElem::array_type&,
                  typename Face::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_delems has not been implemented for faces"); }
  void get_delems(typename DElem::array_type&,
                  typename Cell::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_delems has not been implemented for cells"); }

  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an edge.
  template<class VECTOR>
  void pwl_approx_edge(VECTOR &,
                       typename Elem::index_type,
                       unsigned int,
                       unsigned int) const
  {
    ASSERTFAIL("PointCloudMesh: cannot approximiate edges");
  }

  template<class VECTOR>
  void pwl_approx_face(VECTOR &,
                       typename Elem::index_type,
                       unsigned int,
                       unsigned int) const
  {
    ASSERTFAIL("PointCloudMesh: cannot approximiate faces");
  }


  /// get the center point (in object space) of an element
  void get_center(Core::Geometry::Point &p, typename Node::index_type i) const
    { p = points_[i]; }
  void get_center(Core::Geometry::Point &, typename Edge::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_center has not been implemented for edges"); }
  void get_center(Core::Geometry::Point &, typename Face::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_center has not been implemented for faces"); }
  void get_center(Core::Geometry::Point &, typename Cell::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_center has not been implemented for cells"); }

  /// Get the size of an elemnt (length, area, volume)
  double get_size(typename Node::index_type) const { return 0.0; }
  double get_size(typename Edge::index_type) const { return 0.0; }
  double get_size(typename Face::index_type) const { return 0.0; }
  double get_size(typename Cell::index_type) const { return 0.0; }
  double get_length(typename Edge::index_type) const { return 0.0; }
  double get_area(typename Face::index_type) const { return 0.0; }
  double get_volume(typename Cell::index_type) const { return 0.0; }

  double get_size(typename Node::array_type&) const { return 0.0; }

  /// Get neighbors of an element or a node
  /// THIS ONE IS FLAWED AS IN 3D SPACE  A NODE CAN BE CONNECTED TO
  /// MANY ELEMENTS
  bool get_neighbor(typename Elem::index_type& /*neighbor*/,
                    typename Elem::index_type /*elem*/,
                    typename DElem::index_type /*delem*/) const
    { return (false); }
  void get_neighbors(std::vector<typename Node::index_type> &array,
                     typename Node::index_type /*node*/) const
    { array.resize(0); }
  bool get_neighbors(std::vector<typename Elem::index_type> &array,
                     typename Elem::index_type /*elem*/,
                     typename DElem::index_type /*delem*/) const
    { array.resize(0); return (false); }

  /// Locate a point in a mesh, find which is the closest node
  bool locate(typename Node::index_type &n, const Core::Geometry::Point &p) const
    { return (locate_node(n,p)); }
  bool locate(typename Edge::index_type &, const Core::Geometry::Point &) const
    { return (false); }
  bool locate(typename Face::index_type &, const Core::Geometry::Point &) const
    { return (false); }
  bool locate(typename Cell::index_type &, const Core::Geometry::Point &) const
    { return (false); }

  template<class ARRAY>
  bool locate(typename Node::index_type &n, ARRAY& coords, const Core::Geometry::Point &p) const
    { coords.resize(0); return (locate_node(n,p)); }

  /// These should become obsolete soon, they do not follow the concept
  /// of the basis functions....
  int get_weights(const Core::Geometry::Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point&, typename Edge::array_type&, double*)
    { ASSERTFAIL("PointCloudField: get_weights for edges isn't supported"); }
  int get_weights(const Core::Geometry::Point&, typename Face::array_type&, double*)
    { ASSERTFAIL("PointCloudField: get_weights for faces isn't supported"); }
  int get_weights(const Core::Geometry::Point&, typename Cell::array_type&, double*)
    { ASSERTFAIL("PointCloudField: get_weights for cells isn't supported"); }


  void get_point(Core::Geometry::Point &p, typename Node::index_type i) const
    { get_center(p,i); }
  void set_point(const Core::Geometry::Point &p, typename Node::index_type i)
    { points_[i] = p; }
  void get_random_point(Core::Geometry::Point &p, const typename Elem::index_type i,
                        FieldRNG& /*rng*/) const
    { get_center(p, i); }

  void get_normal(Core::Geometry::Vector&, typename Node::index_type) const
    { ASSERTFAIL("PointCloudMesh: this mesh type does not have node normals."); }
  template<class VECTOR>
  void get_normal(Core::Geometry::Vector&, VECTOR &, typename Elem::index_type, unsigned int) const
    { ASSERTFAIL("PointCloudMesh: this mesh type does not have element normals."); }

  /// use these to build up a new PointCloudField mesh
  typename Node::index_type add_node(const Core::Geometry::Point &p) { return add_point(p); }
  typename Node::index_type add_point(const Core::Geometry::Point &p);

  template <class ARRAY>
  typename Elem::index_type add_elem(ARRAY a)
  {
    return (static_cast<typename Elem::index_type>(a[0]));
  }

  void node_reserve(size_t s) { points_.reserve(s); }
  void elem_reserve(size_t s) { points_.reserve(s); }
  void resize_nodes(size_t s) { points_.resize(s); }
  void resize_elems(size_t s) { points_.resize(s); }


  /// THESE FUNCTIONS ARE DEFINED INSIDE THE CLASS AS THESE ARE TEMPLATED
  /// FUNCTIONS INSIDE A TEMPLATED CLASS. THIS WAY OF DEFINING THE FUNCTIONS
  /// IS SUPPORTED BY MOST COMPILERS

  /// Get the local coordinates for a certain point within an element
  /// This function uses a couple of newton iterations to find the local
  /// coordinate of a point
  template<class VECTOR, class INDEX>
  bool get_coords(VECTOR &coords, const Core::Geometry::Point& /*p*/, INDEX /*idx*/) const
  {
    coords.resize(1);
    coords[0] = 0.0;
    return true;
  }

  /// Find the location in the global coordinate system for a local coordinate
  /// This function is the opposite of get_coords.
  template<class VECTOR, class INDEX>
  void interpolate(Core::Geometry::Point &pt, const VECTOR& /*coords*/, INDEX idx) const
  {
    get_center(pt, typename Node::index_type(idx));
  }

  /// Interpolate the derivate of the function, This infact will return the
  /// jacobian of the local to global coordinate transformation. This function
  /// is mainly intended for the non linear elements
  template<class VECTOR1, class INDEX, class VECTOR2>
  void derivate(const VECTOR1& /*coords*/, INDEX /*idx*/, VECTOR2 &J) const
  {
    J.resize(1);
    J[0].x(0.0);
    J[0].y(0.0);
    J[0].z(0.0);
  }


  /// Get the determinant of the jacobian, which is the local volume of an element
  /// and is intended to help with the integration of functions over an element.
  template<class VECTOR, class INDEX>
  double det_jacobian(const VECTOR& coords, INDEX idx) const
  {
    return (1.0);
  }

  /// Get the jacobian of the transformation. In case one wants the non inverted
  /// version of this matrix. This is currentl here for completeness of the
  /// interface
  template<class VECTOR, class INDEX>
  void jacobian(const VECTOR& coords, INDEX idx, double* J) const
  {
    J[0] = 1.0;
    J[1] = 0.0;
    J[2] = 0.0;
    J[3] = 0.0;
    J[4] = 1.0;
    J[5] = 0.0;
    J[6] = 0.0;
    J[7] = 0.0;
    J[8] = 1.0;
  }

  /// Get the inverse jacobian of the transformation. This one is needed to
  /// translate local gradients into global gradients. Hence it is crucial for
  /// calculating gradients of fields, or constructing finite elements.
  template<class VECTOR, class INDEX>
  double inverse_jacobian(const VECTOR& /*coords*/, INDEX /*idx*/, double* Ji) const
  {
    Ji[0] = 1.0;
    Ji[1] = 0.0;
    Ji[2] = 0.0;
    Ji[3] = 0.0;
    Ji[4] = 1.0;
    Ji[5] = 0.0;
    Ji[6] = 0.0;
    Ji[7] = 0.0;
    Ji[8] = 1.0;

    return (1.0);
  }


  template<class INDEX>
  double scaled_jacobian_metric(INDEX idx) const
  {
    return (0.0);
  }

  template<class INDEX>
  double jacobian_metric(INDEX idx) const
  {
    return (0.0);
  }

  template<class INDEX>
  double inscribed_circumscribed_radius_metric(INDEX idx) const
  {
    return (0.0);
  }

  /// Find closest node
  /// Loop over all nodes to see which is the closest
  template <class INDEX>
  bool locate_node(INDEX &node, const Core::Geometry::Point &p) const
  {
    /// If there are no nodes, we cannot find a closest one
    typename Node::size_type sz; size(sz);
    if (sz == 0) return (false);

    /// Check accuracity of first guess if we have any
    if (node >= 0 && node < sz)
    {
      /// Estimate is close enough and thence continue as we found the
      /// closest point within an epsilon uncertainty
      if ((p-points_[node]).length2() < epsilon2_) return (true);
    }

    ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
            "PointCloudMesh::locate_node requires synchronize(NODE_LOCATE_E).")

    // get grid sizes
    const size_type ni = grid_->get_ni()-1;
    const size_type nj = grid_->get_nj()-1;
    const size_type nk = grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, bj, bk, ei, ej, ek;
    grid_->unsafe_locate(bi, bj, bk, p);

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
              if (grid_->min_distance_squared(p, i, j, k) < dmin)
              {
                found = false;
                SearchGridT<index_type>::iterator it, eit;
                grid_->lookup_ijk(it, eit, i, j, k);

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

  /// Find whether we are inside the element
  /// If we find an element we return true
  template <class INDEX>
  bool locate_elem(INDEX &idx, const Core::Geometry::Point &p) const
  {
    typename Elem::size_type sz; size(sz);

    /// Check whether the estimate given in idx is the point we are looking for
    if (idx >= 0 && idx < sz)
    {
      if ((p - points_[idx]).length2() < epsilon2_) return (true);
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
        "PointCloudMesh::locate requires synchronize(ELEM_LOCATE_E).")

    typename SearchGridT<index_type>::iterator it, eit;
    if (grid_->lookup(it, eit, p))
    {
      while (it != eit)
      {
        if ((points_[*it]-p).length2() < epsilon2_)
        {
          idx = static_cast<INDEX>(*it);
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
              "PointCloudMesh::locate_elems requires synchronize(ELEM_LOCATE_E).")

    array.clear();
    index_type is,js,ks;
    index_type ie,je,ke;
    grid_->locate_clamp(is,js,ks,b.get_min());
    grid_->locate_clamp(ie,je,ke,b.get_max());
    for (index_type i=is; i<=ie;i++)
      for (index_type j=js; j<je;j++)
        for (index_type k=ks; k<ke; k++)
        {
          typename SearchGridT<index_type>::iterator it, eit;
          grid_->lookup_ijk(it, eit, i, j, k);
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



  template <class INDEX>
  bool find_closest_node(double& pdist, Core::Geometry::Point& result,
                         INDEX &node, const Core::Geometry::Point &p) const
  {
    return(find_closest_node(pdist,result,node,p,-1.0));
  }


  /// Closest node and the location
  template <class INDEX>
  bool find_closest_node(double& pdist, Core::Geometry::Point& result,
                         INDEX &node, const Core::Geometry::Point &p, double maxdist) const
  {
    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;
    typename Node::size_type sz; size(sz);

    /// If there are no nodes we cannot find the closest one
    if(sz == 0) return (false);

    if (node >= 0 && node < sz)
    {
      double dist = (p-points_[node]).length2();
      if (dist < epsilon2_)
      {
        result = p;
        pdist = sqrt(dist);
        return (true);
      }
    }

    ASSERTMSG(synchronized_ & Mesh::NODE_LOCATE_E,
        "PointCloudMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

    // get grid sizes
    const size_type ni = grid_->get_ni()-1;
    const size_type nj = grid_->get_nj()-1;
    const size_type nk = grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, bj, bk, ei, ej, ek;
    grid_->unsafe_locate(bi, bj, bk, p);

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
              if (grid_->min_distance_squared(p, i, j, k) < dmin)
              {
                found = false;
                typename SearchGridT<index_type>::iterator  it, eit;
                grid_->lookup_ijk(it, eit, i, j, k);

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
        "PointCloudMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

    // get grid sizes
    const size_type ni = grid_->get_ni()-1;
    const size_type nj = grid_->get_nj()-1;
    const size_type nk = grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, bj, bk, ei, ej, ek;

    Core::Geometry::Point max = p+Core::Geometry::Vector(maxdist,maxdist,maxdist);
    Core::Geometry::Point min = p+Core::Geometry::Vector(-maxdist,-maxdist,-maxdist);

    grid_->unsafe_locate(bi, bj, bk, min);
    grid_->unsafe_locate(ei, ej, ek, max);

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
          if (grid_->min_distance_squared(p, i, j, k) < maxdist2)
          {
            typename SearchGridT<index_type>::iterator  it, eit;
            grid_->lookup_ijk(it, eit, i, j, k);

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
        "PointCloudMesh::find_closest_node requires synchronize(NODE_LOCATE_E).")

    // get grid sizes
    const size_type ni = grid_->get_ni()-1;
    const size_type nj = grid_->get_nj()-1;
    const size_type nk = grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, bj, bk, ei, ej, ek;

    Core::Geometry::Point max = p+Core::Geometry::Vector(maxdist,maxdist,maxdist);
    Core::Geometry::Point min = p+Core::Geometry::Vector(-maxdist,-maxdist,-maxdist);

    grid_->unsafe_locate(bi, bj, bk, min);
    grid_->unsafe_locate(ei, ej, ek, max);

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
          if (grid_->min_distance_squared(p, i, j, k) < maxdist2)
          {
            typename SearchGridT<index_type>::iterator  it, eit;
            grid_->lookup_ijk(it, eit, i, j, k);

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


  template <class INDEX, class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point& result,
                         ARRAY& coords,
                         INDEX &elem,
                         const Core::Geometry::Point &p) const
  {
    return (find_closest_elem(pdist,result,coords,elem,p,-1.0));
  }


  template <class INDEX, class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point& result,
                         ARRAY& coords,
                         INDEX &elem,
                         const Core::Geometry::Point &p,
                         double maxdist) const
  {
    if (maxdist < 0.0) maxdist = DBL_MAX; else maxdist = maxdist*maxdist;

    coords.resize(0);
    typename Elem::size_type sz; size(sz);

    /// If there are no nodes we cannot find the closest one
    if(sz == 0) return (false);

    if (elem >= 0 && elem < sz)
    {
      double dist = (p-points_[elem]).length2();
      if (dist < epsilon2_)
      {
        result = p;
        pdist = sqrt(dist);
        return (true);
      }
    }

    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
        "PointCloudMesh::find_closest_elem requires synchronize(ELEM_LOCATE_E).")

    // get grid sizes
    const size_type ni = grid_->get_ni()-1;
    const size_type nj = grid_->get_nj()-1;
    const size_type nk = grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, ei, bj, ej, bk, ek;
    grid_->unsafe_locate(bi, bj, bk, p);

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
              if (grid_->min_distance_squared(p, i, j, k) < dmin)
              {
                found = false;
                typename SearchGridT<index_type>::iterator it, eit;
                grid_->lookup_ijk(it,eit, i, j, k);

                while (it != eit)
                {
                  const Core::Geometry::Point point = points_[*it];
                  const double dist  = (p-point).length2();

                  if (dist < dmin)
                  {
                    found_one = true;
                    result = point;
                    elem = INDEX(*it);
                    dmin = dist;

                    /// If we are closer than eps^2 we found a node close enough
                    if (dmin < epsilon2_)
                    {
                      pdist = sqrt(dmin);
                      return( true );
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

  template <class INDEX>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         INDEX &elem,
                         const Core::Geometry::Point &p) const
  {
    StackVector<double,1> coords;
    return(find_closest_elem(pdist,result,coords,elem,p,-1.0));
  }

  template <class ARRAY>
  bool find_closest_elems(double& pdist, Core::Geometry::Point& result,
                          ARRAY &elems, const Core::Geometry::Point &p) const
  {
    typename Elem::size_type sz; size(sz);
    if (sz == 0) return (false);

    elems.clear();

     // Walking the grid like this works really well if we're near the
    // surface.  It's degenerately bad if for example the point is
    // placed in the center of a sphere (because then we still have to
    // test all the faces, but with the grid overhead and triangle
    // duplication as well).
    ASSERTMSG(synchronized_ & Mesh::ELEM_LOCATE_E,
        "PointCloudMesh::find_closest_elems requires synchronize(ELEM_LOCATE_E).")

    // get grid sizes
    const size_type ni = grid_->get_ni()-1;
    const size_type nj = grid_->get_nj()-1;
    const size_type nk = grid_->get_nk()-1;

    // Convert to grid coordinates.
    index_type bi, ei, bj, ej, bk, ek;
    grid_->unsafe_locate(bi, bj, bk, p);

    // Clamp to closest point on the grid.
    if (bi > ni) bi = ni;
    if (bi < 0) bi = 0;
    if (bj > nj) bj = nj;
    if (bj < 0) bj = 0;
    if (bk > nk) bk = nk;
    if (bk < 0) bk = 0;

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
        if (i < 0 || i > ni) continue;
        for (index_type j = bj; j <= ej; j++)
        {
        if (j < 0 || j > nj) continue;
          for (index_type k = bk; k <= ek; k++)
          {
            if (k < 0 || k > nk) continue;
            if (i == bi || i == ei || j == bj || j == ej || k == bk || k == ek)
            {
              if (grid_->min_distance_squared(p, i, j, k) < dmin)
              {
                found = false;
                typename SearchGridT<index_type>::iterator it, eit;
                grid_->lookup_ijk(it,eit, i, j, k);

                while (it != eit)
                {
                  Core::Geometry::Point point = points_[*it];
                  const double dist = (p - point).length2();

                  if (dist < dmin - epsilon2_)
                  {
                    elems.clear();
                    result = point;
                    elems.push_back(typename ARRAY::value_type(*it));
                    found = false;
                    dmin = dist;
                  }
                  else if (dist < dmin + epsilon2_)
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

  /// Export this class using the old Pio system
  virtual void io(Piostream&);

  ///////////////////////////////////////////////////
  // STATIC VARIABLES AND FUNCTIONS

  /// These IDs are created as soon as this class will be instantiated
  static PersistentTypeID pointcloud_typeid;
  /// Core functionality for getting the name of a templated mesh class
  static const std::string type_name(int n = -1);
  virtual std::string dynamic_type_name() const { return pointcloud_typeid.type; }

  /// Type description, used for finding names of the mesh class for
  /// dynamic compilation purposes. Some of this should be obsolete
  virtual const TypeDescription *get_type_description() const;
  static const TypeDescription* node_type_description();
  static const TypeDescription* edge_type_description();
  static const TypeDescription* face_type_description();
  static const TypeDescription* cell_type_description();
  static const TypeDescription* elem_type_description()
  { return node_type_description(); }

  /// This function returns a maker for Pio.
  static Persistent *maker() { return new PointCloudMesh(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return boost::make_shared<PointCloudMesh>(); }

protected:
  template <class ARRAY, class INDEX>
  void get_nodes_from_elem(ARRAY nodes,INDEX idx)
  {
    nodes.resize(1); nodes[0] = idx;
  }

  template <class ARRAY, class INDEX>
  void get_edges_from_elem(ARRAY edges,INDEX idx)
  {
    edges.resize(0);
  }

  template <class ARRAY, class INDEX>
  inline void set_nodes_by_elem(ARRAY &array, INDEX idx)
  {
  }

protected:
  void compute_grid(Core::Geometry::BBox& bb);
  void compute_epsilon(Core::Geometry::BBox& bb);

  void insert_elem_into_grid(typename Elem::index_type ci);
  void remove_elem_from_grid(typename Elem::index_type ci);


  /// the nodes
  std::vector<Core::Geometry::Point> points_;

  /// basis fns
  Basis         basis_;

  boost::shared_ptr<SearchGridT<index_type> > grid_;

  /// Record which parts of the mesh are synchronized
  mask_type     synchronized_;
  /// Lock to synchronize between threads
  Core::Thread::Mutex         synchronize_lock_;

  double        epsilon_;
  double        epsilon2_;

  /// Virtual interface
  boost::shared_ptr<VMesh> vmesh_;

};  // end class PointCloudMesh


template<class Basis>
PointCloudMesh<Basis>::PointCloudMesh() :
  synchronized_(ALL_ELEMENTS_E),
  synchronize_lock_("PointCloudMesh Lock"),
  epsilon_(0.0),
  epsilon2_(0.0)
{
  DEBUG_CONSTRUCTOR("PointCloudMesh")
  /// Initialize the virtual interface when the mesh is created
  vmesh_.reset(CreateVPointCloudMesh(this));
}

template<class Basis>
PointCloudMesh<Basis>::PointCloudMesh(const PointCloudMesh &copy) :
  Mesh(copy),
  points_(copy.points_),
  basis_(copy.basis_),
  synchronized_(copy.synchronized_),
  synchronize_lock_("PointCloudMesh Lock")
{
  DEBUG_CONSTRUCTOR("PointCloudMesh")

  PointCloudMesh &lcopy = (PointCloudMesh &)copy;

  /// We need to lock before we can copy these as these
  /// structures are generate dynamically when they are
  /// needed.
  lcopy.synchronize_lock_.lock();

  // Copy element grid
  synchronized_ &= ~Mesh::LOCATE_E;
  if (copy.grid_)
  {
    grid_.reset(new SearchGridT<index_type>(*copy.grid_));
  }

  synchronized_ |= copy.synchronized_ & Mesh::LOCATE_E;
  synchronized_ |= copy.synchronized_ & Mesh::EPSILON_E;

  // Make sure we got the synchronized version
  epsilon_ = copy.epsilon_;
  epsilon2_ = copy.epsilon2_;

  lcopy.synchronize_lock_.unlock();

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  vmesh_.reset(CreateVPointCloudMesh(this));
}

template<class Basis>
PointCloudMesh<Basis>::~PointCloudMesh()
{
  DEBUG_DESTRUCTOR("PointCloudMesh")
}




template <class Basis>
PersistentTypeID
PointCloudMesh<Basis>::pointcloud_typeid(type_name(-1), "Mesh",
                                 PointCloudMesh<Basis>::maker);


template <class Basis>
Core::Geometry::BBox
PointCloudMesh<Basis>::get_bounding_box() const
{
  Core::Geometry::BBox result;

  typename Node::iterator i, ie;
  begin(i);
  end(ie);

  while (i != ie)
  {
    result.extend(points_[*i]);
    ++i;
  }

  // Make sure we have a bounding box
  if (points_.size() == 1)
  {
    result.extend(points_[0]);
    result.extend(1e-5);
  }

  return result;
}

template <class Basis>
void
PointCloudMesh<Basis>::get_canonical_transform(Core::Geometry::Transform &t) const
{
  t.load_identity();
  Core::Geometry::BBox bbox = get_bounding_box();
  t.pre_scale(bbox.diagonal());
  t.pre_translate(Core::Geometry::Vector(bbox.get_min()));
}

template <class Basis>
void
PointCloudMesh<Basis>::transform(const Core::Geometry::Transform &t)
{
  synchronize_lock_.lock();

  std::vector<Core::Geometry::Point>::iterator itr = points_.begin();
  std::vector<Core::Geometry::Point>::iterator eitr = points_.end();
  while (itr != eitr)
  {
    *itr = t.project(*itr);
    ++itr;
  }

  if (grid_) { grid_->transform(t); }

  synchronize_lock_.unlock();
}


template <class Basis>
int
PointCloudMesh<Basis>::get_weights(const Core::Geometry::Point &p,
                                   typename Node::array_type &l,
                                   double *w)
{
  typename Node::index_type idx;
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
typename PointCloudMesh<Basis>::Node::index_type
PointCloudMesh<Basis>::add_point(const Core::Geometry::Point &p)
{
  points_.push_back(p);
  return points_.size() - 1;
}


#define PointCloudFieldMESH_VERSION 2

template <class Basis>
void
PointCloudMesh<Basis>::io(Piostream& stream)
{
  int version = stream.begin_class(type_name(-1), PointCloudFieldMESH_VERSION);

  Mesh::io(stream);

  // IO data members, in order
  Pio(stream, points_);

  if (version >= 2) {
    basis_.io(stream);
  }

  stream.end_class();

  if (stream.reading())
    vmesh_.reset(CreateVPointCloudMesh(this));
}


template <class Basis>
const std::string
PointCloudMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("PointCloudMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
void
PointCloudMesh<Basis>::begin(typename PointCloudMesh::Node::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
PointCloudMesh<Basis>::end(typename PointCloudMesh::Node::iterator &itr) const
{
  itr = static_cast<index_type>(points_.size());
}


template <class Basis>
void
PointCloudMesh<Basis>::size(typename PointCloudMesh::Node::size_type &s) const
{
  s = static_cast<index_type>(points_.size());
}


template <class Basis>
void
PointCloudMesh<Basis>::begin(typename PointCloudMesh::Edge::iterator&) const
{
  ASSERTFAIL("This mesh type does not have edges use \"elem\".");
}


template <class Basis>
void
PointCloudMesh<Basis>::end(typename PointCloudMesh::Edge::iterator&) const
{
  ASSERTFAIL("This mesh type does not have edges use \"elem\".");
}


template <class Basis>
void
PointCloudMesh<Basis>::size(typename PointCloudMesh::Edge::size_type&) const
{
  ASSERTFAIL("This mesh type does not have edges use \"elem\".");
}


template <class Basis>
void
PointCloudMesh<Basis>::begin(typename PointCloudMesh::Face::iterator&) const
{
  ASSERTFAIL("This mesh type does not have faces use \"elem\".");
}


template <class Basis>
void
PointCloudMesh<Basis>::end(typename PointCloudMesh::Face::iterator&) const
{
  ASSERTFAIL("This mesh type does not have faces use \"elem\".");
}


template <class Basis>
void
PointCloudMesh<Basis>::size(typename PointCloudMesh::Face::size_type&) const
{
  ASSERTFAIL("This mesh type does not have faces use \"elem\".");
}


template <class Basis>
void
PointCloudMesh<Basis>::begin(typename PointCloudMesh::Cell::iterator&) const
{
  ASSERTFAIL("This mesh type does not have cells use \"elem\".");
}


template <class Basis>
void
PointCloudMesh<Basis>::end(typename PointCloudMesh::Cell::iterator&) const
{
  ASSERTFAIL("This mesh type does not have cells use \"elem\".");
}


template <class Basis>
void
PointCloudMesh<Basis>::size(typename PointCloudMesh::Cell::size_type&) const
{
  ASSERTFAIL("This mesh type does not have cells use \"elem\".");
}


template<class Basis>
bool
PointCloudMesh<Basis>::synchronize(mask_type sync)
{
  synchronize_lock_.lock();
  if (sync & (Mesh::LOCATE_E|Mesh::EPSILON_E|Mesh::FIND_CLOSEST_E) &&
      ( !(synchronized_ & Mesh::LOCATE_E) ||
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
    if (sync & (Mesh::LOCATE_E|Mesh::FIND_CLOSEST_E) &&
        !(synchronized_ & Mesh::LOCATE_E))
    {
      compute_grid(bb);
    }
  }

  synchronize_lock_.unlock();

  return(true);
}

template<class Basis>
bool
PointCloudMesh<Basis>::unsynchronize(mask_type sync)
{
  synchronize_lock_.lock();
    if (sync & Mesh::EPSILON_E) synchronized_ &= ~(Mesh::EPSILON_E);
    // No object to synchronize, hence always will succeed
  synchronize_lock_.unlock();
  return(true);
}


template <class Basis>
bool
PointCloudMesh<Basis>::clear_synchronization()
{
  synchronize_lock_.lock();
  // Undo marking the synchronization
  synchronized_ = Mesh::NODES_E | Mesh::ELEMS_E;

  // Free memory where possible
  grid_.reset();

  synchronize_lock_.unlock();
  return (true);
}

template <class Basis>
void
PointCloudMesh<Basis>::insert_elem_into_grid(typename Elem::index_type ni)
{
  /// @todo:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.
  grid_->insert(ni,points_[ni]);
}


template <class Basis>
void
PointCloudMesh<Basis>::remove_elem_from_grid(typename Elem::index_type ni)
{
  grid_->remove(ni,points_[ni]);
}

template <class Basis>
void
PointCloudMesh<Basis>::compute_grid(Core::Geometry::BBox& bb)
{
  if (bb.valid())
  {
    // Cubed root of number of cells to get a subdivision ballpark.

    typename Elem::size_type esz;  size(esz);

    const size_type s =
      3*static_cast<size_type>((ceil(pow(static_cast<double>(esz) , (1.0/3.0))))/2.0 + 1.0);

    Core::Geometry::Vector diag  = bb.diagonal();
    double trace = (diag.x()+diag.y()+diag.z());
    size_type sx = static_cast<size_type>(ceil(0.5+diag.x()/trace*s));
    size_type sy = static_cast<size_type>(ceil(0.5+diag.y()/trace*s));
    size_type sz = static_cast<size_type>(ceil(0.5+diag.z()/trace*s));

    Core::Geometry::BBox b = bb; b.extend(10*epsilon_);
    grid_.reset(new SearchGridT<index_type>(sx, sy, sz, b.get_min(), b.get_max()));

    typename Elem::iterator ci, cie;
    begin(ci); end(cie);
    while(ci != cie)
    {
      insert_elem_into_grid(*ci);
      ++ci;
    }
  }
  else
  {
    grid_.reset(new SearchGridT<index_type>(1,1,1,Core::Geometry::Point(0,0,0.0,0.0),Core::Geometry::Point(1.0,1.0,1.0)));
  }

  synchronized_ |= Mesh::LOCATE_E;
}


template<class Basis>
void
PointCloudMesh<Basis>::compute_epsilon(Core::Geometry::BBox& bb)
{
  bb = get_bounding_box();
  epsilon_ = bb.diagonal().length()*1e-8;
  epsilon2_ = epsilon_*epsilon_;
}


template <class Basis>
const TypeDescription*
get_type_description(PointCloudMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("PointCloudMesh", subs,
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
PointCloudMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((PointCloudMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
PointCloudMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PointCloudMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Node",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
PointCloudMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PointCloudMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Edge",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
PointCloudMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PointCloudMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Face",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
PointCloudMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PointCloudMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Cell",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

} // namespace SCIRun

#endif
