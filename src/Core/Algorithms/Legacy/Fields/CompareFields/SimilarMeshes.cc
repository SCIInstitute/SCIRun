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


#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Util/StringUtil.h>

#include <Core/Algorithms/Fields/CompareFields/SimilarMeshes.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool SimilarMeshesAlgo::run(std::vector<FieldHandle> field_input_handles)
{
  algo_start("SimilarMeshes");

  if (field_input_handles.size() == 0)
  {
    // If we encounter a null pointer we return an error message and return to
    // the program to deal with this error.
    error("No input fields");
    algo_end(); return (false);
  }

  for( size_t i=0; i<field_input_handles.size(); i++ )
  {
    FieldHandle fHandle = field_input_handles[i];

    if (fHandle.get_rep() == 0)
    {
      // If we encounter a null pointer we return an error message and
      // return to the program to deal with this error.
      error("No input field");
      algo_end(); return (false);
    }
  }


  // Make sure each field has some data to transform.
  for( size_t i=0; i<field_input_handles.size(); i++ )
  {
    FieldHandle fHandle = field_input_handles[i];

    if (fHandle->basis_order() == -1)
    {
      error("Field " + to_string(i) + " contains no data to transform.");
      algo_end(); return (false);
    }
  }

  FieldHandle fHandle0 = field_input_handles[0];

  bool same_rep = true;

  // Make sure each field has the same basis and mesh.
  for( size_t i=1; i<field_input_handles.size(); i++ )
  {
    FieldHandle fHandle = field_input_handles[i];

    // Are the basis orders the same?
    if (fHandle0->basis_order() != fHandle->basis_order())
    {
      error("The Input Fields must share the same data location.");
      algo_end(); return (false);
    }

    // Are the meshes the same?
    if (fHandle0->mesh().get_rep() != fHandle->mesh().get_rep() )
      same_rep = false;
  }

  // If the meshes are not the same make sure they are the same type
  // and have the same number of nodes and elements.
  if( !same_rep )
  {
    // Same mesh types?
    for( size_t i=1; i<field_input_handles.size(); i++ )
    {
      FieldHandle fHandle = field_input_handles[i];
      if( fHandle0->get_type_description(Field::MESH_TD_E)->get_name() !=
          fHandle->get_type_description(Field::MESH_TD_E)->get_name() )
      {
        error("The input fields must have the same mesh type.");
        algo_end(); return (false);
      }
    }

    VMesh::size_type num_nodes0 = fHandle0->vmesh()->num_nodes();
    VMesh::size_type num_elems0 = fHandle0->vmesh()->num_elems();

    // Same number of nodes and elements?
    for( size_t i=1; i<field_input_handles.size(); i++ )
    {
      FieldHandle fHandle = field_input_handles[i];

      VMesh::size_type num_nodes = fHandle->vmesh()->num_nodes();
      VMesh::size_type num_elems = fHandle->vmesh()->num_elems();

      if( num_nodes0 != num_nodes )
      {
        error("The input meshes do not have the same number of nodes.");
        algo_end(); return (false);
      }

      if( num_elems0 != num_elems )
      {
        error("The input meshes do not have the same number of elements.");
        algo_end(); return (false);
      }
    }

    warning("The input fields do not have the same mesh,");
    warning("but appear to be the same otherwise.");
  }

  algo_end(); return (true);
}


} // End namespace SCIRunAlgo
