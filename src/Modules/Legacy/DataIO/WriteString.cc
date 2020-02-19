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


#include <fstream>
#include <string>

#include <Dataflow/Network/Ports/StringPort.h>
#include <Dataflow/Modules/DataIO/GenericWriter.h>

namespace SCIRun {

using namespace SCIRun;

/// @class WriteString
/// @brief This module writes a string to file (an ASCII text file).

class WriteString : public GenericWriter<StringHandle> {
  public:
    WriteString(GuiContext*);
    virtual ~WriteString() {}
    virtual void execute();

  protected:
    GuiString gui_types_;
    GuiString gui_exporttype_;
    GuiInt gui_increment_;
    GuiInt gui_current_;

    virtual bool call_exporter(const std::string &filename);
};


DECLARE_MAKER(WriteString)

WriteString::WriteString(GuiContext* ctx)
  : GenericWriter<StringHandle>("WriteString", ctx, "DataIO", "SCIRun"),
    gui_types_(get_ctx()->subVar("types", false)),
    gui_exporttype_(get_ctx()->subVar("exporttype")),
    gui_increment_(get_ctx()->subVar("increment")),
    gui_current_(get_ctx()->subVar("current"))
{
  gui_types_.set("{ {{textfile} {.txt .asc .doc}} {{all files} {.*}} }");
}

bool
WriteString::call_exporter(const std::string &filename)
{
  try
  {
    std::ofstream file(filename.c_str());
    file << handle_->get();
  }
  catch(...)
  {
    error("Could not write file: " + filename);
    return(false);
  }

  return(true);
}

void
WriteString::execute()
{

  exporting_ = true;

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

    // ugly code, potential buffer overflow problem
    char buf[1024];
    int current=gui_current_.get();
    sprintf(buf, filename_.get().c_str(), current);

    filename_.set(buf);
    gui_current_.set(current+1);
  }

  GenericWriter<StringHandle>::execute();

  if (gui_increment_.get())
  filename_.set(oldfilename);
}

} // End namespace SCIRun
