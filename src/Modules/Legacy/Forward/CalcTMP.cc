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
  : Module(staticInfo_)
{
  INITIALIZE_PORT(Amplitude);
  INITIALIZE_PORT(i1);
  INITIALIZE_PORT(i2);
  INITIALIZE_PORT(i3);
  INITIALIZE_PORT(i4);
  INITIALIZE_PORT(i5);
  INITIALIZE_PORT(i6);
  INITIALIZE_PORT(TMPs);
  //algo_.set_progress_reporter(this);
}


void CalcTMP::execute()
{
  MatrixHandle amplitudes = getRequiredInput(Amplitude);
  auto deps = getOptionalInput(i1);




  MatrixHandle depslopes;
  MatrixHandle platslopes;
  MatrixHandle reps;
  MatrixHandle repslopes;
  MatrixHandle rests;
  MatrixHandle tmps;

  // get_input_handle("Depolarization Time", deps, false);
  // get_input_handle("Depolarization Slope", depslopes, false);
  // get_input_handle("Plateau Slope", platslopes, false);
  // get_input_handle("Repolarization Time", reps, false);
  // get_input_handle("Repolarization Slope", repslopes, false);
  // get_input_handle("Rest Potential", rests, false);

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
