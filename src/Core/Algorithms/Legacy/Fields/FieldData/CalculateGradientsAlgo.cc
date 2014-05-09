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

int debug_level = 1;

bool
CalculateGradientsAlgo::run(FieldHandle input, FieldHandle& output) const
{
  if (debug_level>0) std::cout << "Entered algo.run" << std::endl; //
  
  ScopedAlgorithmStatusReporter asr(this, "CalculateGradients");
  
  if (!input)
    THROW_ALGORITHM_INPUT_ERROR("No input field");
  if (debug_level>0) std::cout << "passed no input field check" << std::endl; //
  
  FieldInformation fi(input);
  if (debug_level>0) std::cout << "passed fi(input)" << std::endl; //

  if (fi.is_pointcloudmesh())
    THROW_ALGORITHM_INPUT_ERROR("Cannot calculate gradients for a point cloud");
  if (debug_level>0) std::cout << "passed is_pointcloudmesh check" << std::endl; //

  if (fi.is_nodata())
    THROW_ALGORITHM_INPUT_ERROR("Input field does not have data associated with it");
  if (debug_level>0) std::cout << "passed is_nodata check" << std::endl; //

  if (!(fi.is_scalar()))
    THROW_ALGORITHM_INPUT_ERROR("The data needs to be of scalar type to calculate gradients");
  if (debug_level>0) std::cout << "passed is_scalar check" << std::endl; //

  fi.make_vector();
  if (debug_level>0) std::cout << "passed maker_vector()" << std::endl; //
  
  fi.make_constantdata();
  if (debug_level>0) std::cout << "passed make_constantdata()" << std::endl; //
  
  output = CreateField(fi,input->mesh());
  if (debug_level>0) std::cout << "output set to CreateField(fi,input->mesh())" << std::endl; //

  if (!output)
    THROW_ALGORITHM_INPUT_ERROR("Could not allocate output field");
  if (debug_level>0) std::cout << "passed output allocation check" << std::endl; //

  VField* ifield = input->vfield();
  if (debug_level>0) std::cout << "passed ifield = input->vfield" << std::endl; //
  
  VField* ofield = output->vfield();
  if (debug_level>0) std::cout << "passed ofield = output->vfield" << std::endl; //
  
  VMesh*  imesh  = input->vmesh();
  if (debug_level>0) std::cout << "passed imesh = input->vmesh" << std::endl; //
  
  ofield->resize_values();
  if (debug_level>0) std::cout << "passed ofield->resize values" << std::endl; //
  
  VMesh::coords_type coords;
  if (debug_level>0) std::cout << "passed VMesh::coords_type coords" << std::endl; //
  
  imesh->get_element_center(coords);
  if (debug_level>0) std::cout << "passed imesh->get_element_center(coords)" << std::endl; //
  
  VField::size_type num_elems = imesh->num_elems();
  if (debug_level>0) std::cout << "passed num elems = imesh->num elemes" << std::endl; //
  
  VField::size_type num_nodes = imesh->num_nodes();
  if (debug_level>0) std::cout << "passed num nodes = imesh->num nodes" << std::endl; //
  
  VField::size_type num_fielddata = ifield->num_values();
  if (debug_level>0) std::cout << "passed num fielddata = ifields->num values" << std::endl; //
  
  if ((num_fielddata != num_nodes) && (num_fielddata != num_elems))
    THROW_ALGORITHM_INPUT_ERROR("Input data inconsistent");
  if (debug_level>0) std::cout << "passed data inconsistent check" << std::endl; //
  
  int cnt = 0;
  int debug_idx = 0; //
  StackVector<double,3> grad;
  for (VMesh::Elem::index_type idx = 0; idx < num_elems; idx++)
  {
    if (debug_level>0) std::cout << "inside for loop at debug iteration = " << debug_idx << std::endl; //
    
    ifield->gradient(grad, coords, idx); /** Segmentation Fault Here **/
    if (debug_level>0) std::cout << "passed ifield->gradient(grad,coords,idx)" << std::endl; //
    
    Vector v(grad[0],grad[1],grad[2]);
    if (debug_level>0) std::cout << "passed v(grad[0],grad[1],grad[2])" << std::endl; //
    
    ofield->set_value(v,idx);
    if (debug_level>0) std::cout << "passed ofield->set_value(v,idx)" << std::endl; //
    if (debug_level>0) debug_idx++; //
    
    cnt++;
    if (cnt == 400) 
    {
        cnt = 0; 
        update_progress_max(idx,num_elems); 
    } 
  }
  return (true);
}

AlgorithmInputName CalculateGradientsAlgo::ScalarField("ScalarField");
AlgorithmOutputName CalculateGradientsAlgo::VectorField("VectorField");
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
