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

#include <Core/Datatypes/Mesh/Mesh.h>
#include <Core/Datatypes/Mesh/VMesh.h>
#include <Core/Utils/Exception.h>

//#include <Core/GeometryPrimitives/BBox.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;


void 
VirtualMesh::size(Node::size_type& size) const
{
  size = 0;
}

void 
VirtualMesh::size(ENode::size_type& size) const
{
  size = 0;
}

void 
VirtualMesh::size(Edge::size_type& size) const
{
  size = 0;
}

void 
VirtualMesh::size(Face::size_type& size) const
{
  size = 0;
}

void 
VirtualMesh::size(Cell::size_type& size) const
{
  size = 0;
}

void 
VirtualMesh::size(Elem::size_type& size) const
{
  size = 0;
}

void 
VirtualMesh::size(DElem::size_type& size) const
{
  size = 0;
}

void VirtualMesh::get_dimensions(dimension_type& dim)
{
  dim.resize(1);
  Node::size_type sz;
  size(sz);
  dim[0] = sz;
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
LockingHandle<SearchGridT<SCIRun::index_type> >
VirtualMesh::get_elem_search_grid()
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_elem_search_grid() has not been implemented");
}

LockingHandle<SearchGridT<SCIRun::index_type> >
VirtualMesh::get_node_search_grid()
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_node_search_grid() has not been implemented");
}
#endif

void 
VirtualMesh::get_nodes(Node::array_type& nodes, Node::index_type i) const
{
  nodes.resize(1);
  nodes[0] = i;
}
  
void 
VirtualMesh::get_nodes(Node::array_type&, Edge::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_nodes(Node::array_type,Edge::index_type) has not been implemented");
}

void 
VirtualMesh::get_nodes(Node::array_type&, Face::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_nodes(Node::array_type,Face::index_type) has not been implemented");
}

void 
VirtualMesh::get_nodes(Node::array_type&, Cell::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_nodes(Node::array_type,Cell::index_type) has not been implemented");
}

void 
VirtualMesh::get_nodes(Node::array_type&, Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_nodes(Node::array_type,Elem::index_type) has not been implemented");
}  

void 
VirtualMesh::get_nodes(Node::array_type&, DElem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_nodes(Node::array_type,DElem::index_type) has not been implemented");
}


#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
void 
VirtualMesh::get_enodes(ENode::array_type&, Node::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_nodesl(ENode::array_type,Node::index_type) has not been implemented");
}
  
void 
VirtualMesh::get_enodes(ENode::array_type&, Edge::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_nodes(VNLode::array_type,Edge::index_type) has not been implemented");
}

void 
VirtualMesh::get_enodes(ENode::array_type&, Face::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_nodes(Node::array_type,Face::index_type) has not been implemented");
}

void 
VirtualMesh::get_enodes(ENode::array_type&, Cell::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_nodes(Node::array_type,Cell::index_type) has not been implemented");
}

void 
VirtualMesh::get_enodes(ENode::array_type&, Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_nodes(Node::array_type,Elem::index_type) has not been implemented");
}  

void 
VirtualMesh::get_enodes(ENode::array_type&, DElem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_nodes(Node::array_type,DElem::index_type) has not been implemented");
}
#endif


void 
VirtualMesh::get_edges(Edge::array_type&, Node::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_edges(Edge::array_type,Node::index_type) has not been implemented");
}

void 
VirtualMesh::get_edges(Edge::array_type& edges, Edge::index_type i) const
{
  edges.resize(1);
  edges[0] = i;
}

void 
VirtualMesh::get_edges(Edge::array_type&, Face::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_edges(Edge::array_type,Face:index_type) has not been implemented");
}

void 
VirtualMesh::get_edges(Edge::array_type&, Cell::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_edges(Edge::array_type,Cell::index_type) has not been implemented");
}

void 
VirtualMesh::get_edges(Edge::array_type&, Elem::index_type) const
{  
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_edges(Edge::array_type,Elem::index_type) has not been implemented");
}

void 
VirtualMesh::get_edges(Edge::array_type&, DElem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_edges(Edge::array_type,DElem::index_type) has not been implemented");
}


void 
VirtualMesh::get_faces(Face::array_type&, Node::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_faces(Face::array_type,Node::index_type) has not been implemented");
}

void 
VirtualMesh::get_faces(Face::array_type&, Edge::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_faces(Face::array_type,Edge::index_type) has not been implemented");
}

void 
VirtualMesh::get_faces(Face::array_type& faces, Face::index_type i) const
{
  faces.resize(1);
  faces[0] = i;
}

void 
VirtualMesh::get_faces(Face::array_type&, Cell::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_faces(Face::array_type,Cell::index_type) has not been implemented");
}

void 
VirtualMesh::get_faces(Face::array_type&, Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_faces(Face::array_type,Elem::index_type) has not been implemented");
}

void 
VirtualMesh::get_faces(Face::array_type&, DElem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_faces(Face::array_type,DElem::index_type) has not been implemented");
}



void 
VirtualMesh::get_cells(Cell::array_type&, Node::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_cells(Cell::array_type,Node::index_type) has not been implemented");
}

void 
VirtualMesh::get_cells(Cell::array_type&, Edge::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_cells(Cell::array_type,Edge::index_type) has not been implemented");
}

void 
VirtualMesh::get_cells(Cell::array_type&, Face::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_cells(Cell::array_type,Face::index_type) has not been implemented");
}

void 
VirtualMesh::get_cells(Cell::array_type& cells, Cell::index_type i) const
{
  cells.resize(1);
  cells[0] = i;
}

void 
VirtualMesh::get_cells(Cell::array_type&, Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_cells(Cell::array_type,Elem::index_type) has not been implemented");
}

void 
VirtualMesh::get_cells(Cell::array_type&, DElem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_cells(Cell::array_type,DElem::index_type) has not been implemented");
}

  
  
void 
VirtualMesh::get_elems(Elem::array_type&, Node::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_elems(Elem::array_type,Node::index_type) has not been implemented");
}

void 
VirtualMesh::get_elems(Elem::array_type&, Edge::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_elems(Elem::array_type,Edge::index_type) has not been implemented");
}

void 
VirtualMesh::get_elems(Elem::array_type&, Face::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_elems(Elem::array_type,Face::index_type) has not been implemented");
}

void 
VirtualMesh::get_elems(Elem::array_type&, Cell::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_elems(Elem::array_type,Cell::index_type) has not been implemented");
}

void 
VirtualMesh::get_elems(Elem::array_type& elems, Elem::index_type i) const
{
  elems.resize(1);
  elems[0] = i;
}

void 
VirtualMesh::get_elems(Elem::array_type&, DElem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_elems(Elem::array_type,DElem::index_type) has not been implemented");
}

void 
  VirtualMesh::get_center(Point &, Node::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_center(Point,Node::index_type) has not been implemented");
}

void 
  VirtualMesh::get_center(Point &, ENode::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_center(Point,ENode::index_type) has not been implemented");
}

void 
  VirtualMesh::get_center(Point &, Edge::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_center(Point,Edge::index_type) has not been implemented");
}

void 
  VirtualMesh::get_center(Point &, Face::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_center(Point,Face::index_type) has not been implemented");
}

void 
  VirtualMesh::get_center(Point &, Cell::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_center(Point,Cell::index_type) has not been implemented");
}

void 
  VirtualMesh::get_center(Point &, Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_center(Point,Elem::index_type) has not been implemented");
}

void 
  VirtualMesh::get_center(Point &, DElem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_center(Point,DElem::index_type) has not been implemented");
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

void 
VirtualMesh::get_delems(DElem::array_type&, Node::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_delems(DElem::array_type,Node::index_type) has not been implemented");
}

void 
VirtualMesh::get_delems(DElem::array_type&, Edge::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_delems(DElem::array_type,Edge::index_type) has not been implemented");
}

void 
VirtualMesh::get_delems(DElem::array_type&, Face::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_delems(DElem::array_type,Face::index_type) has not been implemented");
}

void 
VirtualMesh::get_delems(DElem::array_type&, Cell::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_delems(DElem::array_type,Cell::index_type) has not been implemented");
}

void 
VirtualMesh::get_delems(DElem::array_type&, Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_delems(DElem::array_type,Elem::index_type) has not been implemented");
}

void 
VirtualMesh::get_delems(DElem::array_type& delems, DElem::index_type i) const
{
  delems.resize(1);
  delems[0] = i;
}


bool 
VirtualMesh::get_elem(Elem::index_type&, Node::array_type&) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_elem(Elem::index_type,Node::array_type) has not been implemented");
}

bool 
VirtualMesh::get_delem(DElem::index_type&, Node::array_type&) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_delem(DElem::index_type,Node::array_type) has not been implemented");
}

bool 
VirtualMesh::get_cell(Cell::index_type&, Node::array_type&) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_elem(Cell::index_type,Node::array_type) has not been implemented");
}

bool 
VirtualMesh::get_face(Face::index_type&, Node::array_type&) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_elem(Face::index_type,Node::array_type) has not been implemented");
}

bool 
VirtualMesh::get_edge(Edge::index_type&, Node::array_type&) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_elem(Edge::index_type,Node::array_type) has not been implemented");
}

void
VirtualMesh::set_nodes(Node::array_type& array, Edge::index_type idx)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: set_nodes(Node::array_type,Edge::index_type) has not been implemented");
}

void
VirtualMesh::set_nodes(Node::array_type& array, Face::index_type idx)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: set_nodes(Node::array_type,Face::index_type) has not been implemented");
}

void
VirtualMesh::set_nodes(Node::array_type& array, Cell::index_type idx)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: set_nodes(Node::array_type,Cell::index_type) has not been implemented");
}

void
VirtualMesh::set_nodes(Node::array_type& array, Elem::index_type idx)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: set_nodes(Node::array_type,Elem::index_type) has not been implemented");
}




void 
VirtualMesh::get_centers(Point*, Node::array_type&) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_centers(Point*,Node::array_type) has not been implemented");
}

void 
VirtualMesh::get_centers(Point*, Elem::array_type&) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_centers(Point*,Elem::array_type) has not been implemented");
}

double 
VirtualMesh::get_size(VirtualMesh::Edge::index_type) const
{
  return (0.0);
}

double 
VirtualMesh::get_size(VirtualMesh::Face::index_type) const
{
  return (0.0);
} 

double 
VirtualMesh::get_size(VirtualMesh::Cell::index_type) const
{
  return (0.0);
}

double 
VirtualMesh::get_size(VirtualMesh::Elem::index_type) const
{
  return (0.0);
}

double 
VirtualMesh::get_size(VirtualMesh::DElem::index_type) const
{
  return (0.0);
}

double
VirtualMesh::get_size(VirtualMesh::Node::array_type& array) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: size(Node::array_type) has not been implemented");
}

  
bool 
VirtualMesh::locate(VirtualMesh::Node::index_type &, const Point &) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: locate(Node::index_type,Point) has not been implemented");
}

bool 
VirtualMesh::locate(VirtualMesh::Elem::index_type &, const Point &) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: locate(Elem::index_type,Point) has not been implemented");
}

bool 
VirtualMesh::locate(VirtualMesh::Elem::index_type &, 
              VirtualMesh::coords_type &, const Point &) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: locate(Elem::index_type,coords_type,Point) has not been implemented");
}

bool 
VirtualMesh::locate(VirtualMesh::Elem::array_type &, const BBox &) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: locate(Elem::array_type,BBox) has not been implemented");
}

void 
VirtualMesh::mlocate(std::vector<Node::index_type> &, const std::vector<Point> &) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: mlocate(vector<Node::index_type>,Point) has not been implemented");
}

void 
VirtualMesh::mlocate(std::vector<Elem::index_type> &, const std::vector<Point> &) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: mlocate(std::vector<Elem::index_type>,Point) has not been implemented");
}


bool
VirtualMesh::find_closest_node(double&, Point&, VirtualMesh::Node::index_type&, const Point &) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: find_closest_node(dist,Point,Node::index_type,Point) has not been implemented");
}

bool
VirtualMesh::find_closest_node(double&, Point&, VirtualMesh::Node::index_type&, 
                         const Point&, double) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: find_closest_node(dist,Point,Node::index_type,Point,maxdist) has not been implemented");
}

bool
VirtualMesh::find_closest_nodes(std::vector<VirtualMesh::Node::index_type>&, const Point&, double) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: find_closest_nodes(std::vector<Node::index_type>,Point,double) has not been implemented");
}

bool
VirtualMesh::find_closest_nodes(std::vector<double>& distances,
                          std::vector<VirtualMesh::Node::index_type>&, 
                          const Point&, double) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: find_closest_nodes(std::vector<double>&, std::vector<Node::index_type>,Point,double) has not been implemented");
}

bool 
VirtualMesh::find_closest_elem(double&, Point&, VirtualMesh::coords_type&,
                         VirtualMesh::Elem::index_type&, const Point &) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: find_closest_elem(dist,Point,coords,Elem::index_type,Point) has not been implemented");
}

bool 
VirtualMesh::find_closest_elem(double&, Point&, VirtualMesh::coords_type&,
                         VirtualMesh::Elem::index_type&, 
                         const Point&, double) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: find_closest_elem(dist,Point,coords,Elem::index_type,Point,maxdist) has not been implemented");
}

bool 
VirtualMesh::find_closest_elems(double&, Point&, VirtualMesh::Elem::array_type&, 
                          const Point&) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: find_closest_elems(dist,Point,Elem::array_type,Point) has not been implemented");
}

  
bool 
VirtualMesh::get_coords(coords_type&, const Point&, Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_coords(coords_type,Point,Elem::index_type) has not been implemented");
}


void 
VirtualMesh::interpolate(Point &, const coords_type&, Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: interpolate(Point,coords_type,Elem::index_type) has not been implemented");
}

void 
VirtualMesh::minterpolate(std::vector<Point> &, const std::vector<coords_type>&, Elem::index_type ) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: minterpolate(std::vector<Point>,std::vector<coords_type>,Elem::index_type) has not been implemented");
}


void 
VirtualMesh::derivate(dpoints_type&, const coords_type&, Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: derivate(dpoints_type,coords_type,Elem::index_type) has not been implemented");
}


void 
VirtualMesh::get_normal(Vector&, coords_type&, Elem::index_type, DElem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_normal() has not been implemented");
}  


void 
VirtualMesh::get_random_point(Point&, Elem::index_type,FieldRNG&) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_random_point(Point,Elem::index_type) has not been implemented");
}

void 
VirtualMesh::set_point(const Point&, Node::index_type)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: set_point(Point,Node::index_type) has not been implemented");
}
  
void 
VirtualMesh::set_point(const Point&, ENode::index_type)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: set_point(Point,ENode::index_type) has not been implemented");
}  

Point*
VirtualMesh::get_points_pointer() const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_points_pointer() has not been implemented");  
}

VirtualMesh::index_type* 
VirtualMesh::get_elems_pointer() const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_elems_pointer() has not been implemented");  
}

#endif

void 
VirtualMesh::node_reserve(size_t)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: node_reserve(size_t size) has not been implemented");
}

void 
VirtualMesh::elem_reserve(size_t)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: elem_reserve(size_t size) has not been implemented");
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER


void 
VirtualMesh::resize_nodes(size_t)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: resize_nodes(size_t size) has not been implemented");
}

void 
VirtualMesh::resize_elems(size_t)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: resize_elems(size_t size) has not been implemented");
}

#endif

void 
VirtualMesh::add_node(const Point&, Node::index_type&)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: this mesh cannot be edited (add_node)");  
}

void 
VirtualMesh::add_enode(const Point&, ENode::index_type&)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: this mesh cannot be edited (add_enode)");  
}

void  
VirtualMesh::add_elem(const Node::array_type&, Elem::index_type&)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: this mesh cannot be edited (add_elem)");  
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

void 
VirtualMesh::insert_node_into_elem(Elem::array_type&, Node::index_type&,
                             Elem::index_type, Point&)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: insert_node_into_elem has not been implemented for this mesh type");
}


bool
VirtualMesh::get_neighbor(Elem::index_type&, Elem::index_type, DElem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_neighbor(Elem::index_type,Elem::index_type,DElem::index_type) has not been implemented");  
}

void
VirtualMesh::get_neighbors(Elem::array_type&, Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_neighbors(Elem::index_type,Elem::index_type) has not been implemented");  
}

bool
VirtualMesh::get_neighbors(Elem::array_type&, Elem::index_type, DElem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_neighbors(Elem::array_type,Elem::index_type,DElem::index_type) has not been implemented");  
}

void
VirtualMesh::get_neighbors(Node::array_type&, Node::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_neighbors(Node::index_type,Node::index_type) has not been implemented");  
}

void 
VirtualMesh::pwl_approx_edge(std::vector<coords_type >&, Elem::index_type, unsigned int , unsigned int) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: pwl_appprox_edge has not been implemented");  
}

void 
VirtualMesh::pwl_approx_face(std::vector<std::vector<coords_type > >&, Elem::index_type, unsigned int, unsigned int) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: pwl_appprox_face has not been implemented");  
}

void 
VirtualMesh::get_normal(Vector&,Node::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_normal has not been implemented");  
}

double
VirtualMesh::det_jacobian(const coords_type&, Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: det_jacobian has not been implemented");
}

void
VirtualMesh::jacobian(const coords_type&, Elem::index_type, double *) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: jacobian has not been implemented");
}

double
VirtualMesh::inverse_jacobian(const coords_type&, Elem::index_type, double*) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: inverse_jacobian has not been implemented");
}

double
VirtualMesh::scaled_jacobian_metric(Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: scaled_jacobian_metric has not yet been implemented");
}

double
VirtualMesh::jacobian_metric(Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: jacobian_metric has not yet been implemented");
}

double
VirtualMesh::inscribed_circumscribed_radius_metric(Elem::index_type) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: inscribed_circumscribed_radius has not yet been implemented");
}

void
VirtualMesh::get_elem_dimensions(dimension_type& dim)
{
  dim.resize(1);
  Elem::size_type sz;
  size(sz);
  dim[0] = sz;
}


BBox
VirtualMesh::get_bounding_box() const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_bounding_box has not yet been implemented");  
}

bool
VirtualMesh::synchronize(unsigned int)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: synchronize has not yet been implemented");  
}

bool
VirtualMesh::unsynchronize(unsigned int)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: synchronize has not yet been implemented");  
}

bool
VirtualMesh::clear_synchronization()
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: clear_synchronization has not yet been implemented");  
}

void 
VirtualMesh::transform(const Transform &)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: transform has not yet been implemented");  
}

Transform 
VirtualMesh::get_transform() const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_transform has not yet been implemented");  
}

void VirtualMesh::set_transform(const Transform &)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: set_transform has not yet been implemented");  
}
 
void 
VirtualMesh::get_canonical_transform(Transform &)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_canonical_transform has not yet been implemented");  
}

double
VirtualMesh::get_epsilon() const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_epsilon has not yet been implemented");  
}


void 
VirtualMesh::get_interpolate_weights(const coords_type&, Elem::index_type, 
                               ElemInterpolate&, int) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_interpolate_weights has not yet been implemented");  
}
                                       
void 
VirtualMesh::get_interpolate_weights(const Point&, ElemInterpolate&, int) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_interpolate_weights has not yet been implemented");  
}
        

void 
VirtualMesh::get_minterpolate_weights(const std::vector<coords_type>&, 
                                Elem::index_type, 
                                MultiElemInterpolate&,
                                int) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_minterpolate_weights has not yet been implemented");  
}
                                       
void 
VirtualMesh::get_minterpolate_weights(const std::vector<Point>&, 
                                MultiElemInterpolate&,
                                int) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_minterpolate_weights has not yet been implemented");  
}                                       
                                                                                                     

void 
VirtualMesh::get_gradient_weights(const coords_type&, 
                            Elem::index_type, 
                            ElemGradient&,
                            int) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_gradient_weights has not yet been implemented");  
}

void 
VirtualMesh::get_gradient_weights(const Point&, 
                            ElemGradient&,
                            int) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_gradient_weights has not yet been implemented");  
}
                                       

void 
VirtualMesh::get_mgradient_weights(const std::vector<coords_type>&, 
                             Elem::index_type, 
                             MultiElemGradient&,
                             int) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_mgradient_weights has not yet been implemented");  
}

void 
VirtualMesh::get_mgradient_weights(const std::vector<Point>&, 
                             MultiElemGradient&,
                             int) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_mgradient_weights has not yet been implemented");  
}

void 
VirtualMesh::get_weights(const coords_type&, 
                   std::vector<double>&, 
                   int) const                                 
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_weights has not yet been implemented");  
}  

void 
VirtualMesh::get_derivate_weights(const coords_type&, 
                            std::vector<double>&,
                            int) const   
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_derivate_weights has not yet been implemented");  
}  


void 
VirtualMesh::get_linear_weights(coords_type&, std::vector<double>&)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_linear_weights has not yet been implemented");  
}


void 
VirtualMesh::get_quadratic_weights(coords_type&, std::vector<double>&)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_quadratic_weights has not yet been implemented");  
}

void 
VirtualMesh::get_cubic_weights(coords_type&, std::vector<double>&)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_cubic_weights has not yet been implemented");  
}

void 
VirtualMesh::get_linear_derivate_weights(coords_type&, std::vector<double>&)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_linear_derivate_weights has not yet been implemented");  
}

void 
VirtualMesh::get_quadratic_derivate_weights(coords_type&, std::vector<double>&)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_quadratic_derivate_weights has not yet been implemented");  
}

void 
VirtualMesh::get_cubic_derivate_weights(coords_type&, std::vector<double>&)
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_cubic_derivate_weights has not yet been implemented");  
}

void 
VirtualMesh::get_gaussian_scheme(std::vector<VirtualMesh::coords_type>&, 
                           std::vector<double>&, int) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_gaussian_scheme has not yet been implemented");  
}
                                   
void 
VirtualMesh::get_regular_scheme(std::vector<VirtualMesh::coords_type>&, 
                          std::vector<double>&, int) const
{
  REPORT_NOT_IMPLEMENTED("VirtualMesh interface: get_regular_scheme has not yet been implemented");  
}
#endif
