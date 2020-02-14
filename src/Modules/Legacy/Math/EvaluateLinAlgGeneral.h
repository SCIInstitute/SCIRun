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


#ifndef MODULES_LEGACY_MATH_EVALUATELINEARALGEBRAGENERAL_H
#define MODULES_LEGACY_MATH_EVALUATELINEARALGEBRAGENERAL_H

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Math/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Math {

      class SCISHARE EvaluateLinearAlgebraGeneral : public SCIRun::Dataflow::Networks::Module,
        public Has5InputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag>,
        public Has5OutputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag>
      {
      public:
        EvaluateLinearAlgebraGeneral();
        virtual void execute() override;
        virtual void setStateDefaults() override;
        INPUT_PORT(0, i1, Matrix);
        INPUT_PORT(1, i2, Matrix);
        INPUT_PORT(2, i3, Matrix);
        INPUT_PORT(3, i4, Matrix);
        INPUT_PORT(4, i5, Matrix);
        OUTPUT_PORT(0, o1, Matrix);
        OUTPUT_PORT(1, o2, Matrix);
        OUTPUT_PORT(2, o3, Matrix);
        OUTPUT_PORT(3, o4, Matrix);
        OUTPUT_PORT(4, o5, Matrix);

        MODULE_TRAITS_AND_INFO(ModuleHasUI)
      };
}}}

#endif
