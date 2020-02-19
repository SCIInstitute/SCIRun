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


#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Core/Algorithms/Math/ReportComplexMatrixInfo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixMathVisitors.h>

using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;

namespace std
{
  bool operator<(const SCIRun::complex& lhs, const SCIRun::complex& rhs)
  {
    return norm(lhs) < norm(rhs);
  }
}

ReportComplexMatrixInfoAlgo::Outputs ReportComplexMatrixInfoAlgo::runImpl(const Inputs& input) const
{
  ENSURE_ALGORITHM_INPUT_NOT_NULL(input, "Null input matrix");

  const std::string type = matrixIs::whatType(input);

  NumberOfElements<complex> num;
  input->accept(num);
  MinimumCoefficient<complex> min;
  input->accept(min);
  MaximumCoefficient<complex> max;
  input->accept(max);

  return Outputs(type,
    input->nrows(),
    input->ncols(),
    num.value(),
    min.value(),
    max.value()
    );
}

AlgorithmOutput ReportComplexMatrixInfoAlgo::run(const AlgorithmInput& input) const
{
  auto matrix = input.get<ComplexMatrix>(Variables::InputMatrix);

  auto outputs = runImpl(matrix);

  AlgorithmOutput output;
  output.setTransient(outputs);
  return output;
}

std::string ReportComplexMatrixInfoAlgo::summarize(const Outputs& info)
{
  std::ostringstream ostr;
  ostr << "Type:\t\t" << info.get<0>() << "\n"
    << "# Rows:\t\t" << info.get<1>() << "\n"
    << "# Columns:\t\t" << info.get<2>() << "\n"
    << "# Elements:\t\t" << info.get<3>() << "\n"
    << "Minimum (by norm):\t" << info.get<4>() << "\n"
    << "Maximum (by norm):\t" << info.get<5>() << "\n";
  return ostr.str();
}
