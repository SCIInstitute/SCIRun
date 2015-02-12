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

#include <Modules/Visualization/CreateBasicColorMap.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/ColorMap.h>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

CreateBasicColorMap::CreateBasicColorMap() : Module(ModuleLookupInfo("CreateStandardColorMap", "Visualization", "SCIRun"))
{
  INITIALIZE_PORT(ColorMapObject);
}

void CreateBasicColorMap::setStateDefaults()
{
  auto state = get_state();
  state->setValue(ColorMapName, std::string("Rainbow"));
  state->setValue(ColorMapResolution, 256);
  state->setValue(ColorMapInvert, false);
  state->setValue(ColorMapShift, 0.0);
}

void CreateBasicColorMap::execute()
{
  if (needToExecute())
  {
    auto state = get_state();
    auto name = state->getValue(ColorMapName).toString();
    auto res = state->getValue(ColorMapResolution).toInt();
    auto inv = state->getValue(ColorMapInvert).toBool();
    auto shift = state->getValue(ColorMapShift).toDouble();
    //just in case there is a problem with the QT values...
    res = std::min(std::max(res,2),256);
    shift = std::min(std::max(shift,-1.),1.);
    sendOutput(ColorMapObject,StandardColorMapFactory::create(name,res, shift,inv));
  }
}


const AlgorithmParameterName CreateBasicColorMap::ColorMapName("ColorMapName");
const AlgorithmParameterName CreateBasicColorMap::ColorMapInvert("ColorMapInvert");
const AlgorithmParameterName CreateBasicColorMap::ColorMapShift("ColorMapShift");
const AlgorithmParameterName CreateBasicColorMap::ColorMapResolution("ColorMapResolution");