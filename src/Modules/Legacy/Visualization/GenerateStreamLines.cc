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

//    File   : GenerateStreamLines.cc
//    Author : Allen R. Sanderson
//    Date   : July 2006

#include <Modules/Legacy/Visualization/GenerateStreamLines.h>
//#include <Core/Algorithms/Fields/StreamLines/GenerateStreamLines.h>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun;

/*
class GenerateStreamLines : public Module {
public:
  GenerateStreamLines(GuiContext* ctx);
  virtual ~GenerateStreamLines() {}

  virtual void execute();

private:
  GuiDouble                     gui_step_size_;
  GuiDouble                     gui_tolerance_;
  GuiInt                        gui_max_steps_;
  GuiInt                        gui_direction_;
  GuiInt                        gui_value_;
  GuiInt                        gui_remove_colinear_pts_;
  GuiInt                        gui_method_;
  GuiInt                        gui_auto_parameterize_;
  
  SCIRunAlgo::GenerateStreamLinesAlgo algo_;
};
*/

const ModuleLookupInfo GenerateStreamLines::staticInfo_("GenerateStreamLines", "Visualization", "SCIRun");

GenerateStreamLines::GenerateStreamLines() : Module(staticInfo_)
/*
gui_step_size_(get_ctx()->subVar("stepsize"), 0.01),
  gui_tolerance_(get_ctx()->subVar("tolerance"), 0.0001),
  gui_max_steps_(get_ctx()->subVar("maxsteps"), 2000),
  gui_direction_(get_ctx()->subVar("direction"), 1),
  gui_value_(get_ctx()->subVar("value"), 1),
  gui_remove_colinear_pts_(get_ctx()->subVar("remove-colinear-points"), 1),
  gui_method_(get_ctx()->subVar("method"), 4),
  gui_auto_parameterize_(get_ctx()->subVar("auto-parameterize"), false)
  */
{
  INITIALIZE_PORT(Vector_Field);
  INITIALIZE_PORT(Seed_Points);
  INITIALIZE_PORT(Streamlines);
}

void GenerateStreamLines::setStateDefaults()
{
  //TODO
}

void GenerateStreamLines::execute()
{
  /*
  FieldHandle input, seeds, output;

  get_input_handle( "Vector Field", input, true );
  get_input_handle( "Seed Points", seeds, true );
  
  
  if( inputs_changed_                          ||
      !oport_cached("Streamlines")             ||
      gui_tolerance_.changed( true )           ||
      gui_step_size_.changed( true )           ||
      gui_max_steps_.changed( true )           ||
      gui_direction_.changed( true )           ||
      gui_value_.changed( true )               ||
      gui_remove_colinear_pts_.changed( true ) ||
      gui_method_.changed( true )              ||
      gui_auto_parameterize_.changed( true ))
  {
    // Inform module that execution started
    update_state(Executing);

    algo_.set_scalar("tolerance",gui_tolerance_.get());
    algo_.set_scalar("step_size",gui_step_size_.get());
    algo_.set_int("max_steps",gui_max_steps_.get());
    algo_.set_int("direction",gui_direction_.get());
    algo_.set_int("value",gui_value_.get());
    algo_.set_bool("remove_colinear_points",gui_remove_colinear_pts_.get());
    int method = gui_method_.get();
    if (method == 0) algo_.set_option("method","AdamsBashforth");
    else if (method == 2) algo_.set_option("method","Heun");
    else if (method == 3) algo_.set_option("method","RungeKutta");
    else if (method == 4) algo_.set_option("method","RungeKuttaFehlberg");
    else if (method == 5) algo_.set_option("method","CellWalk");
    algo_.set_bool("auto_parameters",gui_auto_parameterize_.get());
    
    if(!(algo_.run(input,seeds,output))) return;
    gui_tolerance_.set(algo_.get_scalar("tolerance"));
    gui_step_size_.set(algo_.get_scalar("step_size"));
    get_ctx()->reset();

    send_output_handle( "Streamlines", output, true );
  }
  */
}
