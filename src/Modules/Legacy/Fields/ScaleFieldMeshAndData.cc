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
/// @todo Documentation Modules/Legacy/Fields/ScaleFieldMeshAndData.cc

#include <Modules/Legacy/Fields/ScaleFieldMeshAndData.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Scalar.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Legacy/Fields/TransformMesh/ScaleFieldMeshAndData.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

const ModuleLookupInfo ScaleFieldMeshAndData::staticInfo_("ScaleFieldMeshAndData", "ChangeMesh", "SCIRun");

ScaleFieldMeshAndData::ScaleFieldMeshAndData()
  : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(GeomScaleFactor);
  INITIALIZE_PORT(DataScaleFactor);
  INITIALIZE_PORT(OutputField);
}

void ScaleFieldMeshAndData::setStateDefaults()
{
  auto state = get_state();
  setStateDoubleFromAlgo(Parameters::data_scale);
  setStateDoubleFromAlgo(Parameters::mesh_scale);
  setStateBoolFromAlgo(Parameters::scale_from_center);
}

void ScaleFieldMeshAndData::execute()
{
  auto input = getRequiredInput(InputField);
  auto dataScale = getOptionalInput(DataScaleFactor);
  auto geomScale = getOptionalInput(GeomScaleFactor);
  
  if (needToExecute())
  {
    update_state(Executing);

    if (dataScale && *dataScale) 
    {
      double scale = (*dataScale)->value();
      get_state()->setValue(Parameters::data_scale, scale);
    }

    if (geomScale && *geomScale)
    {
      double scale = (*geomScale)->value();
      get_state()->setValue(Parameters::mesh_scale, scale);
    }

    setAlgoDoubleFromState(Parameters::data_scale);
    setAlgoDoubleFromState(Parameters::mesh_scale);
    setAlgoBoolFromState(Parameters::scale_from_center);
    auto output = algo().run_generic(withInputData((InputField, input)));
    sendOutputFromAlgorithm(OutputField, output);
  }
}
