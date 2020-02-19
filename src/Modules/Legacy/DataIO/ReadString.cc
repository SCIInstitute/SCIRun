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


///
/// @file  ReadString.cc
///
/// @author
///    jeroen
///

#include <fstream>
#include <string>


#include <Dataflow/Network/Ports/StringPort.h>
#include <Dataflow/Modules/DataIO/GenericReader.h>

namespace SCIRun {

/// @class ReadString
/// @This module reads a string from file (an ASCII text file).

class ReadString : public GenericReader<StringHandle> {
  public:
    ReadString(GuiContext*);
    virtual ~ReadString() {}
    virtual void execute();

  protected:
    GuiString gui_types_;

    virtual bool call_importer(const std::string &filename, StringHandle &sHandle);
};

DECLARE_MAKER(ReadString)

ReadString::ReadString(GuiContext* ctx)
  : GenericReader<StringHandle>("ReadString", ctx, "DataIO", "SCIRun"),
    gui_types_(get_ctx()->subVar("types", false))
{
  gui_types_.set("{ {{textfile} {.txt .asc .doc}} {{all files} {.*}} }");
}

void
ReadString::execute()
{
  importing_ = true;
  GenericReader<StringHandle>::execute();
}


// Simple text reader
bool
ReadString::call_importer(const std::string &filename, StringHandle &sHandle)
{
  if (gui_types_.changed()) inputs_changed_ = true;

  std::string input;
  try
  {
    std::ifstream file(filename.c_str());
    std::string line;

    getline(file,line);
    input += line;

    while(!file.eof())
    {
      getline(file,line);
      input += "\n";
      input += line;
    }
  }
  catch(...)
  {
    error("Could not open file: " + filename);
    return(false);
  }

  sHandle = (StringHandle) new String(input);
  return true;
}

} // End namespace SCIRun
