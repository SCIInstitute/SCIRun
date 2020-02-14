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


#include <Core/Algorithms/Fields/MeshDerivatives/CalculateMeshConnector.h>

#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>

#include <float.h>

namespace SCIRunAlgo {

bool
CalculateMeshConnectorAlgo::run(FieldHandle input1, FieldHandle input2, FieldHandle& output)
{
  algo_start("CalculateMeshConnector");

  /// Safety check
  if (input1.get_rep() == 0)
  {
    error("The first input field is empty");
    algo_end(); return (false);
  }

  if (input2.get_rep() == 0)
  {
    error("The second input field is empty");
    algo_end(); return (false);
  }

  /// Get the information of the input field
  FieldInformation fo("CurveMesh",-1,"double");

  VMesh* imesh1 = input1->vmesh();
  VMesh* imesh2 = input2->vmesh();

  MeshHandle mesh = CreateMesh(fo);

  Point p1(0.0,0.0,0.0);
  Point p2(0.0,0.0,0.0);

  imesh1->synchronize(Mesh::FIND_CLOSEST_ELEM_E);
  imesh2->synchronize(Mesh::FIND_CLOSEST_ELEM_E);

  VMesh::size_type num_nodes1 = imesh1->num_nodes();
  double min_dist = DBL_MAX;
  double dist;
  VMesh::Elem::index_type eidx;

  if (imesh1->num_nodes() == 0 || imesh2->num_nodes() == 0)
  {
    if (imesh1->num_nodes() == 0 )
      warning("Input field 1 does not have any nodes, output will be an empty field.");
    if (imesh2->num_nodes() == 0 )
      warning("Input field 2 does not have any nodes, output will be an empty field.");
    // Create empty field
    output = CreateField(fo);
    algo_end(); return (true);
  }

  for (VMesh::Node::index_type idx=0; idx<num_nodes1; idx++)
  {
    Point p,p3;
    imesh1->get_center(p,idx);
    imesh2->find_closest_elem(dist,p3,eidx,p);
    if (dist < min_dist)
    {
      min_dist = dist;
      p1 = p3;
    }
  }

  imesh1->find_closest_elem(dist,p2,eidx,p1);
  imesh2->find_closest_elem(dist,p1,eidx,p2);

  output = CreateField(fo);
  output->vmesh()->add_point(p1);
  output->vmesh()->add_point(p2);

  VMesh::Node::array_type array(2);
  array[0] = 0;
  array[1] = 1;
  output->vmesh()->add_elem(array);

  output->vfield()->resize_values();

  algo_end(); return (true);
}

} // end namespace SCIRunAlgo
