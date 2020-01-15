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


#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMesh.h>
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMeshCurveAlgoV.h>

#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
// For mapping matrices
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

//STL classes needed
#include <algorithm>
#include <set>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;

///////////////////////////////////////////////////////
// Refine elements for a CurveMesh
RefineMeshCurveAlgoV::RefineMeshCurveAlgoV()
{
}

bool
RefineMeshCurveAlgoV::runImpl(FieldHandle input, FieldHandle& output, const std::string& select, double isoval) const
{
  /// Obtain information on what type of input field we have
  FieldInformation fi(input);

  /// Alter the input so it will become a QuadSurf
  fi.make_curvemesh();
  output = CreateField(fi);

  if (!output)
  {
    error("RefineMesh: Could not create an output field");
    return (false);
  }

  VField* field   = input->vfield();
  VMesh*  mesh    = input->vmesh();
  VMesh*  refined = output->vmesh();
  VField* rfield  = output->vfield();

  VMesh::Node::array_type onodes(2);

  // get all values, make computation easier
  VMesh::size_type num_nodes = mesh->num_nodes();
  VMesh::size_type num_elems = mesh->num_elems();

  std::vector<bool> values(num_nodes,false);

  // Deal with data stored at different locations
  // If data is on the elements make sure that all nodes
  // of that element pass requirement.

  std::vector<double> ivalues;
  std::vector<double> evalues;

  if (field->basis_order() == 0)
  {
    field->get_values(ivalues);

    if (select == "equal")
    {
      for (VMesh::Elem::index_type i=0; i<num_elems; i++)
      {
        mesh->get_nodes(onodes,i);
        if (ivalues[i] == isoval)
          for (size_t j=0; j< onodes.size(); j++)
            values[onodes[j]] = true;
      }
    }
    else if (select == "lessthan")
    {
      for (VMesh::Elem::index_type i=0; i<num_elems; i++)
      {
        mesh->get_nodes(onodes,i);
        if (ivalues[i] < isoval)
          for (size_t j=0; j< onodes.size(); j++)
            values[onodes[j]] = true;
      }
    }
    else if (select == "greaterthan")
    {
      for (VMesh::Elem::index_type i=0; i<num_elems; i++)
      {
        mesh->get_nodes(onodes,i);
        if (ivalues[i] > isoval)
          for (size_t j=0; j< onodes.size(); j++)
            values[onodes[j]] = true;
      }
    }
    else if (select == "all")
    {
      for (size_t j=0;j<values.size();j++) values[j] = true;
    }
    else
    {
      error("RefineMesh: Unknown region selection method encountered");
      return (false);
    }
  }
  else if (field->basis_order() == 1)
  {
    field->get_values(ivalues);

    if (select == "equal")
    {
      for (VMesh::Elem::index_type i=0; i<num_nodes; i++)
      {
        if (ivalues[i] == isoval) values[i] = true;
      }
    }
    else if (select == "lessthan")
    {
      for (VMesh::Elem::index_type i=0; i<num_nodes; i++)
      {
        if (ivalues[i] < isoval) values[i] = true;
      }
    }
    else if (select == "greaterthan")
    {
      for (VMesh::Elem::index_type i=0; i<num_nodes; i++)
      {
        if (ivalues[i] > isoval) values[i] = true;
      }
    }
    else if (select == "all")
    {
      for (size_t j=0;j<values.size();j++) values[j] = true;
    }
    else
    {
      error("RefineMesh: Unknown region selection method encountered");
      return (false);
    }

  }
  else
  {
    for (size_t j=0;j<values.size();j++) values[j] = true;
  }

  // Copy all of the nodes from mesh to refined.  They won't change,
  // we only add nodes.

  VMesh::Node::iterator bni, eni;
  mesh->begin(bni); mesh->end(eni);
  while (bni != eni)
  {
    Point p;
    mesh->get_point(p, *bni);
    refined->add_point(p);
    ++bni;
  }

  std::vector<VMesh::index_type> enodes(mesh->num_edges(),0);

  VMesh::Node::array_type nodes(2), nnodes(2);
  Point p0, p1,p2, p3, p;
  VMesh::Elem::iterator be, ee;
  mesh->begin(be); mesh->end(ee);

  // add all additional nodes we need
  while (be != ee)
  {
    mesh->get_nodes(nodes,*be);
    if ((values[nodes[0]] == true) || (values[nodes[1]] == true))
    {
      mesh->get_center(p0,nodes[0]);
      mesh->get_center(p1,nodes[1]);

      p = Point((p0 + p1)*0.5);
      enodes[*be] = refined->add_point(p);

      nnodes[0] = nodes[0];
      nnodes[1] = enodes[*be];
      refined->add_elem(nnodes);

      nnodes[0] = enodes[*be];
      nnodes[1] = nodes[1];
      refined->add_elem(nnodes);

      if (field->basis_order() == 1)
        ivalues.push_back(0.5*(ivalues[nodes[0]]+ivalues[nodes[1]]));
      else if (field->basis_order() == 0)
	evalues.insert(evalues.end(),2,ivalues[*be]);
    }
    ++be;
  }

  rfield->resize_values();
  if (rfield->basis_order() == 0) rfield->set_values(evalues);
  if (rfield->basis_order() == 1) rfield->set_values(ivalues);
  CopyProperties(*input, *output);
  return (true);
}

AlgorithmOutput RefineMeshCurveAlgoV::run(const AlgorithmInput& input) const
{
  throw "not implemented";
}
