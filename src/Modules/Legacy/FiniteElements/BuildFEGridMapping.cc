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


/// @todo Documentation Modules/Legacy/FiniteElements/BuildFEGridMapping.cc

#include <Core/Algorithms/FiniteElements/Mapping/BuildFEGridMapping.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>

#include <Dataflow/Network/Module.h>

namespace SCIRun {

class BuildFEGridMapping : public Module {
  public:
    BuildFEGridMapping(GuiContext*);
    virtual void execute();

  private:
    SCIRunAlgo::BuildFEGridMappingAlgo algo_;
};


DECLARE_MAKER(BuildFEGridMapping)
BuildFEGridMapping::BuildFEGridMapping(GuiContext* ctx)
  : Module("BuildFEGridMapping", ctx, Source, "FiniteElements", "SCIRun")
{
  algo_.set_progress_reporter(this);
}


void BuildFEGridMapping::execute()
{
  MatrixHandle NodeLink;
  MatrixHandle PotentialGeomToGrid, PotentialGridToGeom;
  MatrixHandle CurrentGeomToGrid, CurrentGridToGeom;

  get_input_handle("NodeLink",NodeLink,true);

  bool need_potential_geomtogrid = oport_connected("PotentialGeomToGrid");
  bool need_potential_gridtogeom = oport_connected("PotentialGridToGeom");
  bool need_current_geomtogrid = oport_connected("CurrentGeomToGrid");
  bool need_current_gridtogeom = oport_connected("CurrentGridToGeom");

  if (inputs_changed_ ||
      (!oport_cached("PotentialGeomToGrid") && need_potential_geomtogrid) ||
      (!oport_cached("PotentialGridToGeom") && need_potential_gridtogeom) ||
      (!oport_cached("CurrentGeomToGrid") && need_current_geomtogrid) ||
      (!oport_cached("CurrentGridToGeom") && need_current_gridtogeom))
  {
    algo_.set_bool("build_potential_geomtogrid",need_potential_geomtogrid);
    algo_.set_bool("build_potential_gridtogeom",need_potential_gridtogeom);
    algo_.set_bool("build_current_geomtogrid",need_current_geomtogrid);
    algo_.set_bool("build_current_gridtogeom",need_current_gridtogeom);

    if(!(algo_.run(NodeLink,PotentialGeomToGrid,PotentialGridToGeom,
                       CurrentGeomToGrid,CurrentGridToGeom))) return;

    send_output_handle("PotentialGeomToGrid",PotentialGeomToGrid,false);
    send_output_handle("PotentialGridToGeom",PotentialGridToGeom,false);
    send_output_handle("CurrentGeomToGrid",CurrentGeomToGrid,false);
    send_output_handle("CurrentGridToGeom",CurrentGridToGeom,false);
  }
}

} // End namespace ModelCreation
