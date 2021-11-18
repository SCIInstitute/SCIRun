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

#include <Modules/Legacy/Fields/ApplyFilterToFieldData.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Legacy/Fields/FilterFieldData/DilateFieldData.h>
#include <Core/Algorithms/Legacy/Fields/FilterFieldData/ErodeFieldData.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

MODULE_INFO_DEF(ApplyFilterToFieldData, ChangeFieldData, SCIRun)

ALGORITHM_PARAMETER_DEF(Fields, Erode)
ALGORITHM_PARAMETER_DEF(Fields, Dilate)

ApplyFilterToFieldData::ApplyFilterToFieldData() :
  Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
}

/// @class ApplyFilterToFieldData
/// @brief Applies a dilate or erode filter to a regular mesh.

void ApplyFilterToFieldData::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Core::Algorithms::Variables::MaxIterations, 2);
  state->setValue(Parameters::Erode, true);
  state->setValue(Parameters::Dilate, false);
}

void ApplyFilterToFieldData::execute()
{
  auto input = getRequiredInput(InputField);

  if (needToExecute())
  {
    auto state = get_state();
    auto iters = state->getValue(Variables::MaxIterations).toInt();
    auto doDilate = state->getValue(Parameters::Dilate).toBool();
    auto doErode = state->getValue(Parameters::Erode).toBool();
    FieldHandle output;
    // Dilate then erode
    if (doDilate)
    {
      DilateFieldDataAlgo dilate;
      dilate.set(Variables::MaxIterations, iters);
      if (!dilate.runImpl(input, output))
      {
        THROW_ALGORITHM_PROCESSING_ERROR("Error in dilate algo");
      }
      
    }
    if (doErode)
    {
      ErodeFieldDataAlgo erode;
      erode.set(Variables::MaxIterations, iters);
      if (!erode.runImpl(input, output))
      {
        THROW_ALGORITHM_PROCESSING_ERROR("Error in erode algo");
      }
    }
    if (doDilate || doErode)
      sendOutput(OutputField, output);
    else //legacy behavior--unfiltered input
      sendOutput(OutputField, input);
  }
}
