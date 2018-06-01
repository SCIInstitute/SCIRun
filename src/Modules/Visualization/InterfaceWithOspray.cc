/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Modules/Visualization/InterfaceWithOspray.h>
#include <Core/Algorithms/Visualization/OsprayDataAlgorithm.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace Dataflow::Networks;
using namespace Modules::Visualization;
using namespace Core;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Visualization;
using namespace Datatypes;

MODULE_INFO_DEF(InterfaceWithOspray, Visualization, SCIRun)

void InterfaceWithOspray::setStateDefaults()
{
  setStateDoubleFromAlgo(Parameters::DefaultColorR);
  setStateDoubleFromAlgo(Parameters::DefaultColorG);
  setStateDoubleFromAlgo(Parameters::DefaultColorB);
  setStateDoubleFromAlgo(Parameters::DefaultColorA);
  setStateDoubleFromAlgo(Parameters::Radius);
  setStateDoubleFromAlgo(Parameters::UseNormals);
  
}

InterfaceWithOspray::InterfaceWithOspray() : Module(staticInfo_)
{
  INITIALIZE_PORT(Field);
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(Streamlines);
  INITIALIZE_PORT(SceneGraph);
}

void InterfaceWithOspray::execute()
{
  auto fields = getOptionalDynamicInputs(Field);
  auto colorMaps = getOptionalDynamicInputs(ColorMapObject);
  auto streamlines = getOptionalDynamicInputs(Streamlines);

  if (needToExecute())
  {
    setAlgoDoubleFromState(Parameters::DefaultColorR);
    setAlgoDoubleFromState(Parameters::DefaultColorG);
    setAlgoDoubleFromState(Parameters::DefaultColorB);
    setAlgoDoubleFromState(Parameters::DefaultColorA);
    setStateDoubleFromAlgo(Parameters::Radius);
    setAlgoDoubleFromState(Parameters::UseNormals);

    if (!fields.empty())
    {
      if (colorMaps.size() < fields.size())
        colorMaps.resize(fields.size());
    }
    
    auto output = algo().run(withInputData((Field, fields)(ColorMapObject, colorMaps)(Streamlines, streamlines)));
    sendOutputFromAlgorithm(SceneGraph, output);
  }
}
