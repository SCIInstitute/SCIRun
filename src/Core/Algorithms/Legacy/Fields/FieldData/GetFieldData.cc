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

#include <Core/Algorithms/Fields/FieldData/GetFieldData.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/FieldInformation.h>

/// Namespace used for SCIRun Algorithmic layer
namespace SCIRunAlgo {

using namespace SCIRun;

bool 
GetScalarFieldDataV(AlgoBase *algo, FieldHandle& input, MatrixHandle& output);

bool 
GetVectorFieldDataV(AlgoBase *algo, FieldHandle& input, MatrixHandle& output);

bool 
GetTensorFieldDataV(AlgoBase *algo, FieldHandle& input, MatrixHandle& output);


/// Function call to convert data from Field into Matrix data
bool 
GetFieldDataAlgo::
run(FieldHandle& input, MatrixHandle& output)
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
  algo_end(); return (false);
}


bool 
GetScalarFieldDataV(AlgoBase *algo, FieldHandle& input, MatrixHandle& output)
{
  /// Obtain virtual interface
  VField* vfield = input->vfield();
  
  /// Obtain the number values in a field
  VMesh::size_type size = vfield->num_values();
  VMesh::size_type esize = vfield->num_evalues();
  
  /// Create output object
  output = new DenseMatrix(size+esize,1);
  if (output.get_rep() == 0)
  {
    algo->error("Could not allocate output matrix");
    algo->algo_end(); return (false);
  }

  double* dataptr = output->get_data_pointer();
  // get all the values as doubles
  vfield->get_values(dataptr,size);
  
  if (vfield->basis_order() == 2)
  {
    vfield->vmesh()->synchronize(Mesh::EDGES_E);
    vfield->get_evalues(dataptr+size,esize);
  }
  
  algo->algo_end(); return (true);
}


bool 
GetVectorFieldDataV(AlgoBase *algo, FieldHandle& input, MatrixHandle& output)
{
  VField* vfield = input->vfield();
  
  VMesh::size_type size = vfield->num_values();
  VMesh::size_type esize = vfield->num_evalues();
  
  output = new DenseMatrix(size+esize,3);
  if (output.get_rep() == 0)
  {
    algo->error("Could not allocate output matrix");
    algo->algo_end(); return (false);
  }
  double* dataptr = output->get_data_pointer();

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
  algo->algo_end(); return (true);
}


bool
GetTensorFieldDataV(AlgoBase *algo, FieldHandle& input, MatrixHandle& output)
{
  VField* vfield = input->vfield();
  
  VMesh::size_type size = vfield->num_values();
  VMesh::size_type esize = vfield->num_evalues();
  
  output = new DenseMatrix(size+esize,6);
  if (output.get_rep() == 0)
  {
    algo->error("Could not allocate output matrix");
    algo->algo_end(); return (false);
  }
  double* dataptr = output->get_data_pointer();

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



bool 
GetScalarFieldDataV(AlgoBase *algo, FieldHandle& input, NrrdDataHandle& output);

bool 
GetVectorFieldDataV(AlgoBase *algo, FieldHandle& input, NrrdDataHandle& output);

bool 
GetTensorFieldDataV(AlgoBase *algo, FieldHandle& input, NrrdDataHandle& output);



/// Function call to convert data from Field into Matrix data
bool 
GetFieldDataAlgo::
run(FieldHandle& input, NrrdDataHandle& output)
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
  algo_end(); return (false);
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
  
  algo->algo_end(); return (true);
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
  algo->algo_end(); return (true);
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
