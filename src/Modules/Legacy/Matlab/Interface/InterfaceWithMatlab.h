/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef MODULES_LEGACY_MATLAB_MATLABINTERFACE_H
#define MODULES_LEGACY_MATLAB_MATLABINTERFACE_H

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Matlab/Interface/share.h>

namespace SCIRun {
namespace Modules {
namespace Matlab {
namespace Interface {
  
  class SCISHARE InterfaceWithMatlab : public SCIRun::Dataflow::Networks::Module,
    public Has3InputPorts<DynamicPortTag<MatrixPortTag>, DynamicPortTag<FieldPortTag>, DynamicPortTag<StringPortTag>>,
    public Has6OutputPorts<FieldPortTag, FieldPortTag, MatrixPortTag, MatrixPortTag, StringPortTag, StringPortTag>
  {
  public:
    InterfaceWithMatlab();
    virtual void execute();
    virtual void setStateDefaults();
    INPUT_PORT_DYNAMIC(0, InputMatrix, Matrix);
    INPUT_PORT_DYNAMIC(1, InputField, LegacyField);
    INPUT_PORT_DYNAMIC(2, InputString, String);
    OUTPUT_PORT(0, OutputField0, LegacyField);
    OUTPUT_PORT(1, OutputField1, LegacyField);
    OUTPUT_PORT(2, OutputMatrix0, Matrix);
    OUTPUT_PORT(3, OutputMatrix1, Matrix);
    OUTPUT_PORT(4, OutputString0, String);
    OUTPUT_PORT(5, OutputString1, String);

    static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
  private:
    boost::shared_ptr<class InterfaceWithMatlabImpl> impl_;
  };

}}}}

#endif