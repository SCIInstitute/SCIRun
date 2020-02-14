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


#include <Core/Algorithms/Legacy/Fields/TransformMesh/TransformMeshWithTransform.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

TransformMeshWithTransformAlgo::TransformMeshWithTransformAlgo()
{
//  add_bool("rotate_data", true);
}

bool TransformMeshWithTransformAlgo::run(FieldHandle input, DenseMatrixHandle transform_matrix, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "TransformMeshWithTransform");

  const bool rotate_data = true;  //  get_bool("rotate_data"); //No gui so this is always constant

  if (!input)
  {
    error("No input field");
    return (false);
  }

  if (!transform_matrix)
  {
    error("No input transform matrix");
    return (false);
  }

  if (!(transform_matrix->ncols() == 4 && transform_matrix->nrows() == 4))
  {
    error("Input matrix needs to be a 4 by 4 dense matrix");
    return (false);
  }

  output.reset(input->deep_clone());

  VMesh* vmesh = output->vmesh();
  VField* vfield = output->vfield();

  Transform transform;
  transform.set(transform_matrix->data());

  vmesh->transform(transform);

  if (vfield->is_vector() || vfield->is_tensor())
  {
    if (rotate_data)
    {
      if (vfield->is_vector())
      {
        VMesh::size_type sz = vfield->num_values();
        for (VMesh::index_type i=0; i < sz; i++)
        {
          Vector v;
          vfield->get_value(v,i);
          v = transform*v;
          vfield->set_value(v,i);
        }
      }
      if (vfield->is_tensor())
      {
        VMesh::size_type sz = vfield->num_values();
        for (VMesh::index_type i=0; i < sz; i++)
        {
          Tensor v;
          vfield->get_value(v,i);
          v = transform*v*transform;
          vfield->set_value(v,i);
        }
      }
    }
  }

  CopyProperties(*input, *output);

  return (true);
}

const AlgorithmInputName TransformMeshWithTransformAlgo::TransformMatrix("TransformMatrix");
const AlgorithmOutputName TransformMeshWithTransformAlgo::Transformed_Field("Transformed_Field");

AlgorithmOutput TransformMeshWithTransformAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  auto transform = input.get<DenseMatrix>(TransformMatrix);

  FieldHandle outputField;
  if (!run(inputField, transform, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Transformed_Field] = outputField;
  return output;
}
