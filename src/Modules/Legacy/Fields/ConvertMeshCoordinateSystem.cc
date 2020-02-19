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


///
///@author
///   David Weinstein
///   Department of Computer Science
///   University of Utah
///@date  November 1994
///

#include <Core/Datatypes/Field.h>
#include <Core/Algorithms/Fields/TransformMesh/ConvertMeshCoordinateSystem.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

///@class ConvertMeshCoordinateSystem
///@brief Take in fields and add all of their points into one field

class ConvertMeshCoordinateSystem : public Module {
  public:
    ConvertMeshCoordinateSystem(GuiContext* ctx);
    virtual ~ConvertMeshCoordinateSystem() {}
    virtual void execute();

  private:
    SCIRunAlgo::ConvertMeshCoordinateSystemAlgo algo_;
    GuiString gui_oldsystem_;
    GuiString gui_newsystem_;
};

DECLARE_MAKER(ConvertMeshCoordinateSystem)
ConvertMeshCoordinateSystem::ConvertMeshCoordinateSystem(GuiContext* ctx)
  : Module("ConvertMeshCoordinateSystem", ctx, Filter, "ChangeMesh", "SCIRun"),
    gui_oldsystem_(get_ctx()->subVar("oldsystem"), "Cartesian"),
    gui_newsystem_(get_ctx()->subVar("newsystem"), "Spherical")
{
}

void
ConvertMeshCoordinateSystem::execute()
{
  // The input port (with data) is required.
  FieldHandle input, output;
  get_input_handle("Input Field", input, true);

  if (inputs_changed_ || gui_oldsystem_.changed() ||
      gui_newsystem_.changed() || !oport_cached("Output Field"))
  {
    update_state(Executing);

    algo_.set_option("input_system",gui_oldsystem_.get());
    algo_.set_option("output_system",gui_newsystem_.get());
    algo_.run(input,output);
    send_output_handle("Output Field", output);
  }
}

} // End namespace SCIRun
