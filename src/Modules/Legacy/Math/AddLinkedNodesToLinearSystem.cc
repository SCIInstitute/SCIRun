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


/// @todo Documentation Modules/Legacy/Math/AddLinkedNodesToLinearSystem.cc

#include <Modules/Legacy/Math/AddLinkedNodesToLinearSystem.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Algorithms/Legacy/FiniteElements/Mapping/BuildNodeLink.h>
#include <Core/Algorithms/Legacy/FiniteElements/Mapping/BuildFEGridMapping.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Math;

MODULE_INFO_DEF(AddLinkedNodesToLinearSystem, Math, SCIRun)

AddLinkedNodesToLinearSystem::AddLinkedNodesToLinearSystem() : Module(staticInfo_, false)
{
  INITIALIZE_PORT(LHS);
  INITIALIZE_PORT(RHS);
  INITIALIZE_PORT(LinkedNodes);
  INITIALIZE_PORT(OutputLHS);
  INITIALIZE_PORT(OutputRHS);
  INITIALIZE_PORT(Mapping);
}

#if 0

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
#endif

void
AddLinkedNodesToLinearSystem::execute()
{
  auto A = getRequiredInput(LHS);
  auto RHSoption = getOptionalInput(RHS);
  auto linkedNodes = getRequiredInput(LinkedNodes);

  if (needToExecute())
  {
    if (A->nrows() != A->ncols())
    {
      error("Stiffness matrix needs to be square");
      return;
    }

    DenseColumnMatrixHandle rhs;
    if (RHSoption && *RHSoption)
    {
      rhs = *RHSoption;
    }
    else
    {
      rhs.reset(new DenseColumnMatrix(A->nrows()));
      rhs->setZero();
    }

    if (rhs->nrows() != linkedNodes->nrows())
    {
      error("Linked nodes has the wrong number of rows");
      return;
    }

    MatrixHandle nodeLink;

    //TODO: double-algo module
    BuildNodeLinkAlgo bnl_algo;
    BuildFEGridMappingAlgo grid_algo;

    // Build Linking matrix
    if (!bnl_algo.run(linkedNodes, nodeLink))
    {
      error("BuildNodeLinkAlgo returned false");
      return;
    }

    SparseRowMatrixHandle potentialGeomToGrid, potentialGridToGeom;
    SparseRowMatrixHandle currentGeomToGrid, currentGridToGeom;

    grid_algo.set(Parameters::build_current_gridtogeom, false);
    grid_algo.set(Parameters::build_potential_geomtogrid, false);

    if (!grid_algo.run(nodeLink, potentialGeomToGrid, potentialGridToGeom, currentGeomToGrid, currentGridToGeom))
    {
      error("BuildFEGridMappingAlgo returned false");
      return;
    }

    // Remove the linked nodes from the system and make sure they are the same node

    sendOutput(OutputLHS, boost::make_shared<SparseRowMatrix>(*currentGeomToGrid * *A * *potentialGridToGeom));
    sendOutput(OutputRHS, boost::make_shared<DenseColumnMatrix>(*currentGeomToGrid * *rhs));
    sendOutput(Mapping, potentialGridToGeom);
  }
}
