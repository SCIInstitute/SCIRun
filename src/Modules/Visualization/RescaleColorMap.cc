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


#include <Modules/Visualization/RescaleColorMap.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/ColorMap.h>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Visualization;

MODULE_INFO_DEF(RescaleColorMap, Visualization, SCIRun)

RescaleColorMap::RescaleColorMap() : Module(staticInfo_)
{
  INITIALIZE_PORT(Field);
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(ColorMapOutput);
}

void RescaleColorMap::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::AutoScale, 0);
  state->setValue(Parameters::Symmetric, false);
  state->setValue(Parameters::FixedMin, 0.0);
  state->setValue(Parameters::FixedMax, 1.0);
}
/**
 * @name execute
 *
 * @brief This module has a simple algorithm to ensure the field data scales into ColorMap space.
 *
 * The "input" for this algorithm/module is the field data and the color map from
 *  CreateStandardColorMap module. The "output" is a new color map that applies the rescaling
 *  options from this module: rescale_shift, and rescale_scale.
 */
void RescaleColorMap::execute()
{
  auto fields = getRequiredDynamicInputs(Field);
  auto colorMap = getRequiredInput(ColorMapObject);

  if (needToExecute())
  {
    auto state = get_state();
    auto autoscale = state->getValue(Parameters::AutoScale).toInt() == 0;
    auto symmetric = state->getValue(Parameters::Symmetric).toBool();
    auto fixed_min = state->getValue(Parameters::FixedMin).toDouble();
    auto fixed_max = state->getValue(Parameters::FixedMax).toDouble();

    double cm_scale = 1.;
    double cm_shift = 0.;

    //set the min/max values to the actual min/max if we choose auto
    double actual_min = std::numeric_limits<double>::max();
    double actual_max = -std::numeric_limits<double>::max();
    double min,max;

    for (const auto& field : fields)
    {
      if (!field->vfield()->minmax(min, max))
      {
        error("An input field is not a scalar or vector field.");
        return;
      }
      actual_min = std::min(actual_min, min);
      actual_max = std::max(actual_max, max);
    }

    if (autoscale)
    {
      //center around zero
      if (symmetric)
      {
        double mx = std::max(std::abs(actual_min), std::abs(actual_max));
        fixed_min = -mx;
        fixed_max = mx;
      }
      else
      {
        fixed_min = actual_min;
        fixed_max = actual_max;
      }
      state->setValue(Parameters::FixedMin, fixed_min);
      state->setValue(Parameters::FixedMax, fixed_max);
    }
    cm_shift =  - fixed_min;
    cm_scale = 1. / (fixed_max - fixed_min);

    sendOutput(ColorMapOutput, StandardColorMapFactory::create(
      colorMap->getColorData(), colorMap->getColorMapName(),
      colorMap->getColorMapResolution(), colorMap->getColorMapShift(),
      colorMap->getColorMapInvert(), cm_scale, cm_shift, colorMap->getAlphaLookup()));
  }
}

ALGORITHM_PARAMETER_DEF(Visualization, AutoScale);
ALGORITHM_PARAMETER_DEF(Visualization, Symmetric);
ALGORITHM_PARAMETER_DEF(Visualization, FixedMin);
ALGORITHM_PARAMETER_DEF(Visualization, FixedMax);
