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


#include <Modules/Legacy/Fields/ResampleRegularMesh.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>

#include <Core/Algorithms/Legacy/Fields/ResampleMesh/ResampleRegularMesh.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

/// @class ResampleRegularMesh
/// @brief Resample a regular mesh, such as a LatVol.

MODULE_INFO_DEF(ResampleRegularMesh, ChangeMesh, SCIRun)

ResampleRegularMesh::ResampleRegularMesh() :
  Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
}

void ResampleRegularMesh::setStateDefaults()
{
  auto state = get_state();

  setStateStringFromAlgoOption(Parameters::ResampleMethod);
  setStateDoubleFromAlgo(Parameters::ResampleGaussianSigma);
  setStateDoubleFromAlgo(Parameters::ResampleGaussianExtend);
  setStateDoubleFromAlgo(Parameters::ResampleXDim);
  setStateDoubleFromAlgo(Parameters::ResampleYDim);
  setStateDoubleFromAlgo(Parameters::ResampleZDim);
  setStateBoolFromAlgo(Parameters::ResampleXDimUseScalingFactor);
  setStateBoolFromAlgo(Parameters::ResampleYDimUseScalingFactor);
  setStateBoolFromAlgo(Parameters::ResampleZDimUseScalingFactor);
}

void ResampleRegularMesh::setDimensionParameter(const std::string& name, const AlgorithmParameterName& dim, const AlgorithmParameterName& useScaling)
{
  auto state = get_state();
  auto dimValue = state->getValue(dim).toDouble();
  if (state->getValue(useScaling).toBool())
  {
    if (dimValue <= 0)
    {
      error(name + " axis has an incorrect factor");
      return;
    }
  }
  else
  {
    if (dimValue < 2)
    {
      error(name + " axis has an incorrect dimension");
      return;
    }
  }
  setAlgoDoubleFromState(dim);
  setAlgoBoolFromState(useScaling);
}

void
ResampleRegularMesh::execute()
{
  auto input = getRequiredInput(InputField);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (inputs_changed_ || method_.changed() || sigma_.changed() ||
      extend_.changed() || xaxis_.changed() || yaxis_.changed() ||
      zaxis_.changed() || !oport_cached("Field"))
#endif
  if (needToExecute())
  {
    auto state = get_state();

    setAlgoOptionFromState(Parameters::ResampleMethod);
    setAlgoDoubleFromState(Parameters::ResampleGaussianSigma);
    setAlgoDoubleFromState(Parameters::ResampleGaussianExtend);

    setDimensionParameter("X", Parameters::ResampleXDim, Parameters::ResampleXDimUseScalingFactor);
    setDimensionParameter("Y", Parameters::ResampleYDim, Parameters::ResampleYDimUseScalingFactor);
    setDimensionParameter("Z", Parameters::ResampleZDim, Parameters::ResampleZDimUseScalingFactor);

    auto output = algo().run(withInputData((InputField, input)));

    sendOutputFromAlgorithm(OutputField, output);
  }
}
