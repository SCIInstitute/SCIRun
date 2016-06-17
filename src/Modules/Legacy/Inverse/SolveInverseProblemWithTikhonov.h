
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

#ifndef MODULES_LEGACY_INVERSE_SolveInverseProblemWithTikhonov_H__
#define MODULES_LEGACY_INVERSE_SolveInverseProblemWithTikhonov_H__

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Inverse/share.h>

namespace BioPSE
{
  namespace TikhonovAlgorithm
  {
    struct LCurveInput;
  }
}

namespace SCIRun {
  namespace Core
  {
    namespace Algorithms
    {
      namespace Inverse
      {
        ALGORITHM_PARAMETER_DECL(TikhonovSolutionSubcase);
        ALGORITHM_PARAMETER_DECL(TikhonovResidualSubcase);
      }
    }
  }
  namespace Modules {
    namespace Inverse {

      class SCISHARE SolveInverseProblemWithTikhonov : public Dataflow::Networks::Module,
        public Has4InputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag>,
        public Has3OutputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag>
      {
      public:
        SolveInverseProblemWithTikhonov();
        virtual void setStateDefaults();
        virtual void execute();

        INPUT_PORT(0, ForwardMatrix, Matrix);
        INPUT_PORT(1, WeightingInSourceSpace, Matrix);
        INPUT_PORT(2, MeasuredPotentials, Matrix);
        INPUT_PORT(3, WeightingInSensorSpace, Matrix);
        OUTPUT_PORT(0, InverseSolution, Matrix);
        OUTPUT_PORT(1, RegularizationParameter, Matrix);
        OUTPUT_PORT(2, RegInverse, Matrix);

        static const Dataflow::Networks::ModuleLookupInfo staticInfo_;

        static const Core::Algorithms::AlgorithmParameterName LambdaFromDirectEntry;
        static const Core::Algorithms::AlgorithmParameterName RegularizationMethod;
        static const Core::Algorithms::AlgorithmParameterName LambdaMin;
        static const Core::Algorithms::AlgorithmParameterName LambdaMax;
        static const Core::Algorithms::AlgorithmParameterName LambdaNum;
        static const Core::Algorithms::AlgorithmParameterName LambdaResolution;
        static const Core::Algorithms::AlgorithmParameterName TikhonovCase;
        static const Core::Algorithms::AlgorithmParameterName LambdaSliderValue;
        static const Core::Algorithms::AlgorithmParameterName LambdaCorner;
        static const Core::Algorithms::AlgorithmParameterName LCurveText;

      LEGACY_BIOPSE_MODULE

      private:
        void update_lcurve_gui(const double lambda, const BioPSE::TikhonovAlgorithm::LCurveInput& input, const int lambda_index);
      };

    }
  }
}

#endif
