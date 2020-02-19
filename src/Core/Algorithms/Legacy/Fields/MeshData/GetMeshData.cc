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


#include <Core/Algorithms/Legacy/Fields/MeshData/GetMeshData.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/VField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

bool GetMeshDataAlgo::GetScalarFieldDataV( FieldHandle& input, Core::Datatypes::DenseMatrixHandle& output) const
{
  //! Obtain virtual interface
  VField* vfield = input->vfield();

  //! Obtain the number values in a field
  VMesh::size_type size = vfield->num_values();
  VMesh::size_type esize = vfield->num_evalues();

  //! Create output object
  output = boost::make_shared<DenseMatrix>(size+esize, 1);
  if (!output)
  {
    error("Could not allocate output matrix");
    return (false);
  }

  double* dataptr = output->data();
  // get all the values as doubles
  vfield->get_values(dataptr,size);

  if (vfield->basis_order() == 2)
  {
    vfield->vmesh()->synchronize(Mesh::EDGES_E);
    vfield->get_evalues(dataptr+size,esize);
  }

  return (true);
}

bool GetMeshDataAlgo::GetVectorFieldDataV(FieldHandle& input, Core::Datatypes::DenseMatrixHandle& output) const
{

  VField* vfield = input->vfield();

  VMesh::size_type size = vfield->num_values();
  VMesh::size_type esize = vfield->num_evalues();

  output = boost::make_shared<DenseMatrix>(size+esize,3);
  if ( !output )
  {
    error("Could not allocate output matrix");
    return (false);
  }

  Vector val;
  Point p;
  int cnt = 0;
  double tmp_size=0.0;
  if (vfield->basis_order() == 2) tmp_size=size+esize; else tmp_size=size;

  for (VMesh::index_type i=0; i<size; i++)
  {
     vfield->get_value(val,i);
    (*output)(i,0) = val.x();
    (*output)(i,1) = val.y();
    (*output)(i,2) = val.z();
    cnt++;
    if (cnt == 400)
    {
      cnt = 0;
      update_progress_max(i,tmp_size);
    }
  }
 cnt = 0;
  if (vfield->basis_order() == 2)
  {
    vfield->vmesh()->synchronize(Mesh::EDGES_E);

    for (VMesh::index_type i=0; i<esize; i++)
    {
      vfield->get_evalue(val,i);
      (*output)(i+size,0) = val.x();
      (*output)(i+size,1) = val.y();
      (*output)(i+size,2) = val.z();
      cnt++;
      if (cnt == 400)
      {
       cnt = 0;
       update_progress_max(i+size,tmp_size);
      }
    }
  }
  return (true);
}

bool GetMeshDataAlgo::GetTensorFieldDataV(FieldHandle& input, Core::Datatypes::DenseMatrixHandle& output) const
{
  VField* vfield = input->vfield();

  VMesh::size_type size = vfield->num_values();
  VMesh::size_type esize = vfield->num_evalues();

  output = boost::make_shared<DenseMatrix>(size+esize,6);
  if (!output)
  {
    error("Could not allocate output matrix");
   return (false);
  }
  //double* dataptr = output->get_data_pointer();

  Tensor val;

  for (VMesh::index_type i=0; i<size; i++)
  {
    vfield->get_value(val,i);
    (*output)(i,0) =  static_cast<double>(val.mat_[0][0]);
    (*output)(i,1) =  static_cast<double>(val.mat_[0][1]);
    (*output)(i,2) =  static_cast<double>(val.mat_[0][2]);
    (*output)(i,3) =  static_cast<double>(val.mat_[1][1]);
    (*output)(i,4) =  static_cast<double>(val.mat_[1][2]);
    (*output)(i,5) =  static_cast<double>(val.mat_[2][2]);
  }

  if (vfield->basis_order() == 2)
  {
    vfield->vmesh()->synchronize(Mesh::EDGES_E);

    for (VMesh::index_type i=0; i<esize; i++)
    {
      vfield->get_evalue(val,i);
      (*output)(i+size,0) =  static_cast<double>(val.mat_[0][0]);
      (*output)(i+size,1) =  static_cast<double>(val.mat_[0][1]);
      (*output)(i+size,2) =  static_cast<double>(val.mat_[0][2]);
      (*output)(i+size,3) =  static_cast<double>(val.mat_[1][1]);
      (*output)(i+size,4) =  static_cast<double>(val.mat_[1][2]);
      (*output)(i+size,5) =  static_cast<double>(val.mat_[2][2]);
    }
  }
  return (true);

}



bool GetMeshDataAlgo::run(FieldHandle& input, DenseMatrixHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "GetMeshData");

  //! Check whether we have a field.
  if (!input)
  {
    error("No input source field");
     return (false);
  }

  //! Construct a class with all the type information of this field
  FieldInformation fi(input);

  //! Check whether we have data
  if (fi.is_nodata())
  {
    error("Field does not contain any data");
    return (false);
  }

  //! Depending on the data type select a sub algorithm
  if (fi.is_scalar())
    return(GetScalarFieldDataV(input,output));

  else if (fi.is_vector())
    return(GetVectorFieldDataV(input,output));

  else if (fi.is_tensor())
    return(GetTensorFieldDataV(input,output));

  error("Unknown data type");
  return (false);

}

AlgorithmOutputName GetMeshDataAlgo::MatrixFieldData("MatrixFieldData");

AlgorithmOutput GetMeshDataAlgo::run_generic(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);

  //! Data is only computed if the output port is connected:
  //bool need_matrix_data = oport_connected("Matrix Data");
  //bool need_nrrd_data   = oport_connected("Nrrd Data");

  DenseMatrixHandle matrixfielddata;
  if (!run(inputField, matrixfielddata))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[MatrixFieldData] = matrixfielddata;
  return output;
}
