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


#include <Core/Algorithms/Math/ConvertMatrixType.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

ALGORITHM_PARAMETER_DEF(Math, OutputMatrixType);

/// @class ConvertMatrixType
/// @image html ConvertMatrixType.png
ConvertMatrixTypeAlgorithm::ConvertMatrixTypeAlgorithm()
{
  addOption(Parameters::OutputMatrixType, "passThrough", "passThrough|dense|column|sparse");
}

MatrixHandle ConvertMatrixTypeAlgorithm::run(MatrixHandle input_matrix) const
{
  if (!input_matrix)
  {
    THROW_ALGORITHM_INPUT_ERROR("No input matrix");
  }

  MatrixHandle omH;
  std::ostringstream ostr1,ostr2;

  ostr1 << "Dimensions: (" << input_matrix->nrows() << "," << input_matrix->ncols() << ")";
  remark(ostr1.str());

  if(matrixIs::dense(input_matrix))
  {
    ostr2 << "Input Matrix Type: DENSE MATRIX";
  }
  else if (matrixIs::column(input_matrix))
  {
    ostr2 << "Input Matrix Type: COLUMN MATRIX";
  }
  else if (matrixIs::sparse(input_matrix))
  {
    ostr2 << "Input Matrix Type: SPARSE MATRIX";
  }
  else
  {
    THROW_ALGORITHM_INPUT_ERROR("Unknown input matrix type");
  }

  remark(ostr2.str());

  auto outputType = getOption(Parameters::OutputMatrixType);

  if ("passThrough" == outputType)
  {
    return input_matrix;
  }
  else
  {
    if ("column" == outputType && !matrixIs::column(input_matrix))
    {
      if (input_matrix->ncols()!=1)
      {
        THROW_ALGORITHM_INPUT_ERROR("Input matrix needs to have a single column to be converted to column matrix type.");
      }
      auto output = convertMatrix::toColumn(input_matrix);
      if (!output)
      {
        THROW_ALGORITHM_INPUT_ERROR("Conversion to column matrix failed.");
      }
      return output;
    }
    else if ("dense" == outputType && !matrixIs::dense(input_matrix))
    {
      auto output = convertMatrix::toDense(input_matrix);
      if (!output)
      {
        THROW_ALGORITHM_INPUT_ERROR("Conversion to dense matrix failed.");
      }
      return output;
    }
    else if ("sparse" == outputType && !matrixIs::sparse(input_matrix))
    {
      auto output = convertMatrix::toSparse(input_matrix);
      if (!output)
      {
        THROW_ALGORITHM_INPUT_ERROR("Conversion to sparse matrix failed.");
      }
      return output;
    }
    {
      remark(" Datatype unknown or input and output data type are equal. Passing input matrix through. ");
      return input_matrix;
    }
  }
}

AlgorithmOutput ConvertMatrixTypeAlgorithm::run(const AlgorithmInput& input) const
{
  auto input_matrix = input.get<Matrix>(Variables::InputMatrix);

  MatrixHandle output_matrix = run(input_matrix);

  AlgorithmOutput output;
  output[Variables::ResultMatrix] = output_matrix;

  return output;
}
