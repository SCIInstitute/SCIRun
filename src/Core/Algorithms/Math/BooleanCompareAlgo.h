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


#ifndef CORE_ALGORITHMS_MATH_BooleanCompareALGO_H
#define CORE_ALGORITHMS_MATH_BooleanCompareALGO_H

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>


#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Math/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Math {

  ALGORITHM_PARAMETER_DECL(Value_Option_1);
  ALGORITHM_PARAMETER_DECL(Value_Option_2);
  ALGORITHM_PARAMETER_DECL(Then_Option);
  ALGORITHM_PARAMETER_DECL(Else_Option);
  ALGORITHM_PARAMETER_DECL(Comparison_Option);

class SCISHARE BooleanCompareAlgo : public AlgorithmBase
{

  public:
    BooleanCompareAlgo();
    AlgorithmOutput run(const AlgorithmInput& input) const;
  bool runImpl(Datatypes::DenseMatrixHandle matrixa, Datatypes::DenseMatrixHandle matrixb, std::string valoptA, std::string valoptB, std::string& cond_statement, int& cond_state) const;
  bool runImpl(Datatypes::DenseMatrixHandle matrixa, std::string valoptA, std::string& cond_statement, int& cond_state) const;
  bool return_check(int& cond_state, Datatypes::DenseMatrixHandle& cond_matrix, Datatypes::MatrixHandle& out_matrix, std::string then_result, std::string else_result, Datatypes::MatrixHandle matrixa, Datatypes::MatrixHandle matrixb, Datatypes::MatrixHandle possout) const;
  bool return_value(Datatypes::MatrixHandle& out_matrix, std::string result_statement, Datatypes::MatrixHandle first, Datatypes::MatrixHandle second, Datatypes::MatrixHandle possout) const;
  bool CompareMatrix(Datatypes::DenseMatrixHandle mata, Datatypes::DenseMatrixHandle matb, std::string cond_statement, int& cond_state) const;
  bool CompareMatrix(Datatypes::DenseMatrixHandle mata, int& cond_state) const;
  double ComputeNorm(Datatypes::DenseMatrixHandle mat) const;
};

      }}}}// end SCIRun namespace
#endif
