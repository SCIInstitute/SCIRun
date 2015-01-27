/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

/// @todo Documentation Modules/Math/GetMatrixSlice.cc

#include <Modules/Math/GetMatrixSlice.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Math/GetMatrixSliceAlgo.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Dataflow::Networks;

const ModuleLookupInfo GetMatrixSlice::staticInfo_("GetMatrixSlice", "Math", "SCIRun");

GetMatrixSlice::GetMatrixSlice() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputMatrix);
}

void GetMatrixSlice::setStateDefaults()
{
  setStateBoolFromAlgo(Parameters::IsSliceColumn);
  setStateIntFromAlgo(Parameters::SliceIndex);
}

void GetMatrixSlice::execute()
{
  auto input = getRequiredInput(InputMatrix);
  if (needToExecute())
  {
    setAlgoBoolFromState(Parameters::IsSliceColumn);
    setAlgoIntFromState(Parameters::SliceIndex);
    auto output = algo().run(withInputData((InputMatrix, input)));
    sendOutputFromAlgorithm(OutputMatrix, output);
    get_state()->setValue(Parameters::MaxIndex, output.additionalAlgoOutput()->toInt());

    /*
    auto playMode = get_state()->getValue(Parameters::PlayMode).toBool();
    if (playMode)
    {
      auto nextIndex = algo().get(Parameters::SliceIndex).toInt() + 1;
      auto maxIndex = algo().get(Parameters::IsSliceColumn).toBool() && input ? input->ncols() : input->nrows();
      if (nextIndex >= maxIndex)
      {
        get_state()->setValue(Parameters::PlayMode, false);
      }
      else
      {
        get_state()->setValue(Parameters::SliceIndex, nextIndex);
        enqueueExecuteAgain();
      }
    }
    */
  }
}
