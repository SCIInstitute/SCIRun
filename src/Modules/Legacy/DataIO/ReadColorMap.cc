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
/// @file  ReadColorMap.cc
///
/// @author
///    Steven G. Parker
///    Department of Computer Science
///    University of Utah
/// @date  July 1994
///

#include <Dataflow/Network/Ports/ColorMapPort.h>
#include <Dataflow/Modules/DataIO/GenericReader.h>
#include <Core/ImportExport/ColorMap/ColorMapIEPlugin.h>

namespace SCIRun {

template class GenericReader<ColorMapHandle>;

/// @class ReadColorMap
/// @brief Read a persistent colormap from a file (a SCIRun .cmap file)

class ReadColorMap : public GenericReader<ColorMapHandle> {
  protected:
    GuiString gui_types_;
    GuiString gui_filetype_;

    virtual bool call_importer(const std::string &filename,
             ColorMapHandle & cmHandle);

  public:
    ReadColorMap(GuiContext* ctx);
    virtual ~ReadColorMap() {}

    virtual void execute();
};

DECLARE_MAKER(ReadColorMap)

ReadColorMap::ReadColorMap(GuiContext* ctx)
  : GenericReader<ColorMapHandle>("ReadColorMap", ctx, "DataIO", "SCIRun"),
    gui_types_(get_ctx()->subVar("types", false)),
    gui_filetype_(get_ctx()->subVar("filetype"))
{
  ColorMapIEPluginManager mgr;
  std::vector<std::string> importers;
  mgr.get_importer_list(importers);

  std::string importtypes = "{";
  importtypes += "{{SCIRun Colormap File} {.cmap} } ";

  for (unsigned int i = 0; i < importers.size(); i++)
  {
    ColorMapIEPlugin *pl = mgr.get_plugin(importers[i]);
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
ReadColorMap::call_importer(const std::string &filename, ColorMapHandle & cmHandle)
{
  const std::string ftpre = gui_filetype_.get();
  const std::string::size_type loc = ftpre.find(" (");
  const std::string ft = ftpre.substr(0, loc);

  ColorMapIEPluginManager mgr;
  ColorMapIEPlugin *pl = mgr.get_plugin(ft);
  if (pl)
  {
    cmHandle = pl->filereader(this, filename.c_str());
    return cmHandle.get_rep();
  }
  return false;
}


void
ReadColorMap::execute()
{
  if (gui_types_.changed() || gui_filetype_.changed()) inputs_changed_ = true;

  const std::string ftpre = gui_filetype_.get();
  const std::string::size_type loc = ftpre.find(" (");
  const std::string ft = ftpre.substr(0, loc);

  importing_ = !(ft == "" ||
		 ft == "SCIRun Colormap File" ||
		 ft == "SCIRun Colormap Any");

  GenericReader<ColorMapHandle>::execute();
}


} // End namespace SCIRun
