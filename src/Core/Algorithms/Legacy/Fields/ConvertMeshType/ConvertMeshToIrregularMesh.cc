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


#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToIrregularMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/PropertyManagerExtensions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;

bool
ConvertMeshToIrregularMeshAlgo::runImpl(FieldHandle input, FieldHandle& output) const
{
  // Mark that we are starting the algorithm, but do not report progress
  ScopedAlgorithmStatusReporter asr(this, "ConvertMeshToIrregularMesh");
  // Step 0:
  // Safety test:
  // Test whether we received actually a field. A handle can point to no object.
  // Using a null handle will cause the program to crash. Hence it is a good
  // policy to check all incoming handles and to see whether they point to actual
  // objects.

  if (!input)
  {
    // If we encounter a null pointer we return an error message and return to
    // the program to deal with this error.
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
    error("This function has not yet been defined for non-linear elements yet");
    return (false);
  }

  // If the mesh is already irregular, we only need to copy the input to the
  // output. No algorithm is needed in this case.
  if (fi.is_irregularmesh())
  {
    // Notify the user that no action is done
    remark("The input mesh already is irregular; copying input to output");
    // Copy input to output (output is a reference to the input)
    output = input;
    return (true);
  }

  // If mesh is unstructured we have no good method to make it structured again
  if (fi.is_unstructuredmesh())
  {
    error("Cannot convert an unstructured mesh into a structured mesh");
    return (false);
  }

  // Depending on the element figure out what the output type needs to be
  if (fi.is_hex_element())
  {
    fo.make_structhexvolmesh();
  }
  else if (fi.is_quad_element())
  {
    fo.make_structquadsurfmesh();
  }
  else if (fi.is_crv_element())
  {
    fo.make_structcurvemesh();
  }
  else
  {
    error("No method available for mesh: " + fi.get_mesh_type());
    return (false);
  }

  VField* ifield = input->vfield();
  VMesh*  imesh  = input->vmesh();

   // Set the output dims.
  VMesh::dimension_type dims;
  imesh->get_dimensions(dims);

  MeshHandle outputmesh = CreateMesh(fo,dims);
  if (!outputmesh)
  {
    error("Could not create output mesh");
    return (false);
  }

  output = CreateField(fo,outputmesh);
  if (!output)
  {
    error("Could not create output field");
    return (false);
  }

  VField* ofield = output->vfield();
  VMesh*  omesh  = output->vmesh();

  // Actual algorithm starts here:

  // Synchronize makes sure that all function calls to retrieve the elements and
  // nodes are working. Some mesh types need this.
  imesh->synchronize(Mesh::NODES_E|Mesh::ELEMS_E);

  // Define iterators over the nodes
  VMesh::Node::iterator bn, en;
  VMesh::Node::size_type numnodes;

  imesh->begin(bn); // get begin iterator
  imesh->end(en);   // get end iterator

  // Iterate over all nodes and copy each node
  while (bn != en)
  {
    Point point;
    imesh->get_center(point, *bn);
    omesh->set_point(point, *bn);
    ++bn;
  }

  // Make sure Fdata matches the size of the number of nodes
  ofield->resize_values();

  // Copy data whether it is on the node or the element.
  if (ifield->basis_order() != -1) ofield->copy_values(ifield);

  CopyProperties(*input, *output);

  // Success:
  return (true);
}

AlgorithmOutput ConvertMeshToIrregularMeshAlgo::run(const AlgorithmInput& input) const
{
  auto ifield = input.get<Field>(Variables::InputField);

  FieldHandle ofield;
  if (!runImpl(ifield, ofield))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = ofield;
  return output;
}
