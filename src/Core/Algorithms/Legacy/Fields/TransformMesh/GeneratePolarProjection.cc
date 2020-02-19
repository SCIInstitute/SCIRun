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


#include <Core/Algorithms/Fields/TransformMesh/GeneratePolarProjection.h>
#include <Core/Algorithms/Fields/TransformMesh/PolarProjectionTools.h>


#include <Core/Geometry/Transform.h>

#include <Core/Datatypes/VField.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/FieldInformation.h>


namespace SCIRunAlgo {

using namespace SCIRun;

bool
GeneratePolarProjectionAlgo::
run(FieldHandle input, FieldHandle& output)
{
  // Mark that we are starting the algorithm, but do not report progress
  algo_start("GeneratePolarProjection");

  // Handle: the function get_rep() returns the pointer contained in the handle
  if (input.get_rep() == 0)
  {
    // If we encounter a null pointer we return an error message and return to
    // the program to deal with this error.
    error("No input field");
    algo_end(); return (false);
  }

  FieldInformation fi(input);
  fi.make_unstructuredmesh();

  output = CreateField(fi);

  Transform t;

  Point origin = get_point("origin");
  Vector axis = get_vector("direction");

  axis.normalize();

  Vector v1, v2;
  axis.find_orthogonal(v1,v2);
  v1.normalize();
  v2.normalize();

  VMesh* omesh = output->vmesh();
  VMesh* imesh = input->vmesh();

  // synchronize these to have all tables ready
  imesh->synchronize(Mesh::NODES_E|Mesh::DELEMS_E|
                     Mesh::ELEM_NEIGHBORS_E|Mesh::NODE_NEIGHBORS_E|
                     Mesh::FIND_CLOSEST_NODE_E);

  VMesh::Node::index_type start_node;

  Point p, p_origin;
  imesh->find_closest_node(p_origin,start_node,origin);
  Point point;

  t.load_basis(p_origin,v1,v2,axis);

  VMesh::size_type num_nodes = imesh->num_nodes();

  for (VMesh::Node::index_type idx=0; idx<num_nodes; idx++)
  {

    imesh->get_center(p,idx);

    double length = 0.0;

    Vector normal = Cross(p-p_origin,axis);
    if(normal.normalize() > 0.0)
    {
      if(!(GenerateEdgeProjection(imesh,start_node,normal,axis,-1.0,idx,point,length)))
        length = 0.0;
    }

    Vector v;
    v = t.unproject(p-p_origin);
    v.z(0.0);
    v = t.project(v);
    v.normalize();

    p = origin + length*v;

    omesh->add_point(p);
  }

  omesh->resize_elems(imesh->num_elems());
  omesh->copy_elems(imesh);

  VField* ifield = input->vfield();
  VField* ofield = output->vfield();

  ofield->resize_values();
  ofield->copy_values(ifield);

  /// Copy properties of the property manager
	output->copy_properties(input.get_rep());

  // Success:
  algo_end(); return (true);
}

} // End namespace SCIRunAlgo
