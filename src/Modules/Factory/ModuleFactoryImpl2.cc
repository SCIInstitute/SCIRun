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


#include <Modules/Factory/ModuleDescriptionLookup.h>
#include <Modules/Fields/RefineTetMeshLocally.h>
#include <Modules/Legacy/Fields/MapFieldDataFromElemToNode.h>
#include <Modules/Legacy/Fields/MapFieldDataFromNodeToElem.h>
#include <Modules/Legacy/Fields/SetFieldData.h>
#include <Modules/Legacy/Fields/SplitFieldByConnectedRegion.h>
#include <Modules/Legacy/Math/SelectSubMatrix.h>
#include <Modules/BrainStimulator/SetConductivitiesToTetMesh.h>
#include <Modules/BrainStimulator/SetupRHSforTDCSandTMS.h>
#include <Modules/BrainStimulator/SimulateForwardMagneticField.h>
#include <Modules/Legacy/FiniteElements/BuildTDCSMatrix.h>
#include <Modules/Legacy/Fields/ExtractSimpleIsosurface.h>
#include <Modules/Visualization/ShowString.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Modules::FiniteElements;
using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Modules::Visualization;

void ModuleDescriptionLookup::addBrainSpecificModules()
{
  addModuleDesc<SetConductivitiesToMesh>("SetConductivitiesToMesh", "BrainStimulator", "SCIRun", "New module", " Sets conveniently conductivity profile for tetrahedral mesh ");
  addModuleDesc<SetupTDCS>("SetupTDCS", "BrainStimulator", "SCIRun", " in progress ", " set RHS for tDCS and TMS ");
}

void ModuleDescriptionLookup::addMoreModules()
{
  addModuleDesc<SetFieldData>("SetFieldData", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<SelectSubMatrix>("SelectSubMatrix", "Math", "SCIRun", "Real ported module", "...");
  addModuleDesc<MapFieldDataFromElemToNode>("MapFieldDataFromElemToNode", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<MapFieldDataFromNodeToElem>("MapFieldDataFromNodeToElem", "ChangeFieldData", "SCIRun", "Real ported module", "...");
  addModuleDesc<ExtractIsosurface>("ExtractSimpleIsosurface", "NewField", "SCIRun", "...", "...");
  addModuleDesc<RefineTetMeshLocally>("RefineTetMeshLocally","ChangeMesh","SCIRun","...","...");
  addModuleDesc<ShowString>("in progress--needs testing", "...");
}
