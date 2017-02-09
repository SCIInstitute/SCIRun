
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

#ifndef MODULES_LEGACY_INVERSE_SolveInverseProblemWithTikhonovSVD_H__
#define MODULES_LEGACY_INVERSE_SolveInverseProblemWithTikhonovSVD_H__

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Inverse/SolveInverseProblemWithTikhonov.h>

#include <Modules/Legacy/Inverse/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Inverse {

      class SCISHARE SolveInverseProblemWithTikhonovSVD :
        public Dataflow::Networks::Module,
        public Has4InputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag>,
        public Has3OutputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag>
      {

      public:

          SolveInverseProblemWithTikhonovSVD(); // constructor
          void execute();                       // execute
          virtual void setStateDefaults();      // default params


          // define input ports
          INPUT_PORT(0, ForwardMatrix, Matrix);
          INPUT_PORT(1, WeightingInSourceSpace, Matrix);
          INPUT_PORT(2, MeasuredPotentials, Matrix);
          INPUT_PORT(3, WeightingInSensorSpace, Matrix);
//          INPUT_PORT(4, MatrixU, Matrix);
//          INPUT_PORT(5, MatrixS, Matrix);
//          INPUT_PORT(6, MatrixV, Matrix);

          OUTPUT_PORT(0, InverseSolution, Matrix);
          OUTPUT_PORT(1, RegularizationParameter, Matrix);
          OUTPUT_PORT(2, RegInverse, Matrix);

          // UI declaration
          static const Dataflow::Networks::ModuleLookupInfo staticInfo_;

          // Algorithm Params
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


      private:
          // update L-curve in UI
          void update_lcurve_gui(const double lambda, const BioPSE::TikhonovAlgorithm::LCurveInput& input, const int lambda_index);
      };

    }
  }
}

#endif
