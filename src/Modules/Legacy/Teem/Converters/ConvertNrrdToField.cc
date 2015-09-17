/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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


/*
 *  ConvertNrrdToField.cc:  Convert a Nrrd to a Field
 *
 *  Written by:
 *   David Weinstein
 *   School of Computing
 *   University of Utah
 *   February 2001
 *
 */

#include <Core/Algorithms/Converter/ConverterAlgo.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Datatypes/NrrdData.h>
#include <Dataflow/Network/Ports/NrrdPort.h>

#include <Dataflow/Network/Module.h>


namespace SCITeem {

using namespace SCIRun;

class ConvertNrrdToField : public Module {
public:
  ConvertNrrdToField(GuiContext*);

  virtual void execute();

private:
  GuiString guidatalocation_;
  GuiString guifieldtype_;
  GuiString guiconvertparity_;
};


DECLARE_MAKER(ConvertNrrdToField)

ConvertNrrdToField::ConvertNrrdToField(GuiContext* ctx) : 
  Module("ConvertNrrdToField", ctx, Source, "Converters", "Teem"),
  guidatalocation_(get_ctx()->subVar("datalocation")),
  guifieldtype_(get_ctx()->subVar("fieldtype")),
  guiconvertparity_(get_ctx()->subVar("convertparity"))
{
}

void 
ConvertNrrdToField::execute()
{
  // Define local handles of data objects:
  NrrdDataHandle nrrd;
  FieldHandle ofield;

  // Get the new input data: 
  if (!(get_input_handle("Nrrd",nrrd,true))) return;

  // Only reexecute if the input changed. SCIRun uses simple scheduling
  // that executes every module downstream even if no data has changed:  
  if (inputs_changed_ || guidatalocation_.changed() || guifieldtype_.changed() 
      || !oport_cached("Field"))
  {
    SCIRunAlgo::ConverterAlgo algo(this);
    std::string datalocation = guidatalocation_.get();
    std::string fieldtype = guifieldtype_.get();
    std::string convertparity = guiconvertparity_.get();
    if (!(algo.NrrdToField(nrrd,ofield,datalocation,fieldtype,convertparity))) return;

    // send new output if there is any:    
    send_output_handle("Field", ofield);
  }
}

} // End namespace SCITeem
