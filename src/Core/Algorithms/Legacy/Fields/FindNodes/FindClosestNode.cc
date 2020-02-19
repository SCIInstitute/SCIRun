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


#include <Core/Algorithms/Fields/FindNodes/FindClosestNode.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
FindClosestNodeAlgo::run(FieldHandle input, FieldHandle points, std::vector<index_type>& output)
{
  algo_start("FindClosestNode");

  if (input.get_rep() == 0)
  {
    error("Could not obtain input field");
    algo_end(); return (false);
  }
  VMesh* imesh =   input->vmesh();

  if (points.get_rep() == 0)
  {
    error("Could not obtain input field with node locations");
    algo_end(); return (false);
  }

  VMesh*  pmesh = points->vmesh();

  VMesh::Node::size_type nnodes = pmesh->num_nodes();

  if (nnodes == 0)
  {
    error("No nodes locations are given in node mesh");
    algo_end(); return (false);
  }

  output.resize(nnodes);

  imesh->synchronize(Mesh::FIND_CLOSEST_NODE_E);

  for (VMesh::Node::index_type p=0; p< nnodes; p++)
  {
    Point point;
    pmesh->get_center(point,p);
    double dist;
    Point result;
    VMesh::Node::index_type idx;
    imesh->find_closest_node(dist,result,idx,point);
    output[p] = idx;
  }

  algo_end(); return (true);
}


} // end namespace
