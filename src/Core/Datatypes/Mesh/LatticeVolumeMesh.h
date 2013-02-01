/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#ifndef CORE_DATATYPES_LATVOLMESH_H
#define CORE_DATATYPES_LATVOLMESH_H 

#include <boost/array.hpp>

#include <Core/Utils/Exception.h>
#include <Core/Basis/Locate.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Transform.h>
//#include <Core/Geometry/BBox.h>
//#include <Core/Math/MiscMath.h>

#include <Core/Datatypes/Mesh/FieldIterator.h>
#include <Core/Datatypes/Mesh/Mesh.h>
#include <Core/Datatypes/Mesh/VMesh.h>
//#include <Core/Datatypes/FieldRNG.h>
#include <Core/Datatypes/Mesh/MeshFactory.h> //TODO
#include <Core/Datatypes/Mesh/VirtualMeshFactory.h>

#include <Core/Datatypes/Mesh/Share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

template <class Basis>
class LatVolMesh : public Mesh
{

template <class MESH>
friend class VLatVolMesh;

public:
  typedef SCIRun::index_type                 under_type;
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::size_type                  size_type;
  typedef SCIRun::mask_type                  mask_type;
  
  typedef boost::shared_ptr<LatVolMesh<Basis> > handle_type;
  typedef Basis                             basis_type;

  struct LatIndex;
  friend struct LatIndex;

  struct LatIndex
  {
  public:
    LatIndex();
    LatIndex(const LatVolMesh *m, index_type i, index_type j, index_type k);
    operator index_type() const;
    std::ostream& str_render(std::ostream& os) const;

    // Make sure mesh_ is valid before calling these convenience accessors
    index_type ni() const;
    index_type nj() const;
    index_type nk() const;

    index_type i_, j_, k_;

    // Needs to be here so we can compute a sensible index.
    const LatVolMesh *mesh_;
  };

  struct CellIndex : public LatIndex
  {
    CellIndex();
    CellIndex(const LatVolMesh *m, index_type i, index_type j, index_type k);
    operator index_type() const;
  };

  struct NodeIndex : public LatIndex
  {
    NodeIndex();
    NodeIndex(const LatVolMesh *m, size_type i, size_type j, size_type k);
    static std::string type_name(int i = -1);
  };

  struct LatSize
  {
  public:
    LatSize();
    LatSize(size_type i, size_type j, size_type k);
    operator size_type() const;
    std::ostream& str_render(std::ostream& os) const;
    size_type i_, j_, k_;
  };

  struct CellSize : public LatSize
  {
    CellSize();
    CellSize(size_type i, size_type j, size_type k);
  };

  struct NodeSize : public LatSize
  {
    NodeSize();
    NodeSize(size_type i, size_type j, size_type k);
  };

  struct LatIter : public LatIndex
  {
    LatIter();
    LatIter(const LatVolMesh *m, index_type i, index_type j, index_type k);
    const LatIndex &operator *();
    bool operator==(const LatIter &a) const;
    bool operator!=(const LatIter &a) const;
  };

  struct NodeIter : public LatIter
  {
    NodeIter();
    NodeIter(const LatVolMesh *m, index_type i, index_type j, index_type k);
    const NodeIndex &operator *() const;
    NodeIter &operator++();
  private:
    NodeIter operator++(int);
  };

  struct CellIter : public LatIter
  {
    CellIter();
    CellIter(const LatVolMesh *m, index_type i, index_type j, index_type k);
    const CellIndex &operator *() const;
    operator index_type() const;
    CellIter &operator++();
  private:
    CellIter operator++(int);
  };

  //////////////////////////////////////////////////////////////////
  // Range Iterators
  //
  // These iterators are designed to loop over a sub-set of the mesh
  //

  struct RangeNodeIter : public NodeIter
  {
    RangeNodeIter();
    // Pre: min, and max are both valid iterators over this mesh
    //      min.A <= max.A where A is (i_, j_, k_)
    RangeNodeIter(const LatVolMesh *m, index_type i, index_type j, index_type k,
                  index_type max_i, index_type max_j, index_type max_k);
    const NodeIndex &operator *() const;
    RangeNodeIter &operator++();
    void end(NodeIter &end_iter);
  private:
    // The minimum extents
    index_type min_i_, min_j_, min_k_;
    // The maximum extents
    index_type max_i_, max_j_, max_k_;
    RangeNodeIter operator++(int);
  };

  struct RangeCellIter : public CellIter
  {
    RangeCellIter();
    // Pre: min, and max are both valid iterators over this mesh
    //      min.A <= max.A where A is (i_, j_, k_)
    RangeCellIter(const LatVolMesh *m, index_type i, index_type j, index_type k,
                  index_type max_i, index_type max_j, index_type max_k);
    const CellIndex &operator *() const;
    RangeCellIter &operator++();
    void end(CellIter &end_iter);
  private:
    // The minimum extents
    index_type min_i_, min_j_, min_k_;
    // The maximum extents
    index_type max_i_, max_j_, max_k_;
    RangeCellIter operator++(int);
  };

  //! Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef NodeIndex                  index_type;
    typedef NodeIter                   iterator;
    typedef NodeSize                   size_type;
    typedef boost::array<index_type, 8> array_type;
    typedef RangeNodeIter              range_iter;
  };

  struct Edge {
    typedef EdgeIndex<under_type>          index_type;
    typedef EdgeIterator<under_type>       iterator;
    typedef EdgeIndex<under_type>          size_type;
    typedef std::vector<index_type>        array_type;
  };

  struct Face {
    typedef FaceIndex<under_type>          index_type;
    typedef FaceIterator<under_type>       iterator;
    typedef FaceIndex<under_type>          size_type;
    typedef std::vector<index_type>        array_type;
  };

  struct Cell {
    typedef CellIndex          index_type;
    typedef CellIter           iterator;
    typedef CellSize           size_type;
    typedef std::vector<index_type> array_type;
    typedef RangeCellIter      range_iter;
  };

  typedef Cell Elem;
  typedef Face DElem;

  friend struct NodeIter;
  friend struct CellIter;
  friend struct EdgeIter;
  friend struct FaceIter;

  friend struct RangeCellIter;
  friend struct RangeNodeIter;

  friend class ElemData;

  class ElemData
  {
  public:
    typedef typename LatVolMesh<Basis>::index_type  index_type;
  
    ElemData(const LatVolMesh<Basis>& msh, const typename Cell::index_type ind);

    // the following designed to coordinate with ::get_nodes
    inline index_type node0_index() const;
    inline index_type node1_index() const;
    inline index_type node2_index() const;
    inline index_type node3_index() const;
    inline index_type node4_index() const;
    inline index_type node5_index() const;
    inline index_type node6_index() const;
    inline index_type node7_index() const;
    inline const Geometry::Point node0() const;
    inline const Geometry::Point node1() const;
    inline const Geometry::Point node2() const;
    inline const Geometry::Point node3() const;
    inline const Geometry::Point node4() const;
    inline const Geometry::Point node5() const;
    inline const Geometry::Point node6() const;
    inline const Geometry::Point node7() const;
  private:
    const LatVolMesh<Basis>          &mesh_;
    const typename Cell::index_type  index_;
  };

  LatVolMesh();
  LatVolMesh(size_type x, size_type y, size_type z, const Geometry::Point& min, const Geometry::Point& max);
  LatVolMesh(LatVolMesh* /* mh */, size_type mx, size_type my, size_type mz,
             size_type x, size_type y, size_type z);
  LatVolMesh(const LatVolMesh &copy);
  virtual LatVolMesh *clone() const;
  virtual ~LatVolMesh();
  virtual VirtualMeshHandle vmesh();
  virtual int basis_order();
  virtual bool has_normals() const { return false; }
  virtual bool has_face_normals() const { return false; }
  virtual bool is_editable() const { return false; }

  Basis &get_basis() { return basis_; }

  //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an edge.
  void pwl_approx_edge(std::vector<std::vector<double> > &coords,
                       typename Elem::index_type /*ci*/,
                       unsigned int which_edge,
                       unsigned int div_per_unit) const;

  //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an face.
  void pwl_approx_face(std::vector<std::vector<std::vector<double> > > &coords,
                       typename Elem::index_type /*ci*/,
                       unsigned int which_face,
                       unsigned int div_per_unit) const;

  //! Synchronize functions, as there is nothing to synchronize, these
  //! functions always succeed

  virtual bool synchronize(mask_type /*sync*/) { return (true); }
  virtual bool unsynchronize(mask_type /*sync*/) { return (true); }
  bool clear_synchronization() { return (true); }

  //! Get the local coordinates for a certain point within an element
  //! This function uses a couple of newton iterations to find the local
  //! coordinate of a point
  template<class VECTOR>
  bool get_coords(VECTOR &coords, const Geometry::Point &p, typename Elem::index_type idx) const;

  //! Find the location in the global coordinate system for a local coordinate
  //! This function is the opposite of get_coords.
  template<class VECTOR>
  void interpolate(Geometry::Point& pt, const VECTOR &coords, typename Elem::index_type idx) const;

  //! Interpolate the derivate of the function, This in fact will return the
  //! jacobian of the local to global coordinate transformation. This function
  //! is mainly intended for the non linear elements
  template<class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1& coords, typename Elem::index_type idx, VECTOR2& J) const;

  //! Get the determinant of the jacobian, which is the local volume of an element
  //! and is intended to help with the integration of functions over an element.
  template<class VECTOR>
  double det_jacobian(const VECTOR& coords, typename Elem::index_type idx) const;

  //! Get the jacobian of the transformation. In case one wants the non inverted
  //! version of this matrix. This is currently here for completeness of the 
  //! interface
  template<class VECTOR>
  void jacobian(const VECTOR& coords, typename Elem::index_type idx, double* J) const;
 
  //! Get the inverse jacobian of the transformation. This one is needed to 
  //! translate local gradients into global gradients. Hence it is crucial for
  //! calculating gradients of fields, or constructing finite elements.             
  template<class VECTOR>
  double inverse_jacobian(const VECTOR& /*coords*/, typename Elem::index_type /*idx*/, double* Ji) const;

  double scaled_jacobian_metric(typename Elem::index_type /*idx*/) const;

  double jacobian_metric(typename Elem::index_type /*idx*/) const;
  
  //! get the mesh statistics
  index_type get_min_i() const { return min_i_; }
  index_type get_min_j() const { return min_j_; }
  index_type get_min_k() const { return min_k_; }
  bool get_min(std::vector<index_type>&) const;
  index_type get_ni() const { return ni_; }
  index_type get_nj() const { return nj_; }
  index_type get_nk() const { return nk_; }
  const Geometry::Transform& get_transform() const { return transform_; }
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  virtual bool get_dim(std::vector<size_type>&) const;
  virtual void set_dim(const std::vector<size_type>& dims);
  Geometry::Vector diagonal() const;
  
  virtual BBox get_bounding_box() const;
  virtual void transform(const Transform &t);
  virtual void get_canonical_transform(Transform &t);
#endif

  //! set the mesh statistics
  void set_min_i(index_type i) {min_i_ = i; }
  void set_min_j(index_type j) {min_j_ = j; }
  void set_min_k(index_type k) {min_k_ = k; }
  void set_min(const std::vector<index_type>& mins);
  void set_ni(index_type i);
  void set_nj(index_type j);
  void set_nk(index_type k);

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

  void to_index(typename Node::index_type &index, index_type i) const;
  void to_index(typename Edge::index_type &index, index_type i) const
  { index = i; }
  void to_index(typename Face::index_type &index, index_type i) const
  { index = i; }
  void to_index(typename Cell::index_type &index, index_type i) const;

  //! get the child elements of the given index
  void get_nodes(typename Node::array_type &, typename Edge::index_type) const;
  void get_nodes(typename Node::array_type &, typename Face::index_type) const;
  void get_nodes(typename Node::array_type &,
                 const typename Cell::index_type &) const;
  void get_edges(typename Edge::array_type &,
                 typename Face::index_type) const;
  void get_edges(typename Edge::array_type &,
                 const typename Cell::index_type &) const;
  void get_faces(typename Face::array_type &,
                 const typename Cell::index_type &) const;

  //! get the parent element(s) of the given index
  void get_elems(typename Elem::array_type &result,
                 const typename Node::index_type &idx) const;
  void get_elems(typename Elem::array_type &result,
                 const typename Edge::index_type &idx) const;
  void get_elems(typename Elem::array_type &result,
                 const typename Face::index_type &idx) const;


  //! Wrapper to get the derivative elements from this element.
  void get_delems(typename DElem::array_type &result,
                  const typename Elem::index_type &idx) const
  {
    get_faces(result, idx);
  }

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  //! return all cell_indecies that overlap the BBox in arr.
  void get_cells(typename Cell::array_type &arr, const BBox &box);
  //! returns the min and max indices that fall within or on the BBox
  void get_cells(typename Cell::index_type &begin,
                 typename Cell::index_type &end,
                 const BBox &bbox);
  void get_nodes(typename Node::index_type &begin,
                 typename Node::index_type &end,
                 const BBox &bbox);
#endif

  bool get_neighbor(typename Cell::index_type &neighbor,
                    const typename Cell::index_type &from,
                    typename Face::index_type face) const;

  //! get the center point (in object space) of an element
  void get_center(Geometry::Point &, const typename Node::index_type &) const;
  void get_center(Geometry::Point &, typename Edge::index_type) const;
  void get_center(Geometry::Point &, typename Face::index_type) const;
  void get_center(Geometry::Point &, const typename Cell::index_type &) const;

  //! Get the size of an element (length, area, volume)
  double get_size(const typename Node::index_type &idx) const;
  double get_size(typename Edge::index_type idx) const;
  double get_size(typename Face::index_type idx) const;
  double get_size(const typename Cell::index_type &idx) const;
  double get_length(typename Edge::index_type idx) const
  { return get_size(idx); };
  double get_area(typename Face::index_type idx) const
  { return get_size(idx); };
  double get_volume(const typename Cell::index_type &i) const
  { return get_size(i); };

  bool locate(typename Node::index_type &, const Geometry::Point &) const;
  bool locate(typename Edge::index_type &, const Geometry::Point &) const
  { return false; }
  bool locate(typename Face::index_type &, const Geometry::Point &) const
  { return false; }
  bool locate(typename Elem::index_type &, const Geometry::Point &) const;

  int get_weights(const Geometry::Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Geometry::Point & , typename Edge::array_type & , double * )
    { REPORT_NOT_IMPLEMENTED("LatVolMesh::get_weights for edges isn't supported"); }
  int get_weights(const Geometry::Point & , typename Face::array_type & , double * )
    { REPORT_NOT_IMPLEMENTED("LatVolMesh::get_weights for faces isn't supported"); }
  int get_weights(const Geometry::Point &p, typename Cell::array_type &l, double *w);

  void get_point(Geometry::Point &p, const typename Node::index_type &i) const
  { get_center(p, i); }

  void get_normal(Geometry::Vector &, const typename Node::index_type &) const
  { REPORT_NOT_IMPLEMENTED("This mesh type does not have node normals."); }
  void get_normal(Geometry::Vector &, std::vector<double> &, typename Elem::index_type,
                  unsigned int)
  { REPORT_NOT_IMPLEMENTED("This mesh type does not have element normals."); }
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  void get_random_point(Geometry::Point &,
                        const typename Elem::index_type &,
                        FieldRNG &rng) const;
#endif


  //! This function will find the closest element and the location on that
  //! element that is the closest
  bool find_closest_node(double& pdist, Geometry::Point &result, 
                         typename Node::index_type &elem,
                         const Geometry::Point &p) const;

  bool find_closest_node(double& pdist, Geometry::Point &result, 
                         typename Node::index_type &elem,
                         const Geometry::Point &p, double maxdist) const;

  //! This function will find the closest element and the location on that
  //! element that is the closest
  template <class ARRAY>
  bool find_closest_elem(double& pdist, 
                         Geometry::Point &result,
                         ARRAY& coords, 
                         typename Elem::index_type &elem,
                         const Geometry::Point &p,
                         double maxdist) const;

                           
  //! This function will find the closest element and the location on that
  //! element that is the closest
  template <class ARRAY>
  bool find_closest_elem(double& pdist, 
                         Geometry::Point &result,
                         ARRAY& coords, 
                         typename Elem::index_type &elem,
                         const Geometry::Point &p) const;

  bool find_closest_elem(double& pdist,
                         Geometry::Point& result,
                         typename Elem::index_type &elem,
                         const Geometry::Point &p) const;
                         
  //! This function will return multiple elements if the closest point is
  //! located on a node or edge. All bordering elements are returned in that 
  //! case. 
  bool find_closest_elems(double& pdist, Geometry::Point &result, 
                          std::vector<typename Elem::index_type> &elem,
                          const Geometry::Point &p) const;

  double get_epsilon() const;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  //! Export this class using the old Pio system
  virtual void io(Piostream&);
  //! These IDs are created as soon as this class will be instantiated
  //! The first one is for Pio and the second for the virtual interface
  //! These are currently different as they serve different needs.  static PersistentTypeID type_idts;
#endif

  static const std::string type_name(int n = -1);
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  virtual std::string dynamic_type_name() const { return latvol_typeid.type; }

  // Unsafe due to non-constness of unproject.
  Transform &get_transform();
  Transform &set_transform(const Transform &trans);
#endif

  virtual int dimensionality() const { return 3; }
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  virtual int topology_geometry() const { return (STRUCTURED | REGULAR); }
#endif
    
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  //! Type description, used for finding names of the mesh class for
  //! dynamic compilation purposes. Some of this should be obsolete    
  virtual const TypeDescription *get_type_description() const;
  static const TypeDescription* cell_type_description();
  static const TypeDescription* face_type_description();
  static const TypeDescription* edge_type_description();
  static const TypeDescription* node_type_description();
  static const TypeDescription* elem_type_description()
  { return cell_type_description(); }

  //! This function returns a maker for Pio.
  static Persistent *maker() { return new LatVolMesh(); }
#endif
  //! This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() 
  { 
    MeshHandle ret(new LatVolMesh()); 
    return ret;
  }
  //! This function returns a handle for the virtual interface.
  static MeshHandle latvol_maker(const SCIRun::Core::Datatypes::MeshConstructionParameters& params) 
  { 
    MeshHandle ret(new LatVolMesh(params.x_, params.y_, params.z_, params.min_, params.max_)); 
    return ret;
  }

protected:
  void compute_jacobian();

  //! the min_Node::index_type ( incase this is a subLattice )
  index_type min_i_, min_j_, min_k_;
  //! the Node::index_type space extents of a LatVolMesh
  //! (min=min_Node::index_type, max=min+extents-1)
  size_type ni_, nj_, nk_;

  Geometry::Transform transform_;
  Basis basis_;  

  // The jacobian is the same for every element
  // hence store them as soon as we know the transfrom_
  // This should speed up FE computations on these regular grids.
  static const int JACOBIAN_SIZE = 9;
  double jacobian_[JACOBIAN_SIZE];
  double inverse_jacobian_[JACOBIAN_SIZE];
  double det_jacobian_;
  double scaled_jacobian_;
  double det_inverse_jacobian_;

  VirtualMeshHandle vmesh_;
private:
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  static PersistentTypeID latvol_typeid;
#endif
};

}}}

#endif 

#include <Core/Datatypes/Mesh/LatticeVolumeMeshTemplateImpl.h>


