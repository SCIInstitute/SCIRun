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

#ifndef MODULES_MATH_BASICPLOTTER_H
#define MODULES_MATH_BASICPLOTTER_H

#include <Dataflow/Network/Module.h>
#include <Modules/Math/share.h>

namespace SCIRun {

  namespace Core {
    namespace Algorithms {
      namespace Math {
        ALGORITHM_PARAMETER_DECL(PlotTitle);
        ALGORITHM_PARAMETER_DECL(DataTitle);
        ALGORITHM_PARAMETER_DECL(XAxisLabel);
        ALGORITHM_PARAMETER_DECL(YAxisLabel);
        ALGORITHM_PARAMETER_DECL(VerticalAxisVisible);
        ALGORITHM_PARAMETER_DECL(HorizontalAxisVisible);
        ALGORITHM_PARAMETER_DECL(VerticalAxisPosition);
        ALGORITHM_PARAMETER_DECL(HorizontalAxisPosition);
        ALGORITHM_PARAMETER_DECL(ShowPointSymbols);
        ALGORITHM_PARAMETER_DECL(PlotColors);
        ALGORITHM_PARAMETER_DECL(PlotBackgroundColor);
        ALGORITHM_PARAMETER_DECL(CurveStyle);
        ALGORITHM_PARAMETER_DECL(TransposeData);
      }}}

  namespace Modules {
    namespace Math {

      class SCISHARE BasicPlotter : public Dataflow::Networks::Module,
        public Has1InputPort<MatrixPortTag>,
        public HasNoOutputPorts
      {
      public:
        BasicPlotter();
        virtual void setStateDefaults() override;
        virtual void execute() override;

        INPUT_PORT(0, InputMatrix, DenseMatrix);

        MODULE_TRAITS_AND_INFO(ModuleHasUI)
        NEW_HELP_WEBPAGE_ONLY
        HAS_DYNAMIC_PORTS
      };

    }
  }
};


#endif
