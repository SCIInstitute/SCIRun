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


#include <Modules/Legacy/Fields/CalculateDistanceToFieldBoundary.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateDistanceField.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundaryAlgo.h>

/// @class CalculateDistanceToFieldBoundary
/// @brief Calculate the distance field to a boundary of a given mesh.

//    SCIRunAlgo::CalculateDistanceFieldAlgo algo_;
//    SCIRunAlgo::GetFieldBoundaryAlgo falgo_;

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

MODULE_INFO_DEF(CalculateDistanceToFieldBoundary, ChangeFieldData, SCIRun)

CalculateDistanceToFieldBoundary::CalculateDistanceToFieldBoundary()
  : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(DistanceField);
  INITIALIZE_PORT(ValueField);
}

void CalculateDistanceToFieldBoundary::setStateDefaults()
{
  setStateBoolFromAlgo(Parameters::Truncate);
  setStateDoubleFromAlgo(Parameters::TruncateDistance);
  setStateStringFromAlgoOption(Parameters::BasisType);
  setStateStringFromAlgoOption(Parameters::OutputFieldDatatype);
}

void
CalculateDistanceToFieldBoundary::execute()
{
  auto input = getRequiredInput(InputField);
  FieldHandle object;

  bool value_connected = oport_connected(ValueField);

  if (needToExecute())
//  if (inputs_changed_ || !oport_cached("DistanceField") ||
//      (!oport_cached("ValueField") && value_connected))
  {
    setAlgoBoolFromState(Parameters::Truncate);
    setAlgoDoubleFromState(Parameters::TruncateDistance);
    setAlgoOptionFromState(Parameters::BasisType);
    setAlgoOptionFromState(Parameters::OutputFieldDatatype);

    //TODO: set up secondary algorithm
    GetFieldBoundaryAlgo falgo;
    if (!falgo.run(input,object))
      return;

    auto inputs = make_input((InputField, input));
    inputs[Variables::ObjectField] = object;

    algo().set(Parameters::OutputValueField, value_connected);
    auto output = algo().run(inputs);
    sendOutputFromAlgorithm(DistanceField, output);

    if (value_connected)
    {
      sendOutputFromAlgorithm(ValueField, output);
    }
  }
}
