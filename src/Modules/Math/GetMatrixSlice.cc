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
#include <Core/Datatypes/Scalar.h>
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
  INITIALIZE_PORT(Current_Index);
  INITIALIZE_PORT(Selected_Index);
}

void GetMatrixSlice::setStateDefaults()
{
  setStateBoolFromAlgo(Parameters::IsSliceColumn);
  setStateIntFromAlgo(Parameters::SliceIndex);
}

void GetMatrixSlice::execute()
{
  auto input = getRequiredInput(InputMatrix);
  auto index = getOptionalInput(Current_Index);
  if (needToExecute() || playing_)
  {
    auto state = get_state();
    setAlgoBoolFromState(Parameters::IsSliceColumn);
    if (index && *index)
    {
      state->setValue(Parameters::SliceIndex, (*index)->value());
    }
    setAlgoIntFromState(Parameters::SliceIndex);
    auto output = algo().run(withInputData((InputMatrix, input)));
    sendOutputFromAlgorithm(OutputMatrix, output);
    sendOutput(Selected_Index, boost::make_shared<Int32>(get_state()->getValue(Parameters::SliceIndex).toInt()));
    auto maxIndex = output.additionalAlgoOutput()->toInt();
    state->setValue(Parameters::MaxIndex, maxIndex);

    auto playMode = optional_any_cast_or_default<int>(get_state()->getTransientValue(Parameters::PlayMode));
    if (playMode == GetMatrixSliceAlgo::PLAY)
    {
      auto nextIndex = algo().get(Parameters::SliceIndex).toInt() + 1;
      state->setValue(Parameters::SliceIndex, nextIndex % (maxIndex + 1));
      playing_ = true;
      enqueueExecuteAgain();
    }
    else if (playMode == GetMatrixSliceAlgo::PAUSE)
    {
      playing_ = false;
    }
    else if (playMode != 0)
    {
      playing_ = false;
      remark("Logical error: received invalid play mode value");
    }
  }
}
