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


#include <Core/Algorithms/Legacy/Fields/Cleanup/RemoveUnusedNodes.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

AlgorithmInputName RemoveUnusedNodesAlgo::InputField("InputField");
AlgorithmInputName RemoveUnusedNodesAlgo::OutputField("OutputField");

AlgorithmOutput RemoveUnusedNodesAlgo::run(const AlgorithmInput& input) const
{

  auto inputField = input.get<Field>(InputField);

  FieldHandle out;

  if (!run(inputField, out))
       THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

 AlgorithmOutput output;
 output[OutputField] = out;

  return output;
}

bool RemoveUnusedNodesAlgo::run(FieldHandle input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "RemoveUnusedNodes");
  // Step 0:
  // Safety test:
  // Test whether we received actually a field. A handle can point to no object.
  // Using a null handle will cause the program to crash. Hence it is a good
  // policy to check all incoming handles and to see whether they point to actual
  // objects.

  // Handle: the function returns the pointer contained in the handle
  if (!input)
  {
    error("No input source field");
    return (false);
  }

  // Step 1: determine the type of the input fields and determine what type the
  // output field should be.
  FieldInformation fi(input);
  FieldInformation fo(input);

  // Here we test whether the class is part of any of these newly defined
  // non-linear classes. If so we return an error.
  if (fi.is_nonlinear())
  {
    error("This algorithm has not yet been defined for non-linear elements yet");
    return (false);
  }

  // This one
  if (!(fi.is_unstructuredmesh()))
  {
    // Notify the user that no action is done
    error("This algorithm only works on a unstructured mesh");
    // Copy input to output (output is a reference to the input)
    return (false);
  }

  VField* ifield = input->vfield();
  VMesh*  imesh  = input->vmesh();

  output = CreateField(fo);
  if (!output)
  {
    error("Could not allocate output field");
    return (false);
  }

   VMesh* omesh = output->vmesh();
  VField* ofield = output->vfield();

  VMesh::size_type num_nodes = imesh->num_nodes();
  VMesh::size_type num_elems = imesh->num_elems();

  std::vector<VMesh::Node::index_type> mapping(num_nodes,-1);
  std::vector<bool> used(num_nodes,false);

  VMesh::Node::array_type nodes;
  for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
  {
    imesh->get_nodes(nodes,idx);
    for (size_t j=0; j<nodes.size(); j++) used[nodes[j]] = true;
  }

  for (VMesh::Node::index_type idx=0; idx<num_nodes;idx++)
  {
    Point p;
    if (used[idx])
    {
      imesh->get_center(p,idx);
      mapping[idx] = omesh->add_point(p);
    }
  }

  for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
  {
    imesh->get_nodes(nodes,idx);
    for (size_t j=0; j<nodes.size(); j++)
    {
      nodes[j] = mapping[nodes[j]];
    }
    omesh->add_elem(nodes);
  }

  ofield->resize_values();

  if (ofield->basis_order() == 0)
  {
    ofield->copy_values(ifield);
  }
  else if (ofield->basis_order() == 1)
  {
    for (VMesh::Node::index_type idx=0; idx<num_nodes;idx++)
    {
      if (mapping[idx] >= 0)
        ofield->copy_value(ifield,idx,mapping[idx]);
    }
  }

  return true;
}
