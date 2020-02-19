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
///@author
///   Steven G. Parker,
///   Department of Computer Science,
///   University of Utah.
///@date July 1994
///

#include <Dataflow/Network/Ports/ColorMapPort.h>
#include <Dataflow/Modules/DataIO/GenericWriter.h>
#include <Core/ImportExport/ColorMap/ColorMapIEPlugin.h>

namespace SCIRun {

template class GenericWriter<ColorMapHandle>;

/// @class WriteColorMap
/// @brief This module writes a colormap to file (a SCIRun .cmap file).

class WriteColorMap : public GenericWriter<ColorMapHandle> {
  protected:
    GuiString gui_types_;
    GuiString gui_exporttype_;

    virtual bool call_exporter(const std::string &filename);

  public:
    WriteColorMap(GuiContext* ctx);
    virtual ~WriteColorMap() {}
    virtual void execute();
};


DECLARE_MAKER(WriteColorMap)

WriteColorMap::WriteColorMap(GuiContext* ctx)
  : GenericWriter<ColorMapHandle>("WriteColorMap", ctx, "DataIO", "SCIRun"),
    gui_types_(get_ctx()->subVar("types", false)),
    gui_exporttype_(get_ctx()->subVar("exporttype"), "")
{
  ColorMapIEPluginManager mgr;
  std::vector<std::string> exporters;
  mgr.get_exporter_list(exporters);

  std::string exporttypes = "{";
  exporttypes += "{{SCIRun ColorMap Binary} {.cmap} } ";
  exporttypes += "{{SCIRun ColorMap ASCII} {.cmap} } ";

  for (unsigned int i = 0; i < exporters.size(); i++)
  {
    ColorMapIEPlugin *pl = mgr.get_plugin(exporters[i]);
    if (pl->fileextension != "")
    {
      exporttypes += "{{" + exporters[i] + "} {" + pl->fileextension + "} } ";
    }
    else
    {
      exporttypes += "{{" + exporters[i] + "} {.*} } ";
    }
  }

  exporttypes += "}";

  gui_types_.set(exporttypes);
}

bool
WriteColorMap::call_exporter(const std::string &filename)
{
  const std::string ftpre = gui_exporttype_.get();
  const std::string::size_type loc = ftpre.find(" (");
  const std::string ft = ftpre.substr(0, loc);

  ColorMapIEPluginManager mgr;
  ColorMapIEPlugin *pl = mgr.get_plugin(ft);
  if (pl)
  {
    const bool result = pl->filewriter(this, handle_, filename.c_str());
    return result;
  }
  return false;
}

void
WriteColorMap::execute()
{
  const std::string ftpre = gui_exporttype_.get();
  const std::string::size_type loc = ftpre.find(" (");
  const std::string ft = ftpre.substr(0, loc);

  exporting_ = !(ft == "" ||
		 ft == "SCIRun ColorMap Binary" ||
		 ft == "SCIRun ColorMap ASCII" ||
                 ft == "Binary");

  // Determine if we're ASCII or Binary
  std::string ab = "Binary";
  if (ft == "SCIRun ColorMap ASCII") ab = "ASCII";
  filetype_.set(ab);

  GenericWriter<ColorMapHandle>::execute();
}



} // End namespace SCIRun
