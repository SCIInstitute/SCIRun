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


#include <Modules/Legacy/Fields/CalculateInsideWhichField.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Scalar.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateInsideWhichFieldAlgorithm.h>

#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoNodes.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/ConvertFieldBasisType.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateIsInsideField.h>


using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

MODULE_INFO_DEF(CalculateInsideWhichField, NewField, SCIRun)

CalculateInsideWhichField::CalculateInsideWhichField() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(InputFields);

  INITIALIZE_PORT(OutputField);
}

void CalculateInsideWhichField::setStateDefaults()
{
  setStateStringFromAlgoOption(Parameters::Method);
  setStateStringFromAlgoOption(Parameters::SamplingScheme);
  setStateStringFromAlgoOption(Parameters::ChangeOutsideValue);
  setStateDoubleFromAlgo(Parameters::OutsideValue);
  setStateDoubleFromAlgo(Parameters::StartValue);
  setStateStringFromAlgoOption(Parameters::OutputType);
  setStateStringFromAlgoOption(Parameters::DataLocation);
}

void CalculateInsideWhichField::execute()
{
  auto field=getRequiredInput(InputField);
  auto fields = getRequiredDynamicInputs(InputFields);

  if (needToExecute())
  {
    setAlgoOptionFromState(Parameters::Method);
    setAlgoOptionFromState(Parameters::SamplingScheme);
    setAlgoOptionFromState(Parameters::ChangeOutsideValue);
    setAlgoDoubleFromState(Parameters::OutsideValue);
    setAlgoDoubleFromState(Parameters::StartValue);
    setAlgoOptionFromState(Parameters::OutputType);
    setAlgoOptionFromState(Parameters::DataLocation);

    auto output = algo().run(withInputData((InputField, field)(InputFields, fields)));

    sendOutputFromAlgorithm(OutputField,output);
  }
}
