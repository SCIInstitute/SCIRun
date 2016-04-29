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

#ifndef MODULES_LEGACY_MATLAB_DATAIO_IMPORTFIELDSFROMMATLAB_H
#define MODULES_LEGACY_MATLAB_DATAIO_IMPORTFIELDSFROMMATLAB_H

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Matlab/DataIO/share.h>

namespace SCIRun {
  namespace Core
  {
    namespace Algorithms
    {
      namespace Matlab
      {
        ALGORITHM_PARAMETER_DECL(FieldInfoStrings);
        ALGORITHM_PARAMETER_DECL(PortChoices);
      }
    }
  }

namespace Modules {
namespace Matlab {

  class SCISHARE ImportFieldsFromMatlab : public Dataflow::Networks::Module,
    public Has1InputPort<StringPortTag>,
    public Has7OutputPorts<FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, StringPortTag>
  {
  public:
    ImportFieldsFromMatlab();
    virtual void execute() override;
    virtual void setStateDefaults() override;
    INPUT_PORT(0, Filename, String);
    OUTPUT_PORT(0, Field1, LegacyField);
    OUTPUT_PORT(1, Field2, LegacyField);
    OUTPUT_PORT(2, Field3, LegacyField);
    OUTPUT_PORT(3, Field4, LegacyField);
    OUTPUT_PORT(4, Field5, LegacyField);
    OUTPUT_PORT(5, Field6, LegacyField);
    OUTPUT_PORT(6, FilenameOut, String);
    enum { NUMPORTS = 6 };

    static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
  private:
    void indexmatlabfile();
  };
}}}

#endif
