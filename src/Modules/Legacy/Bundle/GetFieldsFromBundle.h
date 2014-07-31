/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#ifndef MODULES_LEGACY_BUNDLE_GETFIELDSFROMBUNDLE_H__
#define MODULES_LEGACY_BUNDLE_GETFIELDSFROMBUNDLE_H__

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Bundle/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Bundles {
      
      /// @class GetFieldsFromBundle
      /// @brief This module retrieves a field object from a bundle.

      class SCISHARE GetFieldsFromBundle : public Dataflow::Networks::Module,
        public Has1InputPort<BundlePortTag>,
        public Has7OutputPorts<BundlePortTag, FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag>
      {
      public:
        GetFieldsFromBundle();
        virtual void setStateDefaults();
        virtual void execute();

        INPUT_PORT(0, InputBundle, Bundle);
        OUTPUT_PORT(0, OutputBundle, Bundle);
        OUTPUT_PORT(1, field1, LegacyField);
        OUTPUT_PORT(2, field2, LegacyField);
        OUTPUT_PORT(3, field3, LegacyField);
        OUTPUT_PORT(4, field4, LegacyField);
        OUTPUT_PORT(5, field5, LegacyField);
        OUTPUT_PORT(6, field6, LegacyField);

        static Core::Algorithms::AlgorithmParameterName FieldNameList;
        static const Core::Algorithms::AlgorithmParameterName FieldNames[];
        static const int NUM_BUNDLE_OUT = 6; //TODO: get from class def

        static Dataflow::Networks::ModuleLookupInfo staticInfo_;
      private:
        std::string makeFieldNameList(const Core::Datatypes::Bundle& bundle) const;
      };

    }
  }
}

#endif
