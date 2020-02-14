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

MODULE_INFO_DEF(SimulateForwardMagneticField, BrainStimulator, SCIRun)

SimulateForwardMagneticField::SimulateForwardMagneticField() : Module(staticInfo_, false)
{
 INITIALIZE_PORT(ElectricField);
 INITIALIZE_PORT(ConductivityTensor);
 INITIALIZE_PORT(DipoleSources);
 INITIALIZE_PORT(DetectorLocations);
 INITIALIZE_PORT(MagneticField);
 INITIALIZE_PORT(MagneticFieldMagnitudes);
}

void SimulateForwardMagneticField::setStateDefaults()
{
}

void SimulateForwardMagneticField::execute()
{
  auto EField = getRequiredInput(ElectricField);
  auto CondTensor = getRequiredInput(ConductivityTensor);
  auto Dipoles = getRequiredInput(DipoleSources);
  auto Detectors = getRequiredInput(DetectorLocations);

  if (needToExecute())
  {
     auto output = algo().run(make_input((ElectricField, EField)(ConductivityTensor, CondTensor)(DipoleSources, Dipoles)(DetectorLocations, Detectors)));
    sendOutputFromAlgorithm(MagneticField, output);
    sendOutputFromAlgorithm(MagneticFieldMagnitudes, output);
  }
}
