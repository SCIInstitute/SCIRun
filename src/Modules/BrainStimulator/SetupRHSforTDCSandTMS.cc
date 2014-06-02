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
  setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc0); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc1); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc2); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc3); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc4); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc5); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc6); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc7); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc8); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc9); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc10); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc11); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc12); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc13); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc14); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc15); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc16); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc17); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc18); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc19); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc20); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc21); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc22); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc23); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc24); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc25); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc26); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc27); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc28); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc29); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc30); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc31); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc32); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc33); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc34); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc35); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc36); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc37); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc38); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc39); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc40); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc41); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc42); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc43); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc44); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc45); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc46); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc47); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc48); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc49); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc50); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc51); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc52); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc53); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc54); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc55); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc56); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc57); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc58); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc59); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc60); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc61); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc62); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc63); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc64); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc65); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc66); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc67); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc68); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc69); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc70); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc71); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc72); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc73); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc74); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc75); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc76); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc77); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc78); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc79); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc80); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc81); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc82); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc83); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc84); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc85); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc86); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc87); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc88); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc89); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc90); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc91); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc92); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc93); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc94); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc95); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc96); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc97); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc98); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc99); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc100); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc101); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc102); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc103); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc104); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc105); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc106); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc107); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc108); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc109); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc110); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc111); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc112); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc113); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc114); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc115); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc116); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc117); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc118); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc119); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc120); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc121); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc122); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc123); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc124); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc125); setStateDoubleFromAlgo(SetupRHSforTDCSandTMSAlgorithm::Elc126);
}

void SetupRHSforTDCSandTMSModule::execute()
{
  auto elc_coil_pos_and_normal = getRequiredInput(ELECTRODE_COIL_POSITIONS_AND_NORMAL);
  auto elc_count = getRequiredInput(ELECTRODE_COUNT);

  setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc0); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc1); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc2); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc3); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc4); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc5); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc6); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc7); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc8); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc9); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc10); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc11); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc12); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc13); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc14); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc15); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc16); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc17); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc18); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc19); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc20); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc21); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc22); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc23); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc24); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc25); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc26); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc27); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc28); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc29); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc30); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc31); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc32); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc33); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc34); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc35); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc36); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc37); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc38); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc39); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc40); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc41); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc42); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc43); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc44); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc45); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc46); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc47); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc48); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc49); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc50); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc51); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc52); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc53); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc54); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc55); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc56); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc57); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc58); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc59); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc60); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc61); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc62); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc63); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc64); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc65); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc66); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc67); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc68); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc69); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc70); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc71); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc72); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc73); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc74); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc75); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc76); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc77); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc78); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc79); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc80); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc81); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc82); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc83); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc84); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc85); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc86); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc87); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc88); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc89); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc90); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc91); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc92); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc93); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc94); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc95); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc96); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc97); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc98); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc99); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc100); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc101); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc102); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc103); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc104); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc105); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc106); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc107); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc108); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc109); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc110); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc111); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc112); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc113); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc114); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc115); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc116); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc117); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc118); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc119); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc120); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc121); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc122); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc123); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc124); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc125); setAlgoDoubleFromState(SetupRHSforTDCSandTMSAlgorithm::Elc126); 

  //  auto elc_tri_mesh = getRequiredInput(ELECTRODE_TRIANGULATION);
  // UI input
  //auto param = get_state()->getValue(Variables::AppendMatrixOption).getInt();
  //algorithm parameter
  //algo_->set(Variables::AppendMatrixOption, param);
 
  if (needToExecute())
  {
    //algorithm input
    auto output = algo().run_generic(make_input((ELECTRODE_COIL_POSITIONS_AND_NORMAL, elc_coil_pos_and_normal)(ELECTRODE_COUNT, elc_count)));
    
    //algorithm output
    sendOutputFromAlgorithm(RHS, output);
    // sendOutputFromAlgorithm(ELECTRODES_FIELD, output);
    // sendOutputFromAlgorithm(COILS_FIELD, output);
  }
}
