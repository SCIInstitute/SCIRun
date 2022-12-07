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

         Author: 							Jaume Coll-Font, Yesim
   Serinagaoglu & Alireza Ghodrati Last Modification:		September 6 2017
*/

#ifndef BioPSE_SolveInverseProblemWithTikhonovSVDimpl_H__
#define BioPSE_SolveInverseProblemWithTikhonovSVDimpl_H__

#include <Core/Algorithms/Legacy/Inverse/TikhonovImpl.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Algorithms/Legacy/Inverse/share.h>

namespace SCIRun {
namespace Core {
  namespace Algorithms {
    namespace Inverse {

      class SCISHARE SolveInverseProblemWithTikhonovSVD_impl : public TikhonovImpl
      {
       public:
        SolveInverseProblemWithTikhonovSVD_impl(
            const Datatypes::DenseMatrix& forwardMatrix_,
            const Datatypes::DenseMatrix& measuredData_,
            const Datatypes::DenseMatrix& sourceWeighting_,
            const Datatypes::DenseMatrix& sensorWeighting_,
            const Datatypes::DenseMatrix& matrixU_,
            const Datatypes::DenseMatrix& singularValues_,
            const Datatypes::DenseMatrix& matrixV_)
        {
          preAlocateInverseMatrices(forwardMatrix_, measuredData_, sourceWeighting_,
              sensorWeighting_, matrixU_, singularValues_, matrixV_);
        }

        SolveInverseProblemWithTikhonovSVD_impl(
            const Datatypes::DenseMatrix& forwardMatrix_,
            const Datatypes::DenseMatrix& measuredData_,
            const Datatypes::DenseMatrix& sourceWeighting_,
            const Datatypes::DenseMatrix& sensorWeighting_)
        {
          preAlocateInverseMatrices(
              forwardMatrix_, measuredData_, sourceWeighting_, sensorWeighting_);
        }

       private:
        // Data Members
        int rank;
        SCIRun::Core::Datatypes::DenseMatrix svd_MatrixU;
        SCIRun::Core::Datatypes::DenseColumnMatrix svd_SingularValues;
        SCIRun::Core::Datatypes::DenseMatrix svd_MatrixV;

        SCIRun::Core::Datatypes::DenseMatrix Uy;

        // Methods
        void preAlocateInverseMatrices(const SCIRun::Core::Datatypes::DenseMatrix& forwardMatrix_,
            const Datatypes::DenseMatrix& measuredData_,
            const Datatypes::DenseMatrix& sourceWeighting_,
            const Datatypes::DenseMatrix& sensorWeighting_,
            const Datatypes::DenseMatrix& matrixU_,
            const Datatypes::DenseMatrix& singularValues_,
            const Datatypes::DenseMatrix& matrixV_);
        void preAlocateInverseMatrices(const SCIRun::Core::Datatypes::DenseMatrix& forwardMatrix_,
            const Datatypes::DenseMatrix& measuredData_,
            const Datatypes::DenseMatrix& sourceWeighting_,
            const Datatypes::DenseMatrix& sensorWeighting_);

        SCIRun::Core::Datatypes::DenseMatrix computeInverseSolution(
            double lambda, bool inverseCalculation) const override;
        //      bool checkInputMatrixSizes(); // DEFINED IN PARENT, MIGHT WANT TO OVERRIDE SOME
        //      OTHER TIME
      };
    }
  }
}
}

#endif
