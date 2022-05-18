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

   Author:              Jaume Coll-Font, Moritz Dannhauer, Ayla Khan, Dan White
   Last Modification:   September 6 2017
*/

#ifndef BioPSE_SolveInverseProblemWithTikhonovChild_H__
#define BioPSE_SolveInverseProblemWithTikhonovChild_H__

#include <Core/Algorithms/Legacy/Inverse/TikhonovImpl.h>
#include <Core/Algorithms/Legacy/Inverse/TikhonovAlgoAbstractBase.h>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Algorithms/Legacy/Inverse/share.h>

namespace SCIRun {
namespace Core {
  namespace Algorithms {
    namespace Inverse {

      class SCISHARE SolveInverseProblemWithStandardTikhonovImpl final : public TikhonovImpl
      {
       public:
        SolveInverseProblemWithStandardTikhonovImpl(
            const Datatypes::DenseMatrix& forwardMatrix,
            const Datatypes::DenseMatrix& measuredData,
            const Datatypes::DenseMatrix& sourceWeighting,
            const Datatypes::DenseMatrix& sensorWeighting,
            const TikhonovAlgoAbstractBase::AlgorithmChoice regularizationChoice, const int regularizationSolutionSubcase,
            const int regularizationResidualSubcase)
        {
          preAllocateInverseMatrices(forwardMatrix, measuredData, sourceWeighting,
              sensorWeighting, regularizationChoice, regularizationSolutionSubcase,
              regularizationResidualSubcase);
        }

       private:
        Datatypes::DenseMatrix M1;
        Datatypes::DenseMatrix M2;
        Datatypes::DenseMatrix M3;
        Datatypes::DenseMatrix M4;
        Datatypes::DenseMatrix y;

        void preAllocateInverseMatrices(const Datatypes::DenseMatrix& forwardMatrix,
            const Datatypes::DenseMatrix& measuredData,
            const Datatypes::DenseMatrix& sourceWeighting,
            const Datatypes::DenseMatrix& sensorWeighting,
            TikhonovAlgoAbstractBase::AlgorithmChoice regularizationChoice, int regularizationSolutionSubcase,
            int regularizationResidualSubcase);

        Datatypes::DenseMatrix computeInverseSolution(double lambda, bool inverseCalculation) const override;
      };
    }
  }
}
}

#endif
