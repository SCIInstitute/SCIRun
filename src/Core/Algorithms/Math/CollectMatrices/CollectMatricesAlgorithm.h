/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#ifndef CORE_ALGORITHMS_MATH_COLLECTMATRICES_H
#define CORE_ALGORITHMS_MATH_COLLECTMATRICES_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Algorithms/Math/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Math {

    ALGORITHM_PARAMETER_DECL(CollectAppendIndicator);
    ALGORITHM_PARAMETER_DECL(CollectRowIndicator);
    ALGORITHM_PARAMETER_DECL(CollectPrependIndicator);
    ALGORITHM_PARAMETER_DECL(ClearCollectMatricesOutput);

    class SCISHARE CollectMatricesAlgorithmBase : public AlgorithmBase
    {
    public:
      virtual Datatypes::MatrixHandle concat_cols(Datatypes::MatrixHandle m1H, Datatypes::MatrixHandle m2H) const = 0;
      virtual Datatypes::MatrixHandle concat_rows(Datatypes::MatrixHandle m1H, Datatypes::MatrixHandle m2H) const = 0;
      virtual AlgorithmOutput run_generic(const AlgorithmInput&) const override { throw "not needed for now"; }
    };

    class SCISHARE CollectDenseMatricesAlgorithm : public CollectMatricesAlgorithmBase
    {
    public:
      virtual Datatypes::MatrixHandle concat_cols(Datatypes::MatrixHandle m1H, Datatypes::MatrixHandle m2H) const override;
      virtual Datatypes::MatrixHandle concat_rows(Datatypes::MatrixHandle m1H, Datatypes::MatrixHandle m2H) const override;
    private:
      void copy_matrix(Datatypes::MatrixHandle mh, Datatypes::DenseMatrix& out) const;
    };

    class SCISHARE CollectSparseRowMatricesAlgorithm : public CollectMatricesAlgorithmBase
    {
    public:
      virtual Datatypes::MatrixHandle concat_cols(Datatypes::MatrixHandle m1H, Datatypes::MatrixHandle m2H) const override;
      virtual Datatypes::MatrixHandle concat_rows(Datatypes::MatrixHandle m1H, Datatypes::MatrixHandle m2H) const override;
    private:
      void check_args(Datatypes::MatrixHandle m1H, Datatypes::MatrixHandle m2H) const;
      void copy_shifted_contents(Datatypes::SparseRowMatrixHandle sparse,
        Datatypes::SparseRowMatrixFromMap::Values& shiftedValues, size_t rowShift, size_t columnShift) const;
    };
  }
}
}}

#endif
