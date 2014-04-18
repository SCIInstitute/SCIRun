/*
 *  For more information, please see: http://software.sci.utah.edu
 *  
 *  The MIT License
 *  
 *  Copyright (c) 2009 Scientific Computing and Imaging Institute,
 *  University of Utah.
 *  
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

/// @todo Documentation Modules/Legacy/Fields/BuildMatrixOfSurfaceNormals.cc

#include <Core/Algorithms/Fields/MeshData/GetSurfaceNodeNormals.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>


namespace SCIRun {

class BuildMatrixOfSurfaceNormals : public Module {
  public:
    BuildMatrixOfSurfaceNormals(GuiContext* ctx);
    virtual ~BuildMatrixOfSurfaceNormals() {}
    virtual void execute();

  private:
    SCIRunAlgo::GetSurfaceNodeNormalsAlgo algo_;
};

DECLARE_MAKER(BuildMatrixOfSurfaceNormals)

BuildMatrixOfSurfaceNormals::BuildMatrixOfSurfaceNormals(GuiContext* ctx) : 
  Module("BuildMatrixOfSurfaceNormals", ctx, Filter, "MiscField", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

void 
BuildMatrixOfSurfaceNormals::execute()
{
  FieldHandle input; MatrixHandle output;
  
  get_input_handle("Surface Field", input);

  if (inputs_changed_ || !oport_cached("Nodal Surface Normals"))
  {
    // Inform module that execution started
    update_state(Executing);
    
    // Core algorithm of the module
    if(!(algo_.run(input,output))) return;

    // Send output to output ports
    send_output_handle("Nodal Surface Normals", output, true);
  }
}

} // End namespace SCIRun


