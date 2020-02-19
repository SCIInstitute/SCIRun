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
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMeshTriSurfAlgoV.h>

#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
// For mapping matrices
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

//STL classes needed
#include <algorithm>
#include <set>

///////////////////////////////////////////////////////
// Refine elements for a TriSurf
using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;

RefineMeshTriSurfAlgoV::RefineMeshTriSurfAlgoV()
{

}

bool
RefineMeshTriSurfAlgoV::runImpl(FieldHandle input, FieldHandle& output,
                       const std::string& select, double isoval) const
{
  /// Obtain information on what type of input field we have
  FieldInformation fi(input);

  /// Alter the input so it will become a QuadSurf
  fi.make_trisurfmesh();
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

  VMesh::Node::array_type onodes(3);

  mesh->synchronize(Mesh::EDGES_E);

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

  VMesh::Node::array_type nodes(2);
  Point p0, p1,p2, p3, p;
  VMesh::Edge::iterator be, ee;
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
      if (field->basis_order() == 1)
        ivalues.push_back(0.5*(ivalues[nodes[0]]+ivalues[nodes[1]]));
    }
    ++be;
  }


  VMesh::Edge::array_type oedges(3);
  VMesh::Node::array_type nnodes(3);

  VMesh::Elem::iterator bi, ei;
  mesh->begin(bi); mesh->end(ei);

  unsigned int cnt = 0;
  unsigned int loopcnt = 0;

  VMesh::Elem::size_type sz; mesh->size(sz);

  while (bi != ei)
  {
			cnt++; if (cnt == 100) { loopcnt +=cnt; cnt = 0; this->update_progress_max(loopcnt,sz); }

    mesh->get_nodes(onodes, *bi);
    mesh->get_edges(oedges, *bi);

    VMesh::index_type i0 = onodes[0];
    VMesh::index_type i1 = onodes[1];
    VMesh::index_type i2 = onodes[2];
    VMesh::index_type i3 = enodes[oedges[0]];
    VMesh::index_type i4 = enodes[oedges[1]];
    VMesh::index_type i5 = enodes[oedges[2]];

    if (i3==0 && i4 == 0 && i5 == 0)
    {
      refined->add_elem(onodes);
      if (field->basis_order() == 0) evalues.push_back(ivalues[*bi]);
    }
    else if (i3 > 0 && i4 > 0 && i5 > 0)
    {
      nnodes[0] =i0; nnodes[1] = i3; nnodes[2] = i5;
      refined->add_elem(nnodes);

      nnodes[0] = i3; nnodes[1] = i1; nnodes[2] = i4;
      refined->add_elem(nnodes);

      nnodes[0] = i4; nnodes[1] = i2; nnodes[2] = i5;
      refined->add_elem(nnodes);

      nnodes[0] = i3; nnodes[1] = i4; nnodes[2] = i5;
      refined->add_elem(nnodes);
      if (field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
    }
    else if (i3 == 0)
    {
      Point p0, p1, p4, p5;
      refined->get_center(p0,VMesh::Node::index_type(i0));
      refined->get_center(p1,VMesh::Node::index_type(i1));
      refined->get_center(p4,VMesh::Node::index_type(i4));
      refined->get_center(p5,VMesh::Node::index_type(i5));

      if ((p0-p4).length2() < (p1-p5).length2())
      {
        nnodes[0] =i4; nnodes[1] = i2; nnodes[2] = i5;
        refined->add_elem(nnodes);

        nnodes[0] =i4; nnodes[1] = i5; nnodes[2] = i0;
        refined->add_elem(nnodes);

        nnodes[0] =i0; nnodes[1] = i1; nnodes[2] = i4;
        refined->add_elem(nnodes);
      }
      else
      {
        nnodes[0] =i4; nnodes[1] = i2; nnodes[2] = i5;
        refined->add_elem(nnodes);

        nnodes[0] =i4; nnodes[1] = i5; nnodes[2] = i1;
        refined->add_elem(nnodes);

        nnodes[0] =i0; nnodes[1] = i1; nnodes[2] = i5;
        refined->add_elem(nnodes);
      }
      if (field->basis_order() == 0) evalues.insert(evalues.end(),3,ivalues[*bi]);
    }
    else if (i4 == 0)
    {
      Point p1, p2, p3, p5;
      refined->get_center(p1,VMesh::Node::index_type(i1));
      refined->get_center(p2,VMesh::Node::index_type(i2));
      refined->get_center(p3,VMesh::Node::index_type(i3));
      refined->get_center(p5,VMesh::Node::index_type(i5));

      if ((p1-p5).length2() < (p2-p3).length2())
      {
        nnodes[0] =i0; nnodes[1] = i3; nnodes[2] = i5;
        refined->add_elem(nnodes);

        nnodes[0] =i3; nnodes[1] = i1; nnodes[2] = i5;
        refined->add_elem(nnodes);

        nnodes[0] =i1; nnodes[1] = i2; nnodes[2] = i5;
        refined->add_elem(nnodes);
      }
      else
      {
        nnodes[0] =i0; nnodes[1] = i3; nnodes[2] = i5;
        refined->add_elem(nnodes);

        nnodes[0] =i3; nnodes[1] = i2; nnodes[2] = i5;
        refined->add_elem(nnodes);

        nnodes[0] =i1; nnodes[1] = i2; nnodes[2] = i3;
        refined->add_elem(nnodes);
      }
      if (field->basis_order() == 0) evalues.insert(evalues.end(),3,ivalues[*bi]);
    }
    else if (i5 == 0)
    {
      Point p2, p0, p4, p3;
      refined->get_center(p2,VMesh::Node::index_type(i2));
      refined->get_center(p0,VMesh::Node::index_type(i0));
      refined->get_center(p4,VMesh::Node::index_type(i4));
      refined->get_center(p3,VMesh::Node::index_type(i3));

      if ((p2-p3).length2() < (p0-p4).length2())
      {
        nnodes[0] =i1; nnodes[1] = i4; nnodes[2] = i3;
        refined->add_elem(nnodes);

        nnodes[0] =i3; nnodes[1] = i2; nnodes[2] = i0;
        refined->add_elem(nnodes);

        nnodes[0] =i2; nnodes[1] = i3; nnodes[2] = i4;
        refined->add_elem(nnodes);
      }
      else
      {
        nnodes[0] =i1; nnodes[1] = i4; nnodes[2] = i3;
        refined->add_elem(nnodes);

        nnodes[0] =i4; nnodes[1] = i2; nnodes[2] = i0;
        refined->add_elem(nnodes);

        nnodes[0] =i0; nnodes[1] = i3; nnodes[2] = i4;
        refined->add_elem(nnodes);
      }
      if (field->basis_order() == 0) evalues.insert(evalues.end(),3,ivalues[*bi]);
    }
    ++bi;
		}

  rfield->resize_values();
  if (rfield->basis_order() == 0) rfield->set_values(evalues);
  if (rfield->basis_order() == 1) rfield->set_values(ivalues);
  CopyProperties(*input, *output);
  return (true);
}

AlgorithmOutput RefineMeshTriSurfAlgoV::run(const AlgorithmInput& input) const
{
  throw "not implemented";
}
