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


/// @todo Documentation Modules/Legacy/Fields/GenerateMedialAxisPoints.cc

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Ports/FieldPort.h>

#include <Core/Algorithms/Fields/MedialAxis/MedialAxis2.h>

namespace SCIRun {

class GenerateMedialAxisPoints : public Module
{
  public:
    GenerateMedialAxisPoints(GuiContext*);
    virtual ~GenerateMedialAxisPoints() {}
    virtual void execute();

  private:
    GuiInt         gui_refinement_levels_;
    GuiDouble      gui_axis_min_angle_;
    GuiDouble      gui_normal_min_angle_;
    GuiDouble      gui_max_distance_;
	GuiDouble	   gui_forward_multiplier_;
	GuiDouble	   gui_back_multiplier_;


    SCIRunAlgo::MedialAxis2Algo algo_;
};


DECLARE_MAKER(GenerateMedialAxisPoints)
GenerateMedialAxisPoints::GenerateMedialAxisPoints(GuiContext* ctx)
  : Module("GenerateMedialAxisPoints", ctx, Source, "NewField", "SCIRun"),
  gui_refinement_levels_(get_ctx()->subVar("refinement-levels"), 1),
  gui_axis_min_angle_(get_ctx()->subVar("axis-min-angle"), 35.0),
  gui_normal_min_angle_(get_ctx()->subVar("normal-min-angle"), 35.0),
  gui_max_distance_(get_ctx()->subVar("max-distance"),-1.0),
  gui_forward_multiplier_(get_ctx()->subVar("forward-multiplier"),2.0),
  gui_back_multiplier_(get_ctx()->subVar("backward-multiplier"),4.0)
{
  algo_.set_progress_reporter(this);
}


void
GenerateMedialAxisPoints::execute()
{
  // define data handles:
  FieldHandle surfH, medialPtsH;

  // get input from ports:
  get_input_handle("Surface", surfH, true);

  // only compute output if inputs changed:
  if (inputs_changed_ || !oport_cached("MedialAxisPoints") ||
      gui_refinement_levels_.changed() ||
      gui_axis_min_angle_.changed() ||
      gui_normal_min_angle_.changed() ||
      gui_max_distance_.changed() ||
	  gui_forward_multiplier_.changed() ||
	  gui_back_multiplier_.changed())
  {
    // Inform module that execution started
    update_state(Executing);

    // Set parameters
    algo_.set_int("refinement_levels", gui_refinement_levels_.get());
    algo_.set_scalar("axis_minimum_angle", gui_axis_min_angle_.get());
    algo_.set_scalar("normal_minimum_angle", gui_normal_min_angle_.get());
    algo_.set_scalar("maximum_distance_difference", gui_max_distance_.get());
	algo_.set_scalar("forward_projection_multiplier", gui_forward_multiplier_.get());
	algo_.set_scalar("backward_projection_multiplier", gui_back_multiplier_.get());

    if(!(algo_.run(surfH, medialPtsH))) return;

    // Send data downstream
    send_output_handle("MedialAxisPoints", medialPtsH);
  }
}

} // End namespace SCIRun
