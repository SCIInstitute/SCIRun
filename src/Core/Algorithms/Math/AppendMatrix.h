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


#ifndef ALGORITHMS_MATH_APPENDMATRIX_H
#define ALGORITHMS_MATH_APPENDMATRIX_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Math/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Math {

  class SCISHARE AppendMatrixAlgorithm : public AlgorithmBase
  {
  public:
    enum Option { ROWS, COLUMNS };
    typedef boost::tuple<SCIRun::Core::Datatypes::MatrixHandle, SCIRun::Core::Datatypes::MatrixHandle> Inputs;
    typedef Option Parameters;
    typedef SCIRun::Core::Datatypes::MatrixHandle Outputs;
    Outputs ConcatenateMatrices(const Datatypes::MatrixHandle base_matrix, const std::vector<boost::shared_ptr<Datatypes::Matrix>> input_matrices, const AppendMatrixAlgorithm::Parameters& params) const;
    Outputs run(const Inputs& input, const Parameters& params) const;
    bool check_dimensions(const Datatypes::Matrix& mat1, const Datatypes::Matrix& mat2, const Parameters& params) const;
    AppendMatrixAlgorithm();
    AlgorithmOutput run(const AlgorithmInput& input) const;
    static const AlgorithmInputName InputMatrices;
  };

}}}}

#endif
