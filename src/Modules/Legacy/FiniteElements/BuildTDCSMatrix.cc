/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Modules/Legacy/FiniteElements/BuildTDCSMatrix.h>
//#include <Core/Algorithms/FiniteElements/BuildMatrix/BuildTDCSMatrix.h>

using namespace SCIRun::Modules::FiniteElements;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;



BuildTDCSMatrix::BuildTDCSMatrix()
  : Module(ModuleLookupInfo("BuildTDCSMatrix","FiniteElements", "SCIRun"), false)
{
 INITIALIZE_PORT(FEM_Stiffness_Matrix);
 INITIALIZE_PORT(FEM_Mesh);
 INITIALIZE_PORT(Eletrode_Element);
 INITIALIZE_PORT(Electrode_Element_Type);
 INITIALIZE_PORT(Electrode_Element_Definition);
 INITIALIZE_PORT(Contact_Impedance);
 INITIALIZE_PORT(TDCSMatrix);
}


void BuildTDCSMatrix::execute()
{
  FieldHandle Mesh;
  SparseRowMatrixHandle Stiffness;
  DenseMatrixHandle ElectrodeElements;
  DenseMatrixHandle ElectrodeElementType;
  DenseMatrixHandle ElectrodeElementDefinition;
  DenseMatrixHandle ContactImpedance; 

  Stiffness=getRequiredInput(FEM_Stiffness_Matrix);
  Mesh=getRequiredInput(FEM_Mesh);
  ElectrodeElements=getRequiredInput(Eletrode_Element);
  ElectrodeElementType=getRequiredInput(Electrode_Element_Type);
  ElectrodeElementDefinition=getRequiredInput(Electrode_Element_Definition);
  ContactImpedance=getRequiredInput(Contact_Impedance);
//  if (!(get_input_handle("FEM Stiffness",Stiffness,true))) return;
//  if (!(get_input_handle("Mesh",Mesh,true))) return;
//  if (!(get_input_handle("Electrode Element",ElectrodeElements,true))) return; 
//  if (!(get_input_handle("Electrode Element Type",ElectrodeElementType,true))) return; 
//  if (!(get_input_handle("Electrode Element Definition",ElectrodeElementDefinition,true))) return; 
//  if (!(get_input_handle("Contact Impedance",ContactImpedance,true))) return;   
 
  auto output = algorithm().run_generic(make_input((FEM_Stiffness_Matrix,Stiffness)(FEM_Mesh,Mesh)(Eletrode_Element,ElectrodeElements)(Electrode_Element_Type,ElectrodeElementType)(Electrode_Element_Definition,ElectrodeElementDefinition)(Contact_Impedance,ContactImpedance)));
//  algo_.run(Stiffness,Mesh,ElectrodeElements,ElectrodeElementType,ElectrodeElementDefinition,ContactImpedance,TDCSMatrix);  

//  send_output_handle("TDCS Matrix", TDCSMatrix);
  sendOutputFromAlgorithm(TDCSMatrix,output);
}




