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
  ScopedAlgorithmStatusReporter asr(this, "CalculateVectorMagnitudes");
  if (!input)
  {
    error("No input field");
    return (false);
  }
 
  FieldInformation fi(input);
  
  if (fi.is_nodata())
  {
    error("Input field does not have data associated with it");
    return (false);    
  }
  
  if (!fi.is_vector())
  {
    error("The data needs to be of vector type to calculate vector magnitudes");
    return (false);    
  }
  
  //check if number of field matches number of nodes,face or cell elements ????
  
  fi.make_scalar();
  fi.make_constantdata();
  output = CreateField(fi,input->mesh());
  
  if (!output)
  {
    error("Could not allocate output field");
    return (false);      
  }
  
  StackVector<double,3> grad;
  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  VMesh*  imesh  = input->vmesh();
  
  VField::size_type num_elems = imesh->num_elems();
  VField::size_type num_nodes = imesh->num_nodes();
  VField::size_type num_fielddata = ifield->num_values();
  
  if ( num_fielddata!=num_nodes &&  num_fielddata!=num_elems)
  {
    error("Input data inconsistent");
    return (false);
  }
  
  Vector* vec = reinterpret_cast<Vector*>(ifield->get_values_pointer());
  double* mag = reinterpret_cast<double*>(ofield->get_values_pointer());
  
  if (!vec)
  {
   error("Could not acces input field pointer");
   return (false); 
  }
  
  if (!mag)
  {
   error("Could not acces output field pointer");
   return (false); 
  }
  
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

AlgorithmOutput CalculateVectorMagnitudesAlgo::run_generic(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(VectorField);

  FieldHandle vectormagnitude;
  if (!run(field, vectormagnitude))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[ScalarField] = vectormagnitude;
  return output;
}
