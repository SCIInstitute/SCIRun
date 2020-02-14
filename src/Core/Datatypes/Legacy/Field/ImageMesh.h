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


#ifndef CORE_DATATYPES_IMAGEMESH_H
#define CORE_DATATYPES_IMAGEMESH_H 1

/// Include what kind of support we want to have
/// Need to fix this and couple it to sci-defs
#include <Core/Datatypes/Legacy/Field/MeshSupport.h>

#include <Core/Containers/StackVector.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/QuadBilinearLgn.h>

#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/BBox.h>

#include <Core/Datatypes/Legacy/Field/FieldIterator.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldRNG.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

/////////////////////////////////////////////////////
// Declarations for virtual interface


/// Functions for creating the virtual interface
/// Declare the functions that instantiate the virtual interface
template <class Basis>
class ImageMesh;

/// make sure any other mesh other than the preinstantiate ones
/// returns no virtual interface. Altering this behavior will allow
/// for dynamically compiling the interface if needed.
template<class MESH>
VMesh* CreateVImageMesh(MESH*) { return (0); }

/// These declarations are needed for a combined dynamic compilation as
/// as well as virtual functions solution.
/// Declare that these can be found in a library that is already
/// precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_IMAGE_SUPPORT > 0)

SCISHARE VMesh* CreateVImageMesh(ImageMesh<Core::Basis::QuadBilinearLgn<Core::Geometry::Point> >* mesh);

#endif
/////////////////////////////////////////////////////


template <class Basis>
class ImageMesh : public Mesh {

/// Make sure the virtual interface has access
template <class MESH> friend class VImageMesh;

public:

  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type                 under_type;
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::size_type                  size_type;
  typedef SCIRun::mask_type                  mask_type;

  typedef boost::shared_ptr<ImageMesh<Basis> >  handle_type;
  typedef Basis                             basis_type;
  struct ImageIndex;
  friend struct ImageIndex;

  struct ImageIndex
  {
  public:
    ImageIndex() : i_(0), j_(0), mesh_(0) {}

    ImageIndex(const ImageMesh *m, size_type i, size_type j)
      : i_(i), j_(j), mesh_(m) {}

    operator size_type() const {
      ASSERT(mesh_);
      return i_ + j_*mesh_->ni_;
    }

    std::ostream& str_render(std::ostream& os) const {
      os << "[" << i_ << "," << j_ << "]";
      return os;
    }

    size_type i_, j_;

    const ImageMesh *mesh_;
  };

  struct IFaceIndex : public ImageIndex
  {
    IFaceIndex() : ImageIndex() {}
    IFaceIndex(const ImageMesh *m, index_type i, index_type j)
      : ImageIndex(m, i, j) {}

    operator index_type() const {
      ASSERT(this->mesh_);
      return this->i_ + this->j_ * (this->mesh_->ni_-1);
    }
  };

  struct INodeIndex : public ImageIndex
  {
    INodeIndex() : ImageIndex() {}
    INodeIndex(const ImageMesh *m, index_type i, index_type j)
      : ImageIndex(m, i, j) {}
  };

  struct ImageIter : public ImageIndex
  {
    ImageIter() : ImageIndex() {}
    ImageIter(const ImageMesh *m, index_type i, index_type j)
      : ImageIndex(m, i, j) {}

    const ImageIndex &operator *() { return *this; }

    bool operator ==(const ImageIter &a) const
    {
      return this->i_ == a.i_ && this->j_ == a.j_ && this->mesh_ == a.mesh_;
    }

    bool operator !=(const ImageIter &a) const
    {
      return !(*this == a);
    }
  };

  struct INodeIter : public ImageIter
  {
    INodeIter() : ImageIter() {}
    INodeIter(const ImageMesh *m, index_type i, index_type j)
      : ImageIter(m, i, j) {}

    const INodeIndex &operator *() const { return (const INodeIndex&)(*this); }

    INodeIter &operator++()
    {
      this->i_++;
      if (this->i_ >= this->mesh_->min_i_ + this->mesh_->ni_) {
        this->i_ = this->mesh_->min_i_;
        this->j_++;
      }
      return *this;
    }

  private:

    INodeIter operator++(int)
    {
      INodeIter result(*this);
      operator++();
      return result;
    }
  };


  struct IFaceIter : public ImageIter
  {
    IFaceIter() : ImageIter() {}
    IFaceIter(const ImageMesh *m, index_type i, index_type j)
      : ImageIter(m, i, j) {}

    const IFaceIndex &operator *() const { return (const IFaceIndex&)(*this); }

    IFaceIter &operator++()
    {
      this->i_++;
      if (this->i_ >= this->mesh_->min_i_+this->mesh_->ni_-1)
      {
        this->i_ = this->mesh_->min_i_;
        this->j_++;
      }
      return *this;
    }

  private:

    IFaceIter operator++(int)
    {
      IFaceIter result(*this);
      operator++();
      return result;
    }
  };

  struct ImageSize
  {
  public:
    ImageSize() : i_(0), j_(0) {}
    ImageSize(index_type i, index_type j) : i_(i), j_(j) {}

    operator index_type() const { return i_*j_; }

    std::ostream& str_render(std::ostream& os) const
    {
      os << i_*j_ << " (" << i_ << " x " << j_ << ")";
      return os;
    }


    index_type i_, j_;
  };

  struct INodeSize : public ImageSize
  {
    INodeSize() : ImageSize() {}
    INodeSize(index_type i, index_type j) : ImageSize(i,j) {}
  };


  struct IFaceSize : public ImageSize
  {
    IFaceSize() : ImageSize() {}
    IFaceSize(index_type i, index_type j) : ImageSize(i,j) {}
  };


  /// Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef INodeIndex                       index_type;
    typedef INodeIter                        iterator;
    typedef INodeSize                        size_type;
    typedef StackVector<index_type, 4>       array_type;
  };

  struct Edge {
    typedef EdgeIndex<under_type>            index_type;
    typedef EdgeIterator<under_type>         iterator;
    typedef EdgeIndex<under_type>            size_type;
    typedef std::vector<index_type>               array_type;
  };

  struct Face {
    typedef IFaceIndex                       index_type;
    typedef IFaceIter                        iterator;
    typedef IFaceSize                        size_type;
    typedef std::vector<index_type>               array_type;
  };

  struct Cell {
    typedef CellIndex<under_type>            index_type;
    typedef CellIterator<under_type>         iterator;
    typedef CellIndex<under_type>            size_type;
    typedef std::vector<index_type>               array_type;
  };

  typedef Face Elem;
  typedef Edge DElem;

  friend struct INodeIter;
  friend struct IFaceIter;
  friend struct IFaceIndex;

  friend class ElemData;

  class ElemData
  {
  public:
     ElemData(const ImageMesh<Basis>& msh,
             const typename Elem::index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    // the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const
    {
      return (index_.i_ + mesh_.get_ni()*index_.j_);
    }
    inline
    index_type node1_index() const
    {
      return (index_.i_+ 1 + mesh_.get_ni()*index_.j_);
    }
    inline
    index_type node2_index() const
    {
      return (index_.i_ + 1 + mesh_.get_ni()*(index_.j_ + 1));

    }
    inline
    index_type node3_index() const
    {
      return (index_.i_ + mesh_.get_ni()*(index_.j_ + 1));
    }

    // the following designed to coordinate with ::get_edges
    inline
    index_type edge0_index() const
    {
      return index_.i_ + index_.j_ * (mesh_.ni_- 1);
    }
    inline
    index_type edge1_index() const
    {
      return index_.i_ + (index_.j_ + 1) * (mesh_.ni_ - 1);
    }
    inline
    index_type edge2_index() const
    {
      return index_.i_    *(mesh_.nj_ - 1) + index_.j_ +
        ((mesh_.ni_ - 1) * mesh_.nj_);
     }
    inline
    index_type edge3_index() const
    {
      return (index_.i_ + 1) * (mesh_.nj_ - 1) + index_.j_ +
        ((mesh_.ni_ - 1) * mesh_.nj_);
    }


    inline
    const Core::Geometry::Point node0() const
    {
      Core::Geometry::Point p(index_.i_, index_.j_, 0.0);
      return mesh_.transform_.project(p);
    }
    inline
    const Core::Geometry::Point node1() const
    {
      Core::Geometry::Point p(index_.i_ + 1, index_.j_, 0.0);
      return mesh_.transform_.project(p);
    }
    inline
    const Core::Geometry::Point node2() const
    {
      Core::Geometry::Point p(index_.i_ + 1, index_.j_ + 1, 0.0);
      return mesh_.transform_.project(p);
    }
    inline
    const Core::Geometry::Point node3() const
    {
      Core::Geometry::Point p(index_.i_, index_.j_ + 1, 0.0);
      return mesh_.transform_.project(p);
    }


  private:
    const ImageMesh<Basis>           &mesh_;
    const typename Elem::index_type  index_;
  };


  ImageMesh()
    : min_i_(0), min_j_(0),
      ni_(1), nj_(1)
  {
    DEBUG_CONSTRUCTOR("ImageMesh")
    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    vmesh_.reset(CreateVImageMesh(this));
  }

  ImageMesh(size_type x, size_type y, const Core::Geometry::Point &min, const Core::Geometry::Point &max);
  ImageMesh(ImageMesh* mh, size_type mx, size_type my,
            size_type x, size_type y)
    : min_i_(mx), min_j_(my), ni_(x), nj_(y), transform_(mh->transform_)
  {
    DEBUG_CONSTRUCTOR("ImageMesh")
    transform_.compute_imat();
    compute_jacobian();

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    vmesh_.reset(CreateVImageMesh(this));
  }

  ImageMesh(const ImageMesh &copy)
    : Mesh(copy),
      min_i_(copy.min_i_), min_j_(copy.min_j_),
      ni_(copy.get_ni()), nj_(copy.get_nj()), transform_(copy.transform_)
  {
    DEBUG_CONSTRUCTOR("ImageMesh")
    transform_.compute_imat();
    compute_jacobian();

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    vmesh_.reset(CreateVImageMesh(this));
  }

  virtual ImageMesh *clone() const { return new ImageMesh(*this); }
  virtual ~ImageMesh()
  {
    DEBUG_DESTRUCTOR("ImageMesh")
  }

  MeshFacadeHandle getFacade() const
  {
    return boost::make_shared<Core::Datatypes::VirtualMeshFacade<VMesh>>(vmesh_);
  }

  /// Access point to virtual interface
  virtual VMesh* vmesh() { return vmesh_.get(); }

  virtual int basis_order() { return (basis_.polynomial_order()); }

  virtual bool has_normals() const { return false; }
  virtual bool has_face_normals() const { return false; }
  virtual bool is_editable() const { return false; }

  Basis& get_basis() { return basis_; }

  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an edge.
  void pwl_approx_edge(std::vector<std::vector<double> > &coords,
                       typename Elem::index_type /*ci*/,
                       unsigned int which_edge,
                       unsigned int div_per_unit) const
  {
    // Needs to match unit_edges in Basis/QuadBilinearLgn.cc
    // compare get_nodes order to the basis order
    int basis_emap[] = {0, 2, 3, 1};
    basis_.approx_edge(basis_emap[which_edge], div_per_unit, coords);
  }

  /// Generate the list of points that make up a sufficiently accurate
  /// piecewise linear approximation of an face.
  void pwl_approx_face(std::vector<std::vector<std::vector<double> > > &coords,
                       typename Elem::index_type,
                       unsigned int,
                       unsigned int div_per_unit) const
  {
    basis_.approx_face(0, div_per_unit, coords);
  }


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
      return basis_.get_coords(coords, p, ed);
    }

    // Cheap implementation that assumes it is a regular grid
    // This implementation should be faster then the interpolate of the linear
    // basis which needs to work as well for the unstructured hexvol :(
    const Core::Geometry::Point r = transform_.unproject(p);

    coords.resize(2);
    coords[0] = static_cast<typename VECTOR::value_type>(r.x()-static_cast<double>(idx.i_));
    coords[1] = static_cast<typename VECTOR::value_type>(r.y()-static_cast<double>(idx.j_));

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
            0.0);
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
    J.resize(2);
    J[0] = (transform_.project(Core::Geometry::Point(1.0,0.0,0.0)));
    J[1] = (transform_.project(Core::Geometry::Point(0.0,1.0,0.0)));
  }

  /// Get the determinant of the jacobian, which is the local volume of an element
  /// and is intended to help with the integration of functions over an element.
  template<class VECTOR>
  double det_jacobian(const VECTOR& coords, typename Elem::index_type idx) const
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
  size_type get_min_i() const { return min_i_; }
  size_type get_min_j() const { return min_j_; }
  bool get_min(std::vector<index_type>&) const;
  size_type get_ni() const { return ni_; }
  size_type get_nj() const { return nj_; }
  virtual bool get_dim(std::vector<size_type>&) const;
  Core::Geometry::Vector diagonal() const;
  virtual Core::Geometry::BBox get_bounding_box() const;
  virtual void transform(const Core::Geometry::Transform &t);
  virtual void get_canonical_transform(Core::Geometry::Transform &t);
  virtual bool synchronize(mask_type sync);
  virtual bool unsynchronize(mask_type sync);
  bool clear_synchronization();

  /// set the mesh statistics
  void set_min_i(index_type i) {min_i_ = i; }
  void set_min_j(index_type j) {min_j_ = j; }
  void set_min(std::vector<index_type> mins);
  void set_ni(size_type i)
  {
    ni_ = i;

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    vmesh_.reset(CreateVImageMesh(this));
  }
  void set_nj(size_type j)
  {
    nj_ = j;

    /// Create a new virtual interface for this copy
    /// all pointers have changed hence create a new
    /// virtual interface class
    vmesh_.reset(CreateVImageMesh(this));
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

  void to_index(typename Node::index_type &index, index_type i) const;
  void to_index(typename Edge::index_type &index, index_type i) const { index= i; }
  void to_index(typename Face::index_type &index, index_type i) const;
  void to_index(typename Cell::index_type &, index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }


  /// get the child elements of the given index
  void get_nodes(typename Node::array_type &, typename Edge::index_type) const;
  void get_nodes(typename Node::array_type &, typename Face::index_type) const;
  void get_nodes(typename Node::array_type &, typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\".");}
  void get_edges(typename Edge::array_type &, typename Face::index_type) const;
  void get_edges(typename Edge::array_type &, typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }
  void get_faces(typename Face::array_type &, typename Cell::index_type) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }
  void get_faces(typename Face::array_type &a,typename Face::index_type f) const
  { a.push_back(f); }

  /// get the parent element(s) of the given index
  void get_elems(typename Elem::array_type &result, typename Node::index_type idx) const;
  void get_elems(typename Elem::array_type &result, typename Edge::index_type idx) const;
  void get_elems(typename Elem::array_type&, typename Face::index_type) const {}


  /// Wrapper to get the derivative elements from this element.
  void get_delems(typename DElem::array_type &result,
                  const typename Elem::index_type &idx) const
  {
    get_edges(result, idx);
  }

  /// return all face_indecies that overlap the BBox in arr.
  void get_faces(typename Face::array_type &arr, const Core::Geometry::BBox &box);

  /// Get the size of an element (length, area, volume)
  double get_size(const typename Node::index_type &/*i*/) const { return 0.0; }
  double get_size(typename Edge::index_type idx) const
  {
    typename Node::array_type arr;
    get_nodes(arr, idx);
    Core::Geometry::Point p0, p1;
    get_center(p0, arr[0]);
    get_center(p1, arr[1]);
    return (p1 - p0).length();
  }
  double get_size(const typename Face::index_type &idx) const
  {
    typename Node::array_type ra;
    get_nodes(ra,idx);
    Core::Geometry::Point p0,p1,p2;
    get_point(p0,ra[0]);
    get_point(p1,ra[1]);
    get_point(p2,ra[2]);
    return (Cross(p0-p1,p2-p0)).length();
  }
  double get_size(typename Cell::index_type /*idx*/) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  double get_length(typename Edge::index_type idx) const { return get_size(idx); };
  double get_area(typename Face::index_type idx) const { return get_size(idx); };
  double get_volume(typename Cell::index_type /*idx*/) const
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); };


  bool get_neighbor(typename Face::index_type &neighbor,
                    typename Face::index_type from,
                    typename Edge::index_type idx) const;

  void get_neighbors(typename Face::array_type &array, typename Face::index_type idx) const;

  void get_normal(Core::Geometry::Vector&, const typename Node::index_type&) const
  { ASSERTFAIL("This mesh type does not have node normals."); }
  void get_normal(Core::Geometry::Vector &result, std::vector<double> &coords,
                  typename Elem::index_type eidx, index_type)
  {
    ElemData ed(*this, eidx);
    std::vector<Core::Geometry::Point> Jv;
    basis_.derivate(coords, ed, Jv);
    result = Cross(Core::Geometry::Vector(Jv[0]), Core::Geometry::Vector(Jv[1]));
    result.normalize();
  }

  /// get the center point (in object space) of an element
  void get_center(Core::Geometry::Point &, const typename Node::index_type &) const;
  void get_center(Core::Geometry::Point &, typename Edge::index_type) const;
  void get_center(Core::Geometry::Point &, const typename Face::index_type &) const;
  void get_center(Core::Geometry::Point &, typename Cell::index_type) const {
    ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  bool locate(typename Node::index_type &, const Core::Geometry::Point &) const;
  bool locate(typename Edge::index_type &, const Core::Geometry::Point &) const {return false;}
  bool locate(typename Face::index_type &, const Core::Geometry::Point &) const;
  bool locate(typename Cell::index_type &, const Core::Geometry::Point &) const {
    ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  int get_weights(const Core::Geometry::Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point & , typename Edge::array_type & , double * )
  { ASSERTFAIL("ImageMesh::get_weights for edges isn't supported"); }
  int get_weights(const Core::Geometry::Point &p, typename Face::array_type &l, double *w);
  int get_weights(const Core::Geometry::Point & , typename Cell::array_type & , double * )
  { ASSERTFAIL("This mesh type does not have cells use \"elem\"."); }

  void get_point(Core::Geometry::Point &p, const typename Node::index_type &i) const
  { get_center(p, i); }

  void get_random_point(Core::Geometry::Point &, const typename Elem::index_type &, FieldRNG &rng) const;


  /// Export this class using the old Pio system
  virtual void io(Piostream&);
  /// These IDs are created as soon as this class will be instantiated
  /// The first one is for Pio and the second for the virtual interface
  /// These are currently different as they serve different needs.  static PersistentTypeID type_idts;
  static PersistentTypeID imagemesh_typeid;
  /// Core functionality for getting the name of a templated mesh class
  static const std::string type_name(int n = -1);
  virtual std::string dynamic_type_name() const { return imagemesh_typeid.type; }

  // Unsafe due to non-constness of unproject.
  Core::Geometry::Transform &get_transform() { return transform_; }
  Core::Geometry::Transform &set_transform(const Core::Geometry::Transform &trans)
  { transform_ = trans; transform_.compute_imat(); compute_jacobian(); return transform_; }

  virtual int dimensionality() const { return 2; }
  virtual int  topology_geometry() const { return (STRUCTURED | REGULAR); }

  /// Type description, used for finding names of the mesh class for
  /// dynamic compilation purposes. Some of this should be obsolete
  virtual const TypeDescription *get_type_description() const;
  static const TypeDescription* node_type_description();
  static const TypeDescription* edge_type_description();
  static const TypeDescription* face_type_description();
  static const TypeDescription* cell_type_description();
  static const TypeDescription* elem_type_description()
    { return face_type_description(); }
  static const TypeDescription* node_index_type_description();
  static const TypeDescription* face_index_type_description();

  /// This function returns a maker for Pio.
  static Persistent *maker() { return new ImageMesh<Basis>(); }
  /// This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return boost::make_shared<ImageMesh<Basis>>();}
  /// This function returns a handle for the virtual interface.
  static MeshHandle image_maker(size_type x, size_type y, const Core::Geometry::Point& min, const Core::Geometry::Point& max)
    { return boost::make_shared<ImageMesh<Basis>>(x,y,min,max); }

  /// This function will find the closest element and the location on that
  /// element that is the closest
  bool find_closest_node(double& pdist,Core::Geometry::Point &result,
                         typename Node::index_type &elem,
                         const Core::Geometry::Point &p) const;

  bool find_closest_node(double& pdist,Core::Geometry::Point &result,
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
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point &result,
                         typename Elem::index_type &elem,
                         const Core::Geometry::Point &p) const
  {
    StackVector<double,2> coords;
    return(find_closest_elem(pdist,result,coords,elem,p));
  }

  template<class ARRAY>
  bool find_closest_elem(double& pdist,
                         Core::Geometry::Point& result,
                         ARRAY& coords,
                         typename Elem::index_type &elem,
                         const Core::Geometry::Point &p) const
  {
    if (ni_ == 0 || nj_ == 0) return (false);

    const Core::Geometry::Point r = transform_.unproject(p);

    double ii = r.x();
    double jj = r.y();
    const double nii = static_cast<double>(ni_-2);
    const double njj = static_cast<double>(nj_-2);

    if (ii < 0.0) ii = 0.0;
    if (ii > nii) ii = nii;
    if (jj < 0.0) jj = 0.0;
    if (jj > njj) jj = njj;

    const double fi = floor(ii);
    const double fj = floor(jj);

    elem.i_ = static_cast<index_type>(fi);
    elem.j_ = static_cast<index_type>(fj);
    elem.mesh_ = this;

    result = transform_.project(Core::Geometry::Point(ii,jj,0));
    pdist = (p-result).length();

    coords.resize(2);
    coords[0] = ii-fi;
    coords[1] = jj-fj;

    return (true);
  }


  /// This function will return multiple elements if the closest point is
  /// located on a node or edge. All bordering elements are returned in that
  /// case.
  bool find_closest_elems(double& pdist, Core::Geometry::Point &result,
                          std::vector<typename Elem::index_type> &elem,
                          const Core::Geometry::Point &p) const;

  double get_epsilon() const;

protected:

  void compute_jacobian();

  /// the min_typename Node::index_type ( incase this is a subLattice )
  index_type              min_i_;
  index_type              min_j_;
  /// the typename Node::index_type space extents of a ImageMesh
  /// (min=min_typename Node::index_type, max=min+extents-1)
  size_type               ni_;
  size_type               nj_;

  /// the object space extents of a ImageMesh
  Core::Geometry::Transform              transform_;

  Core::Geometry::Vector                 normal_;

  /// The basis class
  Basis                  basis_;

  /// Virtual mesh
  boost::shared_ptr<VMesh>          vmesh_;
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
ImageMesh<Basis>::imagemesh_typeid(type_name(-1),"Mesh", maker);


template<class Basis>
ImageMesh<Basis>::ImageMesh(size_type i, size_type j,
                            const Core::Geometry::Point &min, const Core::Geometry::Point &max) :
  min_i_(0), min_j_(0), ni_(i), nj_(j)
{
  DEBUG_CONSTRUCTOR("ImageMesh")
  transform_.pre_scale(Core::Geometry::Vector(1.0 / (i-1.0), 1.0 / (j-1.0), 1.0));

  Core::Geometry::Vector scale = max - min;
  scale[2] = 1.0;

  transform_.pre_scale(scale);
  transform_.pre_translate(Core::Geometry::Vector(min));
  transform_.compute_imat();

  normal_ = Core::Geometry::Vector(0.0, 0.0, 0.0);
  transform_.project_normal(normal_);
  normal_.safe_normalize();

  compute_jacobian();

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  vmesh_.reset(CreateVImageMesh(this));
}


template<class Basis>
double
ImageMesh<Basis>::get_epsilon() const
{
  Core::Geometry::Point p0(static_cast<double>(ni_-1),static_cast<double>(nj_-1),0.0);
  Core::Geometry::Point p1(0.0,0.0,0.0);
  Core::Geometry::Point q0 = transform_.project(p0);
  Core::Geometry::Point q1 = transform_.project(p1);
  return ((q0-q1).length()*1e-8);
}

template<class Basis>
Core::Geometry::BBox
ImageMesh<Basis>::get_bounding_box() const
{
  Core::Geometry::Point p0(0.0,   0.0,   0.0);
  Core::Geometry::Point p1(ni_-1, 0.0,   0.0);
  Core::Geometry::Point p2(ni_-1, nj_-1, 0.0);
  Core::Geometry::Point p3(0.0,   nj_-1, 0.0);

  Core::Geometry::BBox result;
  result.extend(transform_.project(p0));
  result.extend(transform_.project(p1));
  result.extend(transform_.project(p2));
  result.extend(transform_.project(p3));
  return result;
}


template<class Basis>
Core::Geometry::Vector
ImageMesh<Basis>::diagonal() const
{
  return get_bounding_box().diagonal();
}


template<class Basis>
void
ImageMesh<Basis>::get_canonical_transform(Core::Geometry::Transform &t)
{
  t = transform_;
  t.post_scale(Core::Geometry::Vector(ni_ - 1.0, nj_ - 1.0, 1.0));
}


template<class Basis>
bool
ImageMesh<Basis>::synchronize(mask_type sync)
{
  if (sync & Mesh::NORMALS_E)
  {
    normal_ = Core::Geometry::Vector(0.0, 0.0, 0.0);
    transform_.project_normal(normal_);
    normal_.safe_normalize();
    return true;
  }
  return false;
}

template<class Basis>
bool
ImageMesh<Basis>::unsynchronize(mask_type /*sync*/)
{
  return (true);
}

template<class Basis>
bool
ImageMesh<Basis>::clear_synchronization()
{
  return (true);
}

template<class Basis>
void
ImageMesh<Basis>::transform(const Core::Geometry::Transform &t)
{
  transform_.pre_trans(t);
  transform_.compute_imat();
  compute_jacobian();
}


template<class Basis>
bool
ImageMesh<Basis>::get_min(std::vector<index_type> &array) const
{
  array.resize(2);
  array.clear();

  array.push_back(min_i_);
  array.push_back(min_j_);

  return true;
}


template<class Basis>
bool
ImageMesh<Basis>::get_dim(std::vector<size_type> &array) const
{
  array.resize(2);
  array.clear();

  array.push_back(ni_);
  array.push_back(nj_);

  return true;
}


template<class Basis>
void
ImageMesh<Basis>::set_min(std::vector<index_type> min)
{
  min_i_ = min[0];
  min_j_ = min[1];
}


template<class Basis>
void
ImageMesh<Basis>::set_dim(std::vector<size_type> dim)
{
  ni_ = dim[0];
  nj_ = dim[1];

  /// Create a new virtual interface for this copy
  /// all pointers have changed hence create a new
  /// virtual interface class
  vmesh_.reset(CreateVImageMesh(this));
}


template<class Basis>
void
ImageMesh<Basis>::get_nodes(typename Node::array_type &array,
                            typename Face::index_type idx) const
{
  array.resize(4);

  array[0].i_ = idx.i_;   array[0].j_ = idx.j_;
  array[1].i_ = idx.i_+1; array[1].j_ = idx.j_;
  array[2].i_ = idx.i_+1; array[2].j_ = idx.j_+1;
  array[3].i_ = idx.i_;   array[3].j_ = idx.j_+1;

  array[0].mesh_ = idx.mesh_;
  array[1].mesh_ = idx.mesh_;
  array[2].mesh_ = idx.mesh_;
  array[3].mesh_ = idx.mesh_;
}


template<class Basis>
void
ImageMesh<Basis>::get_nodes(typename Node::array_type &array,
                            typename Edge::index_type idx) const
{
  array.resize(2);

  const index_type j_idx = idx - (ni_-1) * nj_;
  if (j_idx >= 0)
  {
    const index_type i = j_idx / (nj_ - 1);
    const index_type j = j_idx % (nj_ - 1);
    array[0] = typename Node::index_type(this, i, j);
    array[1] = typename Node::index_type(this, i, j+1);
  }
  else
  {
    const index_type i = idx % (ni_ - 1);
    const index_type j = idx / (ni_ - 1);
    array[0] = typename Node::index_type(this, i, j);
    array[1] = typename Node::index_type(this, i+1, j);
  }
}


template<class Basis>
void
ImageMesh<Basis>::get_edges(typename Edge::array_type &array,
                                          typename Face::index_type idx) const
{
  array.resize(4);

  const index_type j_idx = (ni_-1) * nj_;

  array[0] = idx.i_             + idx.j_    *(ni_-1);
  array[1] = idx.i_             + (idx.j_+1)*(ni_-1);
  array[2] = idx.i_    *(nj_-1) + idx.j_+ j_idx;
  array[3] = (idx.i_+1)*(nj_-1) + idx.j_+ j_idx;
}

template<class Basis>
void
ImageMesh<Basis>::get_elems(typename Elem::array_type &array, typename Edge::index_type idx) const
{
  array.reserve(2);
  array.clear();

  const index_type offset = (ni_-1)*nj_;

  if (idx < offset)
  {
    const index_type j = idx/(ni_-1);
    const index_type i = idx - j*(ni_-1);
    if (j < (nj_-1)) array.push_back(IFaceIndex(this,i,j));
    if (j > 0) array.push_back(IFaceIndex(this,i,j-1));
  }
  else
  {
    idx = idx - offset;
    const index_type i = idx/(nj_-1);
    const index_type j = idx - i*(nj_-1);
    if (i < (ni_-1)) array.push_back(IFaceIndex(this,i,j));
    if (i > 0) array.push_back(IFaceIndex(this,i-1,j));
  }
}




template<class Basis>
bool
ImageMesh<Basis>::get_neighbor(typename Face::index_type &neighbor,
                               typename Face::index_type from,
                               typename Edge::index_type edge) const
{
  neighbor.mesh_ = this;
  const index_type j_idx = edge - (ni_-1) * nj_;
  if (j_idx >= 0)
  {
    const index_type i = j_idx / (nj_ - 1);
    if (i == 0 || i == ni_-1)
      return false;
    neighbor.j_ = from.j_;
    if (i == from.i_)
      neighbor.i_ = from.i_ - 1;
    else
      neighbor.i_ = from.i_ + 1;
  }
  else
  {
    const index_type j = edge / (ni_ - 1);;
    if (j == 0 || j == nj_-1)
      return false;
    neighbor.i_ = from.i_;
    if (j == from.j_)
      neighbor.j_ = from.j_ - 1;
    else
      neighbor.j_ = from.j_ + 1;
  }
  return true;
}


template<class Basis>
void
ImageMesh<Basis>::get_neighbors(typename Face::array_type &array,
                                typename Face::index_type idx) const
{
  typename Edge::array_type edges;
  get_edges(edges, idx);
  array.clear();
  typename Edge::array_type::iterator iter = edges.begin();
  while(iter != edges.end()) {
    typename Face::index_type nbor;
    if (get_neighbor(nbor, idx, *iter)) {
      array.push_back(nbor);
    }
    ++iter;
  }
}


template<class Basis>
void
ImageMesh<Basis>::get_elems(typename Elem::array_type &result, const typename Node::index_type idx) const
{
  result.reserve(4);
  result.clear();

  const index_type i0 = idx.i_ ? idx.i_ - 1 : 0;
  const index_type j0 = idx.j_ ? idx.j_ - 1 : 0;

  const index_type i1 = idx.i_ < ni_-1 ? idx.i_+1 : ni_-1;
  const index_type j1 = idx.j_ < nj_-1 ? idx.j_+1 : nj_-1;

  index_type i, j;
  for (j = j0; j < j1; j++)
    for (i = i0; i < i1; i++)
      result.push_back(typename Face::index_type(this, i, j));
}


/// return all face_indecies that overlap the Core::Geometry::BBox in arr.
template<class Basis>
void
ImageMesh<Basis>::get_faces(typename Face::array_type &arr, const Core::Geometry::BBox &bbox)
{
  arr.clear();
  typename Face::index_type min;
  locate(min, bbox.get_min());
  typename Face::index_type max;
  locate(max, bbox.get_max());

  if (max.i_ >= ni_ - 1) max.i_ = ni_ - 2;
  if (max.j_ >= nj_ - 1) max.j_ = nj_ - 2;

  for (index_type i = min.i_; i <= max.i_; i++)
  {
    for (index_type j = min.j_; j <= max.j_; j++)
    {
      arr.push_back(typename Face::index_type(this, i,j));
    }
  }
}


template<class Basis>
void
ImageMesh<Basis>::get_center(Core::Geometry::Point &result,
                             const typename Node::index_type &idx) const
{
  Core::Geometry::Point p(idx.i_, idx.j_, 0.0);
  result = transform_.project(p);
}


template<class Basis>
void
ImageMesh<Basis>::get_center(Core::Geometry::Point &result,
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


template<class Basis>
void
ImageMesh<Basis>::get_center(Core::Geometry::Point &result,
                             const typename Face::index_type &idx) const
{
  Core::Geometry::Point p(idx.i_ + 0.5, idx.j_ + 0.5, 0.0);
  result = transform_.project(p);
}

template <class Basis>
int
ImageMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Node::array_type &l,
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
int
ImageMesh<Basis>::get_weights(const Core::Geometry::Point &p, typename Face::array_type &l,
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


/* To generate a random point inside of a triangle, we generate random
   barrycentric coordinates (independent random variables between 0 and
   1 that sum to 1) for the point. */

template<class Basis>
void
ImageMesh<Basis>::get_random_point(Core::Geometry::Point &p,
                                   const typename Elem::index_type &ci,
                                   FieldRNG &rng) const
{
  // Get the positions of the vertices.
  typename Node::array_type ra;
  get_nodes(ra,ci);
  Core::Geometry::Point p00, p10, p11, p01;
  get_center(p00,ra[0]);
  get_center(p10,ra[1]);
  get_center(p11,ra[2]);
  get_center(p01,ra[3]);
  Core::Geometry::Vector dx=p10-p00;
  Core::Geometry::Vector dy=p01-p00;

  // Generate the barycentric coordinates.
  const double u = rng();
  const double v = rng();

  // Compute the position of the random point.
  p = p00 + dx*u + dy*v;
}


template<class Basis>
const TypeDescription*
ImageMesh<Basis>::node_index_type_description()
{
  static TypeDescription* td = 0;
  if(!td){
    td = new TypeDescription(ImageMesh<Basis>::type_name(-1) +
                                std::string("::INodeIndex"),
                                std::string(__FILE__),
                                "SCIRun");
  }
  return td;
}


template<class Basis>
const TypeDescription*
ImageMesh<Basis>::face_index_type_description()
{
  static TypeDescription* td = 0;
  if(!td){
    td = new TypeDescription(ImageMesh<Basis>::type_name(-1) +
                                std::string("::IFaceIndex"),
                                std::string(__FILE__),
                                "SCIRun");
  }
  return td;
}


template<class Basis>
void
Pio(Piostream& stream, typename ImageMesh<Basis>::INodeIndex& n)
{
  stream.begin_cheap_delim();
  Pio(stream, n.i_);
  Pio(stream, n.j_);
  stream.end_cheap_delim();
}


template<class Basis>
void
Pio(Piostream& stream, typename ImageMesh<Basis>::IFaceIndex& n)
{
  stream.begin_cheap_delim();
  Pio(stream, n.i_);
  Pio(stream, n.j_);
  stream.end_cheap_delim();
}


#define IMAGEMESH_VERSION 4

template<class Basis>
void
ImageMesh<Basis>::io(Piostream& stream)
{
  int version = stream.begin_class(type_name(-1), IMAGEMESH_VERSION);

  Mesh::io(stream);

  // IO data members, in order
  if (version < 4)
  {
    unsigned int ni = static_cast<unsigned int>(ni_);
    unsigned int nj = static_cast<unsigned int>(nj_);
    Pio(stream, ni);
    Pio(stream, nj);
    ni_ = static_cast<size_type>(ni);
    nj_ = static_cast<size_type>(nj);
  }
  else
  {
    Pio_size(stream, ni_);
    Pio_size(stream, nj_);
  }

  if (version >= 2) {
    basis_.io(stream);
  }

  if (version >= 3) {
    Pio(stream, transform_);
  }

  stream.end_class();

  if (stream.reading())
  {
    compute_jacobian();
    vmesh_.reset(CreateVImageMesh(this));
  }
}


template<class Basis>
const std::string
ImageMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    static const std::string nm("ImageMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template<class Basis>
void
ImageMesh<Basis>::begin(typename ImageMesh::Node::iterator &itr) const
{
  itr = typename Node::iterator(this, min_i_, min_j_);
}


template<class Basis>
void
ImageMesh<Basis>::end(typename ImageMesh::Node::iterator &itr) const
{
  itr = typename Node::iterator(this, min_i_, min_j_ + nj_);
}


template<class Basis>
void
ImageMesh<Basis>::size(typename ImageMesh::Node::size_type &s) const
{
  s = typename Node::size_type(ni_, nj_);
}


template<class Basis>
void
ImageMesh<Basis>::to_index(typename ImageMesh::Node::index_type &idx, index_type a) const
{
  const index_type i = a % ni_;
  const index_type j = a / ni_;
  idx = typename Node::index_type(this, i, j);

}


template<class Basis>
void
ImageMesh<Basis>::begin(typename ImageMesh::Edge::iterator &itr) const
{
  itr = 0;
}


template<class Basis>
void
ImageMesh<Basis>::end(typename ImageMesh::Edge::iterator &itr) const
{
  itr = (ni_-1) * (nj_) + (ni_) * (nj_ -1);
}


template<class Basis>
void
ImageMesh<Basis>::size(typename ImageMesh::Edge::size_type &s) const
{
  s = (ni_-1) * (nj_) + (ni_) * (nj_ -1);
}


template<class Basis>
void
ImageMesh<Basis>::begin(typename ImageMesh::Face::iterator &itr) const
{
  itr = typename Face::iterator(this,  min_i_, min_j_);
}


template<class Basis>
void
ImageMesh<Basis>::end(typename ImageMesh::Face::iterator &itr) const
{
  itr = typename Face::iterator(this, min_i_, min_j_ + nj_ - 1);
}


template<class Basis>
void
ImageMesh<Basis>::size(typename ImageMesh::Face::size_type &s) const
{
  s = typename Face::size_type(ni_-1, nj_-1);
}


template<class Basis>
void
ImageMesh<Basis>::to_index(typename ImageMesh::Face::index_type &idx,
                                                          index_type a) const
{
  const index_type i = a % (ni_-1);
  const index_type j = a / (ni_-1);
  idx = typename Face::index_type(this, i, j);

}


template<class Basis>
void
ImageMesh<Basis>::begin(typename ImageMesh::Cell::iterator& /*itr*/) const
{
  ASSERTFAIL("This mesh type does not have cells use \"elem\".");
}


template<class Basis>
void
ImageMesh<Basis>::end(typename ImageMesh::Cell::iterator& /*itr*/) const
{
  ASSERTFAIL("This mesh type does not have cells use \"elem\".");
}


template<class Basis>
void
ImageMesh<Basis>::size(typename ImageMesh::Cell::size_type& /*s*/) const
{
  ASSERTFAIL("This mesh type does not have cells use \"elem\".");
}


template<class Basis>
const TypeDescription*
get_type_description(ImageMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = new TypeDescription("ImageMesh", subs,
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template<class Basis>
const TypeDescription*
ImageMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((ImageMesh *)0);
}


template<class Basis>
const TypeDescription*
ImageMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((ImageMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Node",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template<class Basis>
const TypeDescription*
ImageMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((ImageMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Edge",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template<class Basis>
const TypeDescription*
ImageMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((ImageMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Face",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template<class Basis>
const TypeDescription*
ImageMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((ImageMesh<Basis> *)0);
    td = new TypeDescription(me->get_name() + "::Cell",
                                std::string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template<class Basis>
bool
ImageMesh<Basis>::locate(typename Face::index_type &elem, const Core::Geometry::Point &p) const
{
  if (basis_.polynomial_order() > 1) return elem_locate(elem, *this, p);

  if (ni_ == 0 || nj_ == 0) return (false);

  const double epsilon = 1e-7;

  const Core::Geometry::Point r = transform_.unproject(p);

  double ii = r.x();
  double jj = r.y();

  const double nii = static_cast<double>(ni_-1);
  const double njj = static_cast<double>(nj_-1);

  if (ii>=nii && (ii-epsilon)<nii) ii=nii-epsilon;
  if (jj>=njj && (jj-epsilon)<njj) jj=njj-epsilon;

  if (ii<0 && ii>(-epsilon)) ii=0.0;
  if (jj<0 && jj>(-epsilon)) jj=0.0;

  const index_type i = static_cast<index_type>(floor(ii));
  const index_type j = static_cast<index_type>(floor(jj));

  if (i < (ni_-1) && i >= 0 &&
      j < (nj_-1) && j >= 0 &&
      ii >= 0.0 && jj >= 0.0)
  {
    elem.i_ = i;
    elem.j_ = j;
    elem.mesh_ = this;
    return (true);
  }

  return (false);
}


template<class Basis>
bool
ImageMesh<Basis>::locate(typename Node::index_type &node, const Core::Geometry::Point &p) const
{
  if (ni_ == 0 || nj_ == 0) return (false);

  const Core::Geometry::Point r = transform_.unproject(p);

  double rx = floor(r.x() + 0.5);
  double ry = floor(r.y() + 0.5);
  const double nii = static_cast<double>(ni_-1);
  const double njj = static_cast<double>(nj_-1);

  if (rx < 0.0) rx = 0.0; if (rx > nii) rx = nii;
  if (ry < 0.0) ry = 0.0; if (ry > njj) ry = njj;

  node.i_ = static_cast<index_type>(rx);
  node.j_ = static_cast<index_type>(ry);
  node.mesh_ = this;

  return (true);
}


template <class Basis>
bool
ImageMesh<Basis>::find_closest_node(double& pdist,
                                    Core::Geometry::Point &result,
                                    typename Node::index_type &node,
                                    const Core::Geometry::Point &p) const
{
  if (ni_ == 0 || nj_ == 0) return (false);

  const Core::Geometry::Point r = transform_.unproject(p);

  double rx = floor(r.x() + 0.5);
  double ry = floor(r.y() + 0.5);

  const double nii = static_cast<double>(ni_-1);
  const double njj = static_cast<double>(nj_-1);

  if (rx < 0.0) rx = 0.0; if (rx > nii) rx = nii;
  if (ry < 0.0) ry = 0.0; if (ry > njj) ry = njj;

  result = transform_.project(Core::Geometry::Point(rx,ry,0.0));
  node.i_ = static_cast<index_type>(rx);
  node.j_ = static_cast<index_type>(ry);
  node.mesh_ = this;

  pdist = (p-result).length();
  return (true);
}


template <class Basis>
bool
ImageMesh<Basis>::find_closest_node(double& pdist,
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
ImageMesh<Basis>::find_closest_elems(double& pdist,
                                     Core::Geometry::Point &result,
                                     std::vector<typename Elem::index_type> &elems,
                                     const Core::Geometry::Point &p) const
{
  if (ni_ == 0 || nj_ == 0) return (false);

  elems.clear();

  const double epsilon = 1e-8;

  const Core::Geometry::Point r = transform_.unproject(p);

  double ii = r.x();
  double jj = r.y();
  const double nii = static_cast<double>(ni_-2);
  const double njj = static_cast<double>(nj_-2);

  if (ii < 0.0) ii = 0.0; if (ii > nii) ii = nii;
  if (jj < 0.0) jj = 0.0; if (jj > njj) jj = njj;

  const double fii = floor(ii);
  const double fjj = floor(jj);

  index_type i = static_cast<index_type>(fii);
  index_type j = static_cast<index_type>(fjj);

  typename Elem::index_type elem;

  elem.i_ = i;
  elem.j_ = j;
  elem.mesh_ = this;
  elems.push_back(elem);

  if ((fabs(fii-ii) < epsilon) && ((i-1)>0))
  {
    elem.i_ = i-1;
    elem.j_ = j;
    elems.push_back(elem);
  }

  if ((fabs(fii-(ii+1.0)) < epsilon) && (i<(ni_-1)))
  {
    elem.i_ = i+1;
    elem.j_ = j;
    elems.push_back(elem);
  }

  if ((fabs(fjj-jj) < epsilon) && ((j-1)>0))
  {
    elem.i_ = i;
    elem.j_ = j-1;
    elems.push_back(elem);
  }

  if ((fabs(fjj-(jj+1.0)) < epsilon) && (j<(nj_-1)))
  {
    elem.i_ = i;
    elem.j_ = j+1;
    elems.push_back(elem);
  }

  result = transform_.project(Core::Geometry::Point(ii,jj,0));

  pdist = (p-result).length();
  return (true);
}

template <class Basis>
void
ImageMesh<Basis>::compute_jacobian()
{
  if (basis_.polynomial_order() < 2)
  {

    Core::Geometry::Vector J1 = transform_.project(Core::Geometry::Vector(1.0,0.0,0.0));
    Core::Geometry::Vector J2 = transform_.project(Core::Geometry::Vector(0.0,1.0,0.0));
    Core::Geometry::Vector J3 = Cross(J1,J2);
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

} // namespace SCIRun

#endif
