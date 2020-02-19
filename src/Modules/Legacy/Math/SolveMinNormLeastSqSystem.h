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


///
///@file  SolveMinNormLeastSqSystem
///@brief This module computes the minimal norm, least squared solution to a nx3
/// linear system.
///
///@author
///   David Weinstein
///   Department of Computer Science
///   University of Utah
///@date  June 1999
///
///@details
/// This module computes the minimal norm, least squared solution to a
///  nx3 linear system.
/// Given four input ColumnMatrices (v0,v1,v2,b),
///  find the three coefficients (w0,w1,w2) that minimize:
///  | (w0v0 + w1v1 + w2v2) - b |.
/// If more than one minimum exists (the system is under-determined),
///  choose the coefficients such that (w0,w1,w2) has minimum norm.
/// We output the vector (w0,w1,w2) as a row-matrix,
///  and we output the ColumnMatrix (called x), which is: | w0v0 + w1v1 + w2v2 |.
///

#ifndef MODULES_LEGACY_MATH_SOLVEMINNORMLEASTSQSYSTEM_H
#define MODULES_LEGACY_MATH_SOLVEMINNORMLEASTSQSYSTEM_H

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Math/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Math {

      class SCISHARE SolveMinNormLeastSqSystem : public SCIRun::Dataflow::Networks::Module,
        public Has4InputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag>,
        public Has2OutputPorts<MatrixPortTag, MatrixPortTag>
      {
      public:
        SolveMinNormLeastSqSystem();
        virtual void execute() override;
        virtual void setStateDefaults() override {}
        INPUT_PORT(0, BasisVector1, DenseColumnMatrix);
        INPUT_PORT(1, BasisVector2, DenseColumnMatrix);
        INPUT_PORT(2, BasisVector3, DenseColumnMatrix);
        INPUT_PORT(3, TargetVector, DenseColumnMatrix);
        OUTPUT_PORT(0, WeightVector, DenseColumnMatrix);
        OUTPUT_PORT(1, ResultVector, DenseColumnMatrix);
        MODULE_TRAITS_AND_INFO(NoAlgoOrUI)
      };
}}}

#endif
