/*
 * The MIT License
 * 
 * Copyright (c) 2009 Scientific Computing and Imaging Institute,
 * University of Utah.
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/// @todo Documentation Modules/Legacy/Fields/PadRegularMesh.cc

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>

#include <Core/Algorithms/Fields/ResampleMesh/PadRegularMesh.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>


namespace SCIRun {

using namespace SCIRun;

class PadRegularMesh : public Module {
  public:
    PadRegularMesh(GuiContext*);
    virtual ~PadRegularMesh() {}

    virtual void execute();

  private:
    GuiDouble gui_pad_value_;
    GuiInt    gui_pad_size_;

    SCIRunAlgo::PadRegularMeshAlgo algo_;
};


DECLARE_MAKER(PadRegularMesh)

PadRegularMesh::PadRegularMesh(GuiContext* ctx) :
  Module("PadRegularMesh", ctx, Source, "ChangeMesh", "SCIRun"),
  gui_pad_value_(get_ctx()->subVar("pad-value"),0.0),
  gui_pad_size_(get_ctx()->subVar("pad-size"),1)
{
  algo_.set_progress_reporter(this);
}

void
PadRegularMesh::execute()
{
  FieldHandle input, output;
  
  get_input_handle("Field",input,true);
  
  if (inputs_changed_ || gui_pad_value_.changed() ||
      gui_pad_size_.changed() || !oport_cached("Field"))
  {
    algo_.set_scalar("pad-value",gui_pad_value_.get());  
    algo_.set_int("pad-size",gui_pad_size_.get());  

    if(!(algo_.run(input,output))) return;
    
    send_output_handle("Field",output,true);    
  }
}


} // End namespace SCIRun


