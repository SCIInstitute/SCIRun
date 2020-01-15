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

	 Author: 							Jaume Coll-Font, Yesim Serinagaoglu & Alireza Ghodrati
	 Last Modification:		September 6 2017
*/


#ifndef BioPSE_SolveInverseProblemWithTSVDimpl_H__
#define BioPSE_SolveInverseProblemWithTSVDimpl_H__

#include <vector>

#include <boost/utility.hpp>
#include <boost/function.hpp>

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Logging/LoggerFwd.h>

#include <Core/Algorithms/Legacy/Inverse/TikhonovImpl.h>

#include <Core/Algorithms/Legacy/Inverse/share.h>


namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Inverse {

		    class SCISHARE SolveInverseProblemWithTSVD_impl : public TikhonovImpl
		    {


		    public:
		        SolveInverseProblemWithTSVD_impl(const SCIRun::Core::Datatypes::DenseMatrix& forwardMatrix_, const SCIRun::Core::Datatypes::DenseMatrix& measuredData_ , const SCIRun::Core::Datatypes::DenseMatrix& sourceWeighting_, const SCIRun::Core::Datatypes::DenseMatrix& sensorWeighting_, const SCIRun::Core::Datatypes::DenseMatrix& matrixU_, const SCIRun::Core::Datatypes::DenseMatrix& singularValues_, const SCIRun::Core::Datatypes::DenseMatrix& matrixV_)
                                    {
										preAlocateInverseMatrices( forwardMatrix_,  measuredData_ ,  sourceWeighting_,  sensorWeighting_, matrixU_, singularValues_, matrixV_ );
                                    };

				SolveInverseProblemWithTSVD_impl(const SCIRun::Core::Datatypes::DenseMatrix& forwardMatrix_, const SCIRun::Core::Datatypes::DenseMatrix& measuredData_ , const SCIRun::Core::Datatypes::DenseMatrix& sourceWeighting_, const SCIRun::Core::Datatypes::DenseMatrix& sensorWeighting_)
                                    {
										preAlocateInverseMatrices( forwardMatrix_,  measuredData_ ,  sourceWeighting_,  sensorWeighting_);
                                    };

		    private:

				// Data Members
		        int rank;
				SCIRun::Core::Datatypes::DenseMatrix svd_MatrixU;
				SCIRun::Core::Datatypes::DenseColumnMatrix svd_SingularValues;
				SCIRun::Core::Datatypes::DenseMatrix svd_MatrixV;

		        SCIRun::Core::Datatypes::DenseMatrix Uy;

				// Methods
				void preAlocateInverseMatrices(const SCIRun::Core::Datatypes::DenseMatrix& forwardMatrix_, const SCIRun::Core::Datatypes::DenseMatrix& measuredData_ , const SCIRun::Core::Datatypes::DenseMatrix& sourceWeighting_, const SCIRun::Core::Datatypes::DenseMatrix& sensorWeighting_, const SCIRun::Core::Datatypes::DenseMatrix& matrixU_, const SCIRun::Core::Datatypes::DenseMatrix& singularValues_, const SCIRun::Core::Datatypes::DenseMatrix& matrixV_);
				void preAlocateInverseMatrices(const SCIRun::Core::Datatypes::DenseMatrix& forwardMatrix_, const SCIRun::Core::Datatypes::DenseMatrix& measuredData_ , const SCIRun::Core::Datatypes::DenseMatrix& sourceWeighting_, const SCIRun::Core::Datatypes::DenseMatrix& sensorWeighting_);

		        virtual SCIRun::Core::Datatypes::DenseMatrix computeInverseSolution( double truncationPoint, bool inverseCalculation) const;
				std::vector<double> computeLambdaArray( double lambdaMin, double lambdaMax, int nLambda ) const;
		        //      bool checkInputMatrixSizes(); // DEFINED IN PARENT, MIGHT WANT TO OVERRIDE SOME OTHER TIME


		    };
}}}}

#endif
