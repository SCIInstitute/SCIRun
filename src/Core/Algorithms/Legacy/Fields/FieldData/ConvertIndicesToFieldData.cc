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


#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <Core/Algorithms/Fields/FieldData/ConvertIndicesToFieldData.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
ConvertIndicesToFieldDataAlgo::
run( FieldHandle input, MatrixHandle data, FieldHandle& output)
{
  algo_start("ConvertIndicesToFieldData");

  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  if (data.get_rep() == 0)
  {
    error("No matrix with data");
    algo_end(); return (false);
  }

  // no precompiled version available, so compile one

  FieldInformation fi(input);
  FieldInformation fo(input);

  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    algo_end(); return (false);
  }

  if (fi.is_nodata())
  {
    error("This function has not yet been defined for fields with no data");
    algo_end(); return (false);
  }

  if (fi.is_vector()||fi.is_tensor())
  {
    error("This function has not yet been defined for fields with vectors or tensors as indices");
    algo_end(); return (false);
  }

  size_type nrows = data->nrows();
  size_type ncols = data->ncols();

  std::string algotype;

  if (ncols == 1)
  {
    algotype = "Scalar";
  }
  else if (ncols == 3)
  {
    algotype = "Vector";
  }
  else if (ncols == 6 || ncols == 9)
  {
    algotype = "Tensor";
  }
  else
  {
    if (nrows == 1)
    {
      algotype = "Scalar";
    }
    else if (nrows == 3)
    {
      algotype = "Vector";
    }
    else if (nrows == 6 || nrows == 9)
    {
      algotype = "Tensor";
    }
    else
    {
      error("Data does not have dimension of 1, 3, 6, or 9");
      algo_end(); return (false);
    }
  }

  if (algotype == "Scalar")
	{
    std::string datatype;
		get_option("datatype",datatype);
    fo.set_data_type(datatype);
	}
	if (algotype == "Vector") fo.make_vector();
  if (algotype == "Tensor") fo.make_tensor();

  //--------------------------------
  // VIRTUAL INTERFACE

  output = CreateField(fo,input->mesh());
  VField* vinput = input->vfield();
  VField* voutput = output->vfield();
  voutput->resize_fdata();

  DenseMatrix* dm = data->dense();
  MatrixHandle dmh = dm;

  if(algotype == "Scalar")
  {
    int max_index = data->nrows() * data->ncols();
    double *dataptr = dm->get_data_pointer();

    VMesh::size_type sz = vinput->num_values();
    for(VMesh::index_type r=0; r<sz; r++)
    {
      int idx;
      vinput->get_value(idx,r);
      if ((idx < 0)|| (idx >= max_index))
      {
        error("Index exceeds matrix dimensions");
        algo_end(); return (false);
      }
      voutput->set_value(dataptr[idx],r);
    }
    algo_end(); return (true);
  }
  else if (algotype == "Vector")
  {
    if (data->ncols() != 3)
    {
      MatrixHandle temp = dmh;
      dmh = dm->make_transpose();
      dm = dmh->dense();
    }

    double *dataptr = dmh->get_data_pointer();
    int max_index = dmh->nrows();

    VMesh::size_type sz = vinput->num_values();
    for(VMesh::index_type r=0; r<sz; r++)
    {
      int idx;
      vinput->get_value(idx,r);
      if ((idx < 0)|| (idx >= max_index))
      {
        error("Index exceeds matrix dimensions");
        algo_end(); return (false);
      }
      voutput->set_value(Vector(dataptr[3*idx],dataptr[3*idx+1],dataptr[3*idx+2]),r);
    }
    algo_end(); return (true);
  }
  else if (algotype == "Tensor")
  {
    if ((data->ncols() != 6)&&(data->ncols() != 9))
    {
      MatrixHandle temp = dmh;
      dmh = dm->make_transpose();
      dm = dmh->dense();
    }

    int max_index = dmh->nrows();
    double *dataptr = dm->get_data_pointer();
    int ncols = dmh->ncols();

    VMesh::size_type sz = vinput->num_values();
    for(VMesh::index_type r=0; r<sz; r++)
    {
      int idx;
      vinput->get_value(idx,r);
      if ((idx < 0)|| (idx >= max_index))
      {
        error("Index exceeds matrix dimensions");
        algo_end(); return (false);
      }
      if (ncols == 6)
      {
        voutput->set_value(Tensor(dataptr[3*idx],dataptr[3*idx+1],dataptr[3*idx+2],dataptr[3*idx+3],dataptr[3*idx+4],dataptr[3*idx+5]),r);
      }
      else
      {
        voutput->set_value(Tensor(dataptr[3*idx],dataptr[3*idx+1],dataptr[3*idx+2],dataptr[3*idx+4],dataptr[3*idx+5],dataptr[3*idx+8]),r);
      }
    }
    algo_end(); return (true);
  }

  // keep the compiler happy:
  // it seems reasonable to return false if none of the cases apply (AK)
  return (false);
}


} // End namespace SCIRunAlgo
