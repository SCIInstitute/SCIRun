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


#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldDataToConstantValue.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms::Fields::Parameters;
using namespace SCIRun::Core::Datatypes;

ALGORITHM_PARAMETER_DEF(Fields, BasisOrder);
ALGORITHM_PARAMETER_DEF(Fields, DataType);
ALGORITHM_PARAMETER_DEF(Fields, Value);

SetFieldDataToConstantValueAlgo::SetFieldDataToConstantValueAlgo()
{
  //! keep scalar type defines whether we convert to double or not
  addOption(DataType, "same as input", "char|unsigned char|short|unsigned short|int|unsigned int|float|double|same as input");
  addOption(BasisOrder, "same as input", "nodata|constant|linear|quadratic|same as input");
  addParameter(Value, 0.0);
}

bool
SetFieldDataToConstantValueAlgo::runImpl(FieldHandle input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "SetFieldDataToConstantValue");
  if (!input)
  {
    error("No input field was provided");
    return (false);
  }

  FieldInformation fi(input);

  std::string data_type = getOption(DataType);
  if (data_type != "same as input")
  {
    fi.set_data_type_by_string(data_type);
  }
  else if (! fi.is_scalar() )
  {
    fi.make_double();
  }

  std::string basis_order = getOption(BasisOrder);
  if (basis_order != "same as input")
  {
    fi.set_basis_type(basis_order);
  }

  output = CreateField(fi, input->mesh());

  if (!output)
  {
    error("Could not allocate output field");
    return (false);
  }

  double new_value = get(Value).toDouble();

  output->vfield()->resize_values();
  output->vfield()->set_all_values(new_value);

  return (true);
}

AlgorithmOutput SetFieldDataToConstantValueAlgo::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);

  FieldHandle outputField;
  if (!runImpl(field, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;
  return output;
}
