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

	 Author: 							Jaume Coll-Font
	 Last Modification:		September 6 2017
*/


#ifndef BioPSE_TikhonovImpl_H__
#define BioPSE_TikhonovImpl_H__

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Legacy/Inverse/share.h>
#include <vector>


namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Inverse {


	class SCISHARE TikhonovImpl
	{

	public:

		// constructor
		TikhonovImpl() {}
		virtual ~TikhonovImpl() {}

		virtual SCIRun::Core::Datatypes::DenseMatrix computeInverseSolution( double lambda_sq, bool inverseCalculation) const = 0;

		// default lambda step. Can ve overriden if necessary (see TSVD as reference)
		virtual std::vector<double> computeLambdaArray( double lambdaMin, double lambdaMax, int nLambda ) const;

	};

	}}}}

#endif
