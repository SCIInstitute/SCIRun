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

#include <Core/Algorithms/Legacy/Fields/FieldData/ConvertFieldDataType.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

ALGORITHM_PARAMETER_DEF(Fields, FieldDatatype);

ConvertFieldDataTypeAlgo::ConvertFieldDataTypeAlgo()
{
  add_option(Parameters::FieldDatatype, "double", "char|unsigned char|short|unsigned short|int|unsigned int|float|double|Vector|Tensor");
}

bool ConvertFieldDataTypeAlgo::runImpl(FieldHandle input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "ConvertFieldData");
  
  if (!input)
  {
    error("No input field");
    return (false);
  }
  
  /// Get the information of the input field
  FieldInformation fo(input);
  
  std::string datatype = get_option(Parameters::FieldDatatype);
  
  fo.set_data_type(datatype);
  
  output = CreateField(fo,input->mesh());
  
  if (!output)
  {
    error("Could no create output field");
    return (false);
  }
  
  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  
  ofield->resize_values();
  ofield->copy_values(ifield);
  CopyProperties(*input, *output);
  
  /// Support for quadratic fields
  if (ofield->basis_order() == 2)
  {
    ofield->copy_evalues(ifield);
  }
  
  return (true);
}

AlgorithmOutput ConvertFieldDataTypeAlgo::run_generic(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);

  FieldHandle outputField;
  if (!runImpl(field, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;
  return output;
}