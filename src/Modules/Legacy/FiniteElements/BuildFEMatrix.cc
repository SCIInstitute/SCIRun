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


/// @todo Documentation Modules/Legacy/FiniteElements/BuildFEMatrix.cc

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Modules/Legacy/FiniteElements/BuildFEMatrix.h>

using namespace SCIRun::Modules::FiniteElements;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun;

BuildFEMatrix::BuildFEMatrix()
  : Module(ModuleLookupInfo("BuildFEMatrix", "FiniteElements", "SCIRun"), false)
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    gui_use_basis_(get_ctx()->subVar("use-basis"), 0),
    gui_force_symmetry_(get_ctx()->subVar("force-symmetry"), 0),
#endif
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(Conductivity_Table);
  INITIALIZE_PORT(Stiffness_Matrix);
  INITIALIZE_PORT(Stiffness_Matrix_Complex);
}

void BuildFEMatrix::execute()
{
  auto field = getRequiredInput(InputField);
  auto conductivity = getOptionalInput(Conductivity_Table);

  if (needToExecute())
  {
#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
    algo().set("generate_basis", gui_use_basis_.get());
    algo().set("force_symmetry", gui_force_symmetry_.get());
//#else
//    algo().set(GenerateBasis, true);
//    algo().set(ForceSymmetry, true);
#endif

    auto output = algo().run(withInputData((InputField, field)(Conductivity_Table, optionalAlgoInput(conductivity))));

    sendOutputFromAlgorithm(Stiffness_Matrix, output);
    sendOutputFromAlgorithm(Stiffness_Matrix_Complex, output);
  }
}
