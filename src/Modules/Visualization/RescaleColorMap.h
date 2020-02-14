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


#ifndef MODULES_VISUALIZATION_RESCALECOLORMAP_H
#define MODULES_VISUALIZATION_RESCALECOLORMAP_H

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/Geometry.h>
#include <Modules/Visualization/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Visualization
      {
        ALGORITHM_PARAMETER_DECL(AutoScale);
        ALGORITHM_PARAMETER_DECL(Symmetric);
        ALGORITHM_PARAMETER_DECL(FixedMin);
        ALGORITHM_PARAMETER_DECL(FixedMax);
      }
    }
  }

  namespace Modules
  {
    namespace Visualization
    {
      class SCISHARE RescaleColorMap : public SCIRun::Dataflow::Networks::Module,
        public Has2InputPorts<ColorMapPortTag, DynamicPortTag<FieldPortTag>>,
        public Has1OutputPort<ColorMapPortTag>
      {
      public:
        RescaleColorMap();
        virtual void execute() override;
        virtual void setStateDefaults() override;
        HAS_DYNAMIC_PORTS
        INPUT_PORT(0, ColorMapObject, ColorMap);
        INPUT_PORT_DYNAMIC(1, Field, Field);

        OUTPUT_PORT(0, ColorMapOutput, ColorMap);

        MODULE_TRAITS_AND_INFO(ModuleHasUI)
      };
    }
  }
}

#endif
