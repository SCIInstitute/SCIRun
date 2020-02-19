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


/// @todo Documentation Modules/Legacy/FiniteElements/BuildTDCSMatrix.cc

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Modules/Legacy/FiniteElements/BuildTDCSMatrix.h>
#include <Core/Algorithms/Legacy/FiniteElements/BuildMatrix/BuildTDCSMatrix.h>

using namespace SCIRun::Modules::FiniteElements;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

MODULE_INFO_DEF(BuildTDCSMatrix, FiniteElements, SCIRun)

BuildTDCSMatrix::BuildTDCSMatrix()
  : Module(staticInfo_, HasUI<BuildTDCSMatrix>::value)
{
 INITIALIZE_PORT(FEM_Stiffness_Matrix);
 INITIALIZE_PORT(FEM_Mesh);
 INITIALIZE_PORT(Electrode_Element);
 INITIALIZE_PORT(Electrode_Element_Type);
 INITIALIZE_PORT(Electrode_Element_Definition);
 INITIALIZE_PORT(Contact_Impedance);
 INITIALIZE_PORT(TDCSMatrix);
}

void BuildTDCSMatrix::execute()
{
  auto Stiffness = getRequiredInput(FEM_Stiffness_Matrix);
  auto Mesh = getRequiredInput(FEM_Mesh);
  auto ElectrodeElements = getRequiredInput(Electrode_Element);
  auto ElectrodeElementType = getRequiredInput(Electrode_Element_Type);
  auto ElectrodeElementDefinition = getRequiredInput(Electrode_Element_Definition);
  auto ContactImpedance = getRequiredInput(Contact_Impedance);

  if (needToExecute())
  {
    auto output = algo().run(withInputData((FEM_Stiffness_Matrix,Stiffness)(FEM_Mesh,Mesh)(Electrode_Element,ElectrodeElements)(Electrode_Element_Type,ElectrodeElementType)(Electrode_Element_Definition,ElectrodeElementDefinition)(Contact_Impedance,ContactImpedance)));

    sendOutputFromAlgorithm(TDCSMatrix,output);
  }
}
