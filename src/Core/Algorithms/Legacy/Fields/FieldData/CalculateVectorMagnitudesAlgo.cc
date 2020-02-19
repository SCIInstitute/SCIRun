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

   Author:          Moritz Dannhauer, Spencer Frisby
   Last Modified:   May 2014
*/


#include <Core/Algorithms/Legacy/Fields/FieldData/CalculateVectorMagnitudesAlgo.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

bool
CalculateVectorMagnitudesAlgo::run(FieldHandle input, FieldHandle& output) const
{
  if (!input)
    THROW_ALGORITHM_INPUT_ERROR("No input field");

  FieldInformation fi(input);

  if (fi.is_nodata())
    THROW_ALGORITHM_INPUT_ERROR("Input field does not have data associated with it");

  if (!fi.is_vector())
    THROW_ALGORITHM_INPUT_ERROR("The data needs to be vector type to calculate vector magnitudes");

  fi.make_scalar();
  fi.make_constantdata();
  output = CreateField(fi,input->mesh());
  if (!output)
    THROW_ALGORITHM_PROCESSING_ERROR("Could not allocate output field");

  StackVector<double,3> grad;
  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  VMesh*  imesh  = input->vmesh();

  VField::size_type num_elems = imesh->num_elems();
  VField::size_type num_nodes = imesh->num_nodes();
  VField::size_type num_fielddata = ifield->num_values();

  if (num_fielddata!=num_nodes &&  num_fielddata!=num_elems)
    THROW_ALGORITHM_INPUT_ERROR("Input data inconsistent");

  Vector* vec = reinterpret_cast<Vector*>(ifield->get_values_pointer());
  double* mag = reinterpret_cast<double*>(ofield->get_values_pointer());

  if (!vec)
   THROW_ALGORITHM_INPUT_ERROR("Could not acces input field pointer");

  if (!mag)
   THROW_ALGORITHM_INPUT_ERROR("Could not access output field pointer");

  int cnt = 0;
  for (VMesh::Elem::index_type idx = 0; idx < num_elems; idx++)
  {
   mag[idx] = vec[idx].length();
   cnt++;
   if (cnt == 400)
      {
        cnt = 0;
        update_progress_max(idx,num_elems);
      }
  }
  return (true);
}

AlgorithmOutputName CalculateVectorMagnitudesAlgo::ScalarField("ScalarField");
AlgorithmInputName CalculateVectorMagnitudesAlgo::VectorField("VectorField");

AlgorithmOutput CalculateVectorMagnitudesAlgo::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(VectorField);

  FieldHandle vectormagnitude;
  if (!run(field, vectormagnitude))
    THROW_ALGORITHM_PROCESSING_ERROR("An input or processing error has occurred.");

  AlgorithmOutput output;
  output[ScalarField] = vectormagnitude;
  return output;
}
