/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
/// @file  ReadField.cc
/// 
/// @author
///    Steven G. Parker
///    Department of Computer Science
///    University of Utah
/// @date  July 1994
/// 

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Modules/DataIO/GenericReader.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>

namespace SCIRun {

template class GenericReader<FieldHandle>;

/// @class ReadField
/// @brief This module reads a field from file 
///
/// (a SCIRun .fld file and various other formats using a plugin system).  

class ReadField : public GenericReader<FieldHandle> {
  protected:
    GuiString gui_types_;
    GuiString gui_filetype_;
    GuiString gui_filename_base_;
    GuiInt    gui_number_in_series_;
    GuiInt    gui_delay_;

    virtual bool call_importer(const std::string &filename, FieldHandle &fHandle);

  public:
    ReadField(GuiContext* ctx);
    virtual ~ReadField() {}
    virtual void execute();
};


DECLARE_MAKER(ReadField)

ReadField::ReadField(GuiContext* ctx)
  : GenericReader<FieldHandle>("ReadField", ctx, "DataIO", "SCIRun"),
    gui_types_(get_ctx()->subVar("types", false)),
    gui_filetype_(get_ctx()->subVar("filetype")),
    gui_filename_base_(get_ctx()->subVar("filename_base"), ""),
    gui_number_in_series_(get_ctx()->subVar("number_in_series"), 0),
    gui_delay_(get_ctx()->subVar("delay"), 0)
{
  FieldIEPluginManager mgr;
  std::vector<std::string> importers;
  mgr.get_importer_list(importers);
  
  std::string importtypes = "{";
  importtypes += "{{SCIRun Field File} {.fld} } ";

  for (unsigned int i = 0; i < importers.size(); i++)
  {
    FieldIEPlugin *pl = mgr.get_plugin(importers[i]);
    if (pl->fileextension != "")
    {
      importtypes += "{{" + importers[i] + "} {" + pl->fileextension + "} } ";
    }
    else
    {
      importtypes += "{{" + importers[i] + "} {.*} } ";
    }
  }

  importtypes += "}";

  gui_types_.set(importtypes);
}

bool
ReadField::call_importer(const std::string &filename,
			 FieldHandle & fHandle)
{
  const std::string ftpre = gui_filetype_.get();
  const std::string::size_type loc = ftpre.find(" (");
  const std::string ft = ftpre.substr(0, loc);
  
  FieldIEPluginManager mgr;
  FieldIEPlugin *pl = mgr.get_plugin(ft);
  if (pl)
  {
    fHandle = pl->filereader(this, filename.c_str());
    return fHandle.get_rep();
  }
  return false;
}

void
ReadField::execute()
{     
  if (gui_types_.changed() || gui_filetype_.changed()) inputs_changed_ = true; 

  const std::string ftpre = gui_filetype_.get();
  const std::string::size_type loc = ftpre.find(" (");
  const std::string ft = ftpre.substr(0, loc);

  importing_ = !(ft == "" ||
		 ft == "SCIRun Field File" ||
		 ft == "SCIRun Field Any");
     
  GenericReader<FieldHandle>::execute();
}

} // End namespace SCIRun
