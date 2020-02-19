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


#include <Modules/Legacy/Bundle/InsertMatricesIntoBundle.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::Bundles;
using namespace SCIRun::Core::Datatypes;

MODULE_INFO_DEF(InsertMatricesIntoBundle, Bundle, SCIRun)

const AlgorithmParameterName InsertMatricesIntoBundle::NumMatrices("NumMatrices");
const AlgorithmParameterName InsertMatricesIntoBundle::MatrixNames("MatrixNames");
const AlgorithmParameterName InsertMatricesIntoBundle::MatrixReplace("MatrixReplace");

InsertMatricesIntoBundle::InsertMatricesIntoBundle() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputBundle);
  INITIALIZE_PORT(OutputBundle);
  INITIALIZE_PORT(InputMatrices);
}

void InsertMatricesIntoBundle::setStateDefaults()
{
  get_state()->setValue(MatrixReplace, VariableList());
}

void InsertMatricesIntoBundle::execute()
{
  auto bundleOption = getOptionalInput(InputBundle);
  auto matrices = getRequiredDynamicInputs(InputMatrices);

  if (needToExecute())
  {
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
    auto replace = get_state()->getValue(MatrixReplace).toVector();
    auto iPorts = inputPorts();
    if (matrices.size() != iPorts.size() - 2)
      warning("Problem in state of dynamic ports");
    auto matrixPortNameIterator = iPorts.begin() + 1; // bundle port is first
    auto state = get_state();

    for (int i = 0; i < matrices.size(); ++i)
    {
      auto matrix = matrices[i];
      auto stateName = state->getValue(Name((*matrixPortNameIterator++)->id().toString())).toString();
      if (matrix)
      {
        auto name = !stateName.empty() ? stateName : ("matrix" + boost::lexical_cast<std::string>(i));
        auto replaceMatrix = i < replace.size() ? replace[i].toBool() : true;
        if (replaceMatrix || !bundle->isMatrix(name))
        {
          bundle->set(name, matrix);
        }
      }
    }

    sendOutput(OutputBundle, bundle);
  }
}
