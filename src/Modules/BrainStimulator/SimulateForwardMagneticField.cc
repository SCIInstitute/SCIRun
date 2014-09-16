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

#include <Modules/BrainStimulator/SimulateForwardMagneticField.h>
#include <Core/Algorithms/BrainStimulator/SimulateForwardMagneticFieldAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <vector>

using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;

SimulateForwardMagneticFieldModule::SimulateForwardMagneticFieldModule() : Module(ModuleLookupInfo("SimulateForwardMagneticField", "BrainStimulator", "SCIRun"),false)
{
 INITIALIZE_PORT(ElectricField);
 INITIALIZE_PORT(ConductivityTensor);
 INITIALIZE_PORT(DipoleSources);
 INITIALIZE_PORT(DetectorLocations);
 INITIALIZE_PORT(MagneticField);
 INITIALIZE_PORT(MagneticFieldMagnitudes);
}

void SimulateForwardMagneticFieldModule::setStateDefaults()
{
 // setStateIntFromAlgo(SimulateForwardMagneticFieldAlgorithm::refnode());
}

void SimulateForwardMagneticFieldModule::execute()
{ 
  auto EField = getRequiredInput(ElectricField);
  auto CondTensor = getRequiredInput(ConductivityTensor);
  auto Dipoles = getRequiredInput(DipoleSources);
  auto Detectors = getRequiredInput(DetectorLocations);
  
  // obtaining electrode values from state map
 // auto elc_vals_from_state = get_state()->getValue(Parameters::ElectrodeTableValues).getList();
 // algo().set(Parameters::ELECTRODE_VALUES, elc_vals_from_state);
 
  if(!CondTensor)
  {
   std::cout << " xxxx "<< std::endl;
  }
  
  if (needToExecute())
  {
    //algo().set(SimulateForwardMagneticFieldAlgorithm::refnode(), get_state()->getValue(SimulateForwardMagneticFieldAlgorithm::refnode()).getInt());
    std::cout << " 1 "<< std::endl;
    auto output = algo().run_generic(make_input((ElectricField, EField)(ConductivityTensor, CondTensor)(DipoleSources, Dipoles)(DetectorLocations, Detectors)));

    sendOutputFromAlgorithm(MagneticField, output);
    sendOutputFromAlgorithm(MagneticFieldMagnitudes, output);
  }
}
