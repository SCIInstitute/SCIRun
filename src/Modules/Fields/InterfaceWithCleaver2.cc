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


#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Scalar.h>
#include <Modules/Fields/InterfaceWithCleaver2.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Field/InterfaceWithCleaver2Algorithm.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

MODULE_INFO_DEF(InterfaceWithCleaver2, NewField, SCIRun)

InterfaceWithCleaver2::InterfaceWithCleaver2() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputFields);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(SizingField);
  INITIALIZE_PORT(BackgroundField);
  INITIALIZE_PORT(SizingFieldUsed);
  INITIALIZE_PORT(BackgroundFieldUsed);
}

void InterfaceWithCleaver2::setStateDefaults()
{
  setStateBoolFromAlgo(Parameters::Verbose);
  setStateBoolFromAlgo(Parameters::SimpleMode);
  setStateBoolFromAlgo(Parameters::ReverseJacobians);
  setStateDoubleFromAlgo(Parameters::Lipschitz);
  setStateDoubleFromAlgo(Parameters::AlphaLong);
  setStateDoubleFromAlgo(Parameters::AlphaShort);
  setStateDoubleFromAlgo(Parameters::VolumeScaling);
  setStateDoubleFromAlgo(Parameters::VolumeMultiplier);
  setStateIntFromAlgo(Parameters::MeshMode);
}

void InterfaceWithCleaver2::execute()
{
  auto fields = getRequiredDynamicInputs(InputFields);
  auto sizing = getOptionalInput(SizingField);
  auto background = getOptionalInput(BackgroundField);

  if (needToExecute())
  {
    setAlgoBoolFromState(Parameters::Verbose);
    setAlgoBoolFromState(Parameters::SimpleMode);
    setAlgoBoolFromState(Parameters::ReverseJacobians);
    setAlgoDoubleFromState(Parameters::Lipschitz);
    setAlgoDoubleFromState(Parameters::AlphaLong);
    setAlgoDoubleFromState(Parameters::AlphaShort);
    setAlgoDoubleFromState(Parameters::VolumeScaling);
    setAlgoDoubleFromState(Parameters::VolumeMultiplier);
    setAlgoIntFromState(Parameters::MeshMode);

    auto output = algo().run(withInputData((InputFields, fields)
      (SizingField, optionalAlgoInput(sizing))
      (BackgroundField, optionalAlgoInput(background))
    ));

    sendOutputFromAlgorithm(OutputField, output);
    sendOutputFromAlgorithm(SizingFieldUsed, output);
    sendOutputFromAlgorithm(BackgroundFieldUsed, output);
  }
}
