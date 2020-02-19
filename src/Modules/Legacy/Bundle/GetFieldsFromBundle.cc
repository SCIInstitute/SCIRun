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


#include <Modules/Legacy/Bundle/GetFieldsFromBundle.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Bundles;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

MODULE_INFO_DEF(GetFieldsFromBundle, Bundle, SCIRun)

const AlgorithmParameterName GetFieldsFromBundle::FieldNameList("FieldNameList");

const AlgorithmParameterName GetFieldsFromBundle::FieldNames[] = {
  AlgorithmParameterName("field1-name"),
  AlgorithmParameterName("field2-name"),
  AlgorithmParameterName("field3-name"),
  AlgorithmParameterName("field4-name"),
  AlgorithmParameterName("field5-name"),
  AlgorithmParameterName("field6-name")
};

GetFieldsFromBundle::GetFieldsFromBundle() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputBundle);
  INITIALIZE_PORT(OutputBundle);
  INITIALIZE_PORT(field1);
  INITIALIZE_PORT(field2);
  INITIALIZE_PORT(field3);
  INITIALIZE_PORT(field4);
  INITIALIZE_PORT(field5);
  INITIALIZE_PORT(field6);
}

void GetFieldsFromBundle::setStateDefaults()
{
  auto state = get_state();

  for (int i = 0; i < NUM_BUNDLE_OUT; ++i)
  {
    state->setValue(FieldNames[i], "field" + boost::lexical_cast<std::string>(i));
  }
}

void GetFieldsFromBundle::execute()
{
  auto bundle = getRequiredInput(InputBundle);

  if (needToExecute())
  {
    auto state = get_state();
    state->setTransientValue(FieldNameList.name(), bundle->getFieldNames());

    FieldHandle outputs[NUM_BUNDLE_OUT];
    for (int i = 0; i < NUM_BUNDLE_OUT; ++i)
    {
      auto fieldName = state->getValue(FieldNames[i]).toString();
      if (bundle->isField(fieldName))
      {
        auto field = bundle->getField(fieldName);
        outputs[i] = field;
      }
    }

    sendOutput(OutputBundle, bundle);

    //TODO: fix duplication
    if (outputs[0])
      sendOutput(field1, outputs[0]);
    if (outputs[1])
      sendOutput(field2, outputs[1]);
    if (outputs[2])
      sendOutput(field3, outputs[2]);
    if (outputs[3])
      sendOutput(field4, outputs[3]);
    if (outputs[4])
      sendOutput(field5, outputs[4]);
    if (outputs[5])
      sendOutput(field6, outputs[5]);
  }
}

std::string GetFieldsFromBundle::makeFieldNameList(const Bundle& bundle) const
{
  auto fieldNames = bundle.getFieldNames();
  std::ostringstream vars;
  std::copy(fieldNames.begin(), fieldNames.end(), std::ostream_iterator<std::string>(vars, "\n"));
  return vars.str();
}
