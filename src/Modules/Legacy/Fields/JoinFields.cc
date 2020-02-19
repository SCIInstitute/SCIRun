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


#include <Modules/Legacy/Fields/JoinFields.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Legacy/Fields/MergeFields/JoinFieldsAlgo.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToPointCloudMeshAlgo.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;

MODULE_INFO_DEF(JoinFields, NewField, SCIRun)

const AlgorithmParameterName JoinFields::ForcePointCloud("ForcePointCloud");

JoinFields::JoinFields() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputFields);
  INITIALIZE_PORT(OutputField);
}

void JoinFields::setStateDefaults()
{
  setStateBoolFromAlgo(JoinFieldsAlgo::MergeElems);
  setStateBoolFromAlgo(JoinFieldsAlgo::MergeNodes);
  setStateBoolFromAlgo(JoinFieldsAlgo::MatchNodeValues);
  setStateBoolFromAlgo(JoinFieldsAlgo::MakeNoData);
  setStateDoubleFromAlgo(JoinFieldsAlgo::Tolerance);
  get_state()->setValue(ForcePointCloud, false);
}

void JoinFields::execute()
{
  auto fields = getRequiredDynamicInputs(InputFields);

  if (needToExecute())
  {
    bool forcepointcloud = get_state()->getValue(ForcePointCloud).toBool();

    setAlgoBoolFromState(JoinFieldsAlgo::MergeElems);
    setAlgoBoolFromState(JoinFieldsAlgo::MatchNodeValues);
    setAlgoBoolFromState(JoinFieldsAlgo::MergeNodes);
    setAlgoBoolFromState(JoinFieldsAlgo::MakeNoData);
    setAlgoDoubleFromState(JoinFieldsAlgo::Tolerance);

    auto output = algo().run(withInputData((InputFields, fields)));
    auto outputField = output.get<Field>(Core::Algorithms::AlgorithmParameterName(OutputField));

    if (forcepointcloud)
    {
      ConvertMeshToPointCloudMeshAlgo calgo;
      calgo.runImpl(outputField, outputField);
    }

    sendOutput(OutputField, outputField);
  }
}
