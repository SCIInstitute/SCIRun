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


#ifndef MODULES_LEGACY_FIELDS_INTERFACEWITHTETGEN_H__
#define MODULES_LEGACY_FIELDS_INTERFACEWITHTETGEN_H__

#include <Dataflow/Network/Module.h>
#include <Core/Algorithms/Base/AlgorithmMacros.h>
#include <Modules/Legacy/Fields/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {
      namespace Fields {
        ALGORITHM_PARAMETER_DECL(PiecewiseFlag);
        ALGORITHM_PARAMETER_DECL(AssignFlag);
        ALGORITHM_PARAMETER_DECL(SetNonzeroAttributeFlag);
        ALGORITHM_PARAMETER_DECL(SuppressSplitFlag);
        ALGORITHM_PARAMETER_DECL(SetSplitFlag);
        ALGORITHM_PARAMETER_DECL(QualityFlag);
        ALGORITHM_PARAMETER_DECL(SetRatioFlag);
        ALGORITHM_PARAMETER_DECL(VolConstraintFlag);
        ALGORITHM_PARAMETER_DECL(SetMaxVolConstraintFlag);
        ALGORITHM_PARAMETER_DECL(MinRadius);
        ALGORITHM_PARAMETER_DECL(MaxVolConstraint);
        ALGORITHM_PARAMETER_DECL(DetectIntersectionsFlag);
        ALGORITHM_PARAMETER_DECL(MoreSwitches);
      }}}

  namespace Modules {
    namespace Fields {

      class SCISHARE InterfaceWithTetGen : public Dataflow::Networks::Module,
        public Has4InputPorts<FieldPortTag, FieldPortTag, FieldPortTag, DynamicPortTag<FieldPortTag>>,
        public Has1OutputPort<FieldPortTag>
      {
      public:
        InterfaceWithTetGen();

        void execute() override;
        void setStateDefaults() override;
        bool hasDynamicPorts() const override { return true; }

        #ifndef WITH_TETGEN
          DISABLED_WITHOUT_ABOVE_COMPILE_FLAG
        #endif

        INPUT_PORT(0, Main, Field);
        INPUT_PORT(1, Points, Field);
        INPUT_PORT(2, Region_Attribs, Field);
        INPUT_PORT_DYNAMIC(3, Regions, Field);
        OUTPUT_PORT(0, TetVol, Field);

        MODULE_TRAITS_AND_INFO(ModuleHasUI)
      };
    }
  }
}

#endif
