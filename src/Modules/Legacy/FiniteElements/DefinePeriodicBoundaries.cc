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


/// @todo Documentation Modules/Legacy/FiniteElements/DefinePeriodicBoundaries.cc

#include <Core/Algorithms/FiniteElements/Periodic/DefinePeriodicBoundaries.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>

#include <Dataflow/Network/Module.h>

namespace SCIRun {

class DefinePeriodicBoundaries : public Module {
public:
  DefinePeriodicBoundaries(GuiContext*);
  virtual void execute();

private:
  GuiInt gui_linkx_;
  GuiInt gui_linky_;
  GuiInt gui_linkz_;
  GuiDouble gui_tolerance_;

  SCIRunAlgo::DefinePeriodicBoundariesAlgo algo_;
};


DECLARE_MAKER(DefinePeriodicBoundaries)
DefinePeriodicBoundaries::DefinePeriodicBoundaries(GuiContext* ctx)
  : Module("DefinePeriodicBoundaries", ctx, Source, "FiniteElements", "SCIRun"),
  gui_linkx_(get_ctx()->subVar("linkx"),true),
  gui_linky_(get_ctx()->subVar("linky"),true),
  gui_linkz_(get_ctx()->subVar("linkz"),true),
  gui_tolerance_(get_ctx()->subVar("tol"),-1.0)
{
  algo_.set_progress_reporter(this);
}


void
DefinePeriodicBoundaries::execute()
{
  FieldHandle input;
  MatrixHandle periodic_nodelink, periodic_delemlink;

  if(!(get_input_handle("Field",input,true))) return;

  bool need_nodelink = oport_connected("PeriodicNodeLink");
  bool need_delemlink = oport_connected("PeriodicDElemLink");

  if (inputs_changed_ ||  gui_linkx_.changed() || gui_linky_.changed() ||
      gui_linkz_.changed() || gui_tolerance_.changed() ||
      (!oport_cached("PeriodicNodeLink") && need_nodelink ) ||
      (!oport_cached("PeriodicDElemLink") && need_delemlink))
  {

    algo_.set_bool("link_x_boundary",gui_linkx_.get());
    algo_.set_bool("link_y_boundary",gui_linky_.get());
    algo_.set_bool("link_z_boundary",gui_linkz_.get());
    algo_.set_scalar("tolerance",gui_tolerance_.get());
    algo_.set_bool("build_periodic_nodelink",need_nodelink);
    algo_.set_bool("build_periodic_delemlink",need_delemlink);

    if(!(algo_.run(input,periodic_nodelink,periodic_delemlink))) return;

    send_output_handle("PeriodicNodeLink", periodic_nodelink);
    send_output_handle("PeriodicDElemLink", periodic_delemlink);
  }
}

} // End namespace SCIRun
