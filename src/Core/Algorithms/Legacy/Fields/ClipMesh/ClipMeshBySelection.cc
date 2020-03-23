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


#include <Core/Algorithms/Legacy/Fields/ClipMesh/ClipMeshBySelection.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Logging/Log.h>

#include <algorithm>
#include <set>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

ALGORITHM_PARAMETER_DEF(Fields, ClipMethod);
ALGORITHM_PARAMETER_DEF(Fields, BuildMapping);

ClipMeshBySelectionAlgo::ClipMeshBySelectionAlgo()
{
  addOption(Parameters::ClipMethod, "Element Center", "Element Center|One Node|Most Nodes|All Nodes");
  addParameter(Parameters::BuildMapping, true);
}

// Version without building mapping matrix
bool
ClipMeshBySelectionAlgo::runImpl(FieldHandle input,
                             FieldHandle selection,
                             FieldHandle& output) const
{
  // Store old setting
  bool temp = get(Parameters::BuildMapping).toBool();
  //TODO: yuck
  const_cast<ClipMeshBySelectionAlgo&>(*this).set(Parameters::BuildMapping, false);
  MatrixHandle dummy;
  // Run full algorithm
  bool ret = runImpl(input, selection, output, dummy);
  // Reset old setting
  const_cast<ClipMeshBySelectionAlgo&>(*this).set(Parameters::BuildMapping, temp);
  return ret;
}

// Version with building mapping matrix

bool
ClipMeshBySelectionAlgo::runImpl(FieldHandle input,
                             FieldHandle selection,
                             FieldHandle& output,
                             MatrixHandle& mapping) const
{
  ScopedAlgorithmStatusReporter asr(this, "ClipMeshBySelection");

  if (!input)
  {
    error("No input field.");
    return (false);
  }

  if (!selection)
  {
    error("No selection input field.");
    return (false);
  }

  // Step 1: determine the type of the input fields and determine what type the
  // output field should be.

  FieldInformation fi(input);
  // Here we test whether the class is part of any of these newly defined
  // non-linear classes. If so we return an error.
  if (fi.is_nonlinear())
  {
    error("This algorithm has not yet been defined for non-linear elements yet.");
    return (false);
  }

  LOG_DEBUG("Num Elems {}; Num Tets {}", input->vmesh()->num_elems(), selection->vfield()->num_values());

  FieldInformation fo(input);
  fo.make_unstructuredmesh();

  output = CreateField(fo);
  if (!output)
  {
    error("Could not generate output field.");
    return (false);
  }

  std::string method = getOption(Parameters::ClipMethod);

  VMesh*   imesh = input->vmesh();
  VMesh*   omesh =  output->vmesh();
  VField*  ifield = input->vfield();
  VField*  ofield = output->vfield();
  VField*  sfield = selection->vfield();

  VMesh::size_type num_elems = imesh->num_elems();
  VMesh::size_type num_nodes = imesh->num_nodes();

  // For point clouds elements and nodes are the same, but the basis order is
  // 0, so force it to run through the element method.
  if (imesh->is_pointcloudmesh()) method = "Element Center";

  if (method == "Element Center")
  {
    LOG_DEBUG("Num Elems {}; Num Tets {}", imesh->num_elems(), sfield->num_values());

    if (imesh->num_elems() != sfield->num_values())
    {
      error("Number of elements in input mesh does not match number of values in selection mesh.");
      return (false);
    }

    std::vector<index_type> node_mapping(imesh->num_nodes(),-1);
    std::vector<index_type> elem_mapping(imesh->num_elems(),-1);
    std::vector<index_type> node_mapping2(imesh->num_nodes(),-1);
    std::vector<index_type> elem_mapping2(imesh->num_elems(),-1);

    VMesh::Node::array_type nodes;
    VMesh::points_type points;

    int cnt = 0;

    for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
    {
      char val;
      sfield->get_value(val,idx);
      if (val)
      {
        imesh->get_nodes(nodes,idx);
        imesh->get_centers(points,nodes);

        for (size_t j=0; j<nodes.size();j++)
        {
          if (node_mapping[nodes[j]] < 0)
          {
            node_mapping[nodes[j]] = omesh->add_node(points[j]);
            node_mapping2[node_mapping[nodes[j]]] = nodes[j];
          }
          nodes[j] = node_mapping[nodes[j]];
        }

        elem_mapping[idx] = omesh->add_elem(nodes);
        elem_mapping2[elem_mapping[idx]] = idx;
      }
      cnt++; if (cnt == 100) { cnt=0; update_progress_max(idx,num_elems);}
    }

    ofield->resize_values();
    VMesh::size_type num_oelems = omesh->num_elems();
    VMesh::size_type num_onodes = omesh->num_nodes();

    if (ofield->basis_order() == 0)
    {
      for(VMesh::Elem::index_type idx=0; idx<num_oelems; idx++)
      {
        ofield->copy_value(ifield,elem_mapping2[idx],idx);
      }
    }
    else if (ofield->basis_order() == 1)
    {
      for(VMesh::Node::index_type idx=0; idx<num_onodes; idx++)
      {
        ofield->copy_value(ifield,node_mapping2[idx],idx);
      }
    }

    bool build_mapping = get(Parameters::BuildMapping).toBool();
    if (build_mapping)
    {
      size_type m,n;

      if (ofield->basis_order() == 0)
      {
        if (num_elems > 0 && num_oelems > 0)
        {
          SparseRowMatrixFromMap::Values map;

          n =   num_elems;
          m =   num_oelems;

          for (index_type idx=0;idx<m;idx++)
          {
            map[idx][elem_mapping2[idx]] = 1.0;
          }

          mapping = SparseRowMatrixFromMap::make(m, n, map);
        }
      }
      else if (ofield->basis_order() == 1)
      {
        if (num_nodes > 0 && num_onodes >0)
        {
          SparseRowMatrixFromMap::Values map;

          n =   num_nodes;
          m =   num_onodes;

          for (index_type idx=0;idx<m;idx++)
          {
            map[idx][node_mapping2[idx]] = 1.0;
          }

          mapping = SparseRowMatrixFromMap::make(m, n, map);
        }
      }
      // provide an empty matrix
      if (!mapping)
        mapping.reset(new DenseMatrix(0,0));
    }
  }
  else
  {
    int target = 1;
    if (method == "One Node") target = 1;
    else if (method == "Most Nodes") target = omesh->num_nodes_per_elem()/2;
    else if (method == "All Nodes") target = omesh->num_nodes_per_elem();

    std::vector<index_type> node_mapping(imesh->num_nodes(),-1);
    std::vector<index_type> elem_mapping(imesh->num_elems(),-1);
    std::vector<index_type> node_mapping2(imesh->num_nodes(),-1);
    std::vector<index_type> elem_mapping2(imesh->num_elems(),-1);

    LOG_DEBUG("Num Nodes {}; Num Tets {}", imesh->num_nodes(), sfield->num_values());

    if (imesh->num_nodes() != sfield->num_values())
    {
      error("Number of nodes in input mesh does not match number of values in selection mesh.");
      return (false);
    }

    VMesh::Node::array_type nodes;
    VMesh::points_type points;
    std::vector<char> values;

    int cnt = 0;

    for(VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
    {
      imesh->get_nodes(nodes,idx);
      sfield->get_values(values,nodes);
      int ctarget = 0;
      for (size_t j=0; j<values.size(); j++) if (values[j]) ctarget++;

      if (ctarget >= target)
      {
        imesh->get_centers(points,nodes);
        for (size_t j=0; j<nodes.size();j++)
        {
          if (node_mapping[nodes[j]] < 0)
          {
            node_mapping[nodes[j]] = omesh->add_node(points[j]);
            // reverse mapping
            node_mapping2[node_mapping[nodes[j]]] = nodes[j];
          }
          nodes[j] = node_mapping[nodes[j]];
        }

        elem_mapping[idx] = omesh->add_elem(nodes);
        elem_mapping2[elem_mapping[idx]] = idx;
      }

      cnt++; if (cnt == 100) { cnt=0; update_progress_max(idx,num_elems);}
    }

    ofield->resize_values();
    VMesh::size_type num_oelems = omesh->num_elems();
    VMesh::size_type num_onodes = omesh->num_nodes();

    if (ofield->basis_order() == 0)
    {
      for(VMesh::Elem::index_type idx=0; idx<num_oelems; idx++)
      {
        ofield->copy_value(ifield,elem_mapping2[idx],idx);
      }
    }
    else if (ofield->basis_order() == 1)
    {
      for(VMesh::Node::index_type idx=0; idx<num_onodes; idx++)
      {
        ofield->copy_value(ifield,node_mapping2[idx],idx);
      }
    }

    bool build_mapping = get(Parameters::BuildMapping).toBool();
    if (build_mapping)
    {
      size_type m,n;

      if (ofield->basis_order() == 0)
      {
        if (num_elems > 0 && num_oelems > 0)
        {
          SparseRowMatrixFromMap::Values map;

          n =   num_elems;
          m =   num_oelems;

          for (index_type idx=0;idx<m;idx++)
          {
            map[idx][elem_mapping2[idx]] = 1.0;
          }

          mapping = SparseRowMatrixFromMap::make(m, n, map);
        }
      }
      else if (ofield->basis_order() == 1)
      {
        if (num_nodes > 0 && num_onodes > 0)
        {
          SparseRowMatrixFromMap::Values map;

          n =   num_nodes;
          m =   num_onodes;

          for (index_type idx=0;idx<m;idx++)
          {
            map[idx][node_mapping2[idx]] = 1.0;
          }

          mapping = SparseRowMatrixFromMap::make(m, n, map);
        }
      }
      // provide an empty matrix
      if (!mapping) mapping.reset(new DenseMatrix(0,0));
    }
  }

  /// Copy properties of the property manager
  CopyProperties(*input, *output);

  return (true);
}

const AlgorithmInputName ClipMeshBySelectionAlgo::SelectionField("SelectionField");
const AlgorithmOutputName ClipMeshBySelectionAlgo::Mapping("Mapping");

AlgorithmOutput ClipMeshBySelectionAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  auto selectionField = input.get<Field>(SelectionField);

  FieldHandle outputField;
  MatrixHandle mapping;

  if (!runImpl(inputField, selectionField, outputField, mapping))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;
  output[Mapping] = mapping;
  return output;
}
