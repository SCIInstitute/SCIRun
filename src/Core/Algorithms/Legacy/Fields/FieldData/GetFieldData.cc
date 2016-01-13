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
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>

using namespace SCIRun;
using namespace Core::Algorithms::Fields;
using namespace Core::Geometry;
using namespace Core::Datatypes;
using namespace Core::Utility;
using namespace Core::Algorithms;
using namespace Core::Logging;

ALGORITHM_PARAMETER_DEF(Fields, CalcMatrix);
ALGORITHM_PARAMETER_DEF(Fields, CalcNrrd);

GetFieldDataAlgo::GetFieldDataAlgo()
{
  addParameter(Parameters::CalcMatrix, true);
  addParameter(Parameters::CalcNrrd, false);
}

AlgorithmOutput GetFieldDataAlgo::run_generic(const AlgorithmInput& input) const
{
  auto input_field = input.get<Field>(Variables::InputField);

  AlgorithmOutput output;

  if (get(Parameters::CalcMatrix).toBool())
  {
    output[Variables::OutputMatrix] = run(input_field);;
  }
  if (get(Parameters::CalcNrrd).toBool())
  {
    output[Variables::OutputNrrd] = runNrrd(input_field);;
  }

  return output;
}

DenseMatrixHandle GetFieldDataAlgo::run(FieldHandle input_field) const
{
  return runImpl<DenseMatrix>(input_field);
}

NrrdDataHandle GetFieldDataAlgo::runNrrd(FieldHandle input_field) const
{
  return runImpl<NrrdData>(input_field);
}

template <class MatrixReturnType>
boost::shared_ptr<MatrixReturnType> GetFieldDataAlgo::runImpl(FieldHandle input_field) const
{
  ScopedAlgorithmStatusReporter asr(this, "GetFieldData");

  if (!input_field)
  {
    THROW_ALGORITHM_INPUT_ERROR("No input field was provided");
  }

  DenseMatrixHandle output;

  VField* vfield1 = input_field->vfield();

  /// Check whether we have data
  if (!vfield1)
  {
    THROW_ALGORITHM_INPUT_ERROR("Could not obtain VField interface");
  }

  if (vfield1->is_nodata())
  {
    THROW_ALGORITHM_INPUT_ERROR("Invalid input field (no data)");
  }

  boost::shared_ptr<MatrixReturnType> mat;
  if (vfield1->is_scalar())
    if (GetScalarFieldDataV(input_field, mat)) return mat;
  if (vfield1->is_vector())
    if (GetVectorFieldDataV(input_field, mat)) return mat;
  if (vfield1->is_tensor())
    if (GetTensorFieldDataV(input_field, mat)) return mat;
  
  THROW_ALGORITHM_INPUT_ERROR("Unknown field data type!");
  return nullptr;
}

namespace SCIRun {
  namespace Core {
    namespace Algorithms {
      namespace Fields {
        template <>
        bool GetFieldDataAlgo::GetScalarFieldDataV(FieldHandle input, DenseMatrixHandle& output) const
        {
          /// Obtain virtual interface
          VField* vfield = input->vfield();

          /// Obtain the number values in a field
          VMesh::size_type size = vfield->num_values();
          VMesh::size_type esize = vfield->num_evalues();

          /// Create output object
          output.reset(new DenseMatrix(size + esize, 1));

          if (!output)
          {
            THROW_ALGORITHM_INPUT_ERROR("Could not allocate output matrix");
            return false;
          }

          for (VMesh::Elem::index_type idx = 0; idx < size; ++idx)
          {
            vfield->get_value((*output)(idx, 0), idx);
          }
          if (vfield->basis_order() == 2)
          {
            vfield->vmesh()->synchronize(Mesh::EDGES_E);
            for (VMesh::Elem::index_type idx = size; idx < size + esize; idx++)
            {
              vfield->get_evalue((*output)(idx, 0), idx);
            }
          }

          return true;
        }

        template <>
        bool GetFieldDataAlgo::GetVectorFieldDataV(FieldHandle input, DenseMatrixHandle& output) const
        {
          VField* vfield = input->vfield();

          VMesh::size_type size = vfield->num_values();
          VMesh::size_type esize = vfield->num_evalues();

          output.reset(new DenseMatrix(size + esize, 3));

          if (!output)
          {
            THROW_ALGORITHM_INPUT_ERROR("Could not allocate output matrix");
            return false;
          }

          Vector val;
          for (VMesh::index_type idx = 0; idx < size; idx++)
          {
            vfield->get_value(val, idx);
            (*output)(idx, 0) = val.x();
            (*output)(idx, 1) = val.y();
            (*output)(idx, 2) = val.z();
          }
          if (vfield->basis_order() == 2)
          {
            vfield->vmesh()->synchronize(Mesh::EDGES_E);

            for (VMesh::index_type idx = size; idx < esize + size; idx++)
            {
              vfield->get_evalue(val, idx);
              (*output)(idx, 0) = val.x();
              (*output)(idx, 1) = val.y();
              (*output)(idx, 2) = val.z();
            }
          }

          return true;
        }

        template <>
        bool GetFieldDataAlgo::GetTensorFieldDataV(FieldHandle input, DenseMatrixHandle& output) const
        {
          VField* vfield = input->vfield();

          VMesh::size_type size = vfield->num_values();
          VMesh::size_type esize = vfield->num_evalues();

          output.reset(new DenseMatrix(size + esize, 6));

          if (!output)
          {
            THROW_ALGORITHM_INPUT_ERROR("Could not allocate output matrix");
            return false;
          }

          Tensor tensor;
          for (VMesh::index_type idx = 0; idx < size; idx++)
          {
            vfield->get_value(tensor, idx);
            (*output)(idx, 0) = tensor.val(0, 0);
            (*output)(idx, 1) = tensor.val(0, 1);
            (*output)(idx, 2) = tensor.val(0, 2);
            (*output)(idx, 3) = tensor.val(1, 1);
            (*output)(idx, 4) = tensor.val(1, 2);
            (*output)(idx, 5) = tensor.val(2, 2);
          }
          if (vfield->basis_order() == 2)
          {
            vfield->vmesh()->synchronize(Mesh::EDGES_E);

            for (VMesh::index_type idx = size; idx < esize + size; idx++)
            {
              vfield->get_evalue(tensor, idx);
              (*output)(idx, 0) = tensor.val(0, 0);
              (*output)(idx, 1) = tensor.val(0, 1);
              (*output)(idx, 2) = tensor.val(0, 2);
              (*output)(idx, 3) = tensor.val(1, 1);
              (*output)(idx, 4) = tensor.val(1, 2);
              (*output)(idx, 5) = tensor.val(2, 2);
            }
          }

          return true;
        }

        template <>
        bool GetFieldDataAlgo::GetScalarFieldDataV(FieldHandle input, NrrdDataHandle& output) const
        {
          /// Obtain virtual interface
          VField* vfield = input->vfield();

          /// Obtain the number values in a field
          VMesh::size_type size = vfield->num_values();
          VMesh::size_type esize = vfield->num_evalues();

          /// Create output object
          output.reset(new NrrdData());

          if (!output)
          {
            error("Could not allocate output nrrd");
            return (false);
          }

          if (!output->getNrrd())
          {
            error("Could not allocate output nrrd");
            return (false);
          }

          size_t nrrddims[1];
          nrrddims[0] = size + esize;

          if (vfield->is_char())
          {
            nrrdAlloc_nva(output->getNrrd(), nrrdTypeChar, 1, nrrddims);
            char* dataptr = reinterpret_cast<char *>(output->getNrrd()->data);
            // get all the values as doubles
            vfield->get_values(dataptr, size);

            if (vfield->basis_order() == 2)
            {
              vfield->vmesh()->synchronize(Mesh::EDGES_E);
              vfield->get_evalues(dataptr + size, esize);
            }
          }

          if (vfield->is_unsigned_char())
          {
            nrrdAlloc_nva(output->getNrrd(), nrrdTypeUChar, 1, nrrddims);
            unsigned char* dataptr = reinterpret_cast<unsigned char *>(output->getNrrd()->data);
            // get all the values as doubles
            vfield->get_values(dataptr, size);

            if (vfield->basis_order() == 2)
            {
              vfield->vmesh()->synchronize(Mesh::EDGES_E);
              vfield->get_evalues(dataptr + size, esize);
            }
          }

          if (vfield->is_short())
          {
            nrrdAlloc_nva(output->getNrrd(), nrrdTypeShort, 1, nrrddims);
            short* dataptr = reinterpret_cast<short *>(output->getNrrd()->data);
            // get all the values as doubles
            vfield->get_values(dataptr, size);

            if (vfield->basis_order() == 2)
            {
              vfield->vmesh()->synchronize(Mesh::EDGES_E);
              vfield->get_evalues(dataptr + size, esize);
            }
          }

          if (vfield->is_unsigned_short())
          {
            nrrdAlloc_nva(output->getNrrd(), nrrdTypeUShort, 1, nrrddims);
            unsigned short* dataptr = reinterpret_cast<unsigned short *>(output->getNrrd()->data);
            // get all the values as doubles
            vfield->get_values(dataptr, size);

            if (vfield->basis_order() == 2)
            {
              vfield->vmesh()->synchronize(Mesh::EDGES_E);
              vfield->get_evalues(dataptr + size, esize);
            }
          }

          if (vfield->is_int())
          {
            nrrdAlloc_nva(output->getNrrd(), nrrdTypeInt, 1, nrrddims);
            int* dataptr = reinterpret_cast<int *>(output->getNrrd()->data);
            // get all the values as doubles
            vfield->get_values(dataptr, size);

            if (vfield->basis_order() == 2)
            {
              vfield->vmesh()->synchronize(Mesh::EDGES_E);
              vfield->get_evalues(dataptr + size, esize);
            }
          }

          if (vfield->is_unsigned_int())
          {
            nrrdAlloc_nva(output->getNrrd(), nrrdTypeUInt, 1, nrrddims);
            unsigned int* dataptr = reinterpret_cast<unsigned int *>(output->getNrrd()->data);
            // get all the values as doubles
            vfield->get_values(dataptr, size);

            if (vfield->basis_order() == 2)
            {
              vfield->vmesh()->synchronize(Mesh::EDGES_E);
              vfield->get_evalues(dataptr + size, esize);
            }
          }

          if (vfield->is_longlong())
          {
            nrrdAlloc_nva(output->getNrrd(), nrrdTypeLLong, 1, nrrddims);
            long long* dataptr = reinterpret_cast<long long *>(output->getNrrd()->data);
            // get all the values as doubles
            vfield->get_values(dataptr, size);

            if (vfield->basis_order() == 2)
            {
              vfield->vmesh()->synchronize(Mesh::EDGES_E);
              vfield->get_evalues(dataptr + size, esize);
            }
          }

          if (vfield->is_unsigned_longlong())
          {
            nrrdAlloc_nva(output->getNrrd(), nrrdTypeULLong, 1, nrrddims);
            unsigned long long* dataptr = reinterpret_cast<unsigned long long *>(output->getNrrd()->data);
            // get all the values as doubles
            vfield->get_values(dataptr, size);

            if (vfield->basis_order() == 2)
            {
              vfield->vmesh()->synchronize(Mesh::EDGES_E);
              vfield->get_evalues(dataptr + size, esize);
            }
          }

          if (vfield->is_float())
          {
            nrrdAlloc_nva(output->getNrrd(), nrrdTypeFloat, 1, nrrddims);
            float* dataptr = reinterpret_cast<float *>(output->getNrrd()->data);
            // get all the values as doubles
            vfield->get_values(dataptr, size);

            if (vfield->basis_order() == 2)
            {
              vfield->vmesh()->synchronize(Mesh::EDGES_E);
              vfield->get_evalues(dataptr + size, esize);
            }
          }

          if (vfield->is_double())
          {
            nrrdAlloc_nva(output->getNrrd(), nrrdTypeDouble, 1, nrrddims);
            double* dataptr = reinterpret_cast<double *>(output->getNrrd()->data);
            // get all the values as doubles
            vfield->get_values(dataptr, size);

            if (vfield->basis_order() == 2)
            {
              vfield->vmesh()->synchronize(Mesh::EDGES_E);
              vfield->get_evalues(dataptr + size, esize);
            }
          }

          return (true);
        }

        template <>
        bool GetFieldDataAlgo::GetVectorFieldDataV(FieldHandle input, NrrdDataHandle& output) const
        {
          VField* vfield = input->vfield();

          VMesh::size_type size = vfield->num_values();
          VMesh::size_type esize = vfield->num_evalues();

          output.reset(new NrrdData());

          if (!output)
          {
            error("Could not allocate output nrrd");
            return (false);
          }

          if (!output->getNrrd())
          {
            error("Could not allocate output nrrd");
            return (false);
          }

          size_t nrrddims[2];
          nrrddims[0] = 3;
          nrrddims[1] = size + esize;

          nrrdAlloc_nva(output->getNrrd(), nrrdTypeDouble, 2, nrrddims);
          double* dataptr = reinterpret_cast<double *>(output->getNrrd()->data);

          Vector val;
          int k = 0;
          for (VMesh::index_type i = 0; i < size; i++)
          {
            vfield->get_value(val, i);
            dataptr[k] = val.x();
            dataptr[k + 1] = val.y();
            dataptr[k + 2] = val.z();
            k += 3;
          }

          if (vfield->basis_order() == 2)
          {
            vfield->vmesh()->synchronize(Mesh::EDGES_E);

            for (VMesh::index_type i = 0; i < esize; i++)
            {
              vfield->get_evalue(val, i);
              dataptr[k] = val.x();
              dataptr[k + 1] = val.y();
              dataptr[k + 2] = val.z();
              k += 3;
            }
          }
          return (true);
        }

        template <>
        bool GetFieldDataAlgo::GetTensorFieldDataV(FieldHandle input, NrrdDataHandle& output) const
        {
          VField* vfield = input->vfield();

          VMesh::size_type size = vfield->num_values();
          VMesh::size_type esize = vfield->num_evalues();

          output.reset(new NrrdData());

          if (!output)
          {
            error("Could not allocate output nrrd");
            return (false);
          }

          if (!output->getNrrd())
          {
            error("Could not allocate output nrrd");
            return (false);
          }

          size_t nrrddims[2];
          nrrddims[0] = 6;
          nrrddims[1] = size + esize;

          nrrdAlloc_nva(output->getNrrd(), nrrdTypeDouble, 2, nrrddims);
          double* dataptr = reinterpret_cast<double *>(output->getNrrd()->data);

          Tensor tensor;
          int k = 0;
          for (VMesh::index_type i = 0; i < size; i++)
          {
            vfield->get_value(tensor, i);
            dataptr[k] = tensor.val(0, 0);
            dataptr[k + 1] = tensor.val(0, 1);
            dataptr[k + 2] = tensor.val(0, 2);
            dataptr[k + 3] = tensor.val(1, 1);
            dataptr[k + 4] = tensor.val(1, 2);
            dataptr[k + 5] = tensor.val(2, 2);
            k += 6;
          }

          if (vfield->basis_order() == 2)
          {
            vfield->vmesh()->synchronize(Mesh::EDGES_E);

            for (VMesh::index_type i = 0; i < esize; i++)
            {
              vfield->get_evalue(tensor, i);
              dataptr[k] = tensor.val(0, 0);
              dataptr[k + 1] = tensor.val(0, 1);
              dataptr[k + 2] = tensor.val(0, 2);
              dataptr[k + 3] = tensor.val(1, 1);
              dataptr[k + 4] = tensor.val(1, 2);
              dataptr[k + 5] = tensor.val(2, 2);
              k += 6;
            }
          }
          return (true);
        }
      }
    }
  }
}