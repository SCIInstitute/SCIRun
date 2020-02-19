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


#ifndef MODULES_LEGACY_MATH_ReportColumnMatrixMisfit_H
#define MODULES_LEGACY_MATH_ReportColumnMatrixMisfit_H

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Math/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Math {

      class SCISHARE ReportColumnMatrixMisfit : public SCIRun::Dataflow::Networks::Module,
        public Has2InputPorts<MatrixPortTag, MatrixPortTag>,
        public Has1OutputPort<ScalarPortTag>
      {
      public:
        ReportColumnMatrixMisfit();
        virtual void execute() override;
        virtual void setStateDefaults() override;
        INPUT_PORT(0, Vec1, Matrix);
        INPUT_PORT(1, Vec2, Matrix);
        OUTPUT_PORT(0, Error_Out, Double);

        MODULE_TRAITS_AND_INFO(ModuleHasUI)
      private:
        void showGraph(const Core::Datatypes::DenseColumnMatrix& v1, const Core::Datatypes::DenseColumnMatrix& v2,
          double ccInv, double rmsRel);
        bool containsInfiniteComponent(const Core::Datatypes::DenseColumnMatrix& v) const;
      };
}}}

#endif
