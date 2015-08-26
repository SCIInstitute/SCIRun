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
   Author            : Moritz Dannhauer
   Last modification : March 16 2014 (ported from SCIRun4)
   TODO: Nrrd aoutput
*/

#include <Core/Algorithms/Legacy/Fields/FieldData/GetFieldData.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

using namespace SCIRun;
using namespace Core::Algorithms::Fields;
using namespace Core::Geometry;
using namespace Core::Datatypes;
using namespace Core::Utility;
using namespace Core::Algorithms;
using namespace Core::Logging;

GetFieldDataAlgo::GetFieldDataAlgo()
{

}

AlgorithmOutput GetFieldDataAlgo::run_generic(const AlgorithmInput& input) const
{
  auto input_field = input.get<Field>(Variables::InputField);
 
  DenseMatrixHandle output_matrix;
  output_matrix = run(input_field);
  
  AlgorithmOutput output;
  output[Variables::OutputMatrix] = output_matrix;

  return output;
}

/// Function call to convert data from Field into Matrix data
DenseMatrixHandle GetFieldDataAlgo::run(FieldHandle input_field) const
{
  if (!input_field)
  {
    THROW_ALGORITHM_INPUT_ERROR("No input field was provided");
  }

  DenseMatrixHandle output;

  VField* vfield1 = input_field->vfield();

  /// Check whether we have data
  if (! vfield1)
  {
    THROW_ALGORITHM_INPUT_ERROR("Could not obtain VField interface");
  }

  if (vfield1->is_nodata())
  {
    THROW_ALGORITHM_INPUT_ERROR("Invalid input field (no data)");
  }

  if (vfield1->is_scalar())
    return (GetScalarFieldDataV(input_field));
  if (vfield1->is_vector())
    return (GetVectorFieldDataV(input_field));
  if (vfield1->is_tensor())
    return (GetTensorFieldDataV(input_field));
  
  THROW_ALGORITHM_INPUT_ERROR("Unknown field data type!");
}


DenseMatrixHandle GetFieldDataAlgo::GetScalarFieldDataV(FieldHandle& input) const
{
  /// Obtain virtual interface
  VField* vfield = input->vfield();
  
  /// Obtain the number values in a field
  VMesh::size_type size = vfield->num_values();
  VMesh::size_type esize = vfield->num_evalues();
  
  /// Create output object
  DenseMatrixHandle output(new DenseMatrix(size+esize, 1));
  
  if (!output)
  {
    THROW_ALGORITHM_INPUT_ERROR("Could not allocate output matrix");
    return DenseMatrixHandle();
  }
    
  for (VMesh::Elem::index_type idx = 0; idx < size; idx++)
  { 
     vfield->get_value((*output)(idx, 0),idx);
  }
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (vfield->basis_order() == 2)
  {
    vfield->vmesh()->synchronize(Mesh::EDGES_E);
    for (VMesh::Elem::index_type idx = size; idx < size+esize; idx++)
    {
      vfield->get_evalue((*output)(idx, 0),idx);
    }
  } 
#endif
  
  return output;
}


DenseMatrixHandle GetFieldDataAlgo::GetVectorFieldDataV(FieldHandle& input) const
{
  VField* vfield = input->vfield();
  
  VMesh::size_type size = vfield->num_values();
  VMesh::size_type esize = vfield->num_evalues();
  
  DenseMatrixHandle output(new DenseMatrix(size+esize, 3));

  if (!output)
  {
    THROW_ALGORITHM_INPUT_ERROR("Could not allocate output matrix");
    return DenseMatrixHandle();
  }

  Vector val;
  for (VMesh::index_type idx=0; idx<size; idx++)
  {
    vfield->get_value(val,idx);
    (*output)(idx, 0) = val.x();
    (*output)(idx, 1) = val.y();
    (*output)(idx, 2) = val.z();
  }
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (vfield->basis_order() == 2)
  {
    vfield->vmesh()->synchronize(Mesh::EDGES_E);
   
    for (VMesh::index_type idx=size; idx<esize+size; idx++)
    {
      vfield->get_evalue(val,idx);
      (*output)(idx, 0) = val.x();
      (*output)(idx, 1) = val.y();
      (*output)(idx, 2) = val.z();
    }  
  }
  #endif
 
  return output;
}


DenseMatrixHandle GetFieldDataAlgo::GetTensorFieldDataV(FieldHandle& input) const
{
  VField* vfield = input->vfield();
  
  VMesh::size_type size = vfield->num_values();
  VMesh::size_type esize = vfield->num_evalues();
  
  DenseMatrixHandle output(new DenseMatrix(size+esize, 6));

  if (!output)
  {
    THROW_ALGORITHM_INPUT_ERROR("Could not allocate output matrix");
    return DenseMatrixHandle();
  }

  Tensor val;
  for (VMesh::index_type idx=0; idx<size; idx++)
  {
    vfield->get_value(val,idx);
    (*output)(idx, 0) = val.val(0,0);
    (*output)(idx, 1) = val.val(0,1);
    (*output)(idx, 2) = val.val(0,2);
    (*output)(idx, 3) = val.val(1,1);
    (*output)(idx, 4) = val.val(1,2);
    (*output)(idx, 5) = val.val(2,2);   
  }
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (vfield->basis_order() == 2)
  {
     vfield->vmesh()->synchronize(Mesh::EDGES_E);
   
    for (VMesh::index_type idx=size; idx<esize+size; idx++)
    {
      vfield->get_evalue(val,idx);
      (*output)(idx, 0) = static_cast<double>(val.mat_[0][0]);
      (*output)(idx, 1) = static_cast<double>(val.mat_[0][1]);
      (*output)(idx, 2) = static_cast<double>(val.mat_[0][2]);
      (*output)(idx, 3) = static_cast<double>(val.mat_[1][1]);
      (*output)(idx, 4) = static_cast<double>(val.mat_[1][2]);
      (*output)(idx, 5) = static_cast<double>(val.mat_[2][2]);    
    }  
  }
  #endif

  return output;
}


#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

/// Function call to convert data from Field into Matrix data
bool GetFieldDataAlgo::run(FieldHandle& input)
{
  algo_start("GetFieldData");
  
  /// Check whether we have a field.
  if (input.get_rep() == 0)
  {
    error("No input source field");
    algo_end(); return (false);
  }
  
  /// Construct a class with all the type information of this field
  FieldInformation fi(input);

  /// Check whether we have data
  if (fi.is_nodata())
  {
    error("Field does not contain any data");
    algo_end(); return (false);
  }
  
  /// Depending on the data type select a sub algorithm
  if (fi.is_scalar())
    return(GetScalarFieldDataV(this,input,output));

  else if (fi.is_vector())
    return(GetVectorFieldDataV(this,input,output));

  else if (fi.is_tensor())
    return(GetTensorFieldDataV(this,input,output));

  error("Unknown data type");
  algo_end(); return (false);*/
}

bool 
GetScalarFieldDataV(AlgoBase *algo, FieldHandle& input, NrrdDataHandle& output)
{
  /// Obtain virtual interface
  VField* vfield = input->vfield();
  
  /// Obtain the number values in a field
  VMesh::size_type size = vfield->num_values();
  VMesh::size_type esize = vfield->num_evalues();
  
  /// Create output object
  output = new NrrdData();

  if (output.get_rep() == 0)
  {
    algo->error("Could not allocate output nrrd");
    algo->algo_end(); return (false);
  }
  
  output->nrrd_ = nrrdNew();

  if (output->nrrd_ == 0)
  {
    algo->error("Could not allocate output nrrd");
    algo->algo_end(); return (false);
  }

  size_t nrrddims[1]; 
  nrrddims[0] = size+esize;

  if (vfield->is_char())
  {
    nrrdAlloc_nva(output->nrrd_,nrrdTypeChar,1,nrrddims);
    char* dataptr = reinterpret_cast<char *>(output->nrrd_->data);
    // get all the values as doubles
    vfield->get_values(dataptr,size);
    
    if (vfield->basis_order() == 2)
    {
      vfield->vmesh()->synchronize(Mesh::EDGES_E);
      vfield->get_evalues(dataptr+size,esize);
    }
  }

  if (vfield->is_unsigned_char())
  {
    nrrdAlloc_nva(output->nrrd_,nrrdTypeUChar,1,nrrddims);
    unsigned char* dataptr = reinterpret_cast<unsigned char *>(output->nrrd_->data);
    // get all the values as doubles
    vfield->get_values(dataptr,size);
    
    if (vfield->basis_order() == 2)
    {
      vfield->vmesh()->synchronize(Mesh::EDGES_E);
      vfield->get_evalues(dataptr+size,esize);
    }
  }

  if (vfield->is_short())
  {
    nrrdAlloc_nva(output->nrrd_,nrrdTypeShort,1,nrrddims);
    short* dataptr = reinterpret_cast<short *>(output->nrrd_->data);
    // get all the values as doubles
    vfield->get_values(dataptr,size);
    
    if (vfield->basis_order() == 2)
    {
      vfield->vmesh()->synchronize(Mesh::EDGES_E);
      vfield->get_evalues(dataptr+size,esize);
    }
  }

  if (vfield->is_unsigned_short())
  {
    nrrdAlloc_nva(output->nrrd_,nrrdTypeUShort,1,nrrddims);
    unsigned short* dataptr = reinterpret_cast<unsigned short *>(output->nrrd_->data);
    // get all the values as doubles
    vfield->get_values(dataptr,size);
    
    if (vfield->basis_order() == 2)
    {
      vfield->vmesh()->synchronize(Mesh::EDGES_E);
      vfield->get_evalues(dataptr+size,esize);
    }
  }  

  if (vfield->is_int())
  {
    nrrdAlloc_nva(output->nrrd_,nrrdTypeInt,1,nrrddims);
    int* dataptr = reinterpret_cast<int *>(output->nrrd_->data);
    // get all the values as doubles
    vfield->get_values(dataptr,size);
    
    if (vfield->basis_order() == 2)
    {
      vfield->vmesh()->synchronize(Mesh::EDGES_E);
      vfield->get_evalues(dataptr+size,esize);
    }
  }

  if (vfield->is_unsigned_int())
  {
    nrrdAlloc_nva(output->nrrd_,nrrdTypeUInt,1,nrrddims);
    unsigned int* dataptr = reinterpret_cast<unsigned int *>(output->nrrd_->data);
    // get all the values as doubles
    vfield->get_values(dataptr,size);
    
    if (vfield->basis_order() == 2)
    {
      vfield->vmesh()->synchronize(Mesh::EDGES_E);
      vfield->get_evalues(dataptr+size,esize);
    }
  }

  if (vfield->is_longlong())
  {
    nrrdAlloc_nva(output->nrrd_,nrrdTypeLLong,1,nrrddims);
    long long* dataptr = reinterpret_cast<long long *>(output->nrrd_->data);
    // get all the values as doubles
    vfield->get_values(dataptr,size);
    
    if (vfield->basis_order() == 2)
    {
      vfield->vmesh()->synchronize(Mesh::EDGES_E);
      vfield->get_evalues(dataptr+size,esize);
    }
  }

  if (vfield->is_unsigned_longlong())
  {
    nrrdAlloc_nva(output->nrrd_,nrrdTypeULLong,1,nrrddims);
    unsigned long long* dataptr = reinterpret_cast<unsigned long long *>(output->nrrd_->data);
    // get all the values as doubles
    vfield->get_values(dataptr,size);
    
    if (vfield->basis_order() == 2)
    {
      vfield->vmesh()->synchronize(Mesh::EDGES_E);
      vfield->get_evalues(dataptr+size,esize);
    }
  }  

  if (vfield->is_float())
  {
    nrrdAlloc_nva(output->nrrd_,nrrdTypeFloat,1,nrrddims);
    float* dataptr = reinterpret_cast<float *>(output->nrrd_->data);
    // get all the values as doubles
    vfield->get_values(dataptr,size);
    
    if (vfield->basis_order() == 2)
    {
      vfield->vmesh()->synchronize(Mesh::EDGES_E);
      vfield->get_evalues(dataptr+size,esize);
    }
  }

  if (vfield->is_double())
  {
    nrrdAlloc_nva(output->nrrd_,nrrdTypeDouble,1,nrrddims);
    double* dataptr = reinterpret_cast<double *>(output->nrrd_->data);
    // get all the values as doubles
    vfield->get_values(dataptr,size);
    
    if (vfield->basis_order() == 2)
    {
      vfield->vmesh()->synchronize(Mesh::EDGES_E);
      vfield->get_evalues(dataptr+size,esize);
    }
  }
  
  algo->algo_end(); return (true);*/
}


bool 
GetVectorFieldDataV(AlgoBase *algo, FieldHandle& input, NrrdDataHandle& output)
{
  VField* vfield = input->vfield();
  
  VMesh::size_type size = vfield->num_values();
  VMesh::size_type esize = vfield->num_evalues();
  
  output = new NrrdData();

  if (output.get_rep() == 0)
  {
    algo->error("Could not allocate output nrrd");
    algo->algo_end(); return (false);
  }
  
  output->nrrd_ = nrrdNew();

  if (output->nrrd_ == 0)
  {
    algo->error("Could not allocate output nrrd");
    algo->algo_end(); return (false);
  }

  size_t nrrddims[2]; 
  nrrddims[0] = 3;
  nrrddims[1] = size+esize;

  nrrdAlloc_nva(output->nrrd_,nrrdTypeDouble,2,nrrddims);
  double* dataptr = reinterpret_cast<double *>(output->nrrd_->data);

  Vector val;
  int k = 0;
  for (VMesh::index_type i=0; i<size; i++)
  {
    vfield->get_value(val,i);
    dataptr[k] = val.x();
    dataptr[k+1] = val.y();
    dataptr[k+2] = val.z();
    k+=3;
  }
  
  if (vfield->basis_order() == 2)
  {
    vfield->vmesh()->synchronize(Mesh::EDGES_E);

    for (VMesh::index_type i=0; i<esize; i++)
    {
      vfield->get_evalue(val,i);
      dataptr[k] = val.x();
      dataptr[k+1] = val.y();
      dataptr[k+2] = val.z();
      k+=3;
    }  
  }
  algo->algo_end(); return (true);*/
}


bool
GetTensorFieldDataV(AlgoBase *algo, FieldHandle& input, NrrdDataHandle& output)
{
  VField* vfield = input->vfield();
  
  VMesh::size_type size = vfield->num_values();
  VMesh::size_type esize = vfield->num_evalues();

  output = new NrrdData();

  if (output.get_rep() == 0)
  {
    algo->error("Could not allocate output nrrd");
    algo->algo_end(); return (false);
  }
  
  output->nrrd_ = nrrdNew();

  if (output->nrrd_ == 0)
  {
    algo->error("Could not allocate output nrrd");
    algo->algo_end(); return (false);
  }

  size_t nrrddims[2]; 
  nrrddims[0] = 6;
  nrrddims[1] = size+esize;

  nrrdAlloc_nva(output->nrrd_,nrrdTypeDouble,2,nrrddims);
  double* dataptr = reinterpret_cast<double *>(output->nrrd_->data);

  Tensor val;
  int k = 0;
  for (VMesh::index_type i=0; i<size; i++)
  {
    vfield->get_value(val,i);
    dataptr[k] = static_cast<double>(val.mat_[0][0]);
    dataptr[k+1] = static_cast<double>(val.mat_[0][1]);
    dataptr[k+2] = static_cast<double>(val.mat_[0][2]);
    dataptr[k+3] = static_cast<double>(val.mat_[1][1]);
    dataptr[k+4] = static_cast<double>(val.mat_[1][2]);
    dataptr[k+5] = static_cast<double>(val.mat_[2][2]);    
    k+=6;
  }
  
  if (vfield->basis_order() == 2)
  {
    vfield->vmesh()->synchronize(Mesh::EDGES_E);
    
    for (VMesh::index_type i=0; i<esize; i++)
    {
      vfield->get_evalue(val,i);
      dataptr[k] = static_cast<double>(val.mat_[0][0]);
      dataptr[k+1] = static_cast<double>(val.mat_[0][1]);
      dataptr[k+2] = static_cast<double>(val.mat_[0][2]);
      dataptr[k+3] = static_cast<double>(val.mat_[1][1]);
      dataptr[k+4] = static_cast<double>(val.mat_[1][2]);
      dataptr[k+5] = static_cast<double>(val.mat_[2][2]);    
      k+=6;
    }  
  }
  algo->algo_end(); return (true);

}



} // namespace SCIRunAlgo
#endif
