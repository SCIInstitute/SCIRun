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


#ifndef CORE_DATATYPES_LATVOLMESH_H
#define CORE_DATATYPES_LATVOLMESH_H 1

/// Include what kind of support we want to have
/// Need to fix this and couple it to sci-defs
#include <Core/Datatypes/Legacy/Field/MeshSupport.h>

#include <Core/Containers/StackVector.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/HexTrilinearLgn.h>

#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/Math/MiscMath.h>

#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Datatypes/Legacy/Field/FieldIterator.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldRNG.h>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

/////////////////////////////////////////////////////
// Declarations for virtual interface


/// Functions for creating the virtual interface
/// Declare the functions that instantiate the virtual interface
template <class Basis>
class LatVolMesh;

/// make sure any other mesh other than the preinstantiate ones
/// returns no virtual interface. Altering this behavior will allow
/// for dynamically compiling the interface if needed.
template<class MESH>
VMesh* CreateVLatVolMesh(MESH*) { return (0); }

/// These declarations are needed for a combined dynamic compilation as
/// as well as virtual functions solution.
/// Declare that these can be found in a library that is already
/// precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_LATVOL_SUPPORT > 0)

SCISHARE VMesh* CreateVLatVolMesh(LatVolMesh<Core::Basis::HexTrilinearLgn<Core::Geometry::Point> >* mesh);

#endif
/////////////////////////////////////////////////////


template <class Basis>
class LatVolMesh : public Mesh
{

template <class MESH>
friend class VLatVolMesh;

public:
  // Types that change depending on 32 or 64bits
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
    LatIndex() : i_(0), j_(0), k_(0), mesh_(0) {}
    LatIndex(const LatVolMesh *m, index_type i, index_type j, index_type k) :
      i_(i), j_(j), k_(k), mesh_(m) {}

    operator index_type() const {
      ASSERT(mesh_);
      return i_ + ni()*j_ + ni()*nj()*k_;
    }

    std::ostream& str_render(std::ostream& os) const
    {
      os << "[" << i_ << "," << j_ << "," << k_ << "]";
      return os;
    }

    // Make sure mesh_ is valid before calling these convience accessors
    index_type ni() const { ASSERT(mesh_); return mesh_->get_ni(); }
    index_type nj() const { ASSERT(mesh_); return mesh_->get_nj(); }
    index_type nk() const { ASSERT(mesh_); return mesh_->get_nk(); }

    index_type i_, j_, k_;

    // Needs to be here so we can compute a sensible index.
    const LatVolMesh *mesh_;
  };

  struct CellIndex : public LatIndex
  {
    CellIndex() : LatIndex() {}
    CellIndex(const LatVolMesh *m, index_type i, index_type j, index_type k)
      : LatIndex(m, i,j,k) {}

    operator index_type() const {
      ASSERT(this->mesh_);
      return (this->i_ + (this->ni()-1)*this->j_ +
              (this->ni()-1)*(this->nj()-1)*this->k_);
    }
  };

  struct NodeIndex : public LatIndex
  {
    NodeIndex() : LatIndex() {}
    NodeIndex(const LatVolMesh *m, size_type i, size_type j, size_type k)
      : LatIndex(m, i,j,k) {}
    static std::string type_name(int i=-1) {
      ASSERT(i < 1);
      return LatVolMesh<Basis>::type_name(-1) + "::NodeIndex";
    }
  };


  struct LatSize
  {
  public:
    LatSize() : i_(0), j_(0), k_(0) {}
    LatSize(size_type i, size_type j, size_type k) : i_(i), j_(j), k_(k) {}

    operator size_type() const { return i_*j_*k_; }

    std::ostream& str_render(std::ostream& os) const {
      os << i_*j_*k_ << " (" << i_ << " x " << j_ << " x " << k_ << ")";
      return os;
    }

    size_type i_, j_, k_;
  };

  struct CellSize : public LatSize
  {
    CellSize() : LatSize() {}
    CellSize(size_type i, size_type j, size_type k) : LatSize(i,j,k) {}
  };

  struct NodeSize : public LatSize
  {
    NodeSize() : LatSize() {}
    NodeSize(size_type i, size_type j, size_type k) : LatSize(i,j,k) {}
  };


  struct LatIter : public LatIndex
  {
    LatIter() : LatIndex() {}
    LatIter(const LatVolMesh *m, index_type i, index_type j, index_type k)
      : LatIndex(m, i, j, k) {}

    const LatIndex &operator *() { return *this; }

    bool operator ==(const LatIter &a) const
    {
      return (this->i_ == a.i_ && this->j_ == a.j_ &&
              this->k_ == a.k_ && this->mesh_ == a.mesh_);
    }

    bool operator !=(const LatIter &a) const
    {
      return !(*this == a);
    }
  };


  struct NodeIter : public LatIter
  {
    NodeIter() : LatIter() {}
    NodeIter(const LatVolMesh *m, index_type i, index_type j, index_type k)
      : LatIter(m, i, j, k) {}

    const NodeIndex &operator *() const { return (const NodeIndex&)(*this); }

    NodeIter &operator++()
    {
      this->i_++;
      if (this->i_ >= this->mesh_->min_i_+this->mesh_->get_ni())        {
        this->i_ = this->mesh_->min_i_;
        this->j_++;
        if (this->j_ >=  this->mesh_->min_j_+this->mesh_->get_nj()) {
          this->j_ = this->mesh_->min_j_;
          this->k_++;
        }
      }
      return *this;
    }

  private:

    NodeIter operator++(int)
    {
      NodeIter result(*this);
      operator++();
      return result;
    }
  };


  struct CellIter : public LatIter
  {
    CellIter() : LatIter() {}
    CellIter(const LatVolMesh *m, index_type i, index_type j, index_type k)
      : LatIter(m, i, j, k) {}

    const CellIndex &operator *() const { return (const CellIndex&)(*this); }

    operator index_type() const {
      ASSERT(this->mesh_);
      return this->i_ + (this->ni()-1)*this->j_ + (this->ni()-1)*(this->nj()-1)*this->k_;;
    }

    CellIter &operator++()
    {
      this->i_++;
      if (this->i_ >= this->mesh_->min_i_+this->ni()-1) {
        this->i_ = this->mesh_->min_i_;
        this->j_++;
        if (this->j_ >= this->mesh_->min_j_+this->nj()-1) {
          this->j_ = this->mesh_->min_j_;
          this->k_++;
        }
      }
      return *this;
    }

  private:

    CellIter operator++(int)
    {
      CellIter result(*this);
      operator++();
      return result;
    }
  };

  //////////////////////////////////////////////////////////////////
  // Range Iterators
  //
  // These iterators are designed to loop over a sub-set of the mesh
  //

  struct RangeNodeIter : public NodeIter
  {
    RangeNodeIter() : NodeIter() {}
    // Pre: min, and max are both valid iterators over this mesh
    //      min.A <= max.A where A is (i_, j_, k_)
    RangeNodeIter(const LatVolMesh *m, index_type i, index_type j, index_type k,
                  index_type max_i, index_type max_j, index_type max_k)
      : NodeIter(m, i, j, k), min_i_(i), min_j_(j), min_k_(k),
        max_i_(max_i), max_j_(max_j), max_k_(max_k)
    {}

    const NodeIndex &operator *() const { return (const NodeIndex&)(*this); }

    RangeNodeIter &operator++()
    {
      this->i_++;
      // Did i_ loop over the line
      // mesh_->min_x is the starting point of the x range for the mesh
      // min_i_ is the starting point of the range on x
      // max_i_ is the ending point of the range on x
      if (this->i_ >= this->mesh_->min_i_ + max_i_) {
        // set i_ to the beginning of the range
        this->i_ = min_i_;
        this->j_++;
        // Did j_ loop over the face
        // mesh_->min_j_ is the starting point of the y range for the mesh
        // min_j is the starting point of the range on y
        // max_j is the ending point of the range on y
        if (this->j_ >= this->mesh_->min_j_ + max_j_) {
          this->j_ = min_j_;
          this->k_++;
        }
      }
      return *this;
    }

    void end(NodeIter &end_iter)
    {
      // This tests is designed for a slice in the xy plane.  If z (or k)
      // is equal then you have this condition.  When this happens you
      // need to increment k so that you will iterate over the xy values.
      if (min_k_ != max_k_)
        end_iter = NodeIter(this->mesh_, min_i_, min_j_, max_k_);
      else {
        // We need to check to see if the min and max extents are the same.
        // If they are then set the end iterator such that it will be equal
        // to the beginning.  When they are the same anj for() loop using
        // these iterators [for(;iter != end_iter; iter++)] will never enter.
        if (min_i_ != max_i_ || min_j_ != max_j_)
          end_iter = NodeIter(this->mesh_, min_i_, min_j_, max_k_ + 1);
        else
          end_iter = NodeIter(this->mesh_, min_i_, min_j_, max_k_);
      }
    }

  private:
    // The minimum extents
    index_type min_i_, min_j_, min_k_;
    // The maximum extents
    index_type max_i_, max_j_, max_k_;

    RangeNodeIter operator++(int)
    {
      RangeNodeIter result(*this);
      operator++();
      return result;
    }
  };

  struct RangeCellIter : public CellIter
  {
    RangeCellIter() : CellIter() {}
    // Pre: min, and max are both valid iterators over this mesh
    //      min.A <= max.A where A is (i_, j_, k_)
    RangeCellIter(const LatVolMesh *m, index_type i, index_type j, index_type k,
                  index_type max_i, index_type max_j, index_type max_k)
      : CellIter(m, i, j, k), min_i_(i), min_j_(j), min_k_(k),
        max_i_(max_i), max_j_(max_j), max_k_(max_k)
    {}

    const CellIndex &operator *() const { return (const CellIndex&)(*this); }

    RangeCellIter &operator++()
    {
      this->i_++;
      // Did i_ loop over the line
      // mesh_->min_x is the starting point of the x range for the mesh
      // min_i_ is the starting point of the range on x
      // max_i_ is the ending point of the range on x
      if (this->i_ >= this->mesh_->min_i_ + max_i_) {
        // set i_ to the beginning of the range
        this->i_ = min_i_;
        this->j_++;
        // Did j_ loop over the face
        // mesh_->min_j_ is the starting point of the y range for the mesh
        // min_j is the starting point of the range on y
        // max_j is the ending point of the range on y
        if (this->j_ >= this->mesh_->min_j_ + max_j_) {
          this->j_ = min_j_;
          this->k_++;
        }
      }
      return *this;
    }

    void end(CellIter &end_iter) {
      // This tests is designed for a slice in the xy plane.  If z (or k)
      // is equal then you have this condition.  When this happens you
      // need to increment k so that you will iterate over the xy values.
      if (min_k_ != max_k_)
        end_iter = CellIter(this->mesh_, min_i_, min_j_, max_k_);
      else {
        // We need to check to see if the min and max extents are the same.
        // If they are then set the end iterator such that it will be equal
        // to the beginning.  When they are the same anj for() loop using
        // these iterators [for(;iter != end_iter; iter++)] will never enter.
        if (min_i_ != max_i_ || min_j_ != max_j_)
          end_iter = CellIter(this->mesh_, min_i_, min_j_, max_k_ + 1);
        else
          end_iter = CellIter(this->mesh_, min_i_, min_j_, max_k_);
      }
    }

  private:
    // The minimum extents
    index_type min_i_, min_j_, min_k_;
    // The maximum extents
    index_type max_i_, max_j_, max_k_;

    RangeCellIter operator++(int)
    {
      RangeCellIter result(*this);
      operator++();
      return result;
    }
  };


  /// Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef NodeIndex                  index_type;
    typedef NodeIter                   iterator;
    typedef NodeSize                   size_type;
    typedef StackVector<index_type, 8> array_type;
    typedef RangeNodeIter              range_iter;
  };

  struct Edge {
    typedef EdgeIndex<under_type>          index_type;
    typedef EdgeIterator<under_type>       iterator;
    typedef EdgeIndex<under_type>          size_type;
    typedef std::vector<index_type>               array_type;
  };

  struct Face {
    typedef FaceIndex<under_type>          index_type;
    typedef FaceIterator<under_type>       iterator;
    typedef FaceIndex<under_type>          size_type;
    typedef std::vector<index_type>               array_type;
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

    ElemData(const LatVolMesh<Basis>& msh,
                const typename Cell::index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    // the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const {
      return (index_.i_ + mesh_.get_ni()*index_.j_ +
              mesh_.get_ni()*mesh_.get_nj()*index_.k_);
    }
    inline
    index_type node1_index() const {
      return (index_.i_+ 1 + mesh_.get_ni()*index_.j_ +
              mesh_.get_ni()*mesh_.get_nj()*index_.k_);
    }
    inline
    index_type node2_index() const {
      return (index_.i_ + 1 + mesh_.get_ni()*(index_.j_ + 1) +
              mesh_.get_ni()*mesh_.get_nj()*index_.k_);

    }
    inline
    index_type node3_index() const {
      return (index_.i_ + mesh_.get_ni()*(index_.j_ + 1) +
              mesh_.get_ni()*mesh_.get_nj()*index_.k_);
    }
    inline
    index_type node4_index() const {
      return (index_.i_ + mesh_.get_ni()*index_.j_ +
              mesh_.get_ni()*mesh_.get_nj()*(index_.k_ + 1));
    }
    inline
    index_type node5_index() const {
      return (index_.i_ + 1 + mesh_.get_ni()*index_.j_ +
              mesh_.get_ni()*mesh_.get_nj()*(index_.k_ + 1));
    }

    inline
    index_type node6_index() const {
      return (index_.i_ + 1 + mesh_.get_ni()*(index_.j_ + 1) +
              mesh_.get_ni()*mesh_.get_nj()*(index_.k_ + 1));
    }
    inline
    index_type node7_index() const {
      return (index_.i_ + mesh_.get_ni()*(index_.j_ + 1) +
              mesh_.get_ni()*mesh_.get_nj()*(index_.k_ + 1));
    }

    inline
    const Core::Geometry::Point node0() const {
      Core::Geometry::Point p(index_.i_, index_.j_, index_.k_);
      return mesh_.transform_.project(p);
    }
    inline
    const Core::Geometry::Point node1() const {
      Core::Geometry::Point p(index_.i_ + 1, index_.j_, index_.k_);
      return mesh_.transform_.project(p);
    }
    inline
    const Core::Geometry::Point node2() const {
      Core::Geometry::Point p(index_.i_ + 1, index_.j_ + 1, index_.k_);
      return mesh_.transform_.project(p);
    }
    inline
    const Core::Geometry::Point node3() const {
      Core::Geometry::Point p(index_.i_, index_.j_ + 1, index_.k_);
      return mesh_.transform_.project(p);
    }
    inline
    const Core::Geometry::Point node4() const {
      Core::Geometry::Point p(index_.i_, index_.j_, index_.k_+ 1);
      return mesh_.transform_.project(p);
    }
    inline
    const Core::Geometry::Point node5() const {
      Core::Geometry::Point p(index_.i_ + 1, index_.j_, index_.k_+ 1);
      return mesh_.transform_.project(p);
    }
    inline
    const Core::Geometry::Point node6() const {
      Core::Geometry::Point p(index_.i_ + 1, index_.j_ + 1, index_.k_+ 1);
      return mesh_.transform_.project(p);
    }
    inline
    const Core::Geometry::Point node7() const {
      Core::Geometry::Point p(index_.i_, index_.j_ + 1, index_.k_+ 1);
      return mesh_.transform_.project(p);
    }

  private:
    const LatVolMesh<Basis>          &mesh_;
    const typename Cell::index_type  index_;
  };


  LatVolMesh() :
    min_i_(0),
    min_j_(0),
    min_k_(0),
    ni_(1),
    nj_(1),
    nk_(1)
  {
    DEBUG_CONSTRUCTOR("LatVolMesh")
    compute_jacobian();

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    vmesh_.reset(CreateVLatVolMesh(this));
  }

  LatVolMesh(size_type x, size_type y, size_type z,
             const Core::Geometry::Point &min, const Core::Geometry::Point &max);

  LatVolMesh(LatVolMesh* /* mh */,
             size_type mx, size_type my, size_type mz,
             size_type x, size_type y, size_type z) :
    min_i_(mx),
    min_j_(my),
    min_k_(mz),
    ni_(x),
    nj_(y),
    nk_(z)
  {
    DEBUG_CONSTRUCTOR("LatVolMesh")
    compute_jacobian();

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    vmesh_.reset(CreateVLatVolMesh(this));
  }

  LatVolMesh(const LatVolMesh &copy) :
    Mesh(copy),
    min_i_(copy.min_i_),
    min_j_(copy.min_j_),
    min_k_(copy.min_k_),
    ni_(copy.get_ni()),
    nj_(copy.get_nj()),
    nk_(copy.get_nk()),
    transform_(copy.transform_),
    basis_(copy.basis_)
  {
    DEBUG_CONSTRUCTOR("LatVolMesh")
    transform_.compute_imat();
    compute_jacobian();

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    vmesh_.reset(CreateVLatVolMesh(this));
  }

  virtual LatVolMesh *clone() const { return new LatVolMesh(*this); }
  virtual ~LatVolMesh()
  {
    DEBUG_DESTRUCTOR("LatVolMesh")
  }

  /// Access point to virtual interface
  virtual VMesh* vmesh() {
       return (vmesh_.get());
  }

  virtual MeshFacadeHandle getFacade() const { return boost::make_shared<Core::Datatypes::VirtualMeshFacade<VMesh>>(vmesh_); }

  virtual int basis_order() { return (basis_.polynomial_order()); }

  virtual bool has_normals() const { return false; }
  virtual bool has_face_normals() const { return false; }
  virtual bool is_editable() const { return false; }

  Basis &get_basis() { return basis_; }

  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an edge.
  void pwl_approx_edge(std::vector<std::vector<double> > &coords,
                       typename Elem::index_type /*ci*/,
                       unsigned int which_edge,
                       unsigned int div_per_unit) const
  {
    // Needs to match unit_edges in Basis/HexTrilinearLgn.cc
    // compare get_nodes order to the basis order
    int emap[] = {0, 2, 8, 10, 3, 1, 11, 9, 4, 5, 7, 6};
    basis_.approx_edge(emap[which_edge], div_per_unit, coords);
  }

  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an face.
  void pwl_approx_face(std::vector<std::vector<std::vector<double> > > &coords,
                       typename Elem::index_type /*ci*/,
                       unsigned int which_face,
                       unsigned int div_per_unit) const
  {
    // Needs to match unit_edges in Basis/HexTrilinearLgn.cc
    // compare get_nodes order to the basis order
    int fmap[] = {0, 5, 4, 2, 1, 3};
    basis_.approx_face(fmap[which_face], div_per_unit, coords);
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
    coords.resize(3);
    coords[0] = static_cast<typename VECTOR::value_type>(r.x()-static_cast<double>(idx.i_));
    coords[1] = static_cast<typename VECTOR::value_type>(r.y()-static_cast<double>(idx.j_));
    coords[2] = static_cast<typename VECTOR::value_type>(r.z()-static_cast<double>(idx.k_));

    const double epsilon = 1e-8;

    if (static_cast<double>(coords[0]) < 0.0)
    {
      if (static_cast<double>(coords[0]) > -(epsilon))
        coords[0] = static_cast<typename VECTOR::value_type>(0.0);
      else
        return (false);
    }
    if (static_cast<double>(coords[0]) > 1.0)
    {
      if (static_cast<double>(coords[0]) < 1.0+(epsilon))
        coords[0] = static_cast<typename VECTOR::value_type>(1.0);
      else
        return (false);
    }
    if (static_cast<double>(coords[1]) < 0.0)
    {
      if (static_cast<double>(coords[1]) > -(epsilon))
        coords[1] = static_cast<typename VECTOR::value_type>(0.0);
      else
        return (false);
    }
    if (static_cast<double>(coords[1]) > 1.0)
    {
      if (static_cast<double>(coords[1]) < 1.0+(epsilon))
        coords[1] = static_cast<typename VECTOR::value_type>(1.0);
      else
        return (false);
    }
    if (static_cast<double>(coords[2]) < 0.0)
    {
      if (static_cast<double>(coords[2]) > -(epsilon))
        coords[2] = static_cast<typename VECTOR::value_type>(0.0);
      else
        return (false);
    }
    if (static_cast<double>(coords[2]) > 1.0)
    {
      if (static_cast<double>(coords[2]) < 1.0+(epsilon))
        coords[2] = static_cast<typename VECTOR::value_type>(1.0);
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
    // Cheaper implementation

    Core::Geometry::Point p(static_cast<double>(idx.i_)+static_cast<double>(coords[0]),
            static_cast<double>(idx.j_)+static_cast<double>(coords[1]),
            static_cast<double>(idx.k_)+static_cast<double>(coords[2]));
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
    J.resize(3);
    J[0] = (transform_.project(Core::Geometry::Vector(1.0,0.0,0.0))).asPoint();
    J[1] = (transform_.project(Core::Geometry::Vector(0.0,1.0,0.0))).asPoint();
    J[2] = (transform_.project(Core::Geometry::Vector(0.0,0.0,1.0))).asPoint();

   }

  /// Get the determinant of the jacobian, which is the local volume of an element
  /// and is intended to help with the integration of functions over an element.
  template<class VECTOR>
  double det_jacobian(const VECTOR& coords, typename Elem::index_type idx) const
  {
    return (det_jacobian_);
  }

  /// Get the jacobian of the transformation. In case one wants the non inverted
  /// version of this matrix. This is currentl here for completeness of the
  /// interface
  template<class VECTOR>
  void jacobian(const VECTOR& coords, typename Elem::index_type idx, double* J) const
  {
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
  double inverse_jacobian(const VECTOR& /*coords*/, typename Elem::index_type /*idx*/, double* Ji) const
  {
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

  /// get the mesh statistics
  index_type get_min_i() const { return min_i_; }
  index_type get_min_j() const { return min_j_; }
  index_type get_min_k() const { return min_k_; }
  bool get_min(std::vector<index_type>&) const;
  index_type get_ni() const { return ni_; }
  index_type get_nj() const { return nj_; }
  index_type get_nk() const { return nk_; }
  virtual bool get_dim(std::vector<size_type>&) const;
  Core::Geometry::Vector diagonal() const;

  virtual Core::Geometry::BBox get_bounding_box() const;
  virtual void transform(const Core::Geometry::Transform &t);
  virtual void get_canonical_transform(Core::Geometry::Transform &t);

  /// set the mesh statistics
  void set_min_i(index_type i) {min_i_ = i; }
  void set_min_j(index_type j) {min_j_ = j; }
  void set_min_k(index_type k) {min_k_ = k; }
  void set_min(const std::vector<index_type>& mins);
  void set_ni(index_type i)
  {
    ni_ = i;

    vmesh_.reset(CreateVLatVolMesh(this));
  }
  void set_nj(index_type j)
  {
    nj_ = j;

    vmesh_.reset(CreateVLatVolMesh(this));
  }
  void set_nk(index_type k)
  {
    nk_ = k;

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    vmesh_.reset(CreateVLatVolMesh(this));
  }
  virtual void set_dim(const std::vector<size_type>& dims);

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

  /// get the child elements of the given index
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

  /// get the parent element(s) of the given index
  void get_elems(typename Elem::array_type &result, const typename Node::index_type &idx) const;
  void get_elems(typename Elem::array_type &result, const typename Edge::index_type &idx) const;
  void get_elems(typename Elem::array_type &result, const typename Face::index_type &idx) const;


  /// Wrapper to get the derivative elements from this element.
  void get_delems(typename DElem::array_type &result,
                  const typename Elem::index_type &idx) const
  {
    get_faces(result, idx);
  }

  /// return all cell_indecies that overlap the BBox in arr.
  void get_cells(typename Cell::array_type &arr, const Core::Geometry::BBox &box);
  /// returns the min and max indices that fall within or on the BBox
  void get_cells(typename Cell::index_type &begin,
                 typename Cell::index_type &end,
                 const Core::Geometry::BBox &bbox);
  void get_nodes(typename Node::index_type &begin,
                 typename Node::index_type &end,
                 const Core::Geometry::BBox &bbox);

  bool get_neighbor(typename Cell::index_type &neighbor,
                    const typename Cell::index_type &from,
                    typename Face::index_type face) const;

  /// get the center point (in object space) of an element
  void get_center(Core::Geometry::Point &, const typename Node::index_type &) const;
  void get_center(Core::Geometry::Point &, typename Edge::index_type) const;
  void get_center(Core::Geometry::Point &, typename Face::index_type) const;
  void get_center(Core::Geometry::Point &, const typename Cell::index_type &) const;

  /// Get the size of an elemnt (length, area, volume)
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

  bool locate(typename Node::index_type &, const Core::Geometry::Point &) const;
  bool locate(typename Edge::index_type &, const Core::Geometry::Point &) const
  { return false; }
  bool locate(typename Face::index_type &, const Core::Geometry::Point &) const
  { return false; }
  bool locate(typename Elem::index_type &, const Core::Geometry::Point &) const;

  int get_weights(const Core::Geometry::Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point & , typename Edge::array_type & , double * )
    { ASSERTFAIL("LatVolMesh::get_weights for edges isn't supported"); }
  int get_weights(const Core::Geometry::Point & , typename Face::array_type & , double * )
    { ASSERTFAIL("LatVolMesh::get_weights for faces isn't supported"); }
  int get_weights(const Core::Geometry::Point &p, typename Cell::array_type &l, double *w);

  void get_point(Core::Geometry::Point &p, const typename Node::index_type &i) const
  { get_center(p, i); }

  void get_normal(Core::Geometry::Vector &, const typename Node::index_type &) const
  { ASSERTFAIL("This mesh type does not have node normals."); }
  void get_normal(Core::Geometry::Vector &, std::vector<double> &, typename Elem::index_type,
                  unsigned int)
  { ASSERTFAIL("This mesh type does not have element normals."); }
  void get_random_point(Core::Geometry::Point &,
                        const typename Elem::index_type &,
                        FieldRNG &rng) const;

  /// This function will find the closest element and the location on that
  /// element that is the closest
  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         typename Node::index_type &elem,
                         const Core::Geometry::Point &p) const;

  bool find_closest_node(double& pdist, Core::Geometry::Point &result,
                         typename Node::index_type &elem,
                         const Core::Geometry::Point &p, double maxdist) const;

  /// This function will find the closest element and the location on that
  /// element that is the closest
  template <class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         ARRAY& coords,
                         typename Elem::index_type &elem,
                         const Core::Geometry::Point &p,
                         double maxdist) const
  {
    bool ret = find_closest_elem(pdist,result,coords,elem,p);
    if(!ret) return (false);
    if (maxdist < 0.0 || pdist < maxdist) return (true);
    return (false);
  }


  /// This function will find the closest element and the location on that
  /// element that is the closest
  template <class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         ARRAY& coords,
                         typename Elem::index_type &elem,
                         const Core::Geometry::Point &p) const
  {
    if (ni_ == 0 || nj_ == 0 || nk_ == 0) return (false);

    const Core::Geometry::Point r = transform_.unproject(p);

    double ii = r.x();
    double jj = r.y();
    double kk = r.z();
    const double nii = static_cast<double>(ni_-1);
    const double njj = static_cast<double>(nj_-1);
    const double nkk = static_cast<double>(nk_-1);

    if (ii < 0.0) ii = 0.0; if (ii >= nii) ii = nii;
    if (jj < 0.0) jj = 0.0; if (jj >= njj) jj = njj;
    if (kk < 0.0) kk = 0.0; if (kk >= nkk) kk = nkk;

    double fi = floor(ii); if (fi == nii) fi--;
    double fj = floor(jj); if (fj == njj) fj--;
    double fk = floor(kk); if (fk == nkk) fk--;

    elem.i_ = static_cast<index_type>(fi);
    elem.j_ = static_cast<index_type>(fj);
    elem.k_ = static_cast<index_type>(fk);
    elem.mesh_ = this;

    result = transform_.project(Core::Geometry::Point(ii,jj,kk));
    pdist = (p-result).length();

    coords.resize(3);
    coords[0] = ii-fi;
    coords[1] = jj-fj;
    coords[2] = kk-fk;

    return (true);
  }


  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point& result,
                         typename Elem::index_type &elem,
                         const Core::Geometry::Point &p) const
  {
    StackVector<double,3> coords;
    return(find_closest_elem(pdist,result,coords,elem,p));
  }

  /// This function will return multiple elements if the closest point is
  /// located on a node or edge. All bordering elements are returned in that
  /// case.
  bool find_closest_elems(double& pdist, Core::Geometry::Point &result,
                          std::vector<typename Elem::index_type> &elem,
                          const Core::Geometry::Point &p) const;

  double get_epsilon() const;

  /// Export this class using the old Pio system
  virtual void io(Piostream&);
  /// These IDs are created as soon as this class will be instantiated
  /// The first one is for Pio and the second for the virtual interface
  /// These are currently different as they serve different needs.  static PersistentTypeID type_idts;
private:
  static PersistentTypeID latvol_typeid;
  /// Core functionality for getting the name of a templated mesh class

public:
  static  const std::string type_name(int n = -1);
  virtual std::string dynamic_type_name() const { return latvol_typeid.type; }

  // Unsafe due to non-constness of unproject.
  Core::Geometry::Transform& get_transform() { return transform_; }
  Core::Geometry::Transform& set_transform(const Core::Geometry::Transform &trans)
  {
    transform_ = trans;
    transform_.compute_imat();
    compute_jacobian();
    return transform_;
  }

  virtual int dimensionality() const { return 3; }
  virtual int topology_geometry() const { return (STRUCTURED | REGULAR); }

  /// Type description, used for finding names of the mesh class for
  /// dynamic compilation purposes. Some of this should be obsolete
  virtual const TypeDescription *get_type_description() const;
  static const TypeDescription* cell_type_description();
  static const TypeDescription* face_type_description();
  static const TypeDescription* edge_type_description();
  static const TypeDescription* node_type_description();
  static const TypeDescription* elem_type_description()
  { return cell_type_description(); }

  /// This function returns a maker for Pio.
  static Persistent *maker() { return new LatVolMesh(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return boost::make_shared<LatVolMesh>(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle latvol_maker(size_type x, size_type y, size_type z, const Core::Geometry::Point& min, const Core::Geometry::Point& max)
  {
    return boost::make_shared<LatVolMesh>(x,y,z,min,max);
  }

protected:

  void compute_jacobian();

  /// the min_Node::index_type ( incase this is a subLattice )
  index_type min_i_, min_j_, min_k_;
  /// the Node::index_type space extents of a LatVolMesh
  /// (min=min_Node::index_type, max=min+extents-1)
  size_type ni_, nj_, nk_;

  Core::Geometry::Transform transform_;
  Basis     basis_;

  // The jacobian is the same for every element
  // hence store them as soon as we know the transfrom_
  // This should speed up FE computations on these regular grids.
  double jacobian_[9];
  double inverse_jacobian_[9];
  double det_jacobian_;
  double scaled_jacobian_;
  double det_inverse_jacobian_;

  boost::shared_ptr<VMesh> vmesh_;
};

template <class Basis>
const TypeDescription* get_type_description(LatVolMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("LatVolMesh", subs,
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
LatVolMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((LatVolMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
LatVolMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((LatVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Node",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
LatVolMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((LatVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Edge",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
LatVolMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((LatVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Face",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
LatVolMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((LatVolMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Cell",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

template <class Basis>
PersistentTypeID
LatVolMesh<Basis>::latvol_typeid(type_name(-1), "Mesh", LatVolMesh<Basis>::maker);

template <class Basis>
LatVolMesh<Basis>::LatVolMesh(size_type i, size_type j, size_type k,
                              const Core::Geometry::Point &min, const Core::Geometry::Point &max)
  : min_i_(0), min_j_(0), min_k_(0),
    ni_(i), nj_(j), nk_(k)
{
  DEBUG_CONSTRUCTOR("LatVolMesh")

  transform_.pre_scale(Core::Geometry::Vector(1.0 / (i-1.0), 1.0 / (j-1.0), 1.0 / (k-1.0)));
  transform_.pre_scale(max - min);
  transform_.pre_translate(Core::Geometry::Vector(min));
  transform_.compute_imat();
  compute_jacobian();

  /// Initialize the virtual interface when the mesh is created
  vmesh_.reset(CreateVLatVolMesh(this));
}


template <class Basis>
void
LatVolMesh<Basis>::get_random_point(Core::Geometry::Point &p,
                                    const typename Elem::index_type &ei,
                                    FieldRNG &rng) const
{
  // build the three principal edge vectors
  typename Node::array_type ra;
  get_nodes(ra,ei);
  Core::Geometry::Point p0,p1,p2,p3;
  get_point(p0,ra[0]);
  get_point(p1,ra[1]);
  get_point(p2,ra[3]);
  get_point(p3,ra[4]);
  Core::Geometry::Vector v0(p1-p0);
  Core::Geometry::Vector v1(p2-p0);
  Core::Geometry::Vector v2(p3-p0);

  // Choose a random point in the cell.
  const double t = rng();
  const double u = rng();
  const double v = rng();

  p = p0+(v0*t)+(v1*u)+(v2*v);
}


template <class Basis>
Core::Geometry::BBox
LatVolMesh<Basis>::get_bounding_box() const
{
  std::vector<Core::Geometry::Point> corners = {
    {min_i_, min_j_, min_k_},
    {min_i_ + ni_-1, min_j_,         min_k_},
    {min_i_ + ni_-1, min_j_ + nj_-1, min_k_},
    {min_i_,         min_j_ + nj_-1, min_k_},
    {min_i_,         min_j_,         min_k_ + nk_-1},
    {min_i_ + ni_-1, min_j_,         min_k_ + nk_-1},
    {min_i_ + ni_-1, min_j_ + nj_-1, min_k_ + nk_-1},
    {min_i_,         min_j_ + nj_-1, min_k_ + nk_-1} };


  for (auto& c : corners)
    c = transform_.project(c);

  Core::Geometry::BBox result(corners);
  return result;
}


template <class Basis>
Core::Geometry::Vector
LatVolMesh<Basis>::diagonal() const
{
  return get_bounding_box().diagonal();
}


template <class Basis>
void
LatVolMesh<Basis>::transform(const Core::Geometry::Transform &t)
{
  transform_.pre_trans(t);
  transform_.compute_imat();
  compute_jacobian();
}


template <class Basis>
void
LatVolMesh<Basis>::get_canonical_transform(Core::Geometry::Transform &t)
{
  t = transform_;
  t.post_scale(Core::Geometry::Vector(ni_ - 1.0, nj_ - 1.0, nk_ - 1.0));
}


template <class Basis>
bool
LatVolMesh<Basis>::get_min(std::vector<index_type> &array) const
{
  array.resize(3);
  array.clear();

  array.push_back(min_i_);
  array.push_back(min_j_);
  array.push_back(min_k_);

  return true;
}


template <class Basis>
bool
LatVolMesh<Basis>::get_dim(std::vector<index_type> &array) const
{
  array.resize(3);
  array.clear();

  array.push_back(ni_);
  array.push_back(nj_);
  array.push_back(nk_);

  return true;
}


template <class Basis>
void
LatVolMesh<Basis>::set_min(const std::vector<index_type>& min)
{
  min_i_ = min[0];
  min_j_ = min[1];
  min_k_ = min[2];
}


template <class Basis>
void
LatVolMesh<Basis>::set_dim(const std::vector<index_type>& dim)
{
  ni_ = dim[0];
  nj_ = dim[1];
  nk_ = dim[2];

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  vmesh_.reset(CreateVLatVolMesh(this));
}


// Note: This code does not respect boundaries of the mesh
template <class Basis>
void
LatVolMesh<Basis>::get_nodes(typename Node::array_type &array,
                             typename Edge::index_type idx) const
{
  array.resize(2);
  // The (const unsigned int) on the next line is due
  // to a bug in the OSX 10.4 compiler that gives xidx
  // the wrong type
  const size_type xidx = (const size_type)idx;
  if (xidx < (ni_ - 1) * nj_ * nk_)
  {
    const index_type i = xidx % (ni_ - 1);
    const index_type jk = xidx / (ni_ - 1);
    const index_type j = jk % nj_;
    const index_type k = jk / nj_;

    array[0] = typename Node::index_type(this, i+0, j, k);
    array[1] = typename Node::index_type(this, i+1, j, k);
  }
  else
  {
    const index_type yidx = idx - (ni_ - 1) * nj_ * nk_;
    if (yidx < (ni_ * (nj_ - 1) * nk_))
    {
      const index_type j = yidx % (nj_ - 1);
      const index_type ik = yidx / (nj_ - 1);
      const index_type i = ik / nk_;
      const index_type k = ik % nk_;

      array[0] = typename Node::index_type(this, i, j+0, k);
      array[1] = typename Node::index_type(this, i, j+1, k);
    }
    else
    {
      const index_type zidx = yidx - (ni_ * (nj_ - 1) * nk_);
      const index_type k = zidx % (nk_ - 1);
      const index_type ij = zidx / (nk_ - 1);
      const index_type i = ij % ni_;
      const index_type j = ij / ni_;

      array[0] = typename Node::index_type(this, i, j, k+0);
      array[1] = typename Node::index_type(this, i, j, k+1);
    }
  }
}


// Note: This code does not respect boundaries of the mesh
template <class Basis>
void
LatVolMesh<Basis>::get_nodes(typename Node::array_type &array,
                             typename Face::index_type idx) const
{
  array.resize(4);
  // The (const unsigned int) on the next line is due
  // to a bug in the OSX 10.4 compiler that gives xidx
  // the wrong type
  const index_type xidx = (const index_type)idx;
  if (xidx < (ni_ - 1) * (nj_ - 1) * nk_)
  {
    const index_type i = xidx % (ni_ - 1);
    const index_type jk = xidx / (ni_ - 1);
    const index_type j = jk % (nj_ - 1);
    const index_type k = jk / (nj_ - 1);
    array[0] = typename Node::index_type(this, i+0, j+0, k);
    array[1] = typename Node::index_type(this, i+1, j+0, k);
    array[2] = typename Node::index_type(this, i+1, j+1, k);
    array[3] = typename Node::index_type(this, i+0, j+1, k);
  }
  else
  {
    const index_type yidx = idx - (ni_ - 1) * (nj_ - 1) * nk_;
    if (yidx < ni_ * (nj_ - 1) * (nk_ - 1))
    {
      const index_type j = yidx % (nj_ - 1);
      const index_type ik = yidx / (nj_ - 1);
      const index_type k = ik % (nk_ - 1);
      const index_type i = ik / (nk_ - 1);
      array[0] = typename Node::index_type(this, i, j+0, k+0);
      array[1] = typename Node::index_type(this, i, j+1, k+0);
      array[2] = typename Node::index_type(this, i, j+1, k+1);
      array[3] = typename Node::index_type(this, i, j+0, k+1);
    }
    else
    {
      const index_type zidx = yidx - ni_ * (nj_ - 1) * (nk_ - 1);
      const index_type k = zidx % (nk_ - 1);
      const index_type ij = zidx / (nk_ - 1);
      const index_type i = ij % (ni_ - 1);
      const index_type j = ij / (ni_ - 1);
      array[0] = typename Node::index_type(this, i+0, j, k+0);
      array[1] = typename Node::index_type(this, i+0, j, k+1);
      array[2] = typename Node::index_type(this, i+1, j, k+1);
      array[3] = typename Node::index_type(this, i+1, j, k+0);
    }
  }
}


// Note: This code does not respect boundaries of the mesh.
template <class Basis>
void
LatVolMesh<Basis>::get_nodes(typename Node::array_type &array,
                             const typename Cell::index_type &idx) const
{
  array.resize(8);
  array[0].i_ = idx.i_;   array[0].j_ = idx.j_;   array[0].k_ = idx.k_;
  array[1].i_ = idx.i_+1; array[1].j_ = idx.j_;   array[1].k_ = idx.k_;
  array[2].i_ = idx.i_+1; array[2].j_ = idx.j_+1; array[2].k_ = idx.k_;
  array[3].i_ = idx.i_;   array[3].j_ = idx.j_+1; array[3].k_ = idx.k_;
  array[4].i_ = idx.i_;   array[4].j_ = idx.j_;   array[4].k_ = idx.k_+1;
  array[5].i_ = idx.i_+1; array[5].j_ = idx.j_;   array[5].k_ = idx.k_+1;
  array[6].i_ = idx.i_+1; array[6].j_ = idx.j_+1; array[6].k_ = idx.k_+1;
  array[7].i_ = idx.i_;   array[7].j_ = idx.j_+1; array[7].k_ = idx.k_+1;

  array[0].mesh_ = this;
  array[1].mesh_ = this;
  array[2].mesh_ = this;
  array[3].mesh_ = this;
  array[4].mesh_ = this;
  array[5].mesh_ = this;
  array[6].mesh_ = this;
  array[7].mesh_ = this;
}


template <class Basis>
void
LatVolMesh<Basis>::get_edges(typename Edge::array_type &array,
                             typename Face::index_type idx) const
{
  array.resize(4);
  const index_type num_i_faces = (ni_-1)*(nj_-1)*nk_;  // lie in ij plane ijk
  const index_type num_j_faces = ni_*(nj_-1)*(nk_-1);  // lie in jk plane jki
  const index_type num_k_faces = (ni_-1)*nj_*(nk_-1);  // lie in ki plane kij

  const index_type num_i_edges = (ni_-1)*nj_*nk_; // ijk
  const index_type num_j_edges = ni_*(nj_-1)*nk_; // jki

  index_type facei, facej, facek;
  index_type face = idx;

  if (face < num_i_faces)
  {
    facei = face % (ni_-1);
    facej = (face / (ni_-1)) % (nj_-1);
    facek = face / ((ni_-1)*(nj_-1));
    array[0] = facei+facej*(ni_-1)+facek*(ni_-1)*(nj_);
    array[1] = facei+(facej+1)*(ni_-1)+facek*(ni_-1)*(nj_);
    array[2] = num_i_edges + facei*(nj_-1)*(nk_)+facej+facek*(nj_-1);
    array[3] = num_i_edges + (facei+1)*(nj_-1)*(nk_)+facej+facek*(nj_-1);
  }
  else if (face - num_i_faces < num_j_faces)
  {
    face -= num_i_faces;
    facei = face / ((nj_-1) *(nk_-1));
    facej = face % (nj_-1);
    facek = (face / (nj_-1)) % (nk_-1);
    array[0] = num_i_edges + facei*(nj_-1)*(nk_)+facej+facek*(nj_-1);
    array[1] = num_i_edges + facei*(nj_-1)*(nk_)+facej+(facek+1)*(nj_-1);
    array[2] = (num_i_edges + num_j_edges +
                facei*(nk_-1)+facej*(ni_)*(nk_-1)+facek);
    array[3] = (num_i_edges + num_j_edges +
                facei*(nk_-1)+(facej+1)*(ni_)*(nk_-1)+facek);

  }
  else if (face - num_i_faces - num_j_faces < num_k_faces)
  {
    face -= (num_i_faces + num_j_faces);
    facei = (face / (nk_-1)) % (ni_-1);
    facej = face / ((ni_-1) * (nk_-1));
    facek = face % (nk_-1);
    array[0] = facei+facej*(ni_-1)+facek*(ni_-1)*(nj_);
    array[1] = facei+facej*(ni_-1)+(facek+1)*(ni_-1)*(nj_);
    array[2] = (num_i_edges + num_j_edges +
                facei*(nk_-1)+facej*(ni_)*(nk_-1)+facek);
    array[3] = (num_i_edges + num_j_edges +
                (facei+1)*(nk_-1)+facej*(ni_)*(nk_-1)+facek);
  }
  else {
    ASSERTFAIL(
          "LatVolMesh<Basis>::get_edges(Edge, Face) Face idx out of bounds");
  }
}


template <class Basis>
void
LatVolMesh<Basis>::get_edges(typename Edge::array_type &array,
                             const typename Cell::index_type &idx) const
{
  array.resize(12);
  const index_type j_start= (ni_-1)*nj_*nk_;
  const index_type k_start = ni_*(nj_-1)*nk_ + j_start;

  array[0] = idx.i_ + idx.j_*(ni_-1)     + idx.k_*(ni_-1)*(nj_);
  array[1] = idx.i_ + (idx.j_+1)*(ni_-1) + idx.k_*(ni_-1)*(nj_);
  array[2] = idx.i_ + idx.j_*(ni_-1)     + (idx.k_+1)*(ni_-1)*(nj_);
  array[3] = idx.i_ + (idx.j_+1)*(ni_-1) + (idx.k_+1)*(ni_-1)*(nj_);

  array[4] = j_start + idx.i_*(nj_-1)*(nk_)     + idx.j_ + idx.k_*(nj_-1);
  array[5] = j_start + (idx.i_+1)*(nj_-1)*(nk_) + idx.j_ + idx.k_*(nj_-1);
  array[6] = j_start + idx.i_*(nj_-1)*(nk_)     + idx.j_ + (idx.k_+1)*(nj_-1);
  array[7] = j_start + (idx.i_+1)*(nj_-1)*(nk_) + idx.j_ + (idx.k_+1)*(nj_-1);

  array[8] =  k_start + idx.i_*(nk_-1)     + idx.j_*(ni_)*(nk_-1)     + idx.k_;
  array[9] =  k_start + (idx.i_+1)*(nk_-1) + idx.j_*(ni_)*(nk_-1)     + idx.k_;
  array[10] = k_start + idx.i_*(nk_-1)     + (idx.j_+1)*(ni_)*(nk_-1) + idx.k_;
  array[11] = k_start + (idx.i_+1)*(nk_-1) + (idx.j_+1)*(ni_)*(nk_-1) + idx.k_;

}


template <class Basis>
void
LatVolMesh<Basis>::get_elems(typename Elem::array_type &result,
                             const typename Edge::index_type &eidx) const
{
  result.reserve(4);
  result.clear();
  const index_type offset1 = (ni_-1)*nj_*nk_;
  const index_type offset2 = offset1 + ni_*(nj_-1)*nk_;
  index_type idx = eidx;

  if (idx < offset1)
  {
    index_type k = idx/((nj_)*(ni_-1)); idx -= k*(nj_)*(ni_-1);
    index_type j = idx/(ni_-1); idx -= j*(ni_-1);
    index_type i = idx;

    if (j > 0)
    {
      if (k < (nk_-1)) result.push_back(CellIndex(this,i,j-1,k));
      if (k > 0) result.push_back(CellIndex(this,i,j-1,k-1));
    }

    if (j < (nj_-1))
    {
      if (k < (nk_-1)) result.push_back(CellIndex(this,i,j,k));
      if (k > 0) result.push_back(CellIndex(this,i,j,k-1));
    }
  }
  else if (idx >= offset2)
  {
    idx -= offset2;
    index_type j = idx/((nk_-1)*(ni_)); idx -= j*(nk_-1)*(ni_);
    index_type i = idx/(nk_-1); idx -= i*(nk_-1);
    index_type k = idx;

    if (i > 0)
    {
      if (j < (nj_-1)) result.push_back(CellIndex(this,i-1,j,k));
      if (j > 0) result.push_back(CellIndex(this,i-1,j-1,k));
    }

    if (i < (ni_-1))
    {
      if (j < (nj_-1)) result.push_back(CellIndex(this,i,j,k));
      if (j > 0) result.push_back(CellIndex(this,i,j-1,k));
    }
  }
  else
  {
    idx -= offset1;
    index_type i = idx/((nk_)*(nj_-1)); idx -= i*(nk_)*(nj_-1);
    index_type k = idx/(nj_-1); idx -= k*(nj_-1);
    index_type j = idx;

    if (k > 0)
    {
      if (i < (nk_-1)) result.push_back(CellIndex(this,i,j,k-1));
      if (i > 0) result.push_back(CellIndex(this,i-1,j,k-1));
    }

    if (k < (nk_-1))
    {
      if (i < (ni_-1)) result.push_back(CellIndex(this,i,j,k));
      if (i > 0) result.push_back(CellIndex(this,i-1,j,k));
    }
  }
}


template <class Basis>
void
LatVolMesh<Basis>::get_faces(typename Face::array_type &array,
                             const typename Cell::index_type &idx) const
{
  array.resize(6);

  const index_type i = idx.i_;
  const index_type j = idx.j_;
  const index_type k = idx.k_;

  const index_type offset1 = (ni_ - 1) * (nj_ - 1) * nk_;
  const index_type offset2 = offset1 + ni_ * (nj_ - 1) * (nk_ - 1);

  array[0] = i + (j + k * (nj_-1)) * (ni_-1);
  array[1] = i + (j + (k+1) * (nj_-1)) * (ni_-1);

  array[2] = offset1 + j + (k + i * (nk_-1)) * (nj_-1);
  array[3] = offset1 + j + (k + (i+1) * (nk_-1)) * (nj_-1);

  array[4] = offset2 + k + (i + j * (ni_-1)) * (nk_-1);
  array[5] = offset2 + k + (i + (j+1) * (ni_-1)) * (nk_-1);
}


template <class Basis>
void
LatVolMesh<Basis>::get_elems(typename Elem::array_type &result,
                             const typename Face::index_type &fidx) const
{
  result.reserve(2);
  result.clear();
  const index_type offset1 = (ni_ - 1) * (nj_ - 1) * nk_;
  const index_type offset2 = offset1 + ni_ * (nj_ - 1) * (nk_ - 1);
  index_type idx = fidx;

  if (idx < offset1)
  {
    index_type k = idx/((nj_-1)*(ni_-1)); idx -= k*(nj_-1)*(ni_-1);
    index_type j = idx/(ni_-1); idx -= j*(ni_-1);
    index_type i = idx;

    if (k < (nk_-1)) result.push_back(CellIndex(this,i,j,k));
    if (k > 0) result.push_back(CellIndex(this,i,j,k-1));
  }
  else if (idx >= offset2)
  {
    idx -= offset2;
    index_type j = idx/((nk_-1)*(ni_-1)); idx -= j*(nk_-1)*(ni_-1);
    index_type i = idx/(nk_-1); idx -= i*(nk_-1);
    index_type k = idx;

    if (j < (nj_-1)) result.push_back(CellIndex(this,i,j,k));
    if (j > 0) result.push_back(CellIndex(this,i,j-1,k));
  }
  else
  {
    idx -= offset1;
    index_type i = idx/((nk_-1)*(nj_-1)); idx -= i*(nk_-1)*(nj_-1);
    index_type k = idx/(nj_-1); idx -= k*(nj_-1);
    index_type j = idx;

    if (i < (ni_-1)) result.push_back(CellIndex(this,i,j,k));
    if (i > 0) result.push_back(CellIndex(this,i-1,j,k));
  }
}

template <class Basis>
void
LatVolMesh<Basis>::get_elems(typename Elem::array_type &result,
                             const typename Node::index_type &idx) const
{
  result.reserve(8);
  result.clear();
  const index_type i0 = idx.i_ ? idx.i_ - 1 : 0;
  const index_type j0 = idx.j_ ? idx.j_ - 1 : 0;
  const index_type k0 = idx.k_ ? idx.k_ - 1 : 0;

  const index_type i1 = idx.i_ < ni_-1 ? idx.i_+1 : ni_-1;
  const index_type j1 = idx.j_ < nj_-1 ? idx.j_+1 : nj_-1;
  const index_type k1 = idx.k_ < nk_-1 ? idx.k_+1 : nk_-1;

  index_type i, j, k;
  for (k = k0; k < k1; k++)
    for (j = j0; j < j1; j++)
      for (i = i0; i < i1; i++)
        result.push_back(typename Cell::index_type(this, i, j, k));
}


/// return all cell_indecies that overlap the BBox in arr.

template <class Basis>
void
LatVolMesh<Basis>::get_cells(typename Cell::array_type &arr, const Core::Geometry::BBox &bbox)
{
  // Get our min and max
  typename Cell::index_type min, max;
  get_cells(min, max, bbox);

  // Clear the input array.  Limited to range of ints.
  arr.clear();

  // Loop over over min to max and fill the array
  index_type i, j, k;
  for (i = min.i_; i <= max.i_; i++) {
    for (j = min.j_; j <= max.j_; j++) {
      for (k = min.k_; k <= max.k_; k++) {
        arr.push_back(typename Cell::index_type(this, i,j,k));
      }
    }
  }
}


/// Returns the min and max indices that fall within or on the BBox.

// If the max index lies "in front of" (meaning that any of the
// indexes are negative) then the max will be set to [0,0,0] and the
// min to [1,1,1] in the hopes that they will be used something like
// this: for(unsigned int i = min.i_; i <= max.i_; i++)....  Otherwise
// you can expect the min and max to be set clamped to the boundaries.
template <class Basis>
void
LatVolMesh<Basis>::get_cells(typename Cell::index_type &begin, typename Cell::index_type &end,
                             const Core::Geometry::BBox &bbox) {

  const Core::Geometry::Point minp = transform_.unproject(bbox.get_min());
  index_type mini = (index_type)floor(minp.x());
  index_type minj = (index_type)floor(minp.y());
  index_type mink = (index_type)floor(minp.z());
  if (mini < 0) { mini = 0; }
  if (minj < 0) { minj = 0; }
  if (mink < 0) { mink = 0; }

  const Core::Geometry::Point maxp = transform_.unproject(bbox.get_max());
  index_type maxi = (index_type)floor(maxp.x());
  index_type maxj = (index_type)floor(maxp.y());
  index_type maxk = (index_type)floor(maxp.z());
  if (maxi >= (index_type)(ni_ - 1)) { maxi = ni_ - 2; }
  if (maxj >= (index_type)(nj_ - 1)) { maxj = nj_ - 2; }
  if (maxk >= (index_type)(nk_ - 1)) { maxk = nk_ - 2; }
  // We also need to protect against when any of these guys are
  // negative.  In this case we should not have any iteration.  We
  // can't however express negative numbers with unsigned ints (in the
  // case of index_type).
  if (maxi < 0 || maxj < 0 || maxk < 0) {
    // We should create a range which will not be iterated over
    mini = minj = mink = 1;
    maxi = maxj = maxk = 0;
  }

  begin = typename Cell::index_type(this, mini, minj, mink);
  end   = typename Cell::index_type(this, maxi, maxj, maxk);
}


template <class Basis>
bool
LatVolMesh<Basis>::get_neighbor(typename Cell::index_type &neighbor,
                                const typename Cell::index_type &from,
                                typename Face::index_type face) const
{
  // The (const unsigned int) on the next line is due
  // to a bug in the OSX 10.4 compiler that gives xidx
  // the wrong type
  const index_type xidx = (const index_type)face;
  if (xidx < (ni_ - 1) * (nj_ - 1) * nk_)
  {
    const index_type jk = xidx / (ni_ - 1);
    const index_type k = jk / (nj_ - 1);

    if (k == from.k_ && k > 0)
    {
      neighbor.i_ = from.i_;
      neighbor.j_ = from.j_;
      neighbor.k_ = k-1;
      neighbor.mesh_ = this;
      return true;
    }
    else if (k == (from.k_+1) && k < (nk_-1))
    {
      neighbor.i_ = from.i_;
      neighbor.j_ = from.j_;
      neighbor.k_ = k;
      neighbor.mesh_ = this;
      return true;
    }
  }
  else
  {
    const index_type yidx = xidx - (ni_ - 1) * (nj_ - 1) * nk_;
    if (yidx < ni_ * (nj_ - 1) * (nk_ - 1))
    {
      const index_type ik = yidx / (nj_ - 1);
      const index_type i = ik / (nk_ - 1);

      if (i == from.i_ && i > 0)
      {
        neighbor.i_ = i-1;
        neighbor.j_ = from.j_;
        neighbor.k_ = from.k_;
        neighbor.mesh_ = this;
        return true;
      }
      else if (i == (from.i_+1) && i < (ni_-1))
      {
        neighbor.i_ = i;
        neighbor.j_ = from.j_;
        neighbor.k_ = from.k_;
        neighbor.mesh_ = this;
        return true;
      }
    }
    else
    {
      const index_type zidx = yidx - ni_ * (nj_ - 1) * (nk_ - 1);
      const index_type ij = zidx / (nk_ - 1);
      const index_type j = ij / (ni_ - 1);

      if (j == from.j_ && j > 0)
      {
        neighbor.i_ = from.i_;
        neighbor.j_ = j-1;
        neighbor.k_ = from.k_;
        neighbor.mesh_ = this;
        return true;
      }
      else if (j == (from.j_+1) && j < (nj_-1))
      {
        neighbor.i_ = from.i_;
        neighbor.j_ = j;
        neighbor.k_ = from.k_;
        neighbor.mesh_ = this;
        return true;
      }
    }
  }
  return false;
}


template <class Basis>
void
LatVolMesh<Basis>::get_nodes(typename Node::index_type &begin,
                             typename Node::index_type &end,
                             const Core::Geometry::BBox &bbox)
{
  // get the min and max points of the bbox and make sure that they lie
  // inside the mesh boundaries.
  Core::Geometry::BBox mesh_boundary = get_bounding_box();
  // crop by min boundary
  Core::Geometry::Point min = Max(bbox.get_min(), mesh_boundary.get_min());
  Core::Geometry::Point max = Max(bbox.get_max(), mesh_boundary.get_min());
  // crop by max boundary
  min = Min(min, mesh_boundary.get_max());
  max = Min(max, mesh_boundary.get_max());
  typename Node::index_type min_index, max_index;

  // If one of the locates return true, then we have a valid iteration
  bool min_located = locate(min_index, min);
  bool max_located = locate(max_index, max);
  if (!min_located && !max_located)
  {
    // first check to see if there is a bbox overlap
    Core::Geometry::BBox box;
    box.extend(min);
    box.extend(max);
    if ( box.overlaps(mesh_boundary) )
    {
      Core::Geometry::Point r = transform_.unproject(min);
      double rx = floor(r.x());
      double ry = floor(r.y());
      double rz = floor(r.z());
      min_index.i_ = (index_type)std::max(rx, 0.0);
      min_index.j_ = (index_type)std::max(ry, 0.0);
      min_index.k_ = (index_type)std::max(rz, 0.0);
      r = transform_.unproject(max);
      rx = floor(r.x());
      ry = floor(r.y());
      rz = floor(r.z());
      max_index.i_ = (index_type)std::min(rx, (double)ni_ );
      max_index.j_ = (index_type)std::min(ry, (double)nj_ );
      max_index.k_ = (index_type)std::min(rz, (double)nk_ );
    }
    else
    {
      // Set the min and max extents of the range iterator to be the
      // same thing.  When they are the same end_iter will be set to
      // the starting state of the range iterator, thereby causing any
      // for loop using these iterators [for(;iter != end_iter;
      // iter++)] to never enter.
      min_index = typename Node::index_type(this, 0,0,0);
      max_index = typename Node::index_type(this, 0,0,0);
    }
  }
  else if ( !min_located )
  {
    const Core::Geometry::Point r = transform_.unproject(min);
    const double rx = floor(r.x());
    const double ry = floor(r.y());
    const double rz = floor(r.z());
    min_index.i_ = (index_type)std::max(rx, 0.0);
    min_index.j_ = (index_type)std::max(ry, 0.0);
    min_index.k_ = (index_type)std::max(rz, 0.0);
  }
  else
  { //  !max_located
    const Core::Geometry::Point r = transform_.unproject(max);
    const double rx = floor(r.x());
    const double ry = floor(r.y());
    const double rz = floor(r.z());
    max_index.i_ = (index_type)std::min(rx, (double) ni_ );
    max_index.j_ = (index_type)std::min(ry, (double) nj_ );
    max_index.k_ = (index_type)std::min(rz, (double) nk_ );
  }

  begin = min_index;
  end   = max_index;
}


template <class Basis>
void
LatVolMesh<Basis>::get_center(Core::Geometry::Point &result,
                              typename Edge::index_type idx) const
{
  typename Node::array_type arr;
  get_nodes(arr, idx);
  Core::Geometry::Point p1;
  get_center(result, arr[0]);
  get_center(p1, arr[1]);

  result += p1;
  result *= 0.5;
}


template <class Basis>
void
LatVolMesh<Basis>::get_center(Core::Geometry::Point &result,
                              typename Face::index_type idx) const
{
  typename Node::array_type nodes;
  get_nodes(nodes, idx);
  ASSERT(nodes.size() == 4);
  typename Node::array_type::iterator nai = nodes.begin();
  get_point(result, *nai);
  ++nai;
  Core::Geometry::Point pp;
  while (nai != nodes.end())
  {
    get_point(pp, *nai);
    result += pp;
    ++nai;
  }
  result *= (1.0 / 4.0);
}


template <class Basis>
void
LatVolMesh<Basis>::get_center(Core::Geometry::Point &result,
                              const typename Cell::index_type &idx) const
{
  Core::Geometry::Point p(idx.i_ + 0.5, idx.j_ + 0.5, idx.k_ + 0.5);
  result = transform_.project(p);
}


template <class Basis>
void
LatVolMesh<Basis>::get_center(Core::Geometry::Point &result,
                              const typename Node::index_type &idx) const
{
  Core::Geometry::Point p(idx.i_, idx.j_, idx.k_);
  result = transform_.project(p);
}


template <class Basis>
double
LatVolMesh<Basis>::get_size(const typename Node::index_type& /*idx*/) const
{
  return 0.0;
}


template <class Basis>
double
LatVolMesh<Basis>::get_size(typename Edge::index_type idx) const
{
  typename Node::array_type arr;
  get_nodes(arr, idx);
  Core::Geometry::Point p0, p1;
  get_center(p0, arr[0]);
  get_center(p1, arr[1]);

  return (p1 - p0).length();
}


template <class Basis>
double
LatVolMesh<Basis>::get_size(typename Face::index_type idx) const
{
  typename Node::array_type nodes;
  get_nodes(nodes, idx);

  Core::Geometry::Point p0, p1, p2;
  get_point(p0, nodes[0]);
  get_point(p1, nodes[1]);
  get_point(p2, nodes[2]);
  Core::Geometry::Vector v0 = p1 - p0;
  Core::Geometry::Vector v1 = p2 - p0;

  Core::Geometry::Vector v = Cross(v0,v1);
  return (v.length());
}


template <class Basis>
double
LatVolMesh<Basis>::get_size(const typename Cell::index_type& /*idx*/) const
{
  // Correct implementation, the old one did not take into account skewing
  // the latvol!!!

  // As they are all the same we precompute these
  return (det_jacobian_);
}


template <class Basis>
bool
LatVolMesh<Basis>::locate(typename Elem::index_type &elem, const Core::Geometry::Point &p) const
{
  const double epsilon = 1e-7;

  if (ni_ == 0 || nj_ == 0 || nk_ == 0) return (false);

  const Core::Geometry::Point r = transform_.unproject(p);

  double ii = r.x();
  double jj = r.y();
  double kk = r.z();

  const double nii = static_cast<double>(ni_-1);
  const double njj = static_cast<double>(nj_-1);
  const double nkk = static_cast<double>(nk_-1);

  if (ii>=nii && (ii-epsilon)<nii) ii=nii-epsilon;
  if (jj>=njj && (jj-epsilon)<njj) jj=njj-epsilon;
  if (kk>=nkk && (kk-epsilon)<nkk) kk=nkk-epsilon;

  if (ii<0 && ii>(-epsilon)) ii=0.0;
  if (jj<0 && jj>(-epsilon)) jj=0.0;
  if (kk<0 && kk>(-epsilon)) kk=0.0;

  const index_type i = static_cast<index_type>(floor(ii));
  const index_type j = static_cast<index_type>(floor(jj));
  const index_type k = static_cast<index_type>(floor(kk));

  if (i < (ni_-1) && i >= 0 &&
      j < (nj_-1) && j >= 0 &&
      k < (nk_-1) && k >= 0 &&
      ii >= 0.0 && jj >= 0.0 && kk >= 0.0)
  {
    elem.i_ = i;
    elem.j_ = j;
    elem.k_ = k;
    elem.mesh_ = this;
    return (true);
  }

  return (false);
}


template <class Basis>
bool
LatVolMesh<Basis>::locate(typename Node::index_type &node, const Core::Geometry::Point &p) const
{
  if (ni_ == 0 || nj_ == 0 || nk_ == 0) return (false);

  const Core::Geometry::Point r = transform_.unproject(p);

  double rx = floor(r.x() + 0.5);
  double ry = floor(r.y() + 0.5);
  double rz = floor(r.z() + 0.5);

  const double nii = static_cast<double>(ni_-1);
  const double njj = static_cast<double>(nj_-1);
  const double nkk = static_cast<double>(nk_-1);

  if (rx < 0.0) rx = 0.0;
  if (rx > nii) rx = nii;
  if (ry < 0.0) ry = 0.0;
  if (ry > njj) ry = njj;
  if (rz < 0.0) rz = 0.0;
  if (rz > nkk) rz = nkk;

  node.i_ = static_cast<index_type>(rx);
  node.j_ = static_cast<index_type>(ry);
  node.k_ = static_cast<index_type>(rz);
  node.mesh_ = this;

  return (true);
}



template <class Basis>
bool
LatVolMesh<Basis>::find_closest_node(double& pdist,
                           Core::Geometry::Point &result,
                           typename Node::index_type &node,
                           const Core::Geometry::Point &p) const
{
  if (ni_ == 0 || nj_ == 0 || nk_ == 0) return (false);

  const Core::Geometry::Point r = transform_.unproject(p);

  double rx = floor(r.x() + 0.5);
  double ry = floor(r.y() + 0.5);
  double rz = floor(r.z() + 0.5);

  const double nii = static_cast<double>(ni_-1);
  const double njj = static_cast<double>(nj_-1);
  const double nkk = static_cast<double>(nk_-1);

  if (rx < 0.0) rx = 0.0;
  if (rx > nii) rx = nii;
  if (ry < 0.0) ry = 0.0;
  if (ry > njj) ry = njj;
  if (rz < 0.0) rz = 0.0;
  if (rz > nkk) rz = nkk;

  result = transform_.project(Core::Geometry::Point(rx,ry,rz));
  node.i_ = static_cast<index_type>(rx);
  node.j_ = static_cast<index_type>(ry);
  node.k_ = static_cast<index_type>(rz);
  node.mesh_ = this;

  pdist = (p-result).length();
  return (true);
}



template <class Basis>
bool
LatVolMesh<Basis>::find_closest_node(double& pdist,
                                    Core::Geometry::Point &result,
                                    typename Node::index_type &node,
                                    const Core::Geometry::Point &p, double maxdist) const
{
  bool ret = find_closest_node(pdist,result,node,p);
  if (!ret)  return (false);
  if (maxdist < 0.0 || pdist < maxdist) return (true);
  return (false);
}

template <class Basis>
bool
LatVolMesh<Basis>::find_closest_elems(double& pdist,
                            Core::Geometry::Point &result,
                            std::vector<typename Elem::index_type> &elems,
                            const Core::Geometry::Point &p) const
{
  // For calculations inside the local element
  const double epsilon = 1e-8;
  elems.clear();

  if (ni_ == 0 || nj_ == 0 || nk_ == 0) return (false);

  const Core::Geometry::Point r = transform_.unproject(p);

  double ii = r.x();
  double jj = r.y();
  double kk = r.z();
  const double nii = static_cast<double>(ni_-2);
  const double njj = static_cast<double>(nj_-2);
  //const double nkk = static_cast<double>(nk_-2);

  if (ii < 0.0) ii = 0.0;
  if (ii > nii) ii = nii;
  if (jj < 0.0) jj = 0.0;
  if (jj > njj) jj = njj;
  if (jj < 0.0) jj = 0.0;
  if (jj > njj) jj = njj;
  const double fii = floor(ii);
  const double fjj = floor(jj);
  const double fkk = floor(kk);

  index_type i = static_cast<index_type>(fii);
  index_type j = static_cast<index_type>(fjj);
  index_type k = static_cast<index_type>(fkk);

  typename Elem::index_type elem;

  elem.i_ = i;
  elem.j_ = j;
  elem.k_ = k;
  elem.mesh_ = this;
  elems.push_back(elem);

  if ((fabs(fii-ii) < epsilon) && ((i-1)>0))
  {
    elem.i_ = i-1;
    elem.j_ = j;
    elem.k_ = k;
    elems.push_back(elem);
  }

  if ((fabs(fii-(ii+1.0)) < epsilon) && (i<(ni_-1)))
  {
    elem.i_ = i+1;
    elem.j_ = j;
    elem.k_ = k;
    elems.push_back(elem);
  }

  if ((fabs(fjj-jj) < epsilon) && ((j-1)>0))
  {
    elem.i_ = i;
    elem.j_ = j-1;
    elem.k_ = k;
    elems.push_back(elem);
  }

  if ((fabs(fjj-(jj+1.0)) < epsilon) && (j<(nj_-1)))
  {
    elem.i_ = i;
    elem.j_ = j+1;
    elem.k_ = k;
    elems.push_back(elem);
  }

  if ((fabs(fkk-kk) < epsilon) && ((k-1)>0))
  {
    elem.i_ = i;
    elem.j_ = j;
    elem.k_ = k-1;
    elems.push_back(elem);
  }

  if ((fabs(fkk-(kk+1.0)) < epsilon) && (k<(nk_-1)))
  {
    elem.i_ = i;
    elem.j_ = j;
    elem.k_ = k+1;
    elems.push_back(elem);
  }
  result = transform_.project(Core::Geometry::Point(ii,jj,kk));

  pdist = (p-result).length();
  return (true);
}



template <class Basis>
int
LatVolMesh<Basis>::get_weights(const Core::Geometry::Point &p,
                               typename Node::array_type &locs,
                               double *w)
{
  typename Cell::index_type idx;
  if (locate(idx, p)) {
    get_nodes(locs, idx);
    std::vector<double> coords(3);
    if (get_coords(coords, p, idx)) {
      basis_.get_weights(coords, w);
      return basis_.dofs();
    }
  }
  return 0;
}


template <class Basis>
int
LatVolMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Cell::array_type &l,
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
const std::string
find_type_name(typename LatVolMesh<Basis>::NodeIndex *)
{
  static std::string name = LatVolMesh<Basis>::type_name(-1) + "::NodeIndex";
  return name;
}


template <class Basis>
const std::string
find_type_name(typename LatVolMesh<Basis>::CellIndex *)
{
  static std::string name = LatVolMesh<Basis>::type_name(-1) + "::CellIndex";
  return name;
}

#define LATVOLMESH_VERSION 5

template <class Basis>
void
LatVolMesh<Basis>::io(Piostream& stream)
{
  int version = stream.begin_class(type_name(-1), LATVOLMESH_VERSION);

  Mesh::io(stream);

  // IO data members, in order
  if (version < 5)
  {
    unsigned int ni = static_cast<unsigned int>(ni_);
    unsigned int nj = static_cast<unsigned int>(nj_);
    unsigned int nk = static_cast<unsigned int>(nk_);
    Pio(stream, ni);
    Pio(stream, nj);
    Pio(stream, nk);
    ni_ = static_cast<size_type>(ni);
    nj_ = static_cast<size_type>(nj);
    nk_ = static_cast<size_type>(nk);
  }
  else
  {
    Pio_size(stream, ni_);
    Pio_size(stream, nj_);
    Pio_size(stream, nk_);
  }

  if (version < 2 && stream.reading())
  {
    Core::Geometry::Point min, max;
    Pio(stream, min);
    Pio(stream, max);
    transform_.pre_scale(Core::Geometry::Vector(1.0 / (ni_ - 1.0),1.0 / (nj_ - 1.0),1.0 / (nk_ - 1.0)));
    transform_.pre_scale(max - min);
    transform_.pre_translate(Core::Geometry::Vector(min));
    transform_.compute_imat();
  }
  else if (version < 3 && stream.reading() )
  {
    Pio_old(stream, transform_);
  }
  else
  {
    Pio(stream, transform_);
  }

  if (version >= 4)
  {
    basis_.io(stream);
  }

  stream.end_class();

  if (stream.reading())
  {
    compute_jacobian();
    vmesh_.reset(CreateVLatVolMesh(this));
  }

}

template <class Basis>
const std::string
LatVolMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("LatVolMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
void
LatVolMesh<Basis>::begin(typename LatVolMesh<Basis>::Node::iterator &itr) const
{
  itr = typename Node::iterator(this, min_i_, min_j_, min_k_);
}


template <class Basis>
void
LatVolMesh<Basis>::end(typename LatVolMesh<Basis>::Node::iterator &itr) const
{
  itr = typename Node::iterator(this, min_i_, min_j_, min_k_ + nk_);
}


template <class Basis>
void
LatVolMesh<Basis>::size(typename LatVolMesh<Basis>::Node::size_type &s) const
{
  s = typename Node::size_type(ni_,nj_,nk_);
}


template <class Basis>
void
LatVolMesh<Basis>::to_index(typename LatVolMesh<Basis>::Node::index_type &idx,
                            index_type a) const
{
  const index_type i = a % ni_;
  const index_type jk = a / ni_;
  const index_type j = jk % nj_;
  const index_type k = jk / nj_;
  idx = typename Node::index_type(this, i, j, k);
}


template <class Basis>
void
LatVolMesh<Basis>::begin(typename LatVolMesh<Basis>::Edge::iterator &itr) const
{
  itr = typename Edge::iterator(0);
}


template <class Basis>
void
LatVolMesh<Basis>::end(typename LatVolMesh<Basis>::Edge::iterator &itr) const
{
  itr = ((ni_-1) * nj_ * nk_) + (ni_ * (nj_-1) * nk_) + (ni_ * nj_ * (nk_-1));
}


template <class Basis>
void
LatVolMesh<Basis>::size(typename LatVolMesh<Basis>::Edge::size_type &s) const
{
  s = ((ni_-1) * nj_ * nk_) + (ni_ * (nj_-1) * nk_) + (ni_ * nj_ * (nk_-1));
}


template <class Basis>
void
LatVolMesh<Basis>::begin(typename LatVolMesh<Basis>::Face::iterator &itr) const
{
  itr = typename Face::iterator(0);
}


template <class Basis>
void
LatVolMesh<Basis>::end(typename LatVolMesh<Basis>::Face::iterator &itr) const
{
  itr = (ni_-1) * (nj_-1) * nk_ +
    ni_ * (nj_ - 1 ) * (nk_ - 1) +
    (ni_ - 1) * nj_ * (nk_ - 1);
}


template <class Basis>
void
LatVolMesh<Basis>::size(typename LatVolMesh<Basis>::Face::size_type &s) const
{
  s =  (ni_-1) * (nj_-1) * nk_ +
    ni_ * (nj_ - 1 ) * (nk_ - 1) +
    (ni_ - 1) * nj_ * (nk_ - 1);
}


template <class Basis>
void
LatVolMesh<Basis>::begin(typename LatVolMesh<Basis>::Cell::iterator &itr) const
{
  itr = typename Cell::iterator(this,  min_i_, min_j_, min_k_);
}


template <class Basis>
void
LatVolMesh<Basis>::end(typename LatVolMesh<Basis>::Cell::iterator &itr) const
{
  itr = typename Cell::iterator(this, min_i_, min_j_, min_k_ + nk_-1);
}


template <class Basis>
void
LatVolMesh<Basis>::size(typename LatVolMesh<Basis>::Cell::size_type &s) const
{
  s = typename Cell::size_type(ni_-1, nj_-1,nk_-1);
}


template <class Basis>
void
LatVolMesh<Basis>::to_index(typename LatVolMesh<Basis>::Cell::index_type &idx,
                            index_type a) const
{
  const index_type i = a % (ni_-1);
  const index_type jk = a / (ni_-1);
  const index_type j = jk % (nj_-1);
  const index_type k = jk / (nj_-1);
  idx = typename Cell::index_type(this, i, j, k);
}


template <class Basis>
void
LatVolMesh<Basis>::compute_jacobian()
{
  Core::Geometry::Vector J1 = transform_.project(Core::Geometry::Vector(1.0,0.0,0.0));
  Core::Geometry::Vector J2 = transform_.project(Core::Geometry::Vector(0.0,1.0,0.0));
  Core::Geometry::Vector J3 = transform_.project(Core::Geometry::Vector(0.0,0.0,1.0));

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

template <class Basis>
double
LatVolMesh<Basis>::get_epsilon() const
{
  Core::Geometry::Point p0(static_cast<double>(ni_-1),
           static_cast<double>(nj_-1),
           static_cast<double>(nk_-1));
  Core::Geometry::Point p1(0.0,0.0,0.0);
  Core::Geometry::Point q0 = transform_.project(p0);
  Core::Geometry::Point q1 = transform_.project(p1);
  return ((q0-q1).length()*1e-8);
}

} // namespace SCIRun

#endif
