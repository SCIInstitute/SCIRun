/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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


#ifndef CORE_DATATYPES_VMESH_H
#define CORE_DATATYPES_VMESH_H

#include <Core/GeometryPrimitives/Transform.h>
#include <Core/Containers/StackBasedVector.h>
#include <Core/Containers/StackVector.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/FieldVIndex.h>
#include <Core/Datatypes/Legacy/Field/FieldVIterator.h>

#include <Core/GeometryPrimitives/SearchGridT.h>

#include <Core/Utils/Legacy/Debug.h>

#include <Core/Datatypes/Legacy/Field/share.h>

namespace SCIRun {

class VMesh;
class TypeDescription;

typedef boost::shared_ptr<VMesh> VMeshHandle;

class SCISHARE VMesh {
public:

  /// VIRTUAL INTERFACE
  
  /// typedefs for Node, Edge, Face and Cell
  /// These are the basic types that we use, they are typedefed
  /// so we can alter the underlying type easily
  
  /// All indices are always of index_type
  typedef Mesh::index_type                        index_type;
  /// Weights for interpolation
  typedef double                                  weight_type;
  /// size_type and index_type need to be the same
  /// but they have their own type to make the code more readable
  typedef Mesh::size_type                         size_type;
  /// Array of indices
  typedef std::vector<index_type>                 array_type;
  /// Array of points
  typedef std::vector<Core::Geometry::Point>                      points_type;
  /// Dimensions of a mesh, these are used for the regular grids
  /// Irregular grids only have one dimension, namely the number
  /// of nodes or elements
  typedef std::vector<size_type>                  dimension_type;
  /// A stack vector is a faster class than vector but is limited
  /// to a maximum number of entries
  /// coords_type is used to denote the location inside an element
  /// in local coordinates
  typedef StackVector<double,3>                   coords_type;
  /// An array of coords
  typedef std::vector<StackVector<double,3> >          coords_array_type;
  /// 2D array of coords
  typedef std::vector<std::vector<StackVector<double,3> > > coords_array2_type;
  /// Derivative of points each component contains the x,y, and z
  /// derivative of the point. This is used in higher order elements
  typedef StackVector<Core::Geometry::Point,3>                    dpoints_type;
  /// Mask type for setting bits. This one is set to special type
  /// to remind the user that only bit operations asre valid on masks
  typedef unsigned int                            mask_type;
  
  /// A dual vector will store the first entries on the stack,
  /// if extra space is needed it will create vector to store
  /// data on. This way most operations are done directly from
  /// the stack, only in less common situations memory is allocated
  /// and is used to store data. Here we allow 12 spaces to be on the
  /// stack which should be enough.
  typedef StackBasedVector<index_type,12>         index_array_type;
  typedef StackBasedVector<double,12>             weight_array_type;
  
  /// Virtual indices, iterators, and arrays
  /// Class for indexing nodes
  class Node { 
    public:
      typedef VNodeIterator<VMesh::index_type>   iterator;
      typedef VNodeIndex<VMesh::index_type>      index_type;
      typedef VNodeIndex<VMesh::size_type>       size_type;
      typedef StackBasedVector<index_type,8>     array_type;
  };

  typedef std::vector<Node::array_type>                nodes_array_type;

  /// Class for indexing edge nodes
  /// These are used in quadratic approaches
  class ENode { 
    public:
      typedef VENodeIterator<VMesh::index_type>   iterator;
      typedef VENodeIndex<VMesh::index_type>      index_type;
      typedef VENodeIndex<VMesh::size_type>       size_type;
      typedef StackBasedVector<index_type,12>     array_type;
  };

  /// Class for indexing edges
  class Edge { 
    public:
      typedef VEdgeIterator<VMesh::index_type>   iterator;
      typedef VEdgeIndex<VMesh::index_type>      index_type;
      typedef VEdgeIndex<VMesh::size_type>       size_type;
      typedef StackBasedVector<index_type,12>    array_type;
  };
  
  /// Class for indexing faces
  class Face { 
    public:
      typedef VFaceIterator<VMesh::index_type>   iterator;    
      typedef VFaceIndex<VMesh::index_type>      index_type;
      typedef VFaceIndex<VMesh::size_type>       size_type;
      typedef StackBasedVector<index_type,12>    array_type;
  }; 
  
  /// Class for indexing cells
  /// Note: this is one is here for completion, but one should in most cases
  /// use Elem which denotes the elements of the mesh. Elem indices can be used
  /// in more function calls and are therefore more useful
  class Cell { 
    public:
      typedef VCellIterator<VMesh::index_type>   iterator;
      typedef VCellIndex<VMesh::index_type>      index_type;
      typedef VCellIndex<VMesh::size_type>       size_type;
      typedef StackBasedVector<index_type,12>    array_type;
  };
  
  /// Class for indexing elements
  /// Elements denote the highest topology in the mesh.
  /// For example for TriSurfMeshes it denotes the faces of the surface, however
  /// for the HexVolMesh it denotes the volumetric elements

  /// Elem has become a replacement for most Edge, Face, and Cell index types
  /// and can be used in more functions. The reason this one is preferred is that
  /// it allows the user to write code that works on any type of mesh without
  /// having to think about the dimensionality of the elements.
  /// Only when dimensionality is important the Edge, Face, and Cell indices
  /// should be used.
  class Elem { 
    public:
      typedef VElemIterator<VMesh::index_type>   iterator;
      typedef VElemIndex<VMesh::index_type>      index_type;
      typedef VElemIndex<VMesh::size_type>       size_type;
      typedef StackBasedVector<index_type,12>    array_type;
  };
  
  
  /// Class for indexing topological derivative of elements
  /// Hence for a TriSurfMesh these are the edges of the mesh, and
  /// for a HexVolMesh these are the faces of the mesh.
  /// This class is generally used to do neighborhood and boundary
  /// algorithms. Note that for a PointCloudMesh the derivative is defined
  /// as a Node and both Elem, Node, and DElem are in fact the same.
  class DElem { 
    public:
      typedef VDElemIterator<VMesh::index_type>  iterator;    
      typedef VDElemIndex<VMesh::index_type>     index_type;
      typedef VDElemIndex<VMesh::size_type>      size_type;
      typedef StackBasedVector<index_type,12>    array_type;
 };


  /// All information needed to do one interpolation inside an element
  /// This class is mostly for internal use in interpolation functions.
  /// It is used sporadically in algorithms to speed up the code by preallocating
  /// this structure
  class ElemInterpolate {
    public:
      typedef VMesh::index_type   index_type;      
      typedef VMesh::size_type    size_type;      
      int                         basis_order; // which order was created
      index_type                  elem_index; // which element
      StackVector<index_type,8>   node_index; // nodes that span the element
      StackVector<index_type,12>  edge_index; // edges that span the element
      size_type                   num_hderivs; // number of derivatives per node
      StackBasedVector<double,64> weights;     // weights for given points and derivatives
  };
  
  /// We have special code for doing multiple interpolations at the same time
  /// all for speeding up the code and limit the number of virtual function
  /// calls.
  typedef std::vector<ElemInterpolate> MultiElemInterpolate;  
    
  // All information needed to do one interpolation inside
  // one element to determine the gradient.    
  class ElemGradient {
  public:
      typedef VMesh::index_type    index_type;      
      typedef VMesh::size_type     size_type;      
      int                          basis_order; // which order was created
      index_type                   elem_index; // which element
      StackBasedVector<index_type,8>    node_index; // nodes that span the element
      StackVector<index_type,12>   edge_index; // edges that span the element
      size_type                    num_hderivs; // Number of derivatives per point
      StackBasedVector<double,64>  weights;    // weights for given points

      size_type                    num_derivs; // Number of derivatives in topology   
      StackVector<double,9>        inverse_jacobian; // Inverse jacobian, for local to global transformation
      coords_type                  coords;
  };  

  /// Multiple gradient calculations in parallel.
  typedef std::vector<ElemGradient> MultiElemGradient;  
    
  /// instantiate the element information
  /// as these are protected the various derived constructors can fill
  /// these out.
  VMesh() :
    basis_order_(0),
    dimension_(0),
    has_normals_(false),
    is_editable_(false),
    is_regular_(false),
    is_structured_(false),
    num_nodes_per_elem_(0),
    num_enodes_per_elem_(0),
    num_edges_per_elem_(0),
    num_faces_per_elem_(0),
    num_nodes_per_face_(0),
    num_edges_per_face_(0)
  {
    /// This call is only made in DEBUG mode, to keep a record of all the
    /// objects that are being allocated and freed.
    DEBUG_CONSTRUCTOR("VMesh")
  }

  /// Destructor needs to be virtual to ensure that we can delete the full
  /// virtual interface from the VMesh interface
  virtual ~VMesh() 
  {
    /// This call is only made in DEBUG mode, to keep a record of all the
    /// objects that are being allocated and freed.
    DEBUG_DESTRUCTOR("VMesh")  
  }

  /// iterators for the virtual topology indices. These are not strictly needed
  /// but make the concepts in line with previous version. All iterators now
  /// go from 0 to number of elements, using consecutive unique numbers   

  inline void begin(Node::iterator &it) const
    { it = 0; }
  inline void begin(ENode::iterator &it) const
    { it = 0; }
  inline void begin(Edge::iterator &it) const
    { it = 0; }
  inline void begin(Face::iterator &it) const
    { it = 0; }
  inline void begin(Cell::iterator &it) const
    { it = 0; }
  inline void begin(Elem::iterator &it) const
    { it = 0; }
  inline void begin(DElem::iterator &it) const
    { it = 0; }

  inline void end(Node::iterator &it) const
    { Node::size_type s; size(s); it = static_cast<index_type>(s); }
  inline void end(ENode::iterator &it) const
    { Node::size_type s; size(s); it = static_cast<index_type>(s); }
  inline void end(Edge::iterator &it) const
    { Edge::size_type s; size(s); it = static_cast<index_type>(s); }
  inline void end(Face::iterator &it) const
    { Face::size_type s; size(s); it = static_cast<index_type>(s); }
  inline void end(Cell::iterator &it) const
    { Cell::size_type s; size(s); it = static_cast<index_type>(s); }
  inline void end(Elem::iterator &it) const
    { Elem::size_type s; size(s); it = static_cast<index_type>(s); }
  inline void end(DElem::iterator &it) const
    { DElem::size_type s; size(s); it = static_cast<index_type>(s); }

  
  /// Get the number of elements in the mesh of the specified type
  /// Note: that for any size other then the number of nodes or
  /// elements, one has to synchronize that part of the mesh.
  virtual void size(Node::size_type& size) const;
  virtual void size(ENode::size_type& size) const;
  virtual void size(Edge::size_type& size) const;
  virtual void size(Face::size_type& size) const;
  virtual void size(Cell::size_type& size) const;
  virtual void size(Elem::size_type& size) const;
  virtual void size(DElem::size_type& size) const;
    
  /// We have been using the num_#type#() in other functions as well to
  /// determine the number of nodes, edges etc. These are just shortcuts to
  /// make programs more readable
  inline size_type num_nodes() const
    { Node::index_type s; size(s); return(static_cast<size_t>(s)); }
  inline size_type num_enodes() const
    { ENode::index_type s; size(s); return(static_cast<size_t>(s)); }
  inline size_type num_edges() const
    { Edge::index_type s; size(s); return(static_cast<size_t>(s)); }
  inline size_type num_faces() const
    { Face::index_type s; size(s); return(static_cast<size_t>(s)); }
  inline size_type num_cells() const
    { Cell::index_type s; size(s); return(static_cast<size_t>(s)); }
  inline size_type num_elems() const
    { Elem::index_type s; size(s); return(static_cast<size_t>(s)); }
  inline size_type num_delems() const
    { DElem::index_type s; size(s); return(static_cast<size_t>(s)); }  
  
  /// NOTE NOT VALID FOR EACH MESH:
  virtual boost::shared_ptr<SearchGridT<SCIRun::index_type> > get_elem_search_grid();
  virtual boost::shared_ptr<SearchGridT<SCIRun::index_type> > get_node_search_grid();

  /// test for special case where the mesh is empty
  /// empty meshes may need a special treatment
  inline bool is_empty() const
    { Node::index_type s; size(s); return (s == 0); }
  
  /// Topological functions: note that currently most meshes have an incomplete
  /// set implemented. Currently each mesh has:
  /// Getting cell, face, edge indices from node indices
  /// Getting the indices of the elements that are topologically building up the
  /// the element: e.g. one can derive faces from a cell index but not YET 
  /// vice versa. 
  
  /// Get the nodes that make up an element
  /// Depending on the geometry not every function may be available
  virtual void get_nodes(Node::array_type& nodes, Node::index_type i) const;
  virtual void get_nodes(Node::array_type& nodes, Edge::index_type i) const;
  virtual void get_nodes(Node::array_type& nodes, Face::index_type i) const;
  virtual void get_nodes(Node::array_type& nodes, Cell::index_type i) const;
  virtual void get_nodes(Node::array_type& nodes, Elem::index_type i) const;
  virtual void get_nodes(Node::array_type& nodes, DElem::index_type i) const;


  /// Get the nodes that make up an element
  /// Depending on the geometry not every function may be available
  virtual void get_enodes(ENode::array_type& nodes, Node::index_type i) const;
  virtual void get_enodes(ENode::array_type& nodes, Edge::index_type i) const;
  virtual void get_enodes(ENode::array_type& nodes, Face::index_type i) const;
  virtual void get_enodes(ENode::array_type& nodes, Cell::index_type i) const;
  virtual void get_enodes(ENode::array_type& nodes, Elem::index_type i) const;
  virtual void get_enodes(ENode::array_type& nodes, DElem::index_type i) const;
  

  /// Get the edges that make up an element
  /// or get the edges that contain certain nodes
  /// Depending on the geometry not every function may be available
  virtual void get_edges(Edge::array_type& edges, Node::index_type i) const;
  virtual void get_edges(Edge::array_type& edges, Edge::index_type i) const;
  virtual void get_edges(Edge::array_type& edges, Face::index_type i) const;
  virtual void get_edges(Edge::array_type& edges, Cell::index_type i) const;
  virtual void get_edges(Edge::array_type& edges, Elem::index_type i) const;
  virtual void get_edges(Edge::array_type& edges, DElem::index_type i) const;


  /// Get the faces that make up an element
  /// or get the faces that contain certain nodes or edges
  /// Depending on the geometry not every function may be available
  virtual void get_faces(Face::array_type& faces, Node::index_type i) const;
  virtual void get_faces(Face::array_type& faces, Edge::index_type i) const;
  virtual void get_faces(Face::array_type& faces, Face::index_type i) const;
  virtual void get_faces(Face::array_type& faces, Cell::index_type i) const;
  virtual void get_faces(Face::array_type& faces, Elem::index_type i) const;
  virtual void get_faces(Face::array_type& faces, DElem::index_type i) const;

  /// Get the cell index that contains the specified component
  /// Depending on the geometry not every function may be available
  virtual void get_cells(Cell::array_type& cells, Node::index_type i) const;
  virtual void get_cells(Cell::array_type& cells, Edge::index_type i) const;
  virtual void get_cells(Cell::array_type& cells, Face::index_type i) const;
  virtual void get_cells(Cell::array_type& cells, Cell::index_type i) const;
  virtual void get_cells(Cell::array_type& cells, Elem::index_type i) const;
  virtual void get_cells(Cell::array_type& cells, DElem::index_type i) const;

  /// Get the element index that contains the specified component
  /// Depending on the geometry not every function may be available
  virtual void get_elems(Elem::array_type& elems, Node::index_type i) const;
  virtual void get_elems(Elem::array_type& elems, Edge::index_type i) const;
  virtual void get_elems(Elem::array_type& elems, Face::index_type i) const;
  virtual void get_elems(Elem::array_type& elems, Cell::index_type i) const;
  virtual void get_elems(Elem::array_type& elems, Elem::index_type i) const;
  virtual void get_elems(Elem::array_type& elems, DElem::index_type i) const;

  /// Get the derived element index that contains the specified component
  /// Depending on the geometry not every function may be available
  virtual void get_delems(DElem::array_type& delems, Node::index_type i) const;
  virtual void get_delems(DElem::array_type& delems, Edge::index_type i) const;
  virtual void get_delems(DElem::array_type& delems, Face::index_type i) const;
  virtual void get_delems(DElem::array_type& delems, Cell::index_type i) const;
  virtual void get_delems(DElem::array_type& delems, Elem::index_type i) const;
  virtual void get_delems(DElem::array_type& delems, DElem::index_type i) const;

  /// Get the topology index from the vertex indices
  virtual bool get_elem(Elem::index_type& elem, Node::array_type& nodes) const;
  virtual bool get_delem(DElem::index_type& delem, Node::array_type& nodes) const;
  virtual bool get_cell(Cell::index_type& cell, Node::array_type& nodes) const;
  virtual bool get_face(Face::index_type& face, Node::array_type& nodes) const;
  virtual bool get_edge(Edge::index_type& edge, Node::array_type& nodes) const;

  /// Get the center of a certain mesh element
  virtual void get_center(Core::Geometry::Point &point, Node::index_type i) const;
  virtual void get_center(Core::Geometry::Point &point, ENode::index_type i) const;
  virtual void get_center(Core::Geometry::Point &point, Edge::index_type i) const;
  virtual void get_center(Core::Geometry::Point &point, Face::index_type i) const;
  virtual void get_center(Core::Geometry::Point &point, Cell::index_type i) const;
  virtual void get_center(Core::Geometry::Point &point, Elem::index_type i) const;
  virtual void get_center(Core::Geometry::Point &point, DElem::index_type i) const;

  /// Get the centers of a series of nodes
  virtual void get_centers(Core::Geometry::Point* points, const Node::array_type& array) const;
  virtual void get_centers(Core::Geometry::Point* points, const Elem::array_type& array) const;

  /// Get the centers of a series of nodes, with points in an STL vector
  /// These just overload the function calls defined above.
  inline void get_centers(points_type &points, const Node::array_type& array) const
    { points.resize(array.size()); get_centers(&(points[0]),array); }
  inline void get_centers(points_type &points, const Elem::array_type& array) const
    { points.resize(array.size()); get_centers(&(points[0]),array); }


  inline void get_all_node_centers(points_type &points) const
    { 
      Node::size_type sz = num_nodes();
      Node::array_type array(sz);
      for (Node::index_type k=0;k<sz;k++) array[k] = k;
      points.resize(array.size()); get_centers(&(points[0]),array); 
    }
  inline void get_all_elem_centers(points_type &points) const
    { 
      Elem::size_type sz = num_elems();
      Elem::array_type array(sz);
      for (Elem::index_type k=0;k<sz;k++) array[k] = k;
      points.resize(array.size()); get_centers(&(points[0]),array); 
    }


  /// Get the geometrical sizes of the mesh elements
  /// For nodes and enodes there is no size, hence predefine them in case
  /// some makes the call
  inline  double get_size(Node::index_type /*i*/) const 
    { return (0.0); }
  inline  double get_size(ENode::index_type /*i*/) const
    { return (0.0); }
    
  /// Get the geometrical size of topological mesh components  
  virtual double get_size(Edge::index_type i) const;
  virtual double get_size(Face::index_type i) const;
  virtual double get_size(Cell::index_type i) const;
  virtual double get_size(Elem::index_type i) const;
  virtual double get_size(DElem::index_type i) const;

  /// Get the size of an element. This function is intended to check for
  /// zero volume elements before adding an element to a mesh.
  virtual double get_size(Node::array_type& array) const;

  /// alternative ways to get the size values
  /// Again these are all defined inline so we do not get additional virtual
  /// calls in the interface an to reduce the overhead in the actual implementation
  /// of the virtual interface mesh classes.
  inline double get_length(Edge::index_type i) const
  { return (get_size(i)); }
  inline double get_area(Face::index_type i) const
  { return (get_size(i)); }
  inline double get_volume(Cell::index_type i) const
  { return (get_size(i)); }

  inline double get_length(Elem::index_type i) const
  { return (get_size(i)); }
  inline double get_area(Elem::index_type i) const
  { return (get_size(i)); }
  inline double get_volume(Elem::index_type i) const
  { return (get_size(i)); }
    
  /// Specialized functions to get weights for the interpolation
  /// One should use these instead of get_weights
  
  /// These functions fill out the interpolation structure, so that one can
  /// used in the code. The interpolation structures know about basis order
  /// and hence will do the right interpolation when given to a function of
  /// VField for interpolation
  virtual void get_interpolate_weights(const Core::Geometry::Point& p, 
                                       ElemInterpolate& ei, 
                                       int basis_order) const;
  
  virtual void get_interpolate_weights(const coords_type& coords, 
                                       Elem::index_type elem, 
                                       ElemInterpolate& ei,
                                       int basis_order) const;

  virtual void get_minterpolate_weights(const std::vector<Core::Geometry::Point>& p, 
                                        MultiElemInterpolate& ei, 
                                        int basis_order) const;

  virtual void get_minterpolate_weights(const std::vector<coords_type>& coords, 
                                        Elem::index_type elem, 
                                        MultiElemInterpolate& ei,
                                        int basis_order) const;
                                        
  /// Same functions but now for determining gradients                                        
  virtual void get_gradient_weights(const Core::Geometry::Point& p, 
                                    ElemGradient& ei, 
                                    int basis_order) const;
  
  virtual void get_gradient_weights(const coords_type& coords, 
                                    Elem::index_type elem, 
                                    ElemGradient& ei,
                                    int basis_order) const;

  virtual void get_mgradient_weights(const std::vector<Core::Geometry::Point>& p, 
                                     MultiElemGradient& ei, 
                                     int basis_order) const;

  virtual void get_mgradient_weights(const std::vector<coords_type>& coords, 
                                    Elem::index_type elem, 
                                    MultiElemGradient& eg,
                                    int basis_order) const;

  /// Old get_weights function, in this case the user needs to know about
  /// basis order and the basis order that is needed must be given.
  /// Also this function implies a certain ordering of the weights and this
  /// function only remains in order to provide a sense of backwards compatiblity. 
  virtual void get_weights(const coords_type& coords, 
                           std::vector<double>& weights,
                           int basis_order) const;                                 

  /// Old get_derivate_weights function, which returns weights but now for computing
  /// the gradient of a field.
  virtual void get_derivate_weights(const coords_type& coords, 
                           std::vector<double>& weights,
                           int basis_order) const;

  /// These two functions specify a sampling scheme for interpolation, 
  /// intergration etc. Currently it supports 3 Gaussian schemes that define
  /// the gaussian quadrature points for the meshes. Specify the order between
  /// 1 and 3.
  /// The regular sampling is intended for operations other than integration and
  /// sample using a regular scheme inside the element. Currently an order between
  /// 1 and 5 are supported to indicate the number of samples along one of the
  /// edges. Hence scheme 1 only gives one point in the center and scheme 5 can
  /// generate up to 125 sampling points depending on the topology of the element.

  virtual void get_gaussian_scheme(std::vector<VMesh::coords_type>& coords, 
                                   std::vector<double>& weights, int order) const;
                                   
  virtual void get_regular_scheme(std::vector<VMesh::coords_type>& coords, 
                                  std::vector<double>& weights, int order) const;

///////////////

  /// Locate where a position is in  the mesh
  /// The node version finds the closest node
  /// The element version find the element that contains the point
  virtual bool locate(VMesh::Node::index_type &i, const Core::Geometry::Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, const Core::Geometry::Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, 
                      VMesh::coords_type &coords, const Core::Geometry::Point& point) const;

  /// multi locate functions. An 'm' in front of a function tends to denote
  /// that this function is vectorized for convenience. Depending on the
  /// underlying functionality it calls the single case multiple times
  /// or adds some optimization. In this case optimization occurs by assuming
  /// the points are close together and previous node or element indices are
  /// tested first to see if that is the index for the next one in the array
  /// Hence in optimal cases the search is reduced to a few points for a cloud
  /// of points

  virtual void mlocate(std::vector<Node::index_type> &i, 
                       const std::vector<Core::Geometry::Point> &point) const;
  virtual void mlocate(std::vector<Elem::index_type> &i, 
                       const std::vector<Core::Geometry::Point> &point) const;

  /// Find elements that are inside or close to the bounding box. This function
  /// uses the underlying search structure to find candidates that are close.
  /// This functionality is general intended to speed up searching for elements
  /// in a certain region.
  virtual bool locate(VMesh::Elem::array_type &a, const Core::Geometry::BBox& bbox) const;

  /// Find the closest point on a surface or a curve
  
  /// These functions return the closest node to a certain point
  /// It returns the distance and the node index and the location of the
  /// node. 
  virtual bool find_closest_node(double& dist,
                                 Core::Geometry::Point& result,
                                 VMesh::Node::index_type &i, 
                                 const Core::Geometry::Point &point) const; 

  /// This version uses a maximum radius for searching the node
  /// If no nodes are within radius the function returns false.
  virtual bool find_closest_node(double& dist,
                                 Core::Geometry::Point& result,
                                 VMesh::Node::index_type &i, 
                                 const Core::Geometry::Point &point,
                                 double maxdist) const; 

  /// Simplified version that does not return the distance
  inline bool find_closest_node(Core::Geometry::Point& result,
                                VMesh::Node::index_type& i,
                                const Core::Geometry::Point &point,
                                double maxdist)
    { double dist; return(find_closest_node(dist,result,i,point,maxdist));}

  /// Simplified version that does not return the distance
  inline bool find_closest_node(Core::Geometry::Point& result,
                                VMesh::Node::index_type& i,
                                const Core::Geometry::Point &point)
    { double dist; return(find_closest_node(dist,result,i,point));}

  /// Find the nodes within a spherical region arounf point.
  /// It returns the indices to the nodes
  virtual bool find_closest_nodes(std::vector<VMesh::Node::index_type>& nodes,
                                  const Core::Geometry::Point& p, double maxdist) const;

  virtual bool find_closest_nodes(std::vector<double>& distances,
                                  std::vector<VMesh::Node::index_type>& nodes,
                                  const Core::Geometry::Point& p, 
                                  double maxdist) const;

  /// Find the closest location that is part of an element
  /// This function returns the projection point on the mesh
  /// and the euclidian distance to the field which is the distance
  /// between point and the projection.
  /// It also returns the local coordinates of the element where the 
  /// projection point is located and the element index.
  /// if no elements are found the function returns false.
  virtual bool find_closest_elem(double &dist,
                                 Core::Geometry::Point &result,
                                 VMesh::coords_type &coords,
                                 VMesh::Elem::index_type &i, 
                                 const Core::Geometry::Point &point) const; 

  /// Same function, but now limited to a certain search ratius.
  /// if no elements are found the function returns false.
  virtual bool find_closest_elem(double &dist,
                                 Core::Geometry::Point &result,
                                 VMesh::coords_type &coords,
                                 VMesh::Elem::index_type &i, 
                                 const Core::Geometry::Point &point,
                                 double maxdist) const; 
                                 
  /// Simplified version that does not return the local coordinates.
  inline  bool find_closest_elem(double &dist,
                                 Core::Geometry::Point &result,
                                 VMesh::Elem::index_type &i, 
                                 const Core::Geometry::Point &point) const
  { 
    VMesh::coords_type coords; 
    return(find_closest_elem(dist,result,coords,i,point));
  }

  inline  bool find_closest_elem(double &dist,
                                 Core::Geometry::Point &result,
                                 VMesh::Elem::index_type &i, 
                                 const Core::Geometry::Point &point,
                                 double maxdist) const
  { 
    VMesh::coords_type coords; 
    return(find_closest_elem(dist,result,coords,i,point,maxdist));
  }


  /// Even more simplified version
  inline  bool find_closest_elem(Core::Geometry::Point &result,
                                 VMesh::Elem::index_type &i, 
                                 const Core::Geometry::Point &point) const
  { 
    double dist;
    VMesh::coords_type coords; 
    return(find_closest_elem(dist,result,coords,i,point));
  }

  /// Another simplified version
  inline  bool find_closest_elem(VMesh::coords_type &coords,
                                 VMesh::Elem::index_type &i, 
                                 const Core::Geometry::Point &point) const
  { 
    double dist;
    Core::Geometry::Point result;
    return(find_closest_elem(dist,result,coords,i,point));
  }


  /// @todo: Need to reformulate this one, closest element can have multiple 
  // intersection points
  virtual bool find_closest_elems(double& dist,
                                  Core::Geometry::Point& result,
                                  VMesh::Elem::array_type &i, 
                                  const Core::Geometry::Point &point) const; 

  /// Find the coordinates of a point in a certain element
  virtual bool get_coords(coords_type& coords, 
                                const Core::Geometry::Point &point, Elem::index_type i) const;
  
  /// Interpolate from local coordinates to global coordinates
  virtual void interpolate(Core::Geometry::Point &p, 
                         const coords_type& coords, Elem::index_type i) const;

  /// Multiple interpolations from local coordinates to global coordinates
  virtual void minterpolate(std::vector<Core::Geometry::Point> &p, 
                            const std::vector<coords_type>& coords, 
                            Elem::index_type i) const;

  /// Interpolate from local coordinates to a derivative in local coordinates  
  virtual void derivate(dpoints_type &p, 
                         const coords_type& coords, Elem::index_type i) const;
  
  /// Get the normal to an element if this is a valid operation.
  /// This is only implemented for surface and volume meshes.
  /// As elements can be curved the coordinates specify where on the
  /// element one wants to compute the normal.
  
  /// This is the volumetric version where one has to specify as well which
  /// face is involved.
  virtual void get_normal(Core::Geometry::Vector &result, coords_type& coords, 
                                 Elem::index_type eidx, DElem::index_type fidx) const;
  
  /// This is the surface version.
  inline void get_normal(Core::Geometry::Vector &result, coords_type& coords, 
                                 Elem::index_type eidx) const
    { get_normal(result,coords,eidx,0); }

  /// Multiple normals short cut
  inline void get_normals(std::vector<Core::Geometry::Vector> &result, std::vector<coords_type>& coords, 
                                 Elem::index_type eidx) const
    { result.resize(coords.size()); for (size_t j=0; j<coords.size(); j++)get_normal(result[j],coords[j],eidx,0); }
    
  /// Set and get a node location.
  /// Node set is only available for editable meshes
  
  /// Get the location of a random point inside the mesh
  virtual void get_random_point(Core::Geometry::Point &p, 
                                Elem::index_type i,FieldRNG &rng) const;
                                
  /// Get the location of a point. As the old interface used both get_point and
  /// get_center, these are short cuts to the one implementation that is done
  /// under the name get_center.                             
  inline  void get_point(Core::Geometry::Point &point, Node::index_type i) const
    { get_center(point,i); }
  inline  void get_point(Core::Geometry::Point &point, ENode::index_type i) const
    { get_center(point,i); }
  inline Core::Geometry::Point get_point(Node::index_type i) const
    { Core::Geometry::Point p; get_point(p,i); return (p); } 
  inline Core::Geometry::Point get_point(ENode::index_type i) const
    { Core::Geometry::Point p; get_point(p,i); return (p); } 
      
  /// Set the location of a point.
  /// Note: one must be the single user of the mesh to do this
  virtual void set_point(const Core::Geometry::Point &point, Node::index_type i);
  virtual void set_point(const Core::Geometry::Point &point, ENode::index_type i);
  
  
  /// These should only be used to speed up code within proper wrappers and
  /// after checking the type of the underlying mesh as they allow direct
  /// access to the mesh memory
  
  // Only for irregular data
  virtual Core::Geometry::Point* get_points_pointer() const;
  // Only for unstructured data
  virtual VMesh::index_type* get_elems_pointer() const;
  
  /// Copy nodes from one mesh to another mesh
  /// Note: currently only for irregular meshes
  /// @todo: Add regular meshes to the mix
  inline void copy_nodes(VMesh* imesh, Node::index_type i, 
                          Node::index_type o,Node::size_type size)
  {
    Core::Geometry::Point* ipoint = imesh->get_points_pointer();
    Core::Geometry::Point* opoint = get_points_pointer();
    for (index_type j=0; j<size; j++,i++,o++ ) opoint[o] = ipoint[i];
  }

  inline void copy_nodes(VMesh* imesh)
  {
    size_type size = imesh->num_nodes();
    resize_nodes(size);
    Core::Geometry::Point* ipoint = imesh->get_points_pointer();
    Core::Geometry::Point* opoint = get_points_pointer();
    for (index_type j=0; j<size; j++) opoint[j] = ipoint[j];
  }
  
  inline void copy_elems(VMesh* imesh, Elem::index_type i, 
                          Elem::index_type o,Elem::size_type size,
                          Elem::size_type offset)
  {
    VMesh::index_type* ielem = imesh->get_elems_pointer();
    VMesh::index_type* oelem  = get_elems_pointer();
    index_type ii = i*num_nodes_per_elem_;
    index_type oo = o*num_nodes_per_elem_;
    size_type  ss = size*num_nodes_per_elem_;
    for (index_type j=0; j <ss; j++,ii++,oo++) oelem[oo] = ielem[ii]+offset;
  }

  inline void copy_elems(VMesh* imesh)
  {
    VMesh::index_type* ielem = imesh->get_elems_pointer();
    VMesh::index_type* oelem  = get_elems_pointer();
    size_type  ss = num_elems()*num_nodes_per_elem_;
    for (index_type j=0; j <ss; j++) oelem[j] = ielem[j];
  }
  
  /// Preallocate memory for better performance
  /// We have two versions for historic reasons
  virtual void node_reserve(size_t size);
  inline  void reserve_nodes(size_t size) 
    { node_reserve(size); }
  
  /// reserve memory by specifying the number of elements that is expected
  virtual void elem_reserve(size_t size);
  inline  void reserve_elems(size_t size) 
    { elem_reserve(size); }
  
  /// Actually resize the arrays.
  /// Note: this is limited to certain meshes
  virtual void resize_nodes(size_t size);
  virtual void resize_elems(size_t size);
  inline  void resize_points(size_t size) { resize_nodes(size); }

  /// Add a node to a mesh
  virtual void add_node(const Core::Geometry::Point &point,Node::index_type &i);
  virtual void add_enode(const Core::Geometry::Point &point,ENode::index_type &i);
  
  /// alternative calls
  inline void set_node(const Core::Geometry::Point &point, Node::index_type i)
    { set_point(point,i); }
  inline void set_enode(const Core::Geometry::Point &point, ENode::index_type i)
    { set_point(point,i); }
  
  // Set the nodes that make up an element
  virtual void set_nodes(Node::array_type& array, Edge::index_type idx);
  virtual void set_nodes(Node::array_type& array, Face::index_type idx);
  virtual void set_nodes(Node::array_type& array, Cell::index_type idx);
  virtual void set_nodes(Node::array_type& array, Elem::index_type idx);  
        
  inline Node::index_type add_node(const Core::Geometry::Point& point) 
    { Node::index_type idx; add_node(point,idx); return (idx); }
  
  inline void get_node(Core::Geometry::Point &point, Node::index_type i)
    { get_point(point,i); }
  inline void get_enode(Core::Geometry::Point &point, ENode::index_type i)
    { get_point(point,i); }
  
  /// Do not use this one as it is not clear whether it is a 
  /// element node or edge node  
  inline VMesh::Node::index_type add_point(const Core::Geometry::Point& point) 
    { Node::index_type idx; add_node(point,idx); return (idx); }
    
  /// Add an element to a mesh
  virtual void add_elem(const Node::array_type &nodes,Elem::index_type &i);
  
  /// Alternative calls for add_elem
  inline VMesh::Elem::index_type add_elem(const Node::array_type nodes)
    { Elem::index_type idx; add_elem(nodes,idx); return (idx); }
  
  
  /// Currently only available for tetrahedra, triangles and curves
  virtual void insert_node_into_elem(Elem::array_type& newelems, 
                                     Node::index_type& newnode,
                                     Elem::index_type  elem,
                                     Core::Geometry::Point& point);
  
  /// Get the neighbors of a node or an element
  virtual bool get_neighbor(Elem::index_type &neighbor, 
                      Elem::index_type from, DElem::index_type delem) const;
  virtual bool get_neighbors(Elem::array_type &elems, 
                         Elem::index_type i, DElem::index_type delem) const;
  virtual void get_neighbors(Elem::array_type &elems, 
                             Elem::index_type i) const;
  virtual void get_neighbors(Node::array_type &nodes, 
                             Node::index_type i) const;

  /// Draw non linear elements
  virtual void pwl_approx_edge(coords_array_type &coords, 
                               Elem::index_type ci, unsigned int which_edge, 
                               unsigned int div_per_unit) const;
  virtual void pwl_approx_face(coords_array2_type &coords, 
                               Elem::index_type ci, unsigned int which_face, 
                               unsigned int div_per_unit) const;

  /// Get node normals, needed for visualization
  virtual void get_normal(Core::Geometry::Vector& norm,Node::index_type i) const;

  /// Get the dimensions of the mesh.
  /// This function will replace get_dim()
  virtual void get_dimensions(dimension_type& dim);

  virtual void get_elem_dimensions(dimension_type& dim);

  /// The following functions are intended so one can do the local to global
  /// transformation efficiently. As the transformation matrix is a constant for
  /// certain meshes, it is precomputed and this function looks up the precomputed
  /// jacobians, while for others it depends on the element and it is computed
  /// on the fly. To assure that the fastest method is used, use these functions. 

  /// Get the determinant of the jacobian matrix
  /// Coords determine where the determinant needs o be evaluated
  /// Generally LatVol, ImageMesh, TriMesh, TetMesh have a jacobian that
  /// is independen of the local coordinate system, however HexVol, QuadSurf,
  /// and PrismVol have values that depend on the local position within the 
  /// element
  virtual double det_jacobian(const coords_type& coords,
                              Elem::index_type idx) const; 

  /// Get the jacobian of the local to global transformation
  /// Note J needs to be a least an array of 9 values. 
  /// Coords and idx again specify the element and the position in
  /// local coordinates.
  virtual void jacobian(const coords_type& coords,
                        Elem::index_type idx,
                        double* J) const; 

  /// Get the inverse jacobian matrix. This gives as side product the
  /// determinant of the inverse matrix.
  virtual double inverse_jacobian(const coords_type& coords,
                                   Elem::index_type idx,
                                   double* Ji) const;


  /// Element Quality metrics:
  
  /// Scaled jacobian of local to global transformation
  virtual double scaled_jacobian_metric(const Elem::index_type idx) const;

  /// Jacobian of local to global transformation
  virtual double jacobian_metric(const Elem::index_type idx) const;
  
  /// Volume of an element
  inline double volume_metric(const Elem::index_type idx) const
    { return(get_volume(idx)); }
    
  /// Scaled inscribed to circumscribed ratio
  virtual double inscribed_circumscribed_radius_metric(Elem::index_type idx) const;
  
  
  /// @todo: These should go: we have get_weights and get_derivate_weights
  /// with basis_order call 
  /// Direct access to get weights functions in basis functions
  /// These four are for interpolation
  inline  void get_constant_weights(coords_type& /*coords*/, std::vector<double>& weights)
    { weights.resize(1); weights[0] = 1.0; }
  virtual void get_linear_weights(coords_type& coords, std::vector<double>& weights);
  virtual void get_quadratic_weights(coords_type& coords, std::vector<double>& weights);
  virtual void get_cubic_weights(coords_type& coords, std::vector<double>& weights);

  /// These four are for computating gradients
  inline  void get_constant_derivate_weights(coords_type& /*coords*/, std::vector<double>& weights)
    { weights.resize(1); weights[0] = 0.0; }
  virtual void get_linear_derivate_weights(coords_type& coords, std::vector<double>& weights);
  virtual void get_quadratic_derivate_weights(coords_type& coords, std::vector<double>& weights);
  virtual void get_cubic_derivate_weights(coords_type& coords, std::vector<double>& weights);

  
  /// Rerouting of some of the virtual mesh function calls
  
  virtual Core::Geometry::BBox get_bounding_box() const;
  
  /// This call is for synchronizing tables of precomputed elements
  virtual bool synchronize(unsigned int sync); 
  virtual bool unsynchronize(unsigned int sync);
  
  // Only use this function when this is the only code that uses this mesh
  virtual bool clear_synchronization();
  
  // Transform a full field, this one works on the full field
  virtual void transform(const Core::Geometry::Transform &t);
  
  /// Get the transform from a regular field
  virtual Core::Geometry::Transform get_transform() const;
  /// Set the transform of a regular field
  virtual void set_transform(const Core::Geometry::Transform &t);
  
  virtual void get_canonical_transform(Core::Geometry::Transform &t);
  /// Get the epsilon for doing numerical computations
  /// This one is generally 1e-7*length diagonal of the bounding box
  virtual double get_epsilon() const;

  /// check the type of mesh
  virtual bool is_pointcloudmesh()     { return (false); }
  virtual bool is_curvemesh()          { return (false); }
  virtual bool is_scanlinemesh()       { return (false); }
  virtual bool is_structcurvemesh()    { return (false); }
  virtual bool is_trisurfmesh()        { return (false); }
  virtual bool is_quadsurfmesh()       { return (false); }
  virtual bool is_imagemesh()          { return (false); }
  virtual bool is_structquadsurfmesh() { return (false); }
  virtual bool is_tetvolmesh()         { return (false); }
  virtual bool is_prismvolmesh()       { return (false); }
  virtual bool is_hexvolmesh()         { return (false); }
  virtual bool is_latvolmesh()         { return (false); }
  virtual bool is_structhexvolmesh()   { return (false); }

  /// Check order of mesh
  inline bool is_constantmesh()        { return (basis_order_ == 0); }
  inline bool is_linearmesh()          { return (basis_order_ == 1); }
  inline bool is_quadraticmesh()       { return (basis_order_ == 2); }
  inline bool is_cubicmesh()           { return (basis_order_ == 3); }
  inline bool is_nonlinearmesh()          { return (basis_order_ > 1); }
  

  //----------------------------------------------------------------------

  /// Used for local conversion of vector types
  /// At some point this function should go away
  template <class VEC1, class VEC2>
  inline void convert_vector(VEC1& v1, VEC2 v2) const
  {
    v1.resize(v2.size());
    for (size_t p=0; p < v2.size(); p++) v1[p] = static_cast<typename VEC1::value_type>(v2[p]);
  }

  /// Inline calls to information that is constant for a mesh and does not
  /// change for a mesh. These properties are stored directly in the vmesh
  /// data structure and hence we can replace them by simple inline calls.

  inline int basis_order()
    { return (basis_order_); }

  inline int dimensionality()
    { return (dimension_); }
    
  inline bool is_point()
    { return (dimension_ == 0); }

  inline bool is_line()
    { return (dimension_ == 1); }

  inline bool is_surface()
    { return (dimension_ == 2); }

  inline bool is_volume()
    { return (dimension_ == 3); }

  inline unsigned int num_nodes_per_elem()
    { return (num_nodes_per_elem_); }

  inline unsigned int num_enodes_per_elem()
    { return (num_enodes_per_elem_); }

  inline unsigned int num_edges_per_elem()
    { return (num_edges_per_elem_); }

  inline unsigned int num_faces_per_elem()
    { return (num_faces_per_elem_); }

  inline unsigned int num_nodes_per_face()
    { return (num_nodes_per_face_); }

  inline unsigned int num_nodes_per_edge()
    { return (2); }

  inline unsigned int num_edges_per_face()
    { return (num_edges_per_face_); }
    
  inline unsigned int num_gradients_per_node()
    { return (num_gradients_per_node_); }
    
  inline bool has_normals()
    { return (has_normals_); }

  inline bool is_editable()
    { return (is_editable_); }

  inline bool is_regularmesh()
    { return (is_regular_); }

  inline bool is_irregularmesh()
    { return (!is_regular_); }

  inline bool is_structuredmesh()
    { return (is_structured_); }

  inline bool is_unstructuredmesh()
    { return (!is_structured_); }

  inline size_type get_ni() const 
    { return ni_; }
  inline size_type get_nj() const 
    { return nj_; }
  inline size_type get_nk() const 
    { return nk_; }

  inline int generation() const
    { return (generation_); }

  /// These functions help dealing with regular meshes and translate Node indices
  /// to coordinate indices
  
  /// From index splits up the index into parts
  inline void from_index(index_type& i, index_type& j, index_type& k,Node::index_type idx)
  {
    const index_type xidx = static_cast<const index_type>(idx);
    i = xidx % ni_; index_type jk = xidx / ni_;
    j = jk % nj_; k = jk / nj_;  
  }

  inline void from_index(index_type& i, index_type& j, index_type& k,Elem::index_type idx)
  {
    const index_type xidx = static_cast<const index_type>(idx);
    i = xidx % (ni_-1); index_type jk = xidx / (ni_-1);
    j = jk % (nj_-1); k = jk / (nj_-1);  
  }

  inline void from_index(index_type& i, index_type& j, Node::index_type idx)
  {
    const index_type xidx = static_cast<const index_type>(idx);
    i = xidx % ni_; j = xidx / ni_;
  }

  inline void from_index(index_type& i, index_type& j, Elem::index_type idx)
  {
    const index_type xidx = static_cast<const index_type>(idx);
    i = xidx % (ni_-1); j = xidx / (ni_-1);
  }

  inline void from_index(index_type& i, index_type& j, index_type& k,Cell::index_type idx)
  {
    const index_type xidx = static_cast<const index_type>(idx);
    i = xidx % (ni_-1); index_type jk = xidx / (ni_-1);
    j = jk % (nj_-1); k = jk / (nj_-1);  
  }

  inline void from_index(index_type& i, index_type& j, Face::index_type idx)
  {
    const index_type xidx = static_cast<const index_type>(idx);
    i = xidx % (ni_-1); j = xidx / (ni_-1);
  }

  /// To index generates a new successive index
  inline void to_index(Node::index_type& idx,index_type i, index_type j, index_type k)
  {
    idx = Node::index_type(i+ni_*j+ni_*nj_*k);
  }

  inline void to_index(Node::index_type& idx,index_type i, index_type j)
  {
    idx = Node::index_type(i+ni_*j);
  }
  
  inline void to_index(Elem::index_type& idx,index_type i, index_type j, index_type k)
  {
    idx = Elem::index_type(i+(ni_-1)*j+(ni_-1)*(nj_-1)*k);
  }

  inline void to_index(Elem::index_type& idx,index_type i, index_type j)
  {
    idx = Elem::index_type(i+(ni_-1)*j);
  }    

  inline void to_index(Cell::index_type& idx,index_type i, index_type j, index_type k)
  {
    idx = Cell::index_type(i+(ni_-1)*j+(ni_-1)*(nj_-1)*k);
  }

  inline void to_index(Face::index_type& idx,index_type i, index_type j)
  {
    idx = Face::index_type(i+(ni_-1)*j);
  }    

  /// Test for the type of elements

  inline bool is_pnt_element()
    { return (is_pointcloudmesh()); }
    
  inline bool is_crv_element()
    { return (is_structcurvemesh()||is_curvemesh()||is_scanlinemesh()); }

  inline bool is_tri_element()
    { return (is_trisurfmesh()); }
    
  inline bool is_quad_element()
    { return (is_structquadsurfmesh()||is_quadsurfmesh()||is_imagemesh()); }
    
  inline bool is_tet_element()
    { return (is_tetvolmesh()); }
    
  inline bool is_prism_element()
    { return (is_prismvolmesh()); }
    
  inline bool is_hex_element()
    { return (is_structhexvolmesh()||is_hexvolmesh()||is_latvolmesh()); }

  /// Get the unit vertex coordinates of the unit element
  inline void get_element_vertices(coords_array_type& coords)
    { coords = unit_vertices_; }
    
  inline void get_element_edges(nodes_array_type& edges)
    { edges = unit_edges_; }
 
  inline void get_element_center(coords_type& coords)
    { coords = unit_center_; }
 
  inline double get_element_size()
    { return (element_size_); }
 
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  /// Shortcuts to property manager
  inline void copy_properties(VMesh* imesh)
    { pm_->copy_properties(imesh->pm_); }
    
  template<class T> 
  inline void set_property(const std::string &name, const T &val, bool is_transient)
    { pm_->set_property(name,val,is_transient); }
    
  template<class T> 
  inline bool get_property( const std::string &name, T &val)
    { return(pm_->get_property(name,val)); }
    
  inline bool is_property( const std::string &name)
    { return(pm_->is_property(name)); }
#endif
  
protected:
  /// Properties of meshes that do not change during the lifetime of the mesh
  /// and hence they can be stored for fast use.
  int basis_order_;
  int dimension_;
  
  /// Mesh properties
  bool has_normals_;
  bool is_editable_;
  bool is_regular_;
  bool is_structured_;

  /// Mesh statistics
  unsigned int num_nodes_per_elem_;
  unsigned int num_enodes_per_elem_;
  unsigned int num_edges_per_elem_;
  unsigned int num_faces_per_elem_;
  unsigned int num_nodes_per_face_;
  unsigned int num_edges_per_face_;  
  unsigned int num_gradients_per_node_;
 
  /// Size in local coordinate system
  double element_size_;
  
  /// Definitions of local element topology
  coords_array_type       unit_vertices_;
  nodes_array_type        unit_edges_;
  coords_type             unit_center_;
          
  /// size of structured meshes
  /// unstructured mesh denote the number of nodes in ni_
  size_type ni_;
  size_type nj_;
  size_type nk_;

  /// generation number of mesh
  unsigned int generation_;
  
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  /// Add this one separately to avoid circular dependencies
  /// Pointer to base class of the mesh
  PropertyManager* pm_;
#endif
};

} // end namespace SCIRun

#endif // Datatypes_Mesh_h
