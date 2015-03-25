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
/// @todo Documentation Modules/Legacy/Fields/GetMeshQualityField.cc

#include <Core/Algorithms/Fields/MeshData/GetMeshQualityField.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

using namespace SCIRun;

class GetMeshQualityField : public Module {
  public:
    GetMeshQualityField(GuiContext*);
    virtual ~GetMeshQualityField() {}
    virtual void execute();
    
  private:
    GuiString guimethod_;
    SCIRunAlgo::GetMeshQualityFieldAlgo algo_;
};


DECLARE_MAKER(GetMeshQualityField)

GetMeshQualityField::GetMeshQualityField(GuiContext* ctx) :
  Module("GetMeshQualityField", ctx, Source, "MiscField", "SCIRun"),
  guimethod_(ctx->subVar("method"),"scaled_jacobian")
{
  algo_.set_progress_reporter(this);
}

void
GetMeshQualityField::execute()
{
  FieldHandle input, output;
  get_input_handle("Field",input,true);
  
  if (inputs_changed_ ||  guimethod_.changed() || !oport_cached("MeshQuality"))
  {
    update_state(Executing);
    
    algo_.set_option("metric",guimethod_.get());
    if(!(algo_.run(input,output))) return;

    send_output_handle("MeshQuality", output);
  }
}

} // End namespace SCIRun


