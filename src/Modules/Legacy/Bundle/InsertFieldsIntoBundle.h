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

#ifndef MODULES_LEGACY_BUNDLE_INSERTFIELDSINTOBUNDLE_H__
#define MODULES_LEGACY_BUNDLE_INSERTFIELDSINTOBUNDLE_H__

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Bundle/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Bundles {

      /// @class InserFieldsIntoBundle
      /// @brief This module inserts a field object into a bundle.

      class SCISHARE InsertFieldsIntoBundle : public Dataflow::Networks::Module,
        public Has2InputPorts<BundlePortTag, DynamicPortTag<FieldPortTag>>,
        public Has1OutputPort<BundlePortTag>
      {
      public:
        InsertFieldsIntoBundle();
        virtual void setStateDefaults() override;
        virtual void execute() override;
        virtual bool hasDynamicPorts() const override { return true; }
        virtual void portAddedSlot(const Dataflow::Networks::ModuleId& mid, const Dataflow::Networks::PortId& pid) override;
        virtual void portRemovedSlot(const Dataflow::Networks::ModuleId& mid, const Dataflow::Networks::PortId& pid) override;

        INPUT_PORT(0, InputBundle, Bundle);
        INPUT_PORT_DYNAMIC(1, InputFields, LegacyField);
        OUTPUT_PORT(0, OutputBundle, Bundle);

        static const Core::Algorithms::AlgorithmParameterName BundleName;
        static const Core::Algorithms::AlgorithmParameterName NumFields;
        static const Core::Algorithms::AlgorithmParameterName FieldNames;
        static const Core::Algorithms::AlgorithmParameterName FieldReplace;

        static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
      };

    }
  }
}

#endif
