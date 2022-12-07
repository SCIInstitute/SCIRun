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
///   Michael Callahan,
///   Department of Computer Science,
///   University of Utah.
///@date  September 2004
///

#include <Dataflow/Network/Ports/ColorMap2Port.h>
#include <Dataflow/Modules/DataIO/GenericWriter.h>

namespace SCIRun {

template class GenericWriter<ColorMap2Handle>;

/// @class WriteColorMap2D
/// @brief This module writes a colormap2D to file (a SCIRun .cmap2 file).

class WriteColorMap2D : public GenericWriter<ColorMap2Handle> {
  protected:
    GuiString gui_types_;
    GuiString gui_exporttype_;

    virtual bool call_exporter(const std::string &filename);

  public:
    WriteColorMap2D(GuiContext* ctx);
    virtual ~WriteColorMap2D() {}
    virtual void execute();
};


DECLARE_MAKER(WriteColorMap2D)

WriteColorMap2D::WriteColorMap2D(GuiContext* ctx)
  : GenericWriter<ColorMap2Handle>("WriteColorMap2D", ctx, "DataIO", "SCIRun"),
    gui_types_(get_ctx()->subVar("types", false)),
    gui_exporttype_(get_ctx()->subVar("exporttype"), "")
{
  std::string exporttypes = "{";
  exporttypes += "{{SCIRun ColorMap2D Binary} {.cmap2} } ";
  exporttypes += "{{SCIRun ColorMap2D ASCII} {.cmap2} } ";
  exporttypes += "}";

  gui_types_.set(exporttypes);
}

bool
WriteColorMap2D::call_exporter(const std::string &/*filename*/)
{
  return false;
}

void
WriteColorMap2D::execute()
{
  const std::string ftpre = gui_exporttype_.get();
  const std::string::size_type loc = ftpre.find(" (");
  const std::string ft = ftpre.substr(0, loc);

  exporting_ = false;

  // Determine if we're ASCII or Binary
  std::string ab = "Binary";
  if (ft == "SCIRun ColorMap2D ASCII") ab = "ASCII";
  filetype_.set(ab);

  GenericWriter<ColorMap2Handle>::execute();
}

} // End namespace SCIRun
