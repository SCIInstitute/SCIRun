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
#include <iostream>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Scalar.h>
#include <Modules/BrainStimulator/SetupRHSforTDCSandTMS.h>
#include <Core/Algorithms/BrainStimulator/SetupRHSforTDCSandTMSAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Dataflow::Networks;

SetupRHSforTDCSandTMSModule::SetupRHSforTDCSandTMSModule() : Module(ModuleLookupInfo("SetupRHSforTDCSandTMS", "BrainStimulator", "SCIRun"))
{
 INITIALIZE_PORT(ELECTRODE_COIL_POSITIONS_AND_NORMAL);
 INITIALIZE_PORT(ELECTRODE_COUNT);
 INITIALIZE_PORT(RHS);
  
// INITIALIZE_PORT(ELECTRODE_TRIANGULATION);
// INITIALIZE_PORT(ELECTRODE_TRIANGULATION2);
// INITIALIZE_PORT(COIL);
// INITIALIZE_PORT(COIL2);
// INITIALIZE_PORT(ELECTRODES_FIELD);
// INITIALIZE_PORT(COILS_FIELD);
}

void SetupRHSforTDCSandTMSModule::setStateDefaults()
{
  /// @todo
  
  setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Test);
  
//  setStateDoubleFromAlgo(SetConductivitiesToTetMeshAlgorithm::Skull);
//  setStateDoubleFromAlgo(SetConductivitiesToTetMeshAlgorithm::CSF);
//  setStateDoubleFromAlgo(SetConductivitiesToTetMeshAlgorithm::GM);
//  setStateDoubleFromAlgo(SetConductivitiesToTetMeshAlgorithm::WM);
//  setStateDoubleFromAlgo(SetConductivitiesToTetMeshAlgorithm::Electrode);
}

void SetupRHSforTDCSandTMSModule::execute()
{
  auto elc_coil_pos_and_normal = getRequiredInput(ELECTRODE_COIL_POSITIONS_AND_NORMAL);
  auto elc_count = getRequiredInput(ELECTRODE_COUNT);

  setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Test);
  
//  setAlgoDoubleFromState(SetConductivitiesToTetMeshAlgorithm::Skull);
//  setAlgoDoubleFromState(SetConductivitiesToTetMeshAlgorithm::CSF);
//  setAlgoDoubleFromState(SetConductivitiesToTetMeshAlgorithm::GM);
//  setAlgoDoubleFromState(SetConductivitiesToTetMeshAlgorithm::WM);
//  setAlgoDoubleFromState(SetConductivitiesToTetMeshAlgorithm::Electrode);

  
  //  auto elc_tri_mesh = getRequiredInput(ELECTRODE_TRIANGULATION);
  
  // UI input
  //auto param = get_state()->getValue(Variables::AppendMatrixOption).getInt();

  //algorithm parameter
  //algo_->set(Variables::AppendMatrixOption, param);
 
  
  if (needToExecute())
  {
    //algorithm input and run
    auto output = algo().run_generic(make_input((ELECTRODE_COIL_POSITIONS_AND_NORMAL, elc_coil_pos_and_normal)(ELECTRODE_COUNT, elc_count)));
    
    //algorithm output
//    sendOutputFromAlgorithm(ELECTRODES_FIELD, output);
//    sendOutputFromAlgorithm(COILS_FIELD, output);
    sendOutputFromAlgorithm(RHS, output);
  }
}
