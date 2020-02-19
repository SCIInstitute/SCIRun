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


#include <Modules/BrainStimulator/ElectrodeCoilSetup.h>
#include <iostream>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Scalar.h>
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <vector>

using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Dataflow::Networks;

ElectrodeCoilSetup::ElectrodeCoilSetup() : Module(ModuleLookupInfo("ElectrodeCoilSetup", "BrainStimulator", "SCIRun"))
{
 INITIALIZE_PORT(SCALP_SURF);
 INITIALIZE_PORT(LOCATIONS);
 INITIALIZE_PORT(ELECTRODECOILPROTOTYPES);
 INITIALIZE_PORT(ELECTRODE_SPONGE_LOCATION_AVR);
 INITIALIZE_PORT(COILS_FIELD);
 INITIALIZE_PORT(MOVED_ELECTRODES_FIELD);
 INITIALIZE_PORT(FINAL_ELECTRODES_FIELD);
}

void ElectrodeCoilSetup::setStateDefaults()
{
  setStateIntFromAlgo(Parameters::NumberOfPrototypes);
  setStateBoolFromAlgo(Parameters::ProtoTypeInputCheckbox);
  setStateBoolFromAlgo(Parameters::InvertNormalsCheckBox);
  setStateBoolFromAlgo(Parameters::AllInputsTDCS);
  setStateIntFromAlgo(Parameters::ProtoTypeInputComboBox);
  setStateListFromAlgo(Parameters::TableValues);
  setStateDoubleFromAlgo(Parameters::ElectrodethicknessSpinBox);
  setStateBoolFromAlgo(Parameters::ElectrodethicknessCheckBox);
  setStateBoolFromAlgo(Parameters::OrientTMSCoilRadialToScalpCheckBox);
  setStateBoolFromAlgo(Parameters::PutElectrodesOnScalpCheckBox);
  setStateBoolFromAlgo(Parameters::InterpolateElectrodeShapeCheckbox);
}

void ElectrodeCoilSetup::execute()
{
  auto scalp = getRequiredInput(SCALP_SURF);
  auto locations = getRequiredInput(LOCATIONS);
  auto elc_coil_proto = getRequiredDynamicInputs(ELECTRODECOILPROTOTYPES);

 if (needToExecute())  //newStatePresent
 {
   setAlgoBoolFromState(Parameters::ProtoTypeInputCheckbox);
   setAlgoBoolFromState(Parameters::InvertNormalsCheckBox);
   setAlgoBoolFromState(Parameters::OrientTMSCoilRadialToScalpCheckBox);
   setAlgoBoolFromState(Parameters::AllInputsTDCS);
   setAlgoIntFromState(Parameters::ProtoTypeInputComboBox);
   setAlgoBoolFromState(Parameters::ElectrodethicknessCheckBox);
   setAlgoBoolFromState(Parameters::PutElectrodesOnScalpCheckBox);
   setAlgoBoolFromState(Parameters::InterpolateElectrodeShapeCheckbox);
   setAlgoDoubleFromState(Parameters::ElectrodethicknessSpinBox);

   if(elc_coil_proto.size()>0)
    {
     get_state()->setValue(Parameters::NumberOfPrototypes, (int)elc_coil_proto.size());
     setAlgoIntFromState(Parameters::NumberOfPrototypes);
    }

    setAlgoListFromState(Parameters::TableValues);
    auto input = make_input((SCALP_SURF, scalp)(LOCATIONS, locations)(ELECTRODECOILPROTOTYPES, elc_coil_proto));
    std::vector<AlgorithmParameter> table_handle = (get_state()->getValue(Parameters::TableValues)).toVector();

    algo().set(Parameters::TableValues, table_handle);

    auto output = algo().run(input);

    auto table = output.additionalAlgoOutput();

    if (table)
     get_state()->setValue(Parameters::TableValues, table->value());

    sendOutputFromAlgorithm(FINAL_ELECTRODES_FIELD, output);
    sendOutputFromAlgorithm(MOVED_ELECTRODES_FIELD, output);
    sendOutputFromAlgorithm(ELECTRODE_SPONGE_LOCATION_AVR, output);
    sendOutputFromAlgorithm(COILS_FIELD, output);

 }
}
