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


#ifndef CORE_ALGORITHMS_FIELDS_REFINEMESH_REFINEMESHCOMMON_H
#define CORE_ALGORITHMS_FIELDS_REFINEMESH_REFINEMESHCOMMON_H

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>

#include <string>
#include <vector>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Fields {

/// Classify each mesh node as selected (true) or not (false) based on the
/// field values and the selection criterion.  Also populates ivalues with
/// the field values used for data interpolation on refined nodes.
inline bool computeRefinementMask(
    VField* field, VMesh* mesh,
    const std::string& select, double isoval,
    std::vector<bool>& values,
    std::vector<double>& ivalues,
    const AlgorithmBase& algo)
{
  VMesh::size_type num_nodes = mesh->num_nodes();
  VMesh::size_type num_elems = mesh->num_elems();
  values.assign(num_nodes, false);

  if (field->basis_order() == 0)
  {
    field->get_values(ivalues);
    VMesh::Node::array_type onodes;
    if (select == "equal")
    {
      for (VMesh::Elem::index_type i = 0; i < num_elems; i++)
      {
        mesh->get_nodes(onodes, i);
        if (ivalues[i] == isoval)
          for (size_t j = 0; j < onodes.size(); j++)
            values[onodes[j]] = true;
      }
    }
    else if (select == "lessthan")
    {
      for (VMesh::Elem::index_type i = 0; i < num_elems; i++)
      {
        mesh->get_nodes(onodes, i);
        if (ivalues[i] < isoval)
          for (size_t j = 0; j < onodes.size(); j++)
            values[onodes[j]] = true;
      }
    }
    else if (select == "greaterthan")
    {
      for (VMesh::Elem::index_type i = 0; i < num_elems; i++)
      {
        mesh->get_nodes(onodes, i);
        if (ivalues[i] > isoval)
          for (size_t j = 0; j < onodes.size(); j++)
            values[onodes[j]] = true;
      }
    }
    else if (select == "all")
    {
      for (size_t j = 0; j < values.size(); j++) values[j] = true;
    }
    else
    {
      algo.error("RefineMesh: Unknown region selection method encountered");
      return false;
    }
  }
  else if (field->basis_order() == 1)
  {
    field->get_values(ivalues);
    if (select == "equal")
    {
      for (VMesh::Elem::index_type i = 0; i < num_nodes; i++)
      {
        if (ivalues[i] == isoval) values[i] = true;
      }
    }
    else if (select == "lessthan")
    {
      for (VMesh::Elem::index_type i = 0; i < num_nodes; i++)
      {
        if (ivalues[i] < isoval) values[i] = true;
      }
    }
    else if (select == "greaterthan")
    {
      for (VMesh::Elem::index_type i = 0; i < num_nodes; i++)
      {
        if (ivalues[i] > isoval) values[i] = true;
      }
    }
    else if (select == "all")
    {
      for (size_t j = 0; j < values.size(); j++) values[j] = true;
    }
    else
    {
      algo.error("RefineMesh: Unknown region selection method encountered");
      return false;
    }
  }
  else
  {
    for (size_t j = 0; j < values.size(); j++) values[j] = true;
  }
  return true;
}

}}}}

#endif
