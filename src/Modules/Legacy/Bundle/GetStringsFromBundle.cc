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


#include <Modules/Legacy/Bundle/GetStringsFromBundle.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Bundles;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

/// @class GetStringsFromBundle
/// @brief This module retrieves a string object from a bundle.

MODULE_INFO_DEF(GetStringsFromBundle, Bundle, SCIRun)

const AlgorithmParameterName GetStringsFromBundle::StringNameList("StringNameList");

const AlgorithmParameterName GetStringsFromBundle::StringNames[] = {
  AlgorithmParameterName("string1-name"),
  AlgorithmParameterName("string2-name"),
  AlgorithmParameterName("string3-name"),
  AlgorithmParameterName("string4-name"),
  AlgorithmParameterName("string5-name"),
  AlgorithmParameterName("string6-name")
};

GetStringsFromBundle::GetStringsFromBundle() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputBundle);
  INITIALIZE_PORT(OutputBundle);
  INITIALIZE_PORT(string1);
  INITIALIZE_PORT(string2);
  INITIALIZE_PORT(string3);
  INITIALIZE_PORT(string4);
  INITIALIZE_PORT(string5);
  INITIALIZE_PORT(string6);
}

void GetStringsFromBundle::setStateDefaults()
{
  auto state = get_state();

  for (int i = 0; i < NUM_BUNDLE_OUT; ++i)
  {
    state->setValue(StringNames[i], "string" + boost::lexical_cast<std::string>(i));
  }
}

void
GetStringsFromBundle::execute()
{
  auto bundle = getRequiredInput(InputBundle);

  if (needToExecute())
  {
    auto state = get_state();
    state->setTransientValue(StringNameList.name(), bundle->getStringNames());

    StringHandle outputs[NUM_BUNDLE_OUT];
    for (int i = 0; i < NUM_BUNDLE_OUT; ++i)
    {
      auto stringName = state->getValue(StringNames[i]).toString();
      if (bundle->isString(stringName))
      {
        auto string_tmp = bundle->getString(stringName);
        outputs[i] = string_tmp;
      }
    }

    sendOutput(OutputBundle, bundle);

    //TODO: fix duplication
    if (outputs[0])
      sendOutput(string1, outputs[0]);
    if (outputs[1])
      sendOutput(string2, outputs[1]);
    if (outputs[2])
      sendOutput(string3, outputs[2]);
    if (outputs[3])
      sendOutput(string4, outputs[3]);
    if (outputs[4])
      sendOutput(string5, outputs[4]);
    if (outputs[5])
      sendOutput(string6, outputs[5]);
  }
}

std::string GetStringsFromBundle::makeStringNameList(const Bundle& bundle) const
{
  auto stringNames = bundle.getStringNames();
  std::ostringstream vars;
  std::copy(stringNames.begin(), stringNames.end(), std::ostream_iterator<std::string>(vars, "\n"));
  return vars.str();
}
