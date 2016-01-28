/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

#ifndef MODULES_PYTHON_INTERFACEWITHPYTHON_H
#define MODULES_PYTHON_INTERFACEWITHPYTHON_H

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
        //ALGORITHM_PARAMETER_DECL(PollingIntervalMilliseconds);
        //ALGORITHM_PARAMETER_DECL(NumberOfRetries);
        ALGORITHM_PARAMETER_DECL(PythonCode);
      }
    }
  }

  namespace Modules
  {
    namespace Python
    {
      class SCISHARE InterfaceWithPython : public SCIRun::Dataflow::Networks::Module,
        public Has3InputPorts<DynamicPortTag<MatrixPortTag>, DynamicPortTag<FieldPortTag>, DynamicPortTag<StringPortTag>>,
        public Has3OutputPorts<MatrixPortTag, FieldPortTag, StringPortTag>
      {
      public:
        InterfaceWithPython();
        virtual void execute() override;
        virtual void setStateDefaults() override;
        virtual bool hasDynamicPorts() const override { return true; }
        INPUT_PORT_DYNAMIC(0, InputMatrix, Matrix);
        INPUT_PORT_DYNAMIC(1, InputField, LegacyField);
        INPUT_PORT_DYNAMIC(2, InputString, String);
        OUTPUT_PORT(0, PythonMatrix, Matrix);
        OUTPUT_PORT(1, PythonField, LegacyField);
        OUTPUT_PORT(2, PythonString, String);

        static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
      private:
        static Core::Thread::Mutex lock_;
      };

    }
  }
}

#endif
