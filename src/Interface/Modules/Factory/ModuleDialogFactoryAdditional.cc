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


#include <Interface/Modules/Factory/ModuleDialogFactory.h>
#include <Interface/Modules/BrainStimulator/SetConductivitiesToTetMeshDialog.h>
#include <Interface/Modules/BrainStimulator/ElectrodeCoilSetupDialog.h>
#include <Interface/Modules/BrainStimulator/GenerateROIStatisticsDialog.h>
#include <Interface/Modules/BrainStimulator/SetupRHSforTDCSandTMSDialog.h>
#include <Interface/Modules/Visualization/GenerateStreamLinesDialog.h>
#include <boost/assign.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace boost::assign;

void ModuleDialogFactory::addDialogsToMakerMap2()
{
  insert(dialogMakerMap_)
    ADD_MODULE_DIALOG(ElectrodeCoilSetup, ElectrodeCoilSetupDialog)
    ADD_MODULE_DIALOG(SetConductivitiesToMesh, SetConductivitiesToTetMeshDialog)
    ADD_MODULE_DIALOG(GenerateROIStatistics, GenerateROIStatisticsDialog)
    ADD_MODULE_DIALOG(SetupTDCS, SetupRHSforTDCSandTMSDialog)
    ADD_MODULE_DIALOG(GenerateStreamLines, GenerateStreamLinesDialog)
  ;
}
