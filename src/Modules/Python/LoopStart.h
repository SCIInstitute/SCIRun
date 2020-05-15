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


#ifndef MODULES_PYTHON_LoopStart_H
#define MODULES_PYTHON_LoopStart_H

#include <Dataflow/Network/Module.h>
#include <Core/Thread/Mutex.h>
#include <Modules/Python/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Python
      {
        ALGORITHM_PARAMETER_DECL(LoopStartCode);
        ALGORITHM_PARAMETER_DECL(LoopIncrementCode);
        ALGORITHM_PARAMETER_DECL(IterationCount);
        ALGORITHM_PARAMETER_DECL(LoopOutputCode);
        class InterfaceWithPythonCodeTranslator;
      }
    }
  }

  namespace Modules
  {
    namespace Python
    {
      class SCISHARE LoopStart : public SCIRun::Dataflow::Networks::Module,
        public Has1InputPort<MetadataObjectPortTag>,
        public Has7OutputPorts<MetadataObjectPortTag, MatrixPortTag, MatrixPortTag, FieldPortTag, FieldPortTag, StringPortTag, StringPortTag>
      {
      public:
        LoopStart();
        void execute() override;
        void setStateDefaults() override;
        void postStateChangeInternalSignalHookup() override;
        INPUT_PORT(0, LoopEndCodeObject, MetadataObject);
        OUTPUT_PORT(0, LoopStartCodeObject, MetadataObject);
        OUTPUT_PORT(1, PythonMatrix1, Matrix);
        OUTPUT_PORT(2, PythonMatrix2, Matrix);
        OUTPUT_PORT(3, PythonField1, Field);
        OUTPUT_PORT(4, PythonField2, Field);
        OUTPUT_PORT(5, PythonString1, String);
        OUTPUT_PORT(6, PythonString2, String);

        MODULE_TRAITS_AND_INFO(ModuleHasUI)
        NEW_HELP_WEBPAGE_ONLY
        #ifndef BUILD_WITH_PYTHON
          DISABLED_WITHOUT_ABOVE_COMPILE_FLAG
        #endif
      private:
        SharedPointer<Core::Algorithms::Python::InterfaceWithPythonCodeTranslator> translator_;
      };

    }
  }
}

#endif
