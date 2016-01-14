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
   
   author: Moritz Dannhauer, ported from SCIRun4
   last changes: 11/26/13
*/

#include <Core/Algorithms/Legacy/Fields/FieldData/CalculateGradientsAlgo.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Containers/StackVector.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

bool
CalculateGradientsAlgo::run(FieldHandle input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "CalculateGradients");
  
  if (!input)
    THROW_ALGORITHM_INPUT_ERROR("No input field");
  
  FieldInformation fi(input);

  if (fi.is_pointcloudmesh())
    THROW_ALGORITHM_INPUT_ERROR("Cannot calculate gradients for a point cloud");

  if (fi.is_nodata())
    THROW_ALGORITHM_INPUT_ERROR("Input field does not have data associated with it");

  if (!(fi.is_scalar()))
    THROW_ALGORITHM_INPUT_ERROR("The data needs to be of scalar type to calculate gradients");

  fi.make_vector();
  
  fi.make_constantdata();
  
  output = CreateField(fi,input->mesh());
  
  if (!output)
    THROW_ALGORITHM_INPUT_ERROR("Could not allocate output field");
  
  VField* ifield = input->vfield();
  
  VField* ofield = output->vfield();
  
  VMesh*  imesh  = input->vmesh();
  
  ofield->resize_values();
  
  VMesh::coords_type coords;
  
  imesh->get_element_center(coords);
  
  VField::size_type num_elems = imesh->num_elems();
  
  VField::size_type num_nodes = imesh->num_nodes();
  
  VField::size_type num_fielddata = ifield->num_values();

  if ((num_fielddata != num_nodes) && (num_fielddata != num_elems))
    THROW_ALGORITHM_INPUT_ERROR("Input data inconsistent");

  int cnt = 0;
  StackVector<double, 3> grad;
  for (VMesh::Elem::index_type idx = 0; idx < num_elems; ++idx)
  {
    ifield->gradient(grad, coords, idx); /** Segmentation Fault Here **/

    Vector v(grad[0], grad[1], grad[2]);

    ofield->set_value(v, idx);

    cnt++;
    if (cnt == 400)
    {
      cnt = 0;
      update_progress_max(idx, num_elems);
    }
  }
  return (true);
}

const AlgorithmInputName CalculateGradientsAlgo::ScalarField("ScalarField");
const AlgorithmOutputName CalculateGradientsAlgo::VectorField("VectorField");
AlgorithmOutput CalculateGradientsAlgo::run_generic(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(ScalarField);

  FieldHandle gradient;
  if (!run(field, gradient))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[VectorField] = gradient;
  return output;
}
