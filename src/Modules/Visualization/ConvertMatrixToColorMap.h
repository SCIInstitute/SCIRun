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


#ifndef MODULES_VISUALIZATION_CONVERTMATRIXTOCOLORMAP_H
#define MODULES_VISUALIZATION_CONVERTMATRIXTOCOLORMAP_H

#include <Dataflow/Network/Module.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Visualization {

      /// Converts an Nx3 (RGB) or Nx4 (RGBA) matrix of color values into a ColorMap.
      /// Each matrix row is a control point; values are expected in [0, 1]. Rows whose
      /// entries exceed 1 are assumed to be in [0, 255] and are scaled accordingly.
      class SCISHARE ConvertMatrixToColorMap : public SCIRun::Dataflow::Networks::Module,
        public Has1InputPort<MatrixPortTag>,
        public Has1OutputPort<ColorMapPortTag>
      {
      public:
        ConvertMatrixToColorMap();
        void execute() override;
        void setStateDefaults() override {}

        INPUT_PORT(0, InputMatrix, Matrix);
        OUTPUT_PORT(0, OutputColorMap, ColorMap);

        MODULE_TRAITS_AND_INFO(ModuleFlags::NoAlgoOrUI)

        NEW_HELP_WEBPAGE_ONLY
      };
    }
  }
}

#endif
