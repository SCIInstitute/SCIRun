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


#include <Modules/Legacy/Fields/CalculateSignedDistanceToField.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateSignedDistanceField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;

CalculateSignedDistanceToField::CalculateSignedDistanceToField()
  : Module(ModuleLookupInfo("CalculateSignedDistanceToField", "ChangeFieldData", "SCIRun"), false)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(ObjectField);
  INITIALIZE_PORT(SignedDistanceField);
  INITIALIZE_PORT(ValueField);
}

void CalculateSignedDistanceToField::execute()
{
  FieldHandle input = getRequiredInput(InputField);
  FieldHandle object = getRequiredInput(ObjectField);

  bool value_connected = oport_connected(ValueField);

  if (needToExecute())
  {
    auto inputs = make_input((InputField, input)(ObjectField, object));

    algo().set(CalculateSignedDistanceFieldAlgo::OutputValueField, value_connected);
    auto output = algo().run(inputs);

    sendOutputFromAlgorithm(SignedDistanceField, output);

    if (value_connected)
    {
      sendOutputFromAlgorithm(ValueField, output);
    }
  }
}
