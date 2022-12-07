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


#include <Core/Algorithms/Fields/TransformMesh/GeneratePolarProjectionImage.h>
#include <Core/Algorithms/Fields/TransformMesh/PolarProjectionTools.h>

#include <Core/Geometry/Transform.h>

#include <Core/Datatypes/VField.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/FieldInformation.h>


namespace SCIRunAlgo {

using namespace SCIRun;

bool
GeneratePolarProjectionImageAlgo::
run(FieldHandle input,
    FieldHandle& mask_field,
    FieldHandle& point_field,
    FieldHandle& area_field)
{
  FieldHandle x,y,z;
  return (run(input,mask_field,point_field,area_field,x,y,z));
}


bool
GeneratePolarProjectionImageAlgo::
run(FieldHandle input,
    FieldHandle& point_field,
    FieldHandle& mask_field,
    FieldHandle& area_field,
    FieldHandle& x_field,
    FieldHandle& y_field,
    FieldHandle& z_field)
{
  // Mark that we are starting the algorithm, but do not report progress
  algo_start("GeneratePolarProjectionImage");

  // Handle: the function get_rep() returns the pointer contained in the handle
  if (input.get_rep() == 0)
  {
    // If we encounter a null pointer we return an error message and return to
    // the program to deal with this error.
    error("No input field");
    algo_end(); return (false);
  }

  FieldInformation fi(input);

  // First find dimensions of the mesh.

  Point origin = get_point("origin");
  Vector axis = get_vector("direction");

  // Find the two vectors that span the image plane
  Vector v1, v2;
  axis.normalize();
  axis.find_orthogonal(v1,v2);
  v1.normalize();
  v2.normalize();

  VMesh* imesh = input->vmesh();

  // Find the node that is the origin of the projection
  imesh->synchronize(Mesh::NODES_E|Mesh::DELEMS_E|
                     Mesh::ELEM_NEIGHBORS_E|Mesh::NODE_NEIGHBORS_E|
                     Mesh::FIND_CLOSEST_NODE_E);

  VMesh::Node::index_type start_node;
  Point p_origin;

  imesh->find_closest_node(p_origin,start_node,origin);

  // Get number of nodes in mesh
  VMesh::size_type num_nodes = imesh->num_nodes();

  BBox bbox;
  bbox.extend(Point(0.0,0.0,0.0));

  // compute a bounding box around the projection field
  for (VMesh::Node::index_type idx=0; idx<num_nodes; idx++)
  {
    Point p;
    imesh->get_center(p,idx);

    double length = 0.0;
    Point point;

    Vector normal = Cross(p-p_origin,axis);
    if(normal.normalize() > 0.0)
    {
      if(!(GenerateEdgeProjection(imesh,start_node,normal,axis,-1.0,idx,point,length)))
        length = 0.0;
    }

    Vector v = p - p_origin;
    v.safe_normalize();

    double d1 = Abs(Dot(v,v1));
    double d2 = Abs(Dot(v,v2));

    bbox.extend(Point(2.0*(d1/(d1+d2))*length,2.0*(d2/(d1+d2))*length,0.0));
  }

  bbox.extend(0.1*bbox.diagonal().length());

  Vector d = bbox.diagonal();
  double xy = Max(Abs(d.x()),Abs(d.y()));

  Point corner = origin + 0.5*(v1*xy + v2*xy);
  Transform Trans(corner,-v1*xy,-v2*xy,Cross(-v1,-v2));

  VMesh::size_type xsize = get_int("xsize");
  VMesh::size_type ysize = get_int("ysize");

  FieldInformation mask_fo(IMAGEMESH_E,CONSTANTDATA_E,CHAR_E);
  FieldInformation point_fo(IMAGEMESH_E,CONSTANTDATA_E,VECTOR_E);
  FieldInformation area_fo(IMAGEMESH_E,CONSTANTDATA_E,DOUBLE_E);
  FieldInformation x_fo(IMAGEMESH_E,CONSTANTDATA_E,DOUBLE_E);
  FieldInformation y_fo(IMAGEMESH_E,CONSTANTDATA_E,DOUBLE_E);
  FieldInformation z_fo(IMAGEMESH_E,CONSTANTDATA_E,DOUBLE_E);

  MeshHandle mesh = CreateMesh(mask_fo,xsize,ysize,Point(0.0,0.0,0.0),Point(1.0,1.0,0.0));
  mesh->vmesh()->transform(Trans);

  if (mesh.get_rep() == 0)
  {
    error("Could not generate output mesh");
    algo_end(); return (false);
  }

  mask_field = CreateField(mask_fo,mesh);
  point_field = CreateField(point_fo,mesh);
  area_field = CreateField(area_fo,mesh);
  x_field = CreateField(area_fo,mesh);
  y_field = CreateField(area_fo,mesh);
  z_field = CreateField(area_fo,mesh);

  VMesh* omesh = mesh->vmesh();
  VField* mfield = mask_field->vfield();
  VField* pfield = point_field->vfield();
  VField* afield = area_field->vfield();
  VField* xfield = x_field->vfield();
  VField* yfield = y_field->vfield();
  VField* zfield = z_field->vfield();

  if (mfield == 0)
  {
    error("Could not allocate mask field");
    algo_end(); return (false);
  }

  if (pfield == 0)
  {
    error("Could not allocate projection point field");
    algo_end(); return (false);
  }

  if (afield == 0)
  {
    error("Could not allocate area field");
    algo_end(); return (false);
  }

  if (xfield == 0)
  {
    error("Could not allocate x field");
    algo_end(); return (false);
  }

  if (yfield == 0)
  {
    error("Could not allocate y field");
    algo_end(); return (false);
  }

  if (zfield == 0)
  {
    error("Could not allocate z field");
    algo_end(); return (false);
  }

  // Get number of nodes in mesh
  VMesh::size_type num_elems = omesh->num_elems();
  Point p, point;
  VMesh::Node::array_type nodes;
  std::vector<Point> points(4);

  for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
  {
    omesh->get_center(p,idx);
    Vector v = p-origin;
    Vector normal = Cross(v,axis);
    double distance = v.length();
    double length;
    bool valid = true;

    if(normal.normalize() > 0.0)
    {
      if(!(GenerateEdgeProjection(imesh,start_node,normal,axis,distance,-1,point,length)))
      {
        valid = false;
      }
    }
    else
    {
      length = 0.0; point = p_origin;
    }

    // Set mask indicating which pixels actually contain data
    // Set point of the original data where it is located

    omesh->get_nodes(nodes,idx);
    omesh->get_centers(points,nodes);
    double area = 0.0;

    if (points.size() == 4)
    {
      for (size_t k=0;k<4;k++)
      {
        Vector v = points[k]-origin;
        Vector normal = Cross(v,axis);
        double distance = v.length();
        double length;
        Point  point;
        if(normal.normalize() > 0.0)
        {
          if(!(GenerateEdgeProjection(imesh,start_node,normal,axis,distance,-1,points[k],length)))
          {
            valid = false;
          }
        }
        else
        {
          points[k] = p_origin;
        }
      }
    }

    if (valid)
    {
      area = 0.5*(Cross(points[0]-points[1],points[0]-points[3]).length()) +
             0.5*(Cross(points[2]-points[1],points[2]-points[3]).length());
    }

    if (valid)
    {
      mfield->set_value(1,idx);
      pfield->set_value(point.asVector(),idx);
      afield->set_value(area,idx);
      xfield->set_value(point.x(),idx);
      yfield->set_value(point.y(),idx);
      zfield->set_value(point.z(),idx);
    }
    else
    {
      mfield->set_value(0,idx);
      pfield->set_value(Vector(0.0,0.0,0.0),idx);
      afield->set_value(0.0,idx);
      xfield->set_value(0.0,idx);
      yfield->set_value(0.0,idx);
      zfield->set_value(0.0,idx);
    }


  }


  // Success:
  algo_end(); return (true);
}

} // End namespace SCIRunAlgo
