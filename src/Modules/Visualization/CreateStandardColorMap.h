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


/// @todo Documentation Modules/Visualization/CreateStandardColorMap.h

#ifndef MODULES_VISUALIZATION_CREATESTANDARDCOLORMAP_H
#define MODULES_VISUALIZATION_CREATESTANDARDCOLORMAP_H

#include <Dataflow/Network/Module.h>
#include <Modules/Visualization/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Visualization
      {
        ALGORITHM_PARAMETER_DECL(ColorMapName);
        ALGORITHM_PARAMETER_DECL(ColorMapInvert);
        ALGORITHM_PARAMETER_DECL(ColorMapShift);
        ALGORITHM_PARAMETER_DECL(ColorMapResolution);
        ALGORITHM_PARAMETER_DECL(AlphaUserPointsVector);
        ALGORITHM_PARAMETER_DECL(AlphaFunctionVector);
        ALGORITHM_PARAMETER_DECL(ColorMapOption);
        ALGORITHM_PARAMETER_DECL(CustomColor0);
        ALGORITHM_PARAMETER_DECL(CustomColor1);
        ALGORITHM_PARAMETER_DECL(DefaultAlphaValue);
      }
    }
  }

  namespace Modules
  {
    namespace Visualization
    {
      class SCISHARE CreateStandardColorMap : public SCIRun::Dataflow::Networks::Module,
        public HasNoInputPorts,
        public Has1OutputPort<ColorMapPortTag>
      {
        CONVERTED_VERSION_OF_MODULE(CreateStandardColorMaps)
      public:
        CreateStandardColorMap();
        void execute() override;
        void setStateDefaults() override;

        OUTPUT_PORT(0, ColorMapObject, ColorMap);

        MODULE_TRAITS_AND_INFO(ModuleFlags::ModuleHasUI)

      private:
        enum ColorMapOptionType
        {
          PREDEFINED,
          CUSTOM
        };
      };
    }
  }
}

#endif
