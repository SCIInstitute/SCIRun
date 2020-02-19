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


#include <Core/Algorithms/Fields/MeshDerivatives/CalculateMeshConnector.h>

#include <Core/Datatypes/Field.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

using namespace SCIRun;

/// @class CalculateMeshConnector
/// @brief This module computes a connection between two meshes based on the
/// minimal spatial distance between their nodes.

class CalculateMeshConnector : public Module {
  public:
    CalculateMeshConnector(GuiContext*);
    virtual ~CalculateMeshConnector() {}

    virtual void execute();

  private:
    SCIRunAlgo::CalculateMeshConnectorAlgo algo_;
};


DECLARE_MAKER(CalculateMeshConnector)

CalculateMeshConnector::CalculateMeshConnector(GuiContext* ctx) :
  Module("CalculateMeshConnector", ctx, Source, "MiscField", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

void
CalculateMeshConnector::execute()
{
  FieldHandle input1, input2, output;

  get_input_handle("Mesh1",input1,true);
  get_input_handle("Mesh2",input2,true);

  if (inputs_changed_ || !oport_cached("Output"))
  {
    if (!(algo_.run(input1,input2,output))) return;
    send_output_handle("Output",output,true);
  }
}


} // End namespace SCIRun
