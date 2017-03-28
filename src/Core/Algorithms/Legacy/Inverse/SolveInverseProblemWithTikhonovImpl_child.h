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
#include <Core/Algorithms/Legacy/Inverse/TikhonovAlgoAbstractBase.h>
#include <Core/Algorithms/Legacy/Inverse/share.h>

namespace SCIRun {
	namespace Core {
		namespace Algorithms {
			namespace Inverse {

			    class SCISHARE SolveInverseProblemWithTikhonovImpl_child : public TikhonovAlgoAbstractBase
			    {

			    public:
			        SolveInverseProblemWithTikhonovImpl_child() {};


			    private:

			        SCIRun::Core::Datatypes::DenseMatrix M1;
			        SCIRun::Core::Datatypes::DenseMatrix M2;
			        SCIRun::Core::Datatypes::DenseMatrix M3;
			        SCIRun::Core::Datatypes::DenseMatrix M4;
			        SCIRun::Core::Datatypes::DenseMatrix y;

				protected:

			        virtual SCIRun::Core::Datatypes::DenseMatrix computeInverseSolution( double lambda_sq, bool inverseCalculation) const;
			        void preAlocateInverseMatrices(const SCIRun::Core::Datatypes::DenseMatrix& forwardMatrix_, const SCIRun::Core::Datatypes::DenseMatrix& measuredData_ , const SCIRun::Core::Datatypes::DenseMatrix& sourceWeighting_, const SCIRun::Core::Datatypes::DenseMatrix& sensorWeighting_);
			//      bool checkInputMatrixSizes(); // DEFINED IN PARENT, MIGHT WANT TO OVERRIDE SOME OTHER TIME

			    };
			}
		}
	}
}

#endif
