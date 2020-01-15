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


#include <Core/Algorithms/Math/GetMatrixSliceAlgo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <boost/lexical_cast.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

ALGORITHM_PARAMETER_DEF(Math, IsSliceColumn);
ALGORITHM_PARAMETER_DEF(Math, SliceIndex);
ALGORITHM_PARAMETER_DEF(Math, MaxIndex);
ALGORITHM_PARAMETER_DEF(Math, PlayModeActive);
ALGORITHM_PARAMETER_DEF(Math, PlayModeType);
ALGORITHM_PARAMETER_DEF(Math, SliceIncrement);
ALGORITHM_PARAMETER_DEF(Math, PlayModeDelay);

GetMatrixSliceAlgo::GetMatrixSliceAlgo()
{
  addParameter(Parameters::IsSliceColumn, true);
  addParameter(Parameters::SliceIndex, 0);
  addParameter(Parameters::MaxIndex, 0);
  addParameter(Parameters::PlayModeActive, false);
  //TODO DAN: make overload to handle const char*
  addOption(Parameters::PlayModeType, "looponce", "looponce|loopforever"); //TODO add more play options
  addParameter(Parameters::SliceIncrement, 1);
  addParameter(Parameters::PlayModeDelay, 0);
}

AlgorithmOutput GetMatrixSliceAlgo::run(const AlgorithmInput& input) const
{
  auto inputMatrix = input.get<Matrix>(Variables::InputMatrix);
  auto outputMatrix = runImpl(inputMatrix, get(Parameters::SliceIndex).toInt(), get(Parameters::IsSliceColumn).toBool());

  AlgorithmOutput output;
  output[Variables::OutputMatrix] = outputMatrix.get<0>();
  output.setAdditionalAlgoOutput(boost::make_shared<Variable>(Name("maxIndex"), outputMatrix.get<1>()));

  return output;
}

boost::tuple<MatrixHandle, int> GetMatrixSliceAlgo::runImpl(MatrixHandle matrix, int index, bool getColumn) const
{
  ENSURE_ALGORITHM_INPUT_NOT_NULL(matrix, "Input matrix");
  if (getColumn)
  {
    checkIndex(index, matrix->ncols());
    auto max = matrix->ncols() - 1;

    // dense case only now
    auto dense = castMatrix::toDense(matrix);
    if (dense)
      return boost::make_tuple(boost::make_shared<DenseMatrix>(dense->col(index)), max);
    else
    {
      auto sparse = castMatrix::toSparse(matrix);
      if (sparse)
      {
        //TODO: makes a copy of the transpose. Not good. Should test out manually copying elements, trade speed for memory.
        return boost::make_tuple(boost::make_shared<SparseRowMatrix>(sparse->getColumn(index)), max);
      }
      return boost::make_tuple(nullptr, 0);
    }
  }
  else
  {
    checkIndex(index, matrix->nrows());
    auto max = matrix->nrows() - 1;

    auto dense = castMatrix::toDense(matrix);
    if (dense)
      return boost::make_tuple(boost::make_shared<DenseMatrix>(dense->row(index)), max);
    else
    {
      auto sparse = castMatrix::toSparse(matrix);
      if (sparse)
        return boost::make_tuple(boost::make_shared<SparseRowMatrix>(sparse->row(index)), max);
      return boost::make_tuple(nullptr, 0);
    }
  }
}

void GetMatrixSliceAlgo::checkIndex(int index, int max) const
{
  if (index < 0 || index >= max)
    THROW_ALGORITHM_INPUT_ERROR("Slice index out of range: " + boost::lexical_cast<std::string>(index));
}
