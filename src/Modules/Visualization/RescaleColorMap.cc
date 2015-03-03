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
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
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
  state->setValue(AutoScale,true);
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
      auto autoscale = state->getValue(AutoScale).toInt() == 0;
      auto symmetric = state->getValue(Symmetric).toBool();
      auto fixed_min = state->getValue(FixedMin).toDouble();
      auto fixed_max = state->getValue(FixedMax).toDouble();
      
      double cm_scale = 1.;
      double cm_shift = 0.;
      
      //set the min/max values to the actual min/max if we choose auto
      VField* fld = field->vfield();
      VMesh*  mesh = field->vmesh();
      double sval;
      mesh->synchronize(Mesh::NODES_E);
      VMesh::Node::iterator eiter, eiter_end;
      mesh->begin(eiter);
      mesh->end(eiter_end);
      
      double actual_min = std::numeric_limits<double>::max();
      double actual_max = std::numeric_limits<double>::min();
      
      while(eiter != eiter_end) {
        fld->get_value(sval, *eiter);
        actual_min = std::min(sval,actual_min);
        actual_max = std::max(sval,actual_max);
        ++eiter;
      }
      if (autoscale) {
        //center around zero
        if (symmetric) {
            double mx = std::max(std::abs(actual_min),std::abs(actual_max));
            fixed_min = -mx;
            fixed_max = mx;
        } else {
            fixed_min = actual_min;
            fixed_max = actual_max;
        }
        state->setValue(FixedMin, fixed_min);
        state->setValue(FixedMax, fixed_max);
      }
      cm_scale = (actual_max - actual_min) / (fixed_max - fixed_min);
      cm_shift = (actual_min - fixed_min) / (fixed_max - fixed_min);
      
      sendOutput(ColorMapOutput, StandardColorMapFactory::create(colorMap.get()->getColorMapName(),
                                                                 colorMap.get()->getColorMapResolution(),
                                                                 colorMap.get()->getColorMapShift(),
                                                                 colorMap.get()->getColorMapInvert(),
                                                                 cm_scale, cm_shift,fixed_min,fixed_max));
  }
}

const AlgorithmParameterName RescaleColorMap::AutoScale("AutoScale");
const AlgorithmParameterName RescaleColorMap::Symmetric("Symmetric");
const AlgorithmParameterName RescaleColorMap::FixedMin("FixedMin");
const AlgorithmParameterName RescaleColorMap::FixedMax("FixedMax");