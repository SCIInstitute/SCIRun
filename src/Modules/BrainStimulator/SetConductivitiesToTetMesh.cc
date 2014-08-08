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
   
   Author: Spencer Frisby
   Date:   May 2014
*/
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

SetConductivitiesToTetMeshModule::SetConductivitiesToTetMeshModule() : Module(ModuleLookupInfo("SetConductivitiesToTetMesh", "BrainStimulator", "SCIRun"))
{
 INITIALIZE_PORT(MESH);
 INITIALIZE_PORT(OUTPUTMESH);
}

void SetConductivitiesToTetMeshModule::setStateDefaults()
{
  setStateDoubleFromAlgo(SetConductivitiesToTetMeshAlgorithm::Skin());
  setStateDoubleFromAlgo(SetConductivitiesToTetMeshAlgorithm::SoftBone());
  setStateDoubleFromAlgo(SetConductivitiesToTetMeshAlgorithm::HardBone());
  setStateDoubleFromAlgo(SetConductivitiesToTetMeshAlgorithm::CSF());
  setStateDoubleFromAlgo(SetConductivitiesToTetMeshAlgorithm::GM());
  setStateDoubleFromAlgo(SetConductivitiesToTetMeshAlgorithm::WM());
  setStateDoubleFromAlgo(SetConductivitiesToTetMeshAlgorithm::Electrode());
}

void SetConductivitiesToTetMeshModule::execute()
{
  auto mesh = getRequiredInput(MESH);
 
  setAlgoDoubleFromState(SetConductivitiesToTetMeshAlgorithm::Skin());
  setAlgoDoubleFromState(SetConductivitiesToTetMeshAlgorithm::SoftBone());
  setAlgoDoubleFromState(SetConductivitiesToTetMeshAlgorithm::HardBone());
  setAlgoDoubleFromState(SetConductivitiesToTetMeshAlgorithm::CSF());
  setAlgoDoubleFromState(SetConductivitiesToTetMeshAlgorithm::GM());
  setAlgoDoubleFromState(SetConductivitiesToTetMeshAlgorithm::WM());
  setAlgoDoubleFromState(SetConductivitiesToTetMeshAlgorithm::Electrode());
  
  if (needToExecute())
  {
    /// algorithm input and run, 
    auto output = algo().run_generic(make_input((MESH, mesh)));
   
    /// algorithm output
    sendOutputFromAlgorithm(OUTPUTMESH, output);
  }
}
