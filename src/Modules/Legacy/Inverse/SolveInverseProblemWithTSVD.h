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


///  File       : SolveInverseProblemWithTSVD.h
///  Author     : Jaume Coll-Font, Moritz Dannhauer, Ayla Khan, Dan White
///  Date       : September 06th, 2017 (last update)

#ifndef MODULES_LEGACY_INVERSE_SolveInverseProblemWithTSVD_H__
#define MODULES_LEGACY_INVERSE_SolveInverseProblemWithTSVD_H__

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Inverse/share.h>

namespace SCIRun {
namespace Modules {
namespace Inverse {

	class SCISHARE SolveInverseProblemWithTSVD : public SCIRun::Dataflow::Networks::Module,
		public Has7InputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag>,
		public Has3OutputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag>
	{
	public:
		SolveInverseProblemWithTSVD();
		virtual void execute();
		virtual void setStateDefaults();

		INPUT_PORT(0, ForwardMatrix, DenseMatrix);
		INPUT_PORT(1, WeightingInSourceSpace, DenseMatrix);
		INPUT_PORT(2, MeasuredPotentials, DenseMatrix);
		INPUT_PORT(3, WeightingInSensorSpace, DenseMatrix);
		INPUT_PORT(4, matrixU, DenseMatrix);
		INPUT_PORT(5, singularValues, DenseMatrix);
		INPUT_PORT(6, matrixV, DenseMatrix);
		OUTPUT_PORT(0, InverseSolution, DenseMatrix);
		OUTPUT_PORT(1, RegularizationParameter, DenseMatrix);
		OUTPUT_PORT(2, RegInverse, DenseMatrix);

		MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm)
	};
}}}

#endif
