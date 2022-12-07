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

   Author:        Moritz Dannhauer (reimplementation)
   Date:          August 2017
*/


#include <Core/Algorithms/Math/AppendMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Core::Algorithms;
using namespace Math;
using namespace SCIRun::Core::Datatypes;

const AlgorithmInputName AppendMatrixAlgorithm::InputMatrices("InputMatrices");

AppendMatrixAlgorithm::AppendMatrixAlgorithm()
{
  addParameter(Variables::RowsOrColumns, 0);
}


bool AppendMatrixAlgorithm::check_dimensions(
    const Matrix& mat1, const Matrix& mat2, const Parameters& params)
{
  auto rows_m1 = mat1.nrows(), rows_m2 = mat2.nrows(), cols_m1 = mat1.ncols(),
       cols_m2 = mat2.ncols();
  if (params == Option::ROWS)
  {
    if (cols_m1 != cols_m2) return false;
  }
  else if (rows_m1 != rows_m2)
    return false;

  return true;
}

AppendMatrixAlgorithm::Outputs AppendMatrixAlgorithm::concatenateMatrices(
    MatrixHandle base_matrix, const std::vector<SharedPointer<Matrix>>& input_matrices,
    const Parameters& params) const
{
  if (input_matrices.empty()) 
    return base_matrix;

  auto outputs = run(std::make_tuple(base_matrix, input_matrices[0]), params);
  for (size_t c = 1; c < input_matrices.size(); c++)
  {
    const auto concatenated = run(std::make_tuple(outputs, input_matrices[c]), params);
    outputs = concatenated;
  }

  return outputs;
}

AppendMatrixAlgorithm::Outputs AppendMatrixAlgorithm::run(const Inputs& input, const Parameters& params) const
{
  const auto lhsPtr = std::get<0>(input);
  const auto rhsPtr = std::get<1>(input);
  if (!lhsPtr || !rhsPtr)
   error(" At least two matrices are needed to run this module. ");

  if (!((matrixIs::sparse(lhsPtr) && matrixIs::sparse(rhsPtr)) || (matrixIs::dense(lhsPtr) && matrixIs::dense(rhsPtr)) || (matrixIs::column(lhsPtr) && matrixIs::column(rhsPtr))))
  {
   error(" Mixing of different matrix types as inputs is not supported. ");
   return Outputs();
  }

  if (!check_dimensions(*lhsPtr, *rhsPtr, params))
  {
    error(" Input matrix dimensions do not match. ");
    return Outputs();
  }

  Eigen::MatrixXd result;
  if (matrixIs::dense(lhsPtr) || matrixIs::column(lhsPtr))
  {
    if (params == Option::ROWS)
      result = Eigen::MatrixXd(lhsPtr->nrows() + rhsPtr->nrows(), lhsPtr->ncols());
    else
      result = Eigen::MatrixXd(lhsPtr->nrows(), lhsPtr->ncols() + rhsPtr->ncols());

    if (matrixIs::dense(lhsPtr))
      result << *castMatrix::toDense(lhsPtr), *castMatrix::toDense(rhsPtr);
    else
      result << *castMatrix::toColumn(lhsPtr), *castMatrix::toColumn(rhsPtr);

    if (matrixIs::column(lhsPtr) && (result.rows() == 1 || result.cols() == 1))
      return makeShared<DenseColumnMatrix>(result);

    return makeShared<DenseMatrix>(result);
  }

  if (matrixIs::sparse(lhsPtr))
    return SparseRowMatrixFromMap::concatenateSparseMatrices(
        *castMatrix::toSparse(lhsPtr), *castMatrix::toSparse(rhsPtr), params == Option::ROWS);

  error("This matrix type is not supported");

  return Outputs();
}

AlgorithmOutput AppendMatrixAlgorithm::run(const AlgorithmInput& input) const
{
  const auto lhs = input.get<Matrix>(Variables::FirstMatrix);
  const auto rhs = input.get<Matrix>(Variables::SecondMatrix);
  const auto inputMatrices = input.getList<Matrix>(InputMatrices);

  const auto params = static_cast<Option>(get(Variables::RowsOrColumns).toInt());
  auto outputs = run(std::make_tuple(lhs, rhs), params);

  if (inputMatrices.size() > 0)
   outputs = concatenateMatrices(outputs, inputMatrices, params);

  AlgorithmOutput output;
  output[Variables::ResultMatrix] = outputs;
  return output;
}
