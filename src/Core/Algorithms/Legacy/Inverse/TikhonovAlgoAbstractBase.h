/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef BioPSE_TikhonovAlgoAbstractBase_H__
#define BioPSE_TikhonovAlgoAbstractBase_H__

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/Inverse/TikhonovImpl.h>
#include <Core/Algorithms/Field/share.h>


namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Inverse {


	ALGORITHM_PARAMETER_DECL(TikhonovImplementation);
	ALGORITHM_PARAMETER_DECL(RegularizationMethod);
	ALGORITHM_PARAMETER_DECL(regularizationChoice);
	ALGORITHM_PARAMETER_DECL(regularizationSolutionSubcase);
	ALGORITHM_PARAMETER_DECL(regularizationResidualSubcase);
	ALGORITHM_PARAMETER_DECL(LambdaFromDirectEntry);
	ALGORITHM_PARAMETER_DECL(LambdaMin);
	ALGORITHM_PARAMETER_DECL(LambdaMax);
	ALGORITHM_PARAMETER_DECL(LambdaNum);
	ALGORITHM_PARAMETER_DECL(LambdaResolution);
	ALGORITHM_PARAMETER_DECL(LambdaSliderValue);
	ALGORITHM_PARAMETER_DECL(LambdaCorner);
	ALGORITHM_PARAMETER_DECL(LCurveText);

	class SCISHARE TikhonovAlgoAbstractBase : virtual public AlgorithmBase
	{

	public:

		// define input names
		static const AlgorithmInputName ForwardMatrix;
		static  const AlgorithmInputName MeasuredPotentials;
		static  const AlgorithmInputName WeightingInSourceSpace;
		static  const AlgorithmInputName WeightingInSensorSpace;

		// input names particular for the SVD case
		static  const AlgorithmInputName matrixU;
		static  const AlgorithmInputName singularValues;
		static  const AlgorithmInputName matrixV;

		// define output names
		static const AlgorithmOutputName InverseSolution;
		static const AlgorithmOutputName RegularizationParameter;
		static const AlgorithmOutputName RegInverse;

		// Define algorithm choices
		enum AlgorithmChoice {
			automatic,
			underdetermined,
			overdetermined
		};
		enum AlgorithmSolutionSubcase {
			solution_constrained,
			solution_constrained_squared
		};
		enum AlgorithmResidualSubcase {
			residual_constrained,
			residual_constrained_squared
		};

		// constructor
		TikhonovAlgoAbstractBase();

		// run function
		virtual AlgorithmOutput run(const AlgorithmInput &) const override;

		// defined public functions
		// void update_graph( const AlgorithmInput & input,  double lambda, int lambda_index, double epsilon);
		static double FindCorner( const std::vector<double>& rho, const std::vector<double>& eta, const std::vector<double>& lambdaArray, const int nLambda );
		// static double LambdaLookup(LCurveInput& input, double lambda, int& lambda_index, const double epsilon);
		double computeLcurve( const SCIRun::Core::Algorithms::Inverse::TikhonovImpl * algoImpl,   const AlgorithmInput & input ) const;

		bool checkInputMatrixSizes( const AlgorithmInput & input ) const;

	};

	}}}}

#endif
