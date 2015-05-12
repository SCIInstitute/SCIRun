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
    public Has7InputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, FieldPortTag, FieldPortTag>,
    public Has3OutputPorts<FieldPortTag, MatrixPortTag, StringPortTag>
  {
  public:
    InterfaceWithMatlab();
    virtual void execute();
    virtual void setStateDefaults();
    INPUT_PORT(0, i1, Matrix);
    INPUT_PORT(1, i2, Matrix);
    INPUT_PORT(2, i3, Matrix);
    INPUT_PORT(3, i4, Matrix);
    INPUT_PORT(4, i5, Matrix);
    INPUT_PORT(5, field1, LegacyField);
    INPUT_PORT(6, field2, LegacyField);
    //INPUT_PORT(7, field3, LegacyField);
    OUTPUT_PORT(0, OutputField, LegacyField);
    OUTPUT_PORT(1, OutputMatrix, Matrix);
    OUTPUT_PORT(2, FilenameOut, String);

    static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
  private:
    boost::shared_ptr<class InterfaceWithMatlabImpl> impl_;
  };

}}}}

#endif