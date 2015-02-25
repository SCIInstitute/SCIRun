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

/// @todo Documentation Modules/Visualization/CreateBasicColorMap.cc

#include <Modules/Visualization/RescaleColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Field/ReportFieldInfoAlgorithm.h>
#include <Core/Datatypes/ColorMap.h>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

RescaleColorMap::RescaleColorMap() : Module(ModuleLookupInfo("RescaleColorMap", "Visualization", "SCIRun"))
{
  INITIALIZE_PORT(Field);
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(ColorMapOutput);
}

void RescaleColorMap::setStateDefaults()
{
  auto state = get_state();
  state->setValue(AutoScale,false);
  state->setValue(Symmetric,false);
  state->setValue(FixedMin,0.0);
  state->setValue(FixedMax,1.0);
}

void RescaleColorMap::execute()
{
  if (needToExecute())
  {
      boost::shared_ptr<SCIRun::Field> field = getRequiredInput(Field);
      boost::shared_ptr<SCIRun::Core::Datatypes::ColorMap> colorMap = getRequiredInput(ColorMapObject);
      
      auto state = get_state();
      auto autoscale = state->getValue(AutoScale).toBool();
      auto symmetric = state->getValue(Symmetric).toBool();
      auto fixedmin = state->getValue(FixedMin).toDouble();
      auto fixedmax = state->getValue(FixedMax).toDouble();
      
      double cm_scale = 1.;
      double cm_shift = 0.;
      
      //set the min/max values to the actual min/max if we choose auto
      auto output = algo().run_generic(withInputData((Field, field)));
      auto info = optional_any_cast_or_default<SCIRun::Core::Algorithms::Fields::ReportFieldInfoAlgorithm::Outputs>(output.getTransient());
      double auto_min = info.dataMin;
      double auto_max = info.dataMax;
      
      if (autoscale) {
        //center around zero
        if (symmetric) {
            double mx = std::max(std::abs(auto_min),std::abs(auto_max));
            cm_scale = (auto_max - auto_min) / (2. * mx);
            if (std::abs(auto_min) < std::abs(auto_max))
                cm_shift = 1. - cm_scale;
            auto_min = -mx;
            auto_max = mx;
        }
        state->setValue(FixedMin, auto_min);
        state->setValue(FixedMax, auto_max);
      } else {
        cm_scale = (auto_max - auto_min) / (fixedmax - fixedmin); //TODO
      }
      
      
      ColorMap cm(colorMap.get()->getColorMapName(),
                  colorMap.get()->getColorMapResolution(),
                  colorMap.get()->getColorMapShift(),
                  colorMap.get()->getColorMapInvert());
      
      
      sendOutput(ColorMapOutput, StandardColorMapFactory::create(cm.getColorMapName(),
                                                                 cm.getColorMapResolution(),
                                                                 cm.getColorMapShift(),
                                                                 cm.getColorMapInvert()));
  }
}

const AlgorithmParameterName RescaleColorMap::AutoScale("AutoScale");
const AlgorithmParameterName RescaleColorMap::Symmetric("Symmetric");
const AlgorithmParameterName RescaleColorMap::FixedMin("FixedMin");
const AlgorithmParameterName RescaleColorMap::FixedMax("FixedMax");