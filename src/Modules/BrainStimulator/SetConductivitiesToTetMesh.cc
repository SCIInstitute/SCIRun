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


/// Author: Spencer Frisby
/// Date:   August 2014

#include <iostream>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Scalar.h>
#include <Modules/BrainStimulator/SetConductivitiesToTetMesh.h>
#include <Core/Algorithms/BrainStimulator/SetConductivitiesToTetMeshAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Dataflow::Networks;

SetConductivitiesToMesh::SetConductivitiesToMesh() : Module(ModuleLookupInfo("SetConductivitiesToMesh", "BrainStimulator", "SCIRun"))
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
}

void SetConductivitiesToMesh::setStateDefaults()
{
  setStateDoubleFromAlgo(Parameters::Skin);
  setStateDoubleFromAlgo(Parameters::SoftBone);
  setStateDoubleFromAlgo(Parameters::HardBone);
  setStateDoubleFromAlgo(Parameters::CSF);
  setStateDoubleFromAlgo(Parameters::GM);
  setStateDoubleFromAlgo(Parameters::WM);
  setStateDoubleFromAlgo(Parameters::Electrode);
  setStateDoubleFromAlgo(Parameters::InternalAir);
}

void SetConductivitiesToMesh::execute()
{
  auto mesh = getRequiredInput(InputField);

  if (needToExecute())
  {
    setAlgoDoubleFromState(Parameters::Skin);
    setAlgoDoubleFromState(Parameters::SoftBone);
    setAlgoDoubleFromState(Parameters::HardBone);
    setAlgoDoubleFromState(Parameters::CSF);
    setAlgoDoubleFromState(Parameters::GM);
    setAlgoDoubleFromState(Parameters::WM);
    setAlgoDoubleFromState(Parameters::Electrode);
    setAlgoDoubleFromState(Parameters::InternalAir);

    /// algorithm input and run,
    auto input = make_input((InputField, mesh));
    auto output = algo().run(input);

    /// algorithm output
    sendOutputFromAlgorithm(OutputField, output);
  }
}
