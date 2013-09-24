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

#include <Core/Algorithms/Legacy/Fields/TransformMesh/AlignMeshBoundingBoxes.h>

#include <Core/GeometryPrimitives/Transform.h>

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>

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
AlignMeshBoundingBoxesAlgo::
run(FieldHandle input, FieldHandle object, 
    FieldHandle& output, MatrixHandle& transform_matrix)
{
  // Mark that we are starting the algorithm, but do not report progress
  algo_start("AlignMeshBoundingBoxes");
  
  bool rotate_data = get_bool("rotate_data");
  // Step 0:
  // Safety test:
  // Test whether we received actually a field. A handle can point to no object.
  // Using a null handle will cause the program to crash. Hence it is a good
  // policy to check all incoming handles and to see whether they point to actual
  // objects.
  
  // Handle: the function get_rep() returns the pointer contained in the handle
  if (input.get_rep() == 0)
  {
    // If we encounter a null pointer we return an error message and return to
    // the program to deal with this error. 
    error("No input field");
    algo_end(); return (false);
  }

  // Copy the field
  output = input;
  output.detach();
  output->mesh_detach();
 
  BBox obbox = object->vmesh()->get_bounding_box();
  BBox ibbox = input->vmesh()->get_bounding_box();
  
  SCIRun::Transform transform;
 
  Vector iscale = ibbox.diagonal();
  Vector oscale = obbox.diagonal();
  Vector itrans = -ibbox.min().asVector();
  Vector otrans = obbox.min().asVector();
  transform.pre_translate(itrans);
  transform.pre_scale(Vector(oscale.x()/iscale.x(),oscale.y()/iscale.y(),oscale.z()/iscale.z()));
  transform.pre_translate(otrans);
  
  output->vmesh()->transform(transform);

  VField* field = output->vfield();
  if (field->is_vector() || field->is_tensor())
  {
    if (rotate_data == true)
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

  transform_matrix = new DenseMatrix(transform);
  
  if (transform_matrix.get_rep() == 0)
  {
    error("Could not allocate transform matrix");
    algo_end(); return (false);  
  }
  
  //! Copy properties of the property manager
	output->copy_properties(input.get_rep());
   
  // Success:
  algo_end(); return (true);
}
