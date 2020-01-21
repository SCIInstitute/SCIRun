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
/// @file  ReadColorMap2D.cc
///
/// @author
///    Michael Callahan
///    Department of Computer Science
///    University of Utah
/// @date  Sept 2004
///

#include <Dataflow/Network/Ports/ColorMap2Port.h>
#include <Dataflow/Modules/DataIO/GenericReader.h>

namespace SCIRun {

template class GenericReader<ColorMap2Handle>;

/// @class ReadColorMap2D
/// @brief This module reads a colormap2D from file (a SCIRun .cmap2 file).

class ReadColorMap2D : public GenericReader<ColorMap2Handle> {
  protected:
    GuiString gui_types_;
    GuiString gui_filetype_;

    virtual bool call_importer(const std::string &filename);

  public:
    ReadColorMap2D(GuiContext* ctx);
    virtual ~ReadColorMap2D() {}

    virtual void execute();
};

DECLARE_MAKER(ReadColorMap2D)

ReadColorMap2D::ReadColorMap2D(GuiContext* ctx)
  : GenericReader<ColorMap2Handle>("ReadColorMap2D", ctx, "DataIO", "SCIRun"),
    gui_types_(get_ctx()->subVar("types", false)),
    gui_filetype_(get_ctx()->subVar("filetype"))
{
  std::string importtypes = "{";
  importtypes += "{{SCIRun ColorMap2D File} {.cmap2} } ";
  importtypes += "}";

  gui_types_.set(importtypes);
}

bool
ReadColorMap2D::call_importer(const std::string &/*filename*/)
{
  return false;
}

void
ReadColorMap2D::execute()
{
  if (gui_types_.changed() || gui_filetype_.changed()) inputs_changed_ = true;

  importing_ = false;
  GenericReader<ColorMap2Handle>::execute();
}

} // End namespace SCIRun
