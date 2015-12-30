
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

#include <Modules/Legacy/Bundle/GetMatricesFromBundle.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/Matrix.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Bundles;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

const ModuleLookupInfo GetMatricesFromBundle::staticInfo_("GetMatricesFromBundle", "Bundle", "SCIRun");
const AlgorithmParameterName GetMatricesFromBundle::MatrixNameList("MatrixNameList");
const AlgorithmParameterName GetMatricesFromBundle::MatrixNames[] = {
  AlgorithmParameterName("matrix1-name"),
  AlgorithmParameterName("matrix2-name"),
  AlgorithmParameterName("matrix3-name"),
  AlgorithmParameterName("matrix4-name"),
  AlgorithmParameterName("matrix5-name"),
  AlgorithmParameterName("matrix6-name")
};

GetMatricesFromBundle::GetMatricesFromBundle() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputBundle);
  INITIALIZE_PORT(OutputBundle);
  INITIALIZE_PORT(matrix1);
  INITIALIZE_PORT(matrix2);
  INITIALIZE_PORT(matrix3);
  INITIALIZE_PORT(matrix4);
  INITIALIZE_PORT(matrix5);
  INITIALIZE_PORT(matrix6);
}

void GetMatricesFromBundle::setStateDefaults()
{
  auto state = get_state();

  for (int i = 0; i < NUM_BUNDLE_OUT; ++i)
  {
    state->setValue(MatrixNames[i], "matrix" + boost::lexical_cast<std::string>(i));
  }
}

void GetMatricesFromBundle::execute()
{
  auto bundle = getRequiredInput(InputBundle);

  if (needToExecute())
  {
    update_state(Executing);

    auto state = get_state();
    state->setTransientValue(MatrixNameList.name(), bundle->getMatrixNames());

    MatrixHandle outputs[NUM_BUNDLE_OUT];
    for (int i = 0; i < NUM_BUNDLE_OUT; ++i)
    {
      auto matrixName = state->getValue(MatrixNames[i]).toString();
      if (bundle->isMatrix(matrixName))
      {
        auto matrix = bundle->getMatrix(matrixName);
        outputs[i] = matrix;
      }
    }

    sendOutput(OutputBundle, bundle);

    //TODO: fix duplication
    if (outputs[0])
      sendOutput(matrix1, outputs[0]);
    if (outputs[1])
      sendOutput(matrix2, outputs[1]);
    if (outputs[2])
      sendOutput(matrix3, outputs[2]);
    if (outputs[3])
      sendOutput(matrix4, outputs[3]);
    if (outputs[4])
      sendOutput(matrix5, outputs[4]);
    if (outputs[5])
      sendOutput(matrix6, outputs[5]);
  }
}

std::string GetMatricesFromBundle::makeMatrixNameList(const Bundle& bundle) const
{
  auto matrixNames = bundle.getMatrixNames();
  std::ostringstream vars;
  std::copy(matrixNames.begin(), matrixNames.end(), std::ostream_iterator<std::string>(vars, "\n"));
  return vars.str();
}
