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


#include <Core/Algorithms/Legacy/Fields/TransformMesh/AlignMeshBoundingBoxes.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Transform.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/PropertyManagerExtensions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

AlgorithmParameterName AlignMeshBoundingBoxesAlgo::RotateData("rotate_data");

AlignMeshBoundingBoxesAlgo::AlignMeshBoundingBoxesAlgo()
{
  addParameter(RotateData, true);
}

bool
AlignMeshBoundingBoxesAlgo::run(FieldHandle input, FieldHandle object, FieldHandle& output, MatrixHandle& transform_matrix) const
{
  ScopedAlgorithmStatusReporter asr(this, "AlignMeshBoundingBoxes");

  bool rotate_data = get(RotateData).toBool();
  // Step 0:
  // Safety test:
  // Test whether we received actually a field. A handle can point to no object.
  // Using a null handle will cause the program to crash. Hence it is a good
  // policy to check all incoming handles and to see whether they point to actual
  // objects.

  if (!input)
  {
    error("No input field");
    return (false);
  }

  // Copy the field
  output.reset(input->deep_clone());

  BBox obbox = object->vmesh()->get_bounding_box();
  BBox ibbox = input->vmesh()->get_bounding_box();

  Transform transform;

  Vector iscale = ibbox.diagonal();
  Vector oscale = obbox.diagonal();
  Vector itrans(-ibbox.get_min());
  Vector otrans(obbox.get_min());
  transform.pre_translate(itrans);
  transform.pre_scale(Vector(oscale.x()/iscale.x(),oscale.y()/iscale.y(),oscale.z()/iscale.z()));
  transform.pre_translate(otrans);

  output->vmesh()->transform(transform);

  VField* field = output->vfield();
  if (field->is_vector() || field->is_tensor())
  {
    if (rotate_data)
    {
      if (field->is_vector())
      {
        VMesh::size_type sz = field->num_values();
        for (VMesh::index_type i=0; i < sz; i++)
        {
          Vector v;
          field->get_value(v,i);
          v = transform*v;
          field->set_value(v,i);
        }
      }
      if (field->is_tensor())
      {
        VMesh::size_type sz = field->num_values();
        for (VMesh::index_type i=0; i < sz; i++)
        {
          Tensor v;
          field->get_value(v,i);
          v = transform*v*transform;
          field->set_value(v,i);
        }
      }
    }
  }

  transform_matrix.reset(new DenseMatrix(transform));

  if (!transform_matrix)
  {
    error("Could not allocate transform matrix");
    return (false);
  }

  CopyProperties(*input, *output);

  // Success:
  return (true);
}

AlgorithmInputName AlignMeshBoundingBoxesAlgo::AlignmentField("AlignmentField");
AlgorithmOutputName AlignMeshBoundingBoxesAlgo::OutputField("OutputField");
AlgorithmOutputName AlignMeshBoundingBoxesAlgo::TransformMatrix("TransformMatrix");

AlgorithmOutput AlignMeshBoundingBoxesAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  auto object = input.get<Field>(AlignmentField);

  FieldHandle outputField;
  MatrixHandle transform;
  if (!run(inputField, object, outputField, transform))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[OutputField] = outputField;
  output[TransformMatrix] = transform;
  return output;
}
