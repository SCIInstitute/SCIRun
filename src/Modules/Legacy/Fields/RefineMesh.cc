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

#include <Core/Algorithms/Fields/RefineMesh/RefineMesh.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Field.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>


namespace SCIRun {

class RefineMesh : public Module
{
  public:
    RefineMesh(GuiContext* ctx);
    virtual ~RefineMesh() {}
    virtual void execute();

    //! Fix backwards compatibility
    virtual void post_read();

  private:
    GuiString gui_select_;
    GuiString gui_method_;
    GuiDouble gui_isoval_;
    
    SCIRunAlgo::RefineMeshAlgo algo_;
};


DECLARE_MAKER(RefineMesh)


RefineMesh::RefineMesh(GuiContext* ctx)
  : Module("RefineMesh", ctx, Filter, "NewField", "SCIRun"),
    gui_select_(get_ctx()->subVar("select"),"all"),
    gui_method_(get_ctx()->subVar("method"),"default"),
    gui_isoval_(get_ctx()->subVar("isoval"), 0.0)
{
  algo_.set_progress_reporter(this);
}

void
RefineMesh::execute()
{
  // Get input field.
  FieldHandle input;
  MatrixHandle isomat;

  get_input_handle("Mesh", input,true);
  get_input_handle("Isovalue", isomat, false);
  

  if (inputs_changed_ || gui_select_.changed() || gui_method_.changed() ||
      gui_isoval_.changed() || !oport_cached("RefinedMesh") || 
      !oport_cached("Mapping") )
  {
    update_state(Executing);
      
    FieldHandle output;
    MatrixHandle mapping;

    double isoval = gui_isoval_.get();
    if (isomat.get_rep())
    {
      isoval = 0.0;
      if (isomat->get_data_size() > 0) isoval = isomat->get(0,0);
    }

    algo_.set_option("select",gui_select_.get());
    algo_.set_scalar("isoval",isoval);
    if (gui_method_.get() == "convex") algo_.set_bool("hex_convex",true);

    if(!(algo_.run(input,output)))
    {
      return;
    }
    
    send_output_handle("RefinedMesh",output);
    send_output_handle("Mapping",mapping);
  }
}


void
RefineMesh::post_read()
{
  const std::string modName = get_ctx()->getfullname() + "-";
  std::string val;
  if( TCLInterface::get(modName+"lte", val, get_ctx()) )
  {
    if (val=="0") TCLInterface::set(modName+"select", "greaterthan", get_ctx());
    if (val=="1") TCLInterface::set(modName+"select", "lessthan", get_ctx());
  }
  
  get_ctx()->reset();
}

} // End namespace SCIRun

