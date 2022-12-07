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


#include <Modules/Legacy/Bundle/InsertStringsIntoBundle.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/String.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::Bundles;
using namespace SCIRun::Core::Datatypes;

MODULE_INFO_DEF(InsertStringsIntoBundle, Bundle, SCIRun)

const AlgorithmParameterName InsertStringsIntoBundle::NumStrings("NumStrings");
const AlgorithmParameterName InsertStringsIntoBundle::StringNames("StringNames");
const AlgorithmParameterName InsertStringsIntoBundle::StringReplace("StringReplace");

InsertStringsIntoBundle::InsertStringsIntoBundle() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputBundle);
  INITIALIZE_PORT(OutputBundle);
  INITIALIZE_PORT(InputStrings);
}

void InsertStringsIntoBundle::setStateDefaults()
{
  get_state()->setValue(StringReplace, VariableList());
}

void InsertStringsIntoBundle::execute()
{
  auto bundleOption = getOptionalInput(InputBundle);
  auto strings = getRequiredDynamicInputs(InputStrings);

  if (needToExecute())
  {
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    std::string bundlename = guibundlename_.get();
#endif
    BundleHandle bundle;
    if (bundleOption && *bundleOption)
    {
      bundle.reset((*bundleOption)->clone());
    }
    else
    {
      bundle.reset(new Bundle());
      if (!bundle)
      {
        THROW_ALGORITHM_PROCESSING_ERROR("Could not allocate new bundle");
      }
    }

    //TODO: instead grab a vector of tuple<string,bool>. need to modify Variable::Value again
    auto replace = get_state()->getValue(StringReplace).toVector();
    auto iPorts = inputPorts();
    if (strings.size() != iPorts.size() - 2)
      warning("Problem in state of dynamic ports");
    auto stringPortNameIterator = iPorts.begin() + 1; // bundle port is first
    auto state = get_state();

    for (int i = 0; i < strings.size(); ++i)
    {
      auto str = strings[i];
      auto stateName = state->getValue(Name((*stringPortNameIterator++)->internalId().toString())).toString();
      if (str)
      {
        auto name = !stateName.empty() ? stateName : ("string" + boost::lexical_cast<std::string>(i));
        auto replaceStr = i < replace.size() ? replace[i].toBool() : true;
        if (replaceStr || !bundle->isString(name))
        {
          bundle->set(name, str);
        }
      }
    }

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    if (bundlename != "")
    {
      handle->set_property("name", bundlename, false);
    }
#endif

    sendOutput(OutputBundle, bundle);
  }
}
