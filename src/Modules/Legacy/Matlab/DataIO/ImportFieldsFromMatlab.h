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


#ifndef MODULES_LEGACY_MATLAB_DATAIO_IMPORTFIELDSFROMMATLAB_H
#define MODULES_LEGACY_MATLAB_DATAIO_IMPORTFIELDSFROMMATLAB_H

#include <Dataflow/Network/Module.h>
#include <Core/Matlab/matlabfwd.h>
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

  template <size_t N>
  using StringPortName = Dataflow::Networks::StaticPortName<Core::Datatypes::String, N>;

namespace Modules {
namespace Matlab {

  class SCISHARE MatlabFileIndexModule : public Dataflow::Networks::Module
  {
  public:
    explicit MatlabFileIndexModule(const Dataflow::Networks::ModuleLookupInfo& info) : Dataflow::Networks::Module(info) {}
  protected:
    void indexmatlabfile();
    void executeImpl(const StringPortName<0>& filenameIn, const StringPortName<6>& filenameOut);
    virtual SCIRun::Core::Datatypes::DatatypeHandle processMatlabData(const MatlabIO::matlabarray&) const = 0;
    virtual int indexMatlabFile(MatlabIO::matlabconverter& converter, const MatlabIO::matlabarray& mlarray, std::string& infostring) const = 0;
  };

  class SCISHARE ImportFieldsFromMatlab : public MatlabFileIndexModule,
    public Has1InputPort<StringPortTag>,
    public Has7OutputPorts<FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, StringPortTag>
  {
  public:
    ImportFieldsFromMatlab();
    virtual void execute() override;
    virtual void setStateDefaults() override;
    INPUT_PORT(0, Filename, String);
    OUTPUT_PORT(0, Field1, Field);
    OUTPUT_PORT(1, Field2, Field);
    OUTPUT_PORT(2, Field3, Field);
    OUTPUT_PORT(3, Field4, Field);
    OUTPUT_PORT(4, Field5, Field);
    OUTPUT_PORT(5, Field6, Field);
    OUTPUT_PORT(6, FilenameOut, String);
    enum { NUMPORTS = 6 };

    LEGACY_MATLAB_MODULE

    MODULE_TRAITS_AND_INFO(ModuleHasUI)

  protected:
    virtual void postStateChangeInternalSignalHookup() override;
    virtual SCIRun::Core::Datatypes::DatatypeHandle processMatlabData(const MatlabIO::matlabarray&) const override;
    virtual int indexMatlabFile(MatlabIO::matlabconverter& converter, const MatlabIO::matlabarray& mlarray, std::string& infostring) const override;
  };

}}}

#endif
