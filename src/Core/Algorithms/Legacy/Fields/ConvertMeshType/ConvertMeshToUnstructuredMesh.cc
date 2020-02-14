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


// Get all the class definitions.
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToUnstructuredMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/PropertyManagerExtensions.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;

bool ConvertMeshToUnstructuredMeshAlgo::runImpl(FieldHandle input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "ConvertMeshToUnstructuredMesh");

  // Null check
  if (!input)
  {
    error("No input field");
    return (false);
  }

 // Step 1: determine the type of the input fields and determine what type the
  // output field should be.

  // FieldInformation is a helper class that will store all the names of all the
  // components a field is made of. It takes a handle to a field and then
  // determines what the actual type is of the field.

  // As the current Field class has a variety of functions to query for its type
  // the FieldInformation object will do this for you and will contain a summary of all
  // the type information.

  // As the output field will be a variation on the input field we initialize
  // both with the input handle.
  FieldInformation fi(input);
  FieldInformation fo(input);

  // Recent updates to the software allow for quadratic and cubic hermitian
  // representations. However these methods have not fully been exposed yet.
  // Hence the iterators in the field will not consider the information needed
  // to define these non-linear elements. And hence although the algorithm may
  // provide output for these cases and may not fail, the output is mathematically
  // improper and hence for a proper implementation we have to wait until the
  // mesh and field classes are fully completed.

  // Here we test whether the class is part of any of these newly defined
  // non-linear classes. If so we return an error.
   if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    return (false);
  }

  // If the mesh is already unstructured, we only need to copy the input to the
  // output. No algorithm is needed in this case.
  if (fi.is_unstructuredmesh())
  {
    // Notify the user that no action is done
    remark("Mesh already is unstructured; copying input to output");
    // Copy input to output (output is a reference to the input)
    output = input;
    return (true);
  }

  // Define the output type of the data
  if (fi.is_hex_element())
  {
    fo.make_hexvolmesh();
  }
  else if (fi.is_quad_element())
  {
    fo.make_quadsurfmesh();
  }
  else if (fi.is_crv_element())
  {
    fo.make_curvemesh();
  }
  else
  {
    error("No unstructure method available for mesh: " + fi.get_mesh_type());
    return (false);
  }

  // Create a new output field
  output = CreateField(fo);

  if (!output)
  {
    // Error reporting:
    // we forward the specific message to the ProgressReporter and return a
    // false to indicate that an error has occured.
    error("Could not obtain input field");
    return (false);
  }

  // Get the virtual interface of the objects
  // These two calls get the interfaces to the input and output field
  VField* ifield = input->vfield();
  VField* ofield = output->vfield();

  // These two calls get the interfaces to the meshes
  VMesh*  imesh = input->vmesh();
  VMesh*  omesh = output->vmesh();

  // Get the number of nodes and elements
  VMesh::size_type num_nodes = imesh->num_nodes();
  VMesh::size_type num_elems = imesh->num_elems();

  // Reserve space
  omesh->reserve_nodes(num_nodes);
  omesh->reserve_elems(num_elems);

  // Copy all nodes
  for (VMesh::Node::index_type i=0; i<num_nodes; i++)
  {
    Point p;
    imesh->get_center(p,i);
    omesh->add_node(p);
  }

  // Copy all elements
  VMesh::Node::array_type nodes;
  for (VMesh::Elem::index_type i=0; i<num_elems; i++)
  {
    imesh->get_nodes(nodes,i);
    omesh->add_elem(nodes);
  }

  ofield->resize_values();
  ofield->copy_values(ifield);

  CopyProperties(*input, *output);

  return (true);
}

AlgorithmOutput ConvertMeshToUnstructuredMeshAlgo::run(const AlgorithmInput& input) const
{
  auto ifield = input.get<Field>(Variables::InputField);

  FieldHandle ofield;
  if (!runImpl(ifield, ofield))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = ofield;
  return output;
}
