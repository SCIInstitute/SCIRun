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


#include <Modules/Visualization/ShowFieldWithOspray.h>
#include <Core/Algorithms/Visualization/OsprayDataAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
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
using namespace OsprayVisualization;
using namespace Datatypes;

MODULE_INFO_DEF(ShowFieldWithOspray, Visualization, SCIRun)

void ShowFieldWithOspray::setStateDefaults()
{
  setStateDoubleFromAlgo(Parameters::DefaultColorR);
  setStateDoubleFromAlgo(Parameters::DefaultColorG);
  setStateDoubleFromAlgo(Parameters::DefaultColorB);
  setStateDoubleFromAlgo(Parameters::DefaultColorA);
  setStateDoubleFromAlgo(Parameters::Radius);
  setStateBoolFromAlgo(Parameters::UseNormals);
  setStateBoolFromAlgo(Parameters::ShowEdges);
  setStateIntFromAlgo(Parameters::ModuleID);
}

ShowFieldWithOspray::ShowFieldWithOspray() : Module(staticInfo_)
{

  INITIALIZE_PORT(Field);
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(SceneGraph);

  static uint32_t lastMoudleID = 0;
  id = ++lastMoudleID;
}

void ShowFieldWithOspray::execute()
{
  auto field = getRequiredInput(Field);
  auto colorMap = getOptionalInput(ColorMapObject);



  if (needToExecute())
  {


    setAlgoDoubleFromState(Parameters::DefaultColorR);
    setAlgoDoubleFromState(Parameters::DefaultColorG);
    setAlgoDoubleFromState(Parameters::DefaultColorB);
    setAlgoDoubleFromState(Parameters::DefaultColorA);
    setAlgoDoubleFromState(Parameters::Radius);
    setAlgoBoolFromState(Parameters::UseNormals);
    setAlgoBoolFromState(Parameters::ShowEdges);

    // this is mostly for user feedback for now.
    auto state = get_state();
    FieldInformation info(field);
    if (info.is_curvemesh())
    {
      state->setValue(Parameters::ShowEdges, true);
    }
    else if (info.is_pointcloudmesh())
    {
      state->setValue(Parameters::ShowEdges, false);
    }

    state->setValue(Parameters::ModuleID, static_cast<int>(id));
    setAlgoIntFromState(Parameters::ModuleID);

    auto output = algo().run(withInputData((Field, field)(ColorMapObject, optionalAlgoInput(colorMap))));
    sendOutputFromAlgorithm(SceneGraph, output);
  }
}
