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

#ifndef ALGORITHMS_MATH_EVALUATELINEARALGEBRAUNARY_H
#define ALGORITHMS_MATH_EVALUATELINEARALGEBRAUNARY_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Math/share.h>

/// \addtogroup Algorithms_Math
/// @{
///

namespace SCIRun {
namespace Core {
  namespace Algorithms {
    namespace Math {

      ///
      /// \class EvaluateLinearAlgebraUnaryAlgorithm
      ///
      /// \brief Computes several unary operations on general matrices
      ///

      class SCISHARE EvaluateLinearAlgebraUnaryAlgorithm : public AlgorithmBase
      {
       public:
        enum class Operator
        {
          NEGATE,
          TRANSPOSE,
          SCALAR_MULTIPLY,
          FUNCTION
        };

        using Inputs = Datatypes::MatrixHandle;
        struct Parameters
        {
          Operator op;
          double scalar;
          std::string func;
        };
        using Outputs = Datatypes::MatrixHandle;

        EvaluateLinearAlgebraUnaryAlgorithm();
        Outputs run(const Inputs& matrix, const Parameters& params) const;

        AlgorithmOutput run(const AlgorithmInput& input) const override;
      };

    }
  }
}
}

/*! @} End of Doxygen Groups*/

#endif
