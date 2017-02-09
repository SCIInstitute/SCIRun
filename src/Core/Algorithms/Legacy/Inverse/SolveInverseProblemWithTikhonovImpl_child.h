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

//    File       : SolveInverseProblemWithTikhonov.cc
//    Author     : Moritz Dannhauer and Ayla Khan
//    Date       : 15 Aug. 2012

#ifndef BioPSE_SolveInverseProblemWithTikhonovChild_H__
#define BioPSE_SolveInverseProblemWithTikhonovChild_H__

#include <vector>
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Logging/LoggerFwd.h>
#include <Core/Algorithms/Legacy/Inverse/TikhonovImplAbstractBase.h>
#include <Core/Algorithms/Legacy/Inverse/share.h>

namespace SCIRun {
	namespace Core {
		namespace Algorithms {
			namespace Inverse {

			    class SCISHARE SolveInverseProblemWithTikhonovImpl_child : public TikhonovImplAbstractBase
			    {

			    public:
			        SolveInverseProblemWithTikhonovImpl_child(const SCIRun::Core::Datatypes::DenseMatrixHandle& forwardMatrix,
			                                                  const SCIRun::Core::Datatypes::DenseMatrixHandle& measuredData,
			                                                  AlgorithmChoice regularizationChoice = automatic,
			                                                  AlgorithmSolutionSubcase regularizationSolutionSubcase = solution_constrained,
			                                                  AlgorithmResidualSubcase regularizationResidualSubcase = residual_constrained,
			                                                  const SCIRun::Core::Datatypes::DenseMatrixHandle sourceWeighting = 0,
			                                                  const SCIRun::Core::Datatypes::DenseMatrixHandle sensorWeighting = 0,
			                                                  bool computeRegularizedInverse = false,
			                                                  SCIRun::Core::Logging::LegacyLoggerInterface* pr = 0);

			    private:

			        SCIRun::Core::Datatypes::DenseMatrix M1;
			        SCIRun::Core::Datatypes::DenseMatrix M2;
			        SCIRun::Core::Datatypes::DenseMatrix M3;
			        SCIRun::Core::Datatypes::DenseMatrix M4;
			        SCIRun::Core::Datatypes::DenseColumnMatrix y;


			        SCIRun::Core::Datatypes::DenseColumnMatrix computeInverseSolution( double lambda_sq, bool inverseCalculation);
			        void preAlocateInverseMatrices();
			//      bool checkInputMatrixSizes(); // DEFINED IN PARENT, MIGHT WANT TO OVERRIDE SOME OTHER TIME

			    };
			}
		}
	}
}

#endif
