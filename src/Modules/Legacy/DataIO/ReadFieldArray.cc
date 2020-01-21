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


/// @todo Documentation Modules/Legacy/DataIO/ReadFieldArray.cc

#include <Dataflow/Network/Ports/FieldArrayPort.h>
#include <Dataflow/Modules/DataIO/GenericReader.h>

namespace SCIRun {

template class GenericReader<FieldArrayHandle>;

class ReadFieldArray : public GenericReader<FieldArrayHandle> {
  protected:
    GuiString gui_types_;
    GuiString gui_filetype_;

  public:
    ReadFieldArray(GuiContext* ctx);
    virtual ~ReadFieldArray() {}
    virtual void execute();
};


DECLARE_MAKER(ReadFieldArray)

ReadFieldArray::ReadFieldArray(GuiContext* ctx)
  : GenericReader<FieldArrayHandle>("ReadFieldArray", ctx, "DataIO", "SCIRun"),
    gui_types_(get_ctx()->subVar("types", false)),
    gui_filetype_(get_ctx()->subVar("filetype"))
{
  std::string importtypes = "{";
  importtypes += "{{SCIRun FieldArray File} {.flda} } ";
  importtypes += "}";

  gui_types_.set(importtypes);
}


void
ReadFieldArray::execute()
{
  if (gui_types_.changed() || gui_filetype_.changed()) inputs_changed_ = true;
  importing_ = false;

  GenericReader<FieldArrayHandle>::execute();
}

} // End namespace SCIRun
