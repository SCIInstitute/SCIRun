/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <Core/Algorithms/Math/BuildNoiseColumnMatrix.h>

#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

#include <Core/Math/MusilRNG.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

MusilRNG *musil = new MusilRNG;

BuildNoiseColumnMatrixAlgorithm::BuildNoiseColumnMatrixAlgorithm()
{
	addParameter(SignalToNoiseRatio(),10);
}

void BuildNoiseColumnMatrixAlgorithm::run(MatrixHandle output_matrix) const
{
	if(matrix_is::sparse(output_matrix))
		matrix_convert::to_dense(output_matrix);		
	
	double mean, power, sigma;
	mean=power=sigma=0;
	int r, c;
	int nr = output_matrix->nrows();
	int nc = output_matrix->ncols();
	double curr;
	
	double snr = get(SignalToNoiseRatio()).toDouble();
	
	mean /= nr*nc;
	for(r = 0; r < nr; r++) {
		for(c = 0; c < nc; c++)
		{
			curr = output_matrix->get(r,c);
			power += (curr - mean)*(curr - mean);
		}
	}
	power /= nc*nr;
	
	sigma = sqrt(power)/(snr*Sqrt(2*M_PI));
	
	for(r = 0; r < nr; r++)
	{
		for(c = 0; c < nc; c++)
		{
			const double rnd = 2.0 * (*musil)() -1.0;
			double perturb = rnd * sigma * sqrt((-2.0 * log(rnd*rnd))/(rnd*rnd));
			output_matrix->put(r,c,perturb);
		}
	}
}

AlgorithmOutput BuildNoiseColumnMatrixAlgorithm::run_generic(const AlgorithmInput& input) const
{
	auto input_matrix = input.get<Matrix>(Variables::InputMatrix);
	
	MatrixHandle output_matrix = input_matrix;
	
	run(output_matrix);
	
	AlgorithmOutput output;
	output[Variables::ResultMatrix] = output_matrix;
	
	return output;
}

AlgorithmInputName BuildNoiseColumnMatrixAlgorithm::InputMatrix("InputMatrix");
AlgorithmOutputName BuildNoiseColumnMatrixAlgorithm::ResultMatrix("ResultMatrix");
AlgorithmParameterName BuildNoiseColumnMatrixAlgorithm::SignalToNoiseRatio() {return AlgorithmParameterName("SignalToNoiseRatio"); }



