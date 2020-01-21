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


#ifndef MODULES_TEEM_DATAIO_READ_NRRD_H
#define MODULES_TEEM_DATAIO_READ_NRRD_H

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Teem/DataIO/share.h>

namespace SCIRun {
namespace Modules {
namespace DataIO {

  //TODO: inherit from GenericReader
  class SCISHARE ReadNrrd : public Dataflow::Networks::Module, //GenericReader<NrrdDataHandle, NrrdPortTag>
    public HasNoInputPorts,
    public Has1OutputPort<NrrdPortTag>
  {
  public:
    //typedef GenericReader<NrrdDataHandle, NrrdPortTag> my_base;
    ReadNrrd();
    virtual void execute() override;
    virtual void setStateDefaults() override;
    //virtual bool useCustomImporter(const std::string& filename) const override;
    //virtual bool call_importer(const std::string& filename, NrrdDataHandle& handle) override;

    OUTPUT_PORT(0, Output_Data, NrrdDataType);

    static std::string fileTypeList();

    MODULE_TRAITS_AND_INFO(ModuleHasUI)
  private:
    NrrdDataHandle read_nrrd();
    NrrdDataHandle read_file(const std::string& fn);
    bool write_tmpfile(const std::string& filename, std::string* tmpfilename,
                              const std::string& conv_command);
  };

}}}

#endif
