/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

/// @todo Documentation Modules/Basic/DynamicPortTester.h

#ifndef MODULES_BASIC_DYNAMIC_PORT_TESTER_H
#define MODULES_BASIC_DYNAMIC_PORT_TESTER_H

#include <Dataflow/Network/Module.h>
#include <Modules/Basic/share.h>

namespace SCIRun {
namespace Modules {
namespace Basic {
  
  class SCISHARE DynamicPortTester : public SCIRun::Dataflow::Networks::Module,
    public Has3InputPorts<
      DynamicPortTag<MatrixPortTag>, 
      DynamicPortTag<FieldPortTag>, 
      DynamicPortTag<StringPortTag>>,
    public Has3OutputPorts<ScalarPortTag, ScalarPortTag, ScalarPortTag>
  {
  public:
    DynamicPortTester();
    virtual void execute();
    virtual void setStateDefaults() {}
    virtual bool hasDynamicPorts() const { return true; }

    INPUT_PORT_DYNAMIC(0, DynamicMatrix, Matrix);
    INPUT_PORT_DYNAMIC(1, DynamicField, LegacyField);
    INPUT_PORT_DYNAMIC(2, DynamicString, String);

    OUTPUT_PORT(0, NumMatrices, Int32);
    OUTPUT_PORT(1, NumFields, Int32);
    OUTPUT_PORT(2, NumStrings, Int32);
  };

}}}

#endif
