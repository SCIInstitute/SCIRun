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

#ifndef MODULES_VISUALIZATION_SHOWCOLORMAPMODULE_H
#define MODULES_VISUALIZATION_SHOWCOLORMAPMODULE_H

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/Geometry.h>
#include <Modules/Visualization/TextBuilder.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Visualization {

      class SCISHARE ShowColorMapModule : public SCIRun::Dataflow::Networks::GeometryGeneratingModule,
        public Has1InputPort<ColorMapPortTag>,
        public Has1OutputPort<GeometryPortTag>
      {
      private:
        SCIRun::Modules::Visualization::TextBuilder text_;
      public:
        ShowColorMapModule();
        virtual void execute();
        Core::Datatypes::GeometryHandle buildGeometryObject(Core::Datatypes::ColorMapHandle cm,
          Dataflow::Networks::ModuleStateHandle state,
          const std::string& id);

        static Core::Algorithms::AlgorithmParameterName DisplaySide;
        static Core::Algorithms::AlgorithmParameterName DisplayLength;
        static Core::Algorithms::AlgorithmParameterName TextSize;
        static Core::Algorithms::AlgorithmParameterName TextColor;
        static Core::Algorithms::AlgorithmParameterName Labels;
        static Core::Algorithms::AlgorithmParameterName Scale;
        static Core::Algorithms::AlgorithmParameterName Units;
        static Core::Algorithms::AlgorithmParameterName SignificantDigits;
        static Core::Algorithms::AlgorithmParameterName AddExtraSpace;
        static Core::Algorithms::AlgorithmParameterName TextRed;
        static Core::Algorithms::AlgorithmParameterName TextGreen;
        static Core::Algorithms::AlgorithmParameterName TextBlue;
        static Core::Algorithms::AlgorithmParameterName XTranslation;
        static Core::Algorithms::AlgorithmParameterName YTranslation;

        virtual void setStateDefaults();
        INPUT_PORT(0, ColorMapObject, ColorMap);
        OUTPUT_PORT(0, GeometryOutput, GeometryObject);

      };
    }
  }
}

#endif
