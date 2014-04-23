/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
   Author            : Moritz Dannhauer
   Last modification : March 22 2014 (ported from SCIRun4)
   TODO: Nrrd aoutput
*/

#include <Core/Algorithms/Legacy/Fields/FieldData/MapFieldDataFromNodeToElem.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun;


MapFieldDataFromNodeToElemAlgo::MapFieldDataFromNodeToElemAlgo()
{
  add_option(Method,"interpolate","average|min|max|sum|median|none");
}

AlgorithmInputName MapFieldDataFromNodeToElemAlgo::InputField("InputField");
AlgorithmOutputName MapFieldDataFromNodeToElemAlgo::OutputField("OutputField");
AlgorithmParameterName MapFieldDataFromNodeToElemAlgo::Method("Method");

AlgorithmOutput MapFieldDataFromNodeToElemAlgo::run_generic(const AlgorithmInput& input) const
{
  auto input_field = input.get<Field>(InputField);
  
  FieldHandle output_field;
 // output_field = run(input_field);
  
  AlgorithmOutput output;
  output[OutputField] = output_field;

  return output;
}

//! Function call to convert data from Field into Matrix data
FieldHandle MapFieldDataFromNodeToElemAlgo::run(FieldHandle input_field) const
{   
   FieldHandle output;
 
   
   return output;
}
