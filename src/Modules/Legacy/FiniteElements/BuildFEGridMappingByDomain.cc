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


/// @todo Documentation Modules/Legacy/FiniteElements/BuildFEGridMappingByDomain.cc

#include <Core/Algorithms/FiniteElements/Mapping/BuildFEGridMapping.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>

#include <Dataflow/Network/Module.h>

namespace SCIRun {

class BuildFEGridMappingByDomain : public Module {
  public:
    BuildFEGridMappingByDomain(GuiContext*);
    virtual void execute();

  private:
    SCIRunAlgo::BuildFEGridMappingAlgo algo_;
};


DECLARE_MAKER(BuildFEGridMappingByDomain)
BuildFEGridMappingByDomain::BuildFEGridMappingByDomain(GuiContext* ctx)
  : Module("BuildFEGridMappingByDomain", ctx, Source, "FiniteElements", "SCIRun")
{
  algo_.set_progress_reporter(this);
}


void BuildFEGridMappingByDomain::execute()
{
  FieldHandle  DomainField;
  MatrixHandle NodeLink;
  MatrixHandle PotentialGeomToGrid, PotentialGridToGeom;
  MatrixHandle CurrentGeomToGrid, CurrentGridToGeom;

  get_input_handle("DomainField",DomainField,true);
  get_input_handle("NodeLink",NodeLink,true);

  bool need_potential_gridtocomp = oport_connected("PotentialGeomToGrid");
  bool need_potential_comptogrid = oport_connected("PotentialGridToGeom");
  bool need_current_gridtocomp = oport_connected("CurrentGeomToGrid");
  bool need_current_comptogrid = oport_connected("CurrentGridToGeom");

  if (inputs_changed_ ||
      (!oport_cached("PotentialGeomToGrid") && need_potential_gridtocomp) ||
      (!oport_cached("PotentialGridToGeom") && need_potential_comptogrid) ||
      (!oport_cached("CurrentGeomToGrid") && need_current_gridtocomp) ||
      (!oport_cached("CurrentGridToGeom") && need_current_comptogrid))
  {
    algo_.set_bool("build_potential_gridtocomp",need_potential_gridtocomp);
    algo_.set_bool("build_potential_comptogrid",need_potential_comptogrid);
    algo_.set_bool("build_current_gridtocomp",need_current_gridtocomp);
    algo_.set_bool("build_current_comptogrid",need_current_comptogrid);

    if(!(algo_.run(DomainField,NodeLink,PotentialGeomToGrid,PotentialGridToGeom,
                       CurrentGeomToGrid,CurrentGridToGeom))) return;

    send_output_handle("PotentialGeomToGrid",PotentialGeomToGrid,false);
    send_output_handle("PotentialGridToGeom",PotentialGridToGeom,false);
    send_output_handle("CurrentGeomToGrid",CurrentGeomToGrid,false);
    send_output_handle("CurrentGridToGeom",CurrentGridToGeom,false);
  }
}

} // End namespace ModelCreation
