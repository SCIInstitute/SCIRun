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


//// @file BuildFEVolRHS.h
/// @brief This module computes a volumetric right-hand-side. This module is needed for TMS simulations in the BrainStimulator package.
///
/// @author
///  ported by Moritz Dannhauer (09/24/2014) from SCIRun4
///
/// @details
///  Calculates the divergence of a vector field over the volume. It is designed to calculate the volume integral of the vector field
///  (gradient of the potential in electrical simulations). Builds the volume portion of the RHS of FE calculations where the RHS of
///  the function is GRAD dot F.
///  Input: A FE mesh with field vectors distributed on the elements (constant basis). Output: The Grad dot F

#include <Modules/Legacy/FiniteElements/BuildFEVolRHS.h>
#include <Core/Algorithms/Legacy/FiniteElements/BuildRHS/BuildFEVolRHS.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun::Modules::FiniteElements;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun;

BuildFEVolRHS::BuildFEVolRHS()
  : Module(ModuleLookupInfo("BuildFEVolRHS", "FiniteElements", "SCIRun"),false)
{
  INITIALIZE_PORT(Mesh);
  INITIALIZE_PORT(RHS);
}

void BuildFEVolRHS::setStateDefaults()
{

}

void BuildFEVolRHS::execute()
{
  auto mesh = getRequiredInput(Mesh);
 #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  auto vtable = getRequiredInput(Vector_Table);
 #endif
  if (needToExecute())
  {
    auto output = algo().run(make_input((Mesh, mesh)));
    sendOutputFromAlgorithm(RHS, output);
  }


#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
  FieldHandle Field;
  MatrixHandle VectorTable;
  MatrixHandle RHSMatrix;

  if (!(get_input_handle("Mesh",Field,true))) return;
  get_input_handle("Vector Table", VectorTable, false);

  if (inputs_changed_ || gui_use_basis_.changed() || !oport_cached("RHS") )
  {
    algo_.set_bool("generate_basis",gui_use_basis_.get());
    if(!(algo_.run(Field,VectorTable,RHSMatrix))) return;

    send_output_handle("RHS", RHSMatrix);
  }
#endif

}
