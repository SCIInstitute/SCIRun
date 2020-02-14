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


/// @todo Documentation Modules/Legacy/Fields/GetFieldDomainStructure.cc

#include <Core/Algorithms/Fields/DomainFields/GetDomainStructure.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

using namespace SCIRun;

class GetDomainStructure : public Module {
  public:
    GetDomainStructure(GuiContext*);
    virtual ~GetDomainStructure() {}

    virtual void execute();

  private:
    SCIRunAlgo::GetDomainStructureAlgo algo_;
};


DECLARE_MAKER(GetDomainStructure)

GetDomainStructure::GetDomainStructure(GuiContext* ctx) :
  Module("GetDomainStructure", ctx, Source, "NewField", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

void
GetDomainStructure::execute()
{
  FieldHandle domainfield, separationsurfaces, separationedges, separationpoints;

  get_input_handle("DomainField",domainfield,true);
  if (inputs_changed_ || !oport_cached("SeparationSurfaces") ||
      !oport_cached("SeparationEdges") || !oport_cached("SeparationPoints"))
  {
    update_state(Executing);

    if(!(algo_.run(domainfield,separationsurfaces, separationedges, separationpoints))) return;
    send_output_handle("SeparationSurfaces",separationsurfaces);
    send_output_handle("SeparationEdges",separationedges);
    send_output_handle("SeparationPoints",separationpoints);
  }
}

} // End namespace SCIRun
