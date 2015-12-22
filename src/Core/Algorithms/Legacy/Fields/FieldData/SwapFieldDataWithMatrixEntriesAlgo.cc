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

#include <Core/Algorithms/Legacy/Fields/FieldData/SwapFieldDataWithMatrixEntriesAlgo.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

#include <Core/Algorithms/Legacy/Fields/FieldData/GetFieldData.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldData.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms::Fields::Parameters;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(Fields, PreserveScalar)
ALGORITHM_PARAMETER_DEF(Fields, NeedOutputMatrix)

SwapFieldDataWithMatrixEntriesAlgo::SwapFieldDataWithMatrixEntriesAlgo()
{
  addParameter(Parameters::PreserveScalar, false);
  addParameter(Parameters::NeedOutputMatrix, false);
}

bool
SwapFieldDataWithMatrixEntriesAlgo::runImpl(FieldHandle input_field, MatrixHandle input_matrix, FieldHandle& output_field, MatrixHandle& output_matrix) const
{
  ScopedAlgorithmStatusReporter r(this, "SwapFieldDataWithMatrixEntriesAlgo");

  if (!input_field)
  {
    error("No input field");
    return (false);
  }

  FieldInformation fi(input_field);
  GetFieldDataAlgo get_algo_;
  SetFieldDataAlgo set_algo_;

  const bool preserve_scalar = get(Parameters::PreserveScalar).toBool();
  output_field = CreateField(fi);

  if (get(Parameters::NeedOutputMatrix).toBool())
  {
    output_matrix = get_algo_.run(input_field);
  }
  FieldHandle field_output_handle;

  if (input_matrix)
  {
    if (preserve_scalar)
    {
      set_algo_.set_option(Parameters::keepTypeCheckBox, fi.get_data_type());
    }
    size_type numVal;
    auto denseInput = matrix_convert::to_dense(input_matrix);
    if (set_algo_.verify_input_data(input_field, denseInput, numVal, fi))
    {
      output_field = set_algo_.run(input_field, denseInput);
    }
    else
    {
      THROW_ALGORITHM_INPUT_ERROR("Matrix dimensions do not match any of the fields dimensions");
      CopyProperties(*input_field, *output_field);
    }
  }
  else
  {
    warning("No input matrix passing the field through");
    output_field = input_field;
  }

  AlgorithmOutput output;
  output[Variables::OutputField] = output_field;

  return true;
}

bool
SwapFieldDataWithMatrixEntriesAlgo::runImpl(FieldHandle input, MatrixHandle input_matrix, FieldHandle& output) const
{
  MatrixHandle dummy;
  return runImpl(input, input_matrix, output, dummy);
}

AlgorithmOutput SwapFieldDataWithMatrixEntriesAlgo::run_generic(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);
  auto inputmatrix = input.get<Matrix>(Variables::InputMatrix);

  FieldHandle output_field;
  MatrixHandle outputMatrix;
  if (!runImpl(field, inputmatrix, output_field, outputMatrix))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = output_field;
  output[Variables::OutputMatrix] = outputMatrix;
  return output;
}
