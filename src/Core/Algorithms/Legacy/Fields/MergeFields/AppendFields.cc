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


/// Class definition of this one
#include <Core/Algorithms/Fields/MergeFields/AppendFields.h>

/// Need to find out what type of field we are dealing with
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

bool
AppendFieldsAlgo::run(std::list<FieldHandle>& input, FieldHandle& output)
{
  std::vector<FieldHandle> handles(input.size());
  std::list<FieldHandle>::iterator it = input.begin();
  for (size_t j=0; j<handles.size(); j++, ++it) handles[j] = *it;

  return (run(handles,output));
}

bool
AppendFieldsAlgo::run(std::vector<FieldHandle>& input, FieldHandle& output)
{
  // Mark that we are starting the algorithm
  algo_start("AppendFields");

  std::vector<Field*> inputs;
  for(size_t p=0; p < input.size(); p++)
  {
    if (input[p].get_rep())
    {
      inputs.push_back(input[p].get_rep());
    }
  }

  if (inputs.size() == 1)
  {
    output = inputs[0];
    algo_end(); return (true);
  }

  if (inputs.size() == 0)
  {
    error("No input fields given");
    algo_end(); return (false);
  }

  // Check whether mesh types are the same
  FieldInformation first(inputs[0]);

  if (!(first.is_unstructuredmesh()))
  {
    error("This algorithm only works on unstructured data");
    algo_end(); return (false);
  }

  if (first.is_nonlinearmesh())
  {
    error("This algorithm does not work for non-linear fields");
    algo_end(); return (false);
  }
  /// Make sure mesh and mesh basis order are equal

  VMesh*  mesh  = inputs[0]->vmesh();
  VField* field = inputs[0]->vfield();
  size_type num_nodes  = mesh->num_nodes();
  size_type num_elems  = mesh->num_elems();
  size_type num_values = field->num_values();

  for (size_t p=1; p<inputs.size(); p++)
  {
    FieldInformation fi(inputs[p]);
    if (fi != first)
    {
      error("The fields that are appended are not equal in type");
      algo_end(); return (false);
    }
    mesh  = inputs[p]->vmesh();
    field = inputs[p]->vfield();

    num_nodes  += mesh->num_nodes();
    num_elems  += mesh->num_elems();
    num_values += field->num_values();
  }

  output = CreateField(first);

  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return (false);
  }


  VMesh*  omesh  = output->vmesh();
  VField* ofield = output->vfield();

  omesh->resize_nodes(num_nodes);
  omesh->resize_elems(num_elems);
  ofield->resize_values();

  VMesh::index_type node_offset = 0;
  VMesh::index_type elem_offset = 0;
  VField::index_type value_offset = 0;

  for (size_t j=0; j<inputs.size(); j++)
  {
    VField* ifield = inputs[j]->vfield();
    VMesh* imesh = inputs[j]->vmesh();

    VMesh::size_type nnodes = imesh->num_nodes();
    VMesh::size_type nelems = imesh->num_elems();
    VMesh::size_type nvalues = ifield->num_values();

    omesh->copy_nodes(imesh,0,node_offset,nnodes);
    omesh->copy_elems(imesh,0,elem_offset,nelems,node_offset);
    ofield->copy_values(ifield,0,value_offset,nvalues);

    node_offset += nnodes;
    elem_offset += nelems;
    value_offset += nvalues;
  }

  algo_end(); return (true);
}

} // end namespace SCIRunAlgo
