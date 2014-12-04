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

///@file ElectrodeCoilSetup
///@brief With this module the user is able to position tDCS electrodes and TMS coils to setup a simulation scenario. 
///  
///
///@author
/// Moritz Dannhauer
///
///@details
/// The module gets the triangulated scalp surface as the first input and a matrix of possible locations as the second. The third and any following input fields (dynamic input ports)
/// are considered to be prototypes of electrodes or TMS coils that need to be positioned. After the first execution using a minimum of 3 (valid) inputs the positions appear as table rows
/// in the GUI. The GUI allows to assign a prototype (first table column), normal direction etc. as well as the type of stimulation (second table column). Based on the choices made in the  
/// first (prototype, "Input #") and second coloumn (stimulation type, "Type") information are gathered from the rest of the row. An defined tDCS electrode or TMS coil will be moved using
/// its location ("X","Y","Z"), its normal ("NX","NY","NZ") and can be rotated along its normal by the angle table cell defined in degrees. Throughout this table, multiple electrodes and TMS coils
/// can be located at their final destinations. For a tDCS electrode specification, the placed and oriented prototype (contains electrode dimension) is used to cut the scalp that is covered by the
/// electrode. This piece of scalp and its normals are used to generate the electrode sponge shape by using the thickness table parameter. The module contains three outputs: 1) the averaged scalp 
/// sponge locations (+thickness, as a DenseMatrix) which are designed to be the last input of the SetupTDCS module, 2) a collection of all valid tDCS electrodes and 3) a collection of all
/// valid TMS coil inputs that are represented by rotated points and dipole normals approximating the magnetic field. 

using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Dataflow::Networks;

ElectrodeCoilSetupModule::ElectrodeCoilSetupModule() : Module(ModuleLookupInfo("ElectrodeCoilSetup", "BrainStimulator", "SCIRun"))
{
 INITIALIZE_PORT(SCALP_SURF);
 INITIALIZE_PORT(LOCATIONS);
 INITIALIZE_PORT(ELECTRODECOILPROTOTYPES);
 INITIALIZE_PORT(ELECTRODE_SPONGE_LOCATION_AVR);
 INITIALIZE_PORT(COILS_FIELD);
 INITIALIZE_PORT(ELECTRODES_FIELD);
}

void ElectrodeCoilSetupModule::setStateDefaults()
{
  setStateIntFromAlgo(Parameters::NumberOfPrototypes);
  setStateBoolFromAlgo(Parameters::ProtoTypeInputCheckbox);
  setStateBoolFromAlgo(Parameters::AllInputsTDCS);
  setStateIntFromAlgo(Parameters::ProtoTypeInputComboBox);
  setStateListFromAlgo(Parameters::TableValues);
  setStateDoubleFromAlgo(Parameters::ElectrodethicknessSpinBox);
  setStateBoolFromAlgo(Parameters::ElectrodethicknessCheckBox);
}

void ElectrodeCoilSetupModule::execute()
{
  auto scalp = getRequiredInput(SCALP_SURF);
  auto locations = getRequiredInput(LOCATIONS);
  auto elc_coil_proto = getRequiredDynamicInputs(ELECTRODECOILPROTOTYPES);

 if (needToExecute())  //newStatePresent
 {
   setAlgoBoolFromState(Parameters::ProtoTypeInputCheckbox);
   setAlgoBoolFromState(Parameters::AllInputsTDCS);
   setAlgoIntFromState(Parameters::ProtoTypeInputComboBox);
   setAlgoBoolFromState(Parameters::ElectrodethicknessCheckBox);
   setAlgoDoubleFromState(Parameters::ElectrodethicknessSpinBox);
   
   update_state(Executing);
   if(elc_coil_proto.size()>0)
    {
     get_state()->setValue(Parameters::NumberOfPrototypes, (int)elc_coil_proto.size());
     setAlgoIntFromState(Parameters::NumberOfPrototypes);
    }
   
    setAlgoListFromState(Parameters::TableValues);
    auto input = make_input((SCALP_SURF, scalp)(LOCATIONS, locations)(ELECTRODECOILPROTOTYPES, elc_coil_proto));
    std::vector<AlgorithmParameter> table_handle = (get_state()->getValue(Parameters::TableValues)).toVector();
	
    algo().set(Parameters::TableValues, table_handle);
     
    auto output = algo().run_generic(input);
    auto table = output.additionalAlgoOutput();
    if (table)
     get_state()->setValue(Parameters::TableValues, table->value());
     
    sendOutputFromAlgorithm(ELECTRODES_FIELD, output);
    sendOutputFromAlgorithm(ELECTRODE_SPONGE_LOCATION_AVR, output);
    sendOutputFromAlgorithm(COILS_FIELD, output);
 }
}
