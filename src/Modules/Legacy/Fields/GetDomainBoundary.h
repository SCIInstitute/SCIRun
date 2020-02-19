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


#ifndef MODULES_LEGACY_FIELDS_GETDOMAINBOUNDARY_H__
#define MODULES_LEGACY_FIELDS_GETDOMAINBOUNDARY_H__

#include <Dataflow/Network/Module.h>
#include <Core/Thread/Interruptible.h>
#include <Modules/Legacy/Fields/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Fields {

	/// @class GetDomainBoundary
	/// @brief This module extracts the boundaries of the different regions in a label map.

      class SCISHARE GetDomainBoundary : public Dataflow::Networks::Module,
        public Has4InputPorts<FieldPortTag, ScalarPortTag, ScalarPortTag, MatrixPortTag>,
        public Has1OutputPort<FieldPortTag>,
        public Core::Thread::Interruptible
      {
      public:
        GetDomainBoundary();

        virtual void execute() override;
        virtual void setStateDefaults() override;

        INPUT_PORT(0, InputField, Field);
        INPUT_PORT(1, MinValue, Double);
        INPUT_PORT(2, MaxValue, Double);
        INPUT_PORT(3, ElemLink, SparseRowMatrix);
        OUTPUT_PORT(0, BoundaryField, Field);

        MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm)
      };

    }
  }
}

#endif
