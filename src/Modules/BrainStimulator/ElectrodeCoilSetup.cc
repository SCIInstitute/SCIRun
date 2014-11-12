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
#include <Modules/BrainStimulator/ElectrodeCoilSetup.h>
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>

//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Dataflow::Networks;

ElectrodeCoilSetupModule::ElectrodeCoilSetupModule() : Module(ModuleLookupInfo("ElectrodeCoilSetup", "BrainStimulator", "SCIRun"))
{
 INITIALIZE_PORT(SCALP_SURF);
 INITIALIZE_PORT(LOCATIONS);
 INITIALIZE_PORT(ELECTRODECOILPROTOTYPES);
 INITIALIZE_PORT(ELECTRODE_SPONGE_LOCATION_AVR);
 INITIALIZE_PORT(COILS_FIELD);
}

void ElectrodeCoilSetupModule::setStateDefaults()
{
  /// @todo
}

void ElectrodeCoilSetupModule::execute()
{
  auto scalp = getRequiredInput(SCALP_SURF);
  auto elc_coil_proto = getRequiredDynamicInputs(ELECTRODECOILPROTOTYPES);
  //auto elc_tri_mesh = getRequiredInput(ELECTRODE_TRIANGULATION);
   // UI input
  //auto param = get_state()->getValue(Variables::AppendMatrixOption).toInt();

  //algorithm parameter
  //algo_->set(Variables::AppendMatrixOption, param);
 
  
  //algorithm input and run
  auto output = algo().run_generic(withInputData((SCALP_SURF, scalp)(ELECTRODECOILPROTOTYPES, elc_coil_proto)));

  //algorithm output
  sendOutputFromAlgorithm(ELECTRODE_SPONGE_LOCATION_AVR, output);
  sendOutputFromAlgorithm(COILS_FIELD, output);
}
