/*
 * The MIT License
 * 
 * Copyright (c) 2015 Scientific Computing and Imaging Institute,
 * University of Utah.
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/// @todo Documentation Modules/Legacy/Math/AddLinkedNodesToLinearSystem.cc

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Algorithms/FiniteElements/Mapping/BuildNodeLink.h>
#include <Core/Algorithms/FiniteElements/Mapping/BuildFEGridMapping.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>


namespace SCIRun {

using namespace SCIRun;

class AddLinkedNodesToLinearSystem : public Module {
public:
  AddLinkedNodesToLinearSystem(GuiContext*);

  virtual ~AddLinkedNodesToLinearSystem() {}

  virtual void execute();
  
private:
  SCIRunAlgo::BuildNodeLinkAlgo bnl_algo_;  
  SCIRunAlgo::BuildFEGridMappingAlgo grid_algo_;
};


DECLARE_MAKER(AddLinkedNodesToLinearSystem)

AddLinkedNodesToLinearSystem::AddLinkedNodesToLinearSystem(GuiContext* ctx) :
  Module("AddLinkedNodesToLinearSystem", ctx, Source, "Math", "SCIRun")
{
  bnl_algo_.set_progress_reporter(this);
  grid_algo_.set_progress_reporter(this);
}

void
AddLinkedNodesToLinearSystem::execute()
{
  MatrixHandle A, RHS, LinkedNodes, Mapping;
  
  get_input_handle("Matrix",A,true);
  get_input_handle("RHS",RHS,false);
  get_input_handle("LinkedNodes",LinkedNodes,true);
  
  if (inputs_changed_ || !oport_cached("Matrix") || !oport_cached("RHS") ||
      !oport_cached("Mapping"))
  {
    MatrixHandle NodeLink;
    
    if (A->nrows() != A->ncols())
    {
      error("Stiffness matrix needs to be square");
      return;
    }
    
    if (RHS.get_rep() == 0)
    {
      RHS = new ColumnMatrix(A->nrows());
      RHS->zero();
    }
    
    if (RHS->nrows() != LinkedNodes->nrows())
    {
      error("Linked nodes has the wrong number of rows");
      return;
    }
    
    // Build Linking matrix
    if(!(bnl_algo_.run(LinkedNodes,NodeLink))) return;
    
    MatrixHandle PotentialGeomToGrid, PotentialGridToGeom;
    MatrixHandle CurrentGeomToGrid, CurrentGridToGeom;
    
    grid_algo_.set_bool("build_current_gridtogeom",false);
    grid_algo_.set_bool("build_potential_geomtogrid",false);
    
    if(!(grid_algo_.run(NodeLink,PotentialGeomToGrid,PotentialGridToGeom,
                        CurrentGeomToGrid,CurrentGridToGeom))) return;
                        
    // Remove the linked nodes from the system and make sure they are the
    // same node

    A = CurrentGeomToGrid*A*PotentialGridToGeom;
    RHS = CurrentGeomToGrid*RHS;
    Mapping = PotentialGridToGeom;
  
    send_output_handle("Matrix",A);
    send_output_handle("RHS",RHS);
    send_output_handle("Mapping",Mapping);
  }
  

}

} // End namespace SCIRun


