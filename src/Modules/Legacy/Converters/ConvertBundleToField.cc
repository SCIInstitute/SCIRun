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


// File:   ConvertBundleToField.cc
// Author: Fangxiang Jiao
// Date:   March 25 2010

#include <Modules/Legacy/Converters/ConvertBundleToField.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Legacy/Fields/MergeFields/JoinFieldsAlgo.h>
#include <Core/Algorithms/Legacy/Converter/ConvertBundleToField.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToPointCloudMeshAlgo.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Converters;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Converters;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

#if 0
class ConvertBundleToField : public Module {

public:
  ConvertBundleToField(GuiContext*);
  virtual ~ConvertBundleToField() {}
  virtual void execute();

private:
    GuiInt    guiclear_;
    GuiDouble guitolerance_;
    GuiInt    guimergenodes_;
    GuiInt    guiforcepointcloud_;
    GuiInt    guimatchval_;
    GuiInt    guimeshonly_;

    SCIRunAlgo::ConvertBundleToFieldAlgo algo_;
    SCIRunAlgo::ConvertMeshToPointCloudMeshAlgo calgo_;
};
#endif

MODULE_INFO_DEF(ConvertBundleToField, Converters, SCIRun)

ConvertBundleToField::ConvertBundleToField() :
  Module(staticInfo_)
{
  INITIALIZE_PORT(InputBundle);
  INITIALIZE_PORT(OutputField);
}

void ConvertBundleToField::setStateDefaults()
{
  auto state = get_state();
  setStateBoolFromAlgo(Parameters::MergeNodes);
  setStateBoolFromAlgo(Parameters::MatchNodeValues);
  setStateBoolFromAlgo(Parameters::MakeNoData);
  setStateDoubleFromAlgo(Parameters::Tolerance);
  state->setValue(Fields::Parameters::ForcePointCloud, false);
}

void ConvertBundleToField::execute()
{
  auto bundle = getRequiredInput(InputBundle);

  if (needToExecute())
  {
    setAlgoBoolFromState(Parameters::MakeNoData);
    setAlgoBoolFromState(Parameters::MergeNodes);
    setAlgoBoolFromState(Parameters::MatchNodeValues);
    setAlgoDoubleFromState(Parameters::Tolerance);

    auto output = algo().run(withInputData((InputBundle, bundle)));
    auto outputField = output.get<Field>(Core::Algorithms::AlgorithmParameterName(OutputField));

    // This option is here to be compatible with the old GatherFields module:
    // This is a separate algorithm now
    if (get_state()->getValue(Fields::Parameters::ForcePointCloud).toBool())
    {
      Fields::ConvertMeshToPointCloudMeshAlgo calgo;
      calgo.runImpl(outputField, outputField);
    }

    sendOutput(OutputField, outputField);
  }
}
