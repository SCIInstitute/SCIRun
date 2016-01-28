/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Modules/Legacy/Forward/CalcTMP.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Modules/Legacy/Forward/CalcTMPAlgo.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Forward;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Forward;

const ModuleLookupInfo CalcTMP::staticInfo_("CalcTMP", "Forward", "SCIRun");

CalcTMP::CalcTMP()
  : Module(staticInfo_, false)
{
  INITIALIZE_PORT(Amplitude);
  INITIALIZE_PORT(Depolarization_Time);
  INITIALIZE_PORT(Depolarization_Slope);
  INITIALIZE_PORT(Plateau_Slope);
  INITIALIZE_PORT(Repolarization_Time);
  INITIALIZE_PORT(Repolarization_Slope);
  INITIALIZE_PORT(Rest_Potential);
  INITIALIZE_PORT(TMPs);
}


void CalcTMP::execute()
{
  auto amplitudes = getRequiredInput(Amplitude);
  auto deps = getRequiredInput(Depolarization_Time);
  auto depslopes = getRequiredInput(Depolarization_Slope);
  auto platslopes = getRequiredInput(Plateau_Slope);
  auto reps = getRequiredInput(Repolarization_Time);
  auto repslopes = getRequiredInput(Repolarization_Slope);
  auto rests = getRequiredInput(Rest_Potential);

  DenseMatrixHandle tmps;

  if (needToExecute())
  {
    const int numSamples = 500;
    //TODO: algo hookup
    CalcTMPAlgo algo;
    algo.setLogger(getLogger());
    algo.calc_TMPs(amplitudes,
                    deps,
                    depslopes,
                    platslopes,
                    reps,
                    repslopes,
                    rests,
                    numSamples,
                    tmps);

    sendOutput(TMPs, tmps);
  }
}
