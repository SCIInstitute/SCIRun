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
/// @todo Documentation Core/Datatypes/Legacy/Field/VMesh.cc
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/BBox.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;

void 
VMesh::size(Node::size_type& size) const
{
  size = 0;
}

void 
VMesh::size(ENode::size_type& size) const
{
  size = 0;
}

void 
VMesh::size(Edge::size_type& size) const
{
  size = 0;
}

void 
VMesh::size(Face::size_type& size) const
{
  size = 0;
}

void 
VMesh::size(Cell::size_type& size) const
{
  size = 0;
}

void 
VMesh::size(Elem::size_type& size) const
{
  size = 0;
}

void 
VMesh::size(DElem::size_type& size) const
{
  size = 0;
}

boost::shared_ptr<SearchGridT<SCIRun::index_type> >
VMesh::get_elem_search_grid()
{
  ASSERTFAIL("VMesh interface: get_elem_search_grid() has not been implemented");
}

boost::shared_ptr<SearchGridT<SCIRun::index_type> >
VMesh::get_node_search_grid()
{
  ASSERTFAIL("VMesh interface: get_node_search_grid() has not been implemented");
}

void 
VMesh::get_nodes(Node::array_type& nodes, Node::index_type i) const
{
  nodes.resize(1);
  nodes[0] = i;
}
  
void 
VMesh::get_nodes(Node::array_type&, Edge::index_type) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Edge::index_type) has not been implemented");
}

void 
VMesh::get_nodes(Node::array_type&, Face::index_type) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Face::index_type) has not been implemented");
}

void 
VMesh::get_nodes(Node::array_type&, Cell::index_type) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Cell::index_type) has not been implemented");
}

void 
VMesh::get_nodes(Node::array_type&, Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Elem::index_type) has not been implemented");
}  

void 
VMesh::get_nodes(Node::array_type&, DElem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,DElem::index_type) has not been implemented");
}



void 
VMesh::get_enodes(ENode::array_type&, Node::index_type) const
{
  ASSERTFAIL("VMesh interface: get_nodesl(ENode::array_type,Node::index_type) has not been implemented");
}
  
void 
VMesh::get_enodes(ENode::array_type&, Edge::index_type) const
{
  ASSERTFAIL("VMesh interface: get_nodes(VNLode::array_type,Edge::index_type) has not been implemented");
}

void 
VMesh::get_enodes(ENode::array_type&, Face::index_type) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Face::index_type) has not been implemented");
}

void 
VMesh::get_enodes(ENode::array_type&, Cell::index_type) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Cell::index_type) has not been implemented");
}

void 
VMesh::get_enodes(ENode::array_type&, Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Elem::index_type) has not been implemented");
}  

void 
VMesh::get_enodes(ENode::array_type&, DElem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,DElem::index_type) has not been implemented");
}



void 
VMesh::get_edges(Edge::array_type&, Node::index_type) const
{
  ASSERTFAIL("VMesh interface: get_edges(Edge::array_type,Node::index_type) has not been implemented");
}

void 
VMesh::get_edges(Edge::array_type& edges, Edge::index_type i) const
{
  edges.resize(1);
  edges[0] = i;
}

void 
VMesh::get_edges(Edge::array_type&, Face::index_type) const
{
  ASSERTFAIL("VMesh interface: get_edges(Edge::array_type,Face:index_type) has not been implemented");
}

void 
VMesh::get_edges(Edge::array_type&, Cell::index_type) const
{
  ASSERTFAIL("VMesh interface: get_edges(Edge::array_type,Cell::index_type) has not been implemented");
}

void 
VMesh::get_edges(Edge::array_type&, Elem::index_type) const
{  
  ASSERTFAIL("VMesh interface: get_edges(Edge::array_type,Elem::index_type) has not been implemented");
}

void 
VMesh::get_edges(Edge::array_type&, DElem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_edges(Edge::array_type,DElem::index_type) has not been implemented");
}


void 
VMesh::get_faces(Face::array_type&, Node::index_type) const
{
  ASSERTFAIL("VMesh interface: get_faces(Face::array_type,Node::index_type) has not been implemented");
}

void 
VMesh::get_faces(Face::array_type&, Edge::index_type) const
{
  ASSERTFAIL("VMesh interface: get_faces(Face::array_type,Edge::index_type) has not been implemented");
}

void 
VMesh::get_faces(Face::array_type& faces, Face::index_type i) const
{
  faces.resize(1);
  faces[0] = i;
}

void 
VMesh::get_faces(Face::array_type&, Cell::index_type) const
{
  ASSERTFAIL("VMesh interface: get_faces(Face::array_type,Cell::index_type) has not been implemented");
}

void 
VMesh::get_faces(Face::array_type&, Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_faces(Face::array_type,Elem::index_type) has not been implemented");
}

void 
VMesh::get_faces(Face::array_type&, DElem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_faces(Face::array_type,DElem::index_type) has not been implemented");
}



void 
VMesh::get_cells(Cell::array_type&, Node::index_type) const
{
  ASSERTFAIL("VMesh interface: get_cells(Cell::array_type,Node::index_type) has not been implemented");
}

void 
VMesh::get_cells(Cell::array_type&, Edge::index_type) const
{
  ASSERTFAIL("VMesh interface: get_cells(Cell::array_type,Edge::index_type) has not been implemented");
}

void 
VMesh::get_cells(Cell::array_type&, Face::index_type) const
{
  ASSERTFAIL("VMesh interface: get_cells(Cell::array_type,Face::index_type) has not been implemented");
}

void 
VMesh::get_cells(Cell::array_type& cells, Cell::index_type i) const
{
  cells.resize(1);
  cells[0] = i;
}

void 
VMesh::get_cells(Cell::array_type&, Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_cells(Cell::array_type,Elem::index_type) has not been implemented");
}

void 
VMesh::get_cells(Cell::array_type&, DElem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_cells(Cell::array_type,DElem::index_type) has not been implemented");
}

  
  
void 
VMesh::get_elems(Elem::array_type&, Node::index_type) const
{
  ASSERTFAIL("VMesh interface: get_elems(Elem::array_type,Node::index_type) has not been implemented");
}

void 
VMesh::get_elems(Elem::array_type&, Edge::index_type) const
{
  ASSERTFAIL("VMesh interface: get_elems(Elem::array_type,Edge::index_type) has not been implemented");
}

void 
VMesh::get_elems(Elem::array_type&, Face::index_type) const
{
  ASSERTFAIL("VMesh interface: get_elems(Elem::array_type,Face::index_type) has not been implemented");
}

void 
VMesh::get_elems(Elem::array_type&, Cell::index_type) const
{
  ASSERTFAIL("VMesh interface: get_elems(Elem::array_type,Cell::index_type) has not been implemented");
}

void 
VMesh::get_elems(Elem::array_type& elems, Elem::index_type i) const
{
  elems.resize(1);
  elems[0] = i;
}

void 
VMesh::get_elems(Elem::array_type&, DElem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_elems(Elem::array_type,DElem::index_type) has not been implemented");
}



void 
VMesh::get_delems(DElem::array_type&, Node::index_type) const
{
  ASSERTFAIL("VMesh interface: get_delems(DElem::array_type,Node::index_type) has not been implemented");
}

void 
VMesh::get_delems(DElem::array_type&, Edge::index_type) const
{
  ASSERTFAIL("VMesh interface: get_delems(DElem::array_type,Edge::index_type) has not been implemented");
}

void 
VMesh::get_delems(DElem::array_type&, Face::index_type) const
{
  ASSERTFAIL("VMesh interface: get_delems(DElem::array_type,Face::index_type) has not been implemented");
}

void 
VMesh::get_delems(DElem::array_type&, Cell::index_type) const
{
  ASSERTFAIL("VMesh interface: get_delems(DElem::array_type,Cell::index_type) has not been implemented");
}

void 
VMesh::get_delems(DElem::array_type&, Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_delems(DElem::array_type,Elem::index_type) has not been implemented");
}

void 
VMesh::get_delems(DElem::array_type& delems, DElem::index_type i) const
{
  delems.resize(1);
  delems[0] = i;
}


bool 
VMesh::get_elem(Elem::index_type&, Node::array_type&) const
{
  ASSERTFAIL("VMesh interface: get_elem(Elem::index_type,Node::array_type) has not been implemented");
}

bool 
VMesh::get_delem(DElem::index_type&, Node::array_type&) const
{
  ASSERTFAIL("VMesh interface: get_delem(DElem::index_type,Node::array_type) has not been implemented");
}

bool 
VMesh::get_cell(Cell::index_type&, Node::array_type&) const
{
  ASSERTFAIL("VMesh interface: get_elem(Cell::index_type,Node::array_type) has not been implemented");
}

bool 
VMesh::get_face(Face::index_type&, Node::array_type&) const
{
  ASSERTFAIL("VMesh interface: get_elem(Face::index_type,Node::array_type) has not been implemented");
}

bool 
VMesh::get_edge(Edge::index_type&, Node::array_type&) const
{
  ASSERTFAIL("VMesh interface: get_elem(Edge::index_type,Node::array_type) has not been implemented");
}

void
VMesh::set_nodes(Node::array_type& array, Edge::index_type idx)
{
  ASSERTFAIL("VMesh interface: set_nodes(Node::array_type,Edge::index_type) has not been implemented");
}

void
VMesh::set_nodes(Node::array_type& array, Face::index_type idx)
{
  ASSERTFAIL("VMesh interface: set_nodes(Node::array_type,Face::index_type) has not been implemented");
}

void
VMesh::set_nodes(Node::array_type& array, Cell::index_type idx)
{
  ASSERTFAIL("VMesh interface: set_nodes(Node::array_type,Cell::index_type) has not been implemented");
}

void
VMesh::set_nodes(Node::array_type& array, Elem::index_type idx)
{
  ASSERTFAIL("VMesh interface: set_nodes(Node::array_type,Elem::index_type) has not been implemented");
}


void 
VMesh::get_center(Point &, Node::index_type) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,Node::index_type) has not been implemented");
}

void 
VMesh::get_center(Point &, ENode::index_type) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,ENode::index_type) has not been implemented");
}

void 
VMesh::get_center(Point &, Edge::index_type) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,Edge::index_type) has not been implemented");
}

void 
VMesh::get_center(Point &, Face::index_type) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,Face::index_type) has not been implemented");
}

void 
VMesh::get_center(Point &, Cell::index_type) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,Cell::index_type) has not been implemented");
}

void 
VMesh::get_center(Point &, Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,Elem::index_type) has not been implemented");
}

void 
VMesh::get_center(Point &, DElem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,DElem::index_type) has not been implemented");
}

void 
VMesh::get_centers(Point*, const Node::array_type&) const
{
  ASSERTFAIL("VMesh interface: get_centers(Point*,Node::array_type) has not been implemented");
}

void 
VMesh::get_centers(Point*, const Elem::array_type&) const
{
  ASSERTFAIL("VMesh interface: get_centers(Point*,Elem::array_type) has not been implemented");
}

double 
VMesh::get_size(VMesh::Edge::index_type) const
{
  return (0.0);
}

double 
VMesh::get_size(VMesh::Face::index_type) const
{
  return (0.0);
} 

double 
VMesh::get_size(VMesh::Cell::index_type) const
{
  return (0.0);
}

double 
VMesh::get_size(VMesh::Elem::index_type) const
{
  return (0.0);
}

double 
VMesh::get_size(VMesh::DElem::index_type) const
{
  return (0.0);
}

double
VMesh::get_size(VMesh::Node::array_type& array) const
{
  ASSERTFAIL("VMesh interface: size(Node::array_type) has not been implemented");
}

  
bool 
VMesh::locate(VMesh::Node::index_type &, const Point &) const
{
  ASSERTFAIL("VMesh interface: locate(Node::index_type,Point) has not been implemented");
}

bool 
VMesh::locate(VMesh::Elem::index_type &, const Point &) const
{
  ASSERTFAIL("VMesh interface: locate(Elem::index_type,Point) has not been implemented");
}

bool 
VMesh::locate(VMesh::Elem::index_type &, 
              VMesh::coords_type &, const Point &) const
{
  ASSERTFAIL("VMesh interface: locate(Elem::index_type,coords_type,Point) has not been implemented");
}

bool 
VMesh::locate(VMesh::Elem::array_type &, const BBox &) const
{
  ASSERTFAIL("VMesh interface: locate(Elem::array_type,BBox) has not been implemented");
}

void 
VMesh::mlocate(std::vector<Node::index_type> &, const std::vector<Point> &) const
{
  ASSERTFAIL("VMesh interface: mlocate(vector<Node::index_type>,Point) has not been implemented");
}

void 
VMesh::mlocate(std::vector<Elem::index_type> &, const std::vector<Point> &) const
{
  ASSERTFAIL("VMesh interface: mlocate(std::vector<Elem::index_type>,Point) has not been implemented");
}


bool
VMesh::find_closest_node(double&, Point&, VMesh::Node::index_type&, const Point &) const
{
  ASSERTFAIL("VMesh interface: find_closest_node(dist,Point,Node::index_type,Point) has not been implemented");
}

bool
VMesh::find_closest_node(double&, Point&, VMesh::Node::index_type&, 
                         const Point&, double) const
{
  ASSERTFAIL("VMesh interface: find_closest_node(dist,Point,Node::index_type,Point,maxdist) has not been implemented");
}

bool
VMesh::find_closest_nodes(std::vector<VMesh::Node::index_type>&, const Point&, double) const
{
  ASSERTFAIL("VMesh interface: find_closest_nodes(std::vector<Node::index_type>,Point,double) has not been implemented");
}

bool
VMesh::find_closest_nodes(std::vector<double>& distances,
                          std::vector<VMesh::Node::index_type>&, 
                          const Point&, double) const
{
  ASSERTFAIL("VMesh interface: find_closest_nodes(std::vector<double>&, std::vector<Node::index_type>,Point,double) has not been implemented");
}

bool 
VMesh::find_closest_elem(double&, Point&, VMesh::coords_type&,
                         VMesh::Elem::index_type&, const Point &) const
{
  ASSERTFAIL("VMesh interface: find_closest_elem(dist,Point,coords,Elem::index_type,Point) has not been implemented");
}

bool 
VMesh::find_closest_elem(double&, Point&, VMesh::coords_type&,
                         VMesh::Elem::index_type&, 
                         const Point&, double) const
{
  ASSERTFAIL("VMesh interface: find_closest_elem(dist,Point,coords,Elem::index_type,Point,maxdist) has not been implemented");
}

bool 
VMesh::find_closest_elems(double&, Point&, VMesh::Elem::array_type&, 
                          const Point&) const
{
  ASSERTFAIL("VMesh interface: find_closest_elems(dist,Point,Elem::array_type,Point) has not been implemented");
}

  
bool 
VMesh::get_coords(coords_type&, const Point&, Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_coords(coords_type,Point,Elem::index_type) has not been implemented");
}


void 
VMesh::interpolate(Point &, const coords_type&, Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: interpolate(Point,coords_type,Elem::index_type) has not been implemented");
}

void 
VMesh::minterpolate(std::vector<Point> &, const std::vector<coords_type>&, Elem::index_type ) const
{
  ASSERTFAIL("VMesh interface: minterpolate(std::vector<Point>,std::vector<coords_type>,Elem::index_type) has not been implemented");
}


void 
VMesh::derivate(dpoints_type&, const coords_type&, Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: derivate(dpoints_type,coords_type,Elem::index_type) has not been implemented");
}


void 
VMesh::get_normal(Vector&, coords_type&, Elem::index_type, DElem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_normal() has not been implemented");
}  


void 
VMesh::get_random_point(Point&, Elem::index_type,FieldRNG&) const
{
  ASSERTFAIL("VMesh interface: get_random_point(Point,Elem::index_type) has not been implemented");
}

void 
VMesh::set_point(const Point&, Node::index_type)
{
  ASSERTFAIL("VMesh interface: set_point(Point,Node::index_type) has not been implemented");
}
  
void 
VMesh::set_point(const Point&, ENode::index_type)
{
  ASSERTFAIL("VMesh interface: set_point(Point,ENode::index_type) has not been implemented");
}  

Point*
VMesh::get_points_pointer() const
{
  ASSERTFAIL("VMesh interface: get_points_pointer() has not been implemented");  
}

VMesh::index_type* 
VMesh::get_elems_pointer() const
{
  ASSERTFAIL("VMesh interface: get_elems_pointer() has not been implemented");  
}

void 
VMesh::node_reserve(size_t)
{
  ASSERTFAIL("VMesh interface: node_reserve(size_t size) has not been implemented");
}

void 
VMesh::elem_reserve(size_t)
{
  ASSERTFAIL("VMesh interface: elem_reserve(size_t size) has not been implemented");
}

void 
VMesh::resize_nodes(size_t)
{
  ASSERTFAIL("VMesh interface: resize_nodes(size_t size) has not been implemented");
}

void 
VMesh::resize_elems(size_t)
{
  ASSERTFAIL("VMesh interface: resize_elems(size_t size) has not been implemented");
}


void 
VMesh::add_node(const Point&, Node::index_type&)
{
  ASSERTFAIL("VMesh interface: this mesh cannot be edited (add_node)");  
}

void 
VMesh::add_enode(const Point&, ENode::index_type&)
{
  ASSERTFAIL("VMesh interface: this mesh cannot be edited (add_enode)");  
}

void  
VMesh::add_elem(const Node::array_type&, Elem::index_type&)
{
  ASSERTFAIL("VMesh interface: this mesh cannot be edited (add_elem)");  
}

void 
VMesh::insert_node_into_elem(Elem::array_type&, Node::index_type&,
                             Elem::index_type, Point&)
{
  ASSERTFAIL("VMesh interface: insert_node_into_elem has not been implemented for this mesh type");
}


bool
VMesh::get_neighbor(Elem::index_type&, Elem::index_type, DElem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_neighbor(Elem::index_type,Elem::index_type,DElem::index_type) has not been implemented");  
}

void
VMesh::get_neighbors(Elem::array_type&, Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_neighbors(Elem::index_type,Elem::index_type) has not been implemented");  
}

bool
VMesh::get_neighbors(Elem::array_type&, Elem::index_type, DElem::index_type) const
{
  ASSERTFAIL("VMesh interface: get_neighbors(Elem::array_type,Elem::index_type,DElem::index_type) has not been implemented");  
}

void
VMesh::get_neighbors(Node::array_type&, Node::index_type) const
{
  ASSERTFAIL("VMesh interface: get_neighbors(Node::index_type,Node::index_type) has not been implemented");  
}

void 
VMesh::pwl_approx_edge(std::vector<coords_type >&, Elem::index_type, unsigned int , unsigned int) const
{
  ASSERTFAIL("VMesh interface: pwl_appprox_edge has not been implemented");  
}

void 
VMesh::pwl_approx_face(std::vector<std::vector<coords_type > >&, Elem::index_type, unsigned int, unsigned int) const
{
  ASSERTFAIL("VMesh interface: pwl_appprox_face has not been implemented");  
}

void 
VMesh::get_normal(Vector&,Node::index_type) const
{
  ASSERTFAIL("VMesh interface: get_normal has not been implemented");  
}

double
VMesh::det_jacobian(const coords_type&, Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: det_jacobian has not been implemented");
}

void
VMesh::jacobian(const coords_type&, Elem::index_type, double *) const
{
  ASSERTFAIL("VMesh interface: jacobian has not been implemented");
}

double
VMesh::inverse_jacobian(const coords_type&, Elem::index_type, double*) const
{
  ASSERTFAIL("VMesh interface: inverse_jacobian has not been implemented");
}

double
VMesh::scaled_jacobian_metric(Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: scaled_jacobian_metric has not yet been implemented");
}

double
VMesh::jacobian_metric(Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: jacobian_metric has not yet been implemented");
}

double
VMesh::inscribed_circumscribed_radius_metric(Elem::index_type) const
{
  ASSERTFAIL("VMesh interface: inscribed_circumscribed_radius has not yet been implemented");
}

void
VMesh::get_dimensions(dimension_type& dim)
{
  dim.resize(1);
  Node::size_type sz;
  size(sz);
  dim[0] = sz;
}

void
VMesh::get_elem_dimensions(dimension_type& dim)
{
  dim.resize(1);
  Elem::size_type sz;
  size(sz);
  dim[0] = sz;
}


BBox
VMesh::get_bounding_box() const
{
  ASSERTFAIL("VMesh interface: get_bounding_box has not yet been implemented");  
}

bool
VMesh::synchronize(unsigned int)
{
  ASSERTFAIL("VMesh interface: synchronize has not yet been implemented");  
}

bool
VMesh::unsynchronize(unsigned int)
{
  ASSERTFAIL("VMesh interface: synchronize has not yet been implemented");  
}

bool
VMesh::clear_synchronization()
{
  ASSERTFAIL("VMesh interface: clear_synchronization has not yet been implemented");  
}

void 
VMesh::transform(const Transform &)
{
  ASSERTFAIL("VMesh interface: transform has not yet been implemented");  
}

Transform 
VMesh::get_transform() const
{
  ASSERTFAIL("VMesh interface: get_transform has not yet been implemented");  
}

void VMesh::set_transform(const Transform &)
{
  ASSERTFAIL("VMesh interface: set_transform has not yet been implemented");  
}
 
void 
VMesh::get_canonical_transform(Transform &)
{
  ASSERTFAIL("VMesh interface: get_canonical_transform has not yet been implemented");  
}

double
VMesh::get_epsilon() const
{
  ASSERTFAIL("VMesh interface: get_epsilon has not yet been implemented");  
}


void 
VMesh::get_interpolate_weights(const coords_type&, Elem::index_type, 
                               ElemInterpolate&, int) const
{
  ASSERTFAIL("VMesh interface: get_interpolate_weights has not yet been implemented");  
}
                                       
void 
VMesh::get_interpolate_weights(const Point&, ElemInterpolate&, int) const
{
  ASSERTFAIL("VMesh interface: get_interpolate_weights has not yet been implemented");  
}
        

void 
VMesh::get_minterpolate_weights(const std::vector<coords_type>&, 
                                Elem::index_type, 
                                MultiElemInterpolate&,
                                int) const
{
  ASSERTFAIL("VMesh interface: get_minterpolate_weights has not yet been implemented");  
}
                                       
void 
VMesh::get_minterpolate_weights(const std::vector<Point>&, 
                                MultiElemInterpolate&,
                                int) const
{
  ASSERTFAIL("VMesh interface: get_minterpolate_weights has not yet been implemented");  
}                                       
                                                                                                     

void 
VMesh::get_gradient_weights(const coords_type&, 
                            Elem::index_type, 
                            ElemGradient&,
                            int) const
{
  ASSERTFAIL("VMesh interface: get_gradient_weights has not yet been implemented");  
}

void 
VMesh::get_gradient_weights(const Point&, 
                            ElemGradient&,
                            int) const
{
  ASSERTFAIL("VMesh interface: get_gradient_weights has not yet been implemented");  
}
                                       

void 
VMesh::get_mgradient_weights(const std::vector<coords_type>&, 
                             Elem::index_type, 
                             MultiElemGradient&,
                             int) const
{
  ASSERTFAIL("VMesh interface: get_mgradient_weights has not yet been implemented");  
}

void 
VMesh::get_mgradient_weights(const std::vector<Point>&, 
                             MultiElemGradient&,
                             int) const
{
  ASSERTFAIL("VMesh interface: get_mgradient_weights has not yet been implemented");  
}

void 
VMesh::get_weights(const coords_type&, 
                   std::vector<double>&, 
                   int) const                                 
{
  ASSERTFAIL("VMesh interface: get_weights has not yet been implemented");  
}  

void 
VMesh::get_derivate_weights(const coords_type&, 
                            std::vector<double>&,
                            int) const   
{
  ASSERTFAIL("VMesh interface: get_derivate_weights has not yet been implemented");  
}  


void 
VMesh::get_linear_weights(coords_type&, std::vector<double>&)
{
  ASSERTFAIL("VMesh interface: get_linear_weights has not yet been implemented");  
}


void 
VMesh::get_quadratic_weights(coords_type&, std::vector<double>&)
{
  ASSERTFAIL("VMesh interface: get_quadratic_weights has not yet been implemented");  
}

void 
VMesh::get_cubic_weights(coords_type&, std::vector<double>&)
{
  ASSERTFAIL("VMesh interface: get_cubic_weights has not yet been implemented");  
}

void 
VMesh::get_linear_derivate_weights(coords_type&, std::vector<double>&)
{
  ASSERTFAIL("VMesh interface: get_linear_derivate_weights has not yet been implemented");  
}

void 
VMesh::get_quadratic_derivate_weights(coords_type&, std::vector<double>&)
{
  ASSERTFAIL("VMesh interface: get_quadratic_derivate_weights has not yet been implemented");  
}

void 
VMesh::get_cubic_derivate_weights(coords_type&, std::vector<double>&)
{
  ASSERTFAIL("VMesh interface: get_cubic_derivate_weights has not yet been implemented");  
}

void 
VMesh::get_gaussian_scheme(std::vector<VMesh::coords_type>&, 
                           std::vector<double>&, int) const
{
  ASSERTFAIL("VMesh interface: get_gaussian_scheme has not yet been implemented");  
}
                                   
void 
VMesh::get_regular_scheme(std::vector<VMesh::coords_type>&, 
                          std::vector<double>&, int) const
{
  ASSERTFAIL("VMesh interface: get_regular_scheme has not yet been implemented");  
}
