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

#include <Core/Algorithms/Legacy/Fields/MeshData/GetMeshNodes.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
GetMeshNodesAlgo::
run(FieldHandle& input, MatrixHandle& output)
{
  algo_start("GetMeshNodes");

  if (input.get_rep() == 0)
  {
    error("No input source field");
    algo_end(); return (false);
  }
  
  VMesh* vmesh = input->vmesh();
  VMesh::size_type size = vmesh->num_nodes();
  
  output = new DenseMatrix(size,3);

  if (output.get_rep() == 0)
  {
    error("Could not allocate output matrix");
    algo_end(); return (false);
  }

  FieldInformation fi(input);

  if (fi.is_regularmesh())
  {
    double* dataptr = output->get_data_pointer();
    Point p;
    index_type k = 0;
    int cnt = 0;
    for (VMesh::Node::index_type i=0; i<size; ++i)
    {
      vmesh->get_center(p,i);
      dataptr[k] = p.x();
      dataptr[k+1] = p.y();
      dataptr[k+2] = p.z();
      k += 3;
      cnt++; if (cnt == 400) {cnt=0; update_progress(i,size); }
    }

  }
  else
  {
    double* dataptr = output->get_data_pointer();
    Point*  points  = vmesh->get_points_pointer();

    Point p;
    index_type k = 0;
    int cnt = 0;
    for (VMesh::Node::index_type i=0; i<size; ++i)
    {
      p = points[i];
      dataptr[k] = p.x();
      dataptr[k+1] = p.y();
      dataptr[k+2] = p.z();
      k += 3;
      cnt++; if (cnt == 400) {cnt=0; update_progress(i,size); }
    }
  }
  
  algo_end(); return (true);
}

} // namespace SCIRunAlgo
