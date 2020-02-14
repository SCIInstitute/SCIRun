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


/// @todo Documentation Modules/Legacy/DataIO/WriteFieldArray.cc

#include <Dataflow/Network/Ports/FieldArrayPort.h>
#include <Dataflow/Modules/DataIO/GenericWriter.h>

namespace SCIRun {

template class GenericWriter<FieldArrayHandle>;

class WriteFieldArray : public GenericWriter<FieldArrayHandle> {
  protected:
    GuiString gui_types_;
    GuiString gui_exporttype_;
    GuiInt gui_increment_;
    GuiInt gui_current_;

  public:
    WriteFieldArray(GuiContext* ctx);
    virtual ~WriteFieldArray() {}

    virtual void execute();
};


DECLARE_MAKER(WriteFieldArray)


WriteFieldArray::WriteFieldArray(GuiContext* ctx)
  : GenericWriter<FieldArrayHandle>("WriteFieldArray", ctx, "DataIO", "SCIRun"),
    gui_types_(get_ctx()->subVar("types", false)),
    gui_exporttype_(get_ctx()->subVar("exporttype"), ""),
    gui_increment_(get_ctx()->subVar("increment"), 0),
    gui_current_(get_ctx()->subVar("current"), 0)
{
  std::string exporttypes = "{";
  exporttypes += "{{SCIRun FieldArray Binary} {.flda} } ";
  exporttypes += "{{SCIRun FieldArray ASCII} {.flda} } ";
  exporttypes += "}";

  gui_types_.set(exporttypes);
}

void
WriteFieldArray::execute()
{
  exporting_ = false;

  // Determine if we're ASCII or Binary
  const std::string ftpre = gui_exporttype_.get();
  const std::string::size_type loc = ftpre.find(" (");
  const std::string ft = ftpre.substr(0, loc);
  std::string ab = "Binary";

  if (ft == "SCIRun FieldArray ASCII") ab = "ASCII";
  filetype_.set(ab);

  //get the current file name
  const std::string oldfilename=filename_.get();

  //determine if we should increment an index in the file name
  if (gui_increment_.get())
  {

    //warn the user if they try to use 'Increment' incorrectly
    const std::string::size_type loc2 = oldfilename.find("%d");
    if(loc2 == std::string::npos)
    {
      remark("To use the increment function, there must be a '%d' in the file name.");
    }

    char buf[1024];

    int current=gui_current_.get();
    sprintf(buf, filename_.get().c_str(), current);

    filename_.set(buf);
    gui_current_.set(current+1);
  }

  GenericWriter<FieldArrayHandle>::execute();

   if (gui_increment_.get())
   filename_.set(oldfilename);
}


} // End namespace SCIRun
