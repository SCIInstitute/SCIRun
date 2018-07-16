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

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Field/CalculateMeshCenterAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/GeometryPrimitives/Vector.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;

ALGORITHM_PARAMETER_DEF(Fields, Method);
ALGORITHM_PARAMETER_DEF(Fields, SamplingScheme);
ALGORITHM_PARAMETER_DEF(Fields, ChangeOutsideValue);
ALGORITHM_PARAMETER_DEF(Fields, OutsideValue);
ALGORITHM_PARAMETER_DEF(Fields, StartValue);
ALGORITHM_PARAMETER_DEF(Fields, OutputType);
ALGORITHM_PARAMETER_DEF(Fields, DataLocation);

CalculateInsideWhichFieldAlgorithm::CalculateInsideWhichFieldAlgorithm()
{
  // set parameters defaults UI
  addOption(Parameters::Method,"one","one|most|all");
  addOption(Parameters::SamplingScheme,"regular1","regular1|regular2|regular3|regular4|regular5");
  addOption(Parameters::ChangeOutsideValue,"true|false");
  addParameters(Parameters::OutsideValue, 0.0);
  addParameters(Parameters::StartValue, 1.0);
  addOption(Parameters::OutputType, "same as input", "same as input|char|short|unsigned short|unsigned int| int|float|double");
  addOption(Parameters::DataLocation, "element", "element|node");
}


AlgorithmOutput CalculateMeshCenterAlgorithm::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  FieldHandle output;
  
  //pull parameter from UI
  std::string method=getOption(Parameters::Method);
  
  
  //Safety Check
  if(!inputField)
  {
    error("No input field");
    
    output=nullptr;
    AlgorithmOutput result;
    result[Variables::OutputField] = output ;
    return result;
  }
  
  
  
  AlgorithmOutput result;
  result[Variables::OutputField] = output ;
  return result;
}
