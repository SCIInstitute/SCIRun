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
//#include <Packages/BioPSE/Core/Algorithms/NumApproximation/CalcTMP.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Forward;
using namespace SCIRun::Dataflow::Networks;
//using namespace SCIRun::Core::Algorithms::Forward;

const ModuleLookupInfo CalcTMP::staticInfo_("CalcTMP", "Forward", "SCIRun");

/*
class CalcTMP : public Module, public CalcTMPAlgo {
  public:
    CalcTMP(GuiContext*);
    virtual ~CalcTMP() {}

    virtual void execute();

  private:
    //SCIRunAlgo::CalcTMPAlgo algo_;
    CalcTMPAlgo algo_;
};
*/

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
  //algo_.set_progress_reporter(this);
}


void CalcTMP::execute()
{
  auto amplitudes = getOptionalInput(Amplitude);
  auto deps = getOptionalInput(Depolarization_Time);
  auto depslopes = getOptionalInput(Depolarization_Slope);;
  auto platslopes = getOptionalInput(Plateau_Slope);;
  auto reps = getOptionalInput(Repolarization_Time);;
  auto repslopes = getOptionalInput(Repolarization_Slope);;
  auto rests = getOptionalInput(Rest_Potential);;

  MatrixHandle tmps;

  if (needToExecute())
  {
    /* TODO LATER
    algo_.calc_TMPs(amplitudes,
                    deps,
                    depslopes,
                    platslopes,
                    reps,
                    repslopes,
                    rests,
                    500,
                    TMPs);
                    */

    sendOutput(TMPs, tmps);
  }
}
