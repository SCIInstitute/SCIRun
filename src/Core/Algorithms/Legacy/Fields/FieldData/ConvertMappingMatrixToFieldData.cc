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


#include <Core/Algorithms/Fields/FieldData/ConvertMappingMatrixToFieldData.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool ConvertMappingMatrixToFieldDataAlgo::run(FieldHandle input, MatrixHandle mappingmatrix, FieldHandle& output)
{
  algo_start("ConvertMappingMatrixToFieldData.");

  if (input.get_rep() == 0)
  {
    error("No input field");
  }

  if (mappingmatrix.get_rep() == 0)
  {
    error("No mapping matrix input.");
  }

  // no precompiled version available, so compile one

  FieldInformation fi(input);
  FieldInformation fo(input);
  FieldInformation fo2(input);

  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for higher order elements.");
    algo_end(); return (false);
  }

  fo.set_data_type("unsigned int");
  fo.make_lineardata();

  fo2.set_data_type("unsigned int");
  fo2.make_constantdata();

  VField* field = input->vfield();

  if (!matrix_is::sparse(mappingmatrix))
  {
    error("Matrix is not a sparse matrix");
    algo_end(); return(false);
  }

  SparseRowMatrix* sm = mappingmatrix->sparse();
  index_type* rr  = sm->get_rows();
  index_type* cc  = sm->get_cols();
  size_type   m   = sm->nrows();

  for (size_type p=0; p<m+1; p++)
  {
    if (rr[p] != p)
    {
      error("This mapping matrix does not do a one-to-one mapping.");
      algo_end(); return(false);
    }
  }

  VMesh *mesh = field->vmesh();

  VMesh::Elem::size_type num_elems = mesh->num_elems();
  VMesh::Node::size_type num_nodes = mesh->num_nodes();

  if (m == num_elems)
  {
    output = CreateField(fo2,input->mesh());
    if (output.get_rep()== 0)
    {
      error("Could not allocate output field.");
      algo_end(); return(false);
    }

    VField* ofield = output->vfield();
    for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
    {
      ofield->set_value(cc[idx],idx);
    }
  }
  else if (m == num_nodes)
  {
    output = CreateField(fo,input->mesh());
    if (output.get_rep()== 0)
    {
      error("Could not allocate output field.");
      algo_end(); return(false);
    }

    VField* ofield = output->vfield();
    for (VMesh::Node::index_type idx=0; idx<num_nodes; idx++)
    {
      ofield->set_value(cc[idx],idx);
    }
  }
  else
  {
    error("Number of nodes or elements is not equal to the number of rows of the mapping matrix.");
    algo_end(); return(false);
  }

  return(true);
}

} //end namespace SCIRunAlgo
