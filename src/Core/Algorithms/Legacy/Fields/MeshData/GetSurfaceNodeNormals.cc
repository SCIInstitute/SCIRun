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


#include <Core/Algorithms/Fields/MeshData/GetSurfaceNodeNormals.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
GetSurfaceNodeNormalsAlgo::
run(FieldHandle& input, MatrixHandle& output)
{
  algo_start("GetSurfaceNodeNormals");

  if (input.get_rep() == 0)
  {
    error("No input source field");
    algo_end(); return (false);
  }

  FieldInformation fi(input);
  if (!(fi.is_surface()))
  {
    error("This algorithm only works on a surface mesh");
    algo_end(); return (false);
  }

  VMesh* vmesh = input->vmesh();
  vmesh->synchronize(Mesh::NORMALS_E);
  VMesh::size_type num_nodes = vmesh->num_nodes();

  output = new DenseMatrix(num_nodes,3);

  if (output.get_rep() == 0)
  {
    error("Could not allocate output matrix");
    algo_end(); return (false);
  }

  double* dataptr = output->get_data_pointer();

  index_type k = 0;
  int cnt = 0;
  Vector norm;
  for (VMesh::Node::index_type i=0; i<num_nodes; ++i)
  {
    vmesh->get_normal(norm,i);
    dataptr[k] = norm.x();
    dataptr[k+1] = norm.y();
    dataptr[k+2] = norm.z();
    k += 3;
    cnt++; if (cnt == 400) {cnt=0; update_progress(i,num_nodes); }
  }

  algo_end(); return (true);
}

} // namespace SCIRunAlgo
