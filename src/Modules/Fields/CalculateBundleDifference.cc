/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#include <Modules/Fields/CalculateBundleDifference.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

MODULE_INFO_DEF(CalculateBundleDifference, MiscField, SCIRun)

CalculateBundleDifference::CalculateBundleDifference()
  : Module(staticInfo_)
{
  INITIALIZE_PORT(bundle1);
  INITIALIZE_PORT(bundle2);
  INITIALIZE_PORT(seed);
  INITIALIZE_PORT(bundleOut1);
  INITIALIZE_PORT(bundleOut2);
}

void CalculateBundleDifference::setStateDefaults()
{
  setStateStringFromAlgoOption(Variables::Method);
}

void CalculateBundleDifference::execute()
{
  auto b1 = getRequiredInput(bundle1);
  auto b2 = getRequiredInput(bundle2);
  auto bSeed = getRequiredInput(seed);

  if (needToExecute())
  {
    setAlgoOptionFromState(Variables::Method);
    auto output = algo().run(withInputData((bundle1, b1)(bundle2, b2)(seed, bSeed)));
    sendOutputFromAlgorithm(bundleOut1, output);
    sendOutputFromAlgorithm(bundleOut2, output);
  }
}
