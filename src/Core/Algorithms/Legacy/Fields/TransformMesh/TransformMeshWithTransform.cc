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

#include <Core/Algorithms/Fields/TransformMesh/TransformMeshWithTransform.h>
#include <Core/Datatypes/VField.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
TransformMeshWithTransformAlgo::
run(FieldHandle input, MatrixHandle transform_matrix, FieldHandle& output)
{
  // Mark that we are starting the algorithm, but do not report progress
  algo_start("TransformMeshWithTransform");
  
  bool rotate_data = get_bool("rotate_data");
  
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }
  
  if (transform_matrix.get_rep() == 0)
  {
    error("No input transform matrix");
    algo_end(); return (false);
  }
  
  if ((transform_matrix->ncols() != 4 && transform_matrix->nrows()) || 
      !(matrix_is::dense(transform_matrix)))
  {
    error("Input matrix needs to be a 4 by 4 dense matrix");
    algo_end(); return (false);  
  }
  output = input;
  output.detach();
  output->mesh_detach();
  
  VMesh* mesh = output->vmesh();
  VField* field = output->vfield();
  
  Transform transform;
  transform.set(transform_matrix->get_data_pointer());
  
  mesh->vmesh()->transform(transform);
  
  if (field->is_vector() || field->is_tensor())
  {
    if (rotate_data == true)
    {
      if (field->is_vector())
      {
        VMesh::size_type sz = field->num_values();
        for (VMesh::index_type i=0; i < sz; i++)
        {
          Vector v;
          field->get_value(v,i);
          v = transform*v;
          field->set_value(v,i);
        }
      }
      if (field->is_tensor())
      {
        VMesh::size_type sz = field->num_values();
        for (VMesh::index_type i=0; i < sz; i++)
        {
          Tensor v;
          field->get_value(v,i);
          v = transform*v*transform;
          field->set_value(v,i);
        }
      }    
    }
  }
  
  output->copy_properties(input.get_rep());

  algo_end(); return (true);
}

} // End namespace SCIRunAlgo
