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


#ifndef MODULES_VISUALIZATION_SHOWCOLORMAPMODULE_H
#define MODULES_VISUALIZATION_SHOWCOLORMAPMODULE_H

#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Core/Datatypes/Geometry.h>
#include <Modules/Visualization/TextBuilder.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Visualization {

      class SCISHARE ShowColorMap : public SCIRun::Dataflow::Networks::GeometryGeneratingModule,
        public Has1InputPort<ColorMapPortTag>,
        public Has1OutputPort<GeometryPortTag>
      {
      public:
        ShowColorMap();
        void execute() override;
        Core::Datatypes::GeometryBaseHandle buildGeometryObject(Core::Datatypes::ColorMapHandle cm,
          Dataflow::Networks::ModuleStateHandle state,
          const std::string& id);

        static const Core::Algorithms::AlgorithmParameterName DisplaySide;
        static const Core::Algorithms::AlgorithmParameterName DisplayLength;
        static const Core::Algorithms::AlgorithmParameterName TextSize;
        static const Core::Algorithms::AlgorithmParameterName TextColor;
        static const Core::Algorithms::AlgorithmParameterName Labels;
        static const Core::Algorithms::AlgorithmParameterName Scale;
        static const Core::Algorithms::AlgorithmParameterName Units;
        static const Core::Algorithms::AlgorithmParameterName SignificantDigits;
        static const Core::Algorithms::AlgorithmParameterName AddExtraSpace;
        static const Core::Algorithms::AlgorithmParameterName TextRed;
        static const Core::Algorithms::AlgorithmParameterName TextGreen;
        static const Core::Algorithms::AlgorithmParameterName TextBlue;
        static const Core::Algorithms::AlgorithmParameterName XTranslation;
        static const Core::Algorithms::AlgorithmParameterName YTranslation;
        static const Core::Algorithms::AlgorithmParameterName ColorMapName;

        void setStateDefaults() override;
        INPUT_PORT(0, ColorMapObject, ColorMap);
        OUTPUT_PORT(0, GeometryOutput, GeometryObject);
        MODULE_TRAITS_AND_INFO(ModuleHasUI)

      private:
        TextBuilder textBuilder_;
      };
    }
  }
}

#endif
