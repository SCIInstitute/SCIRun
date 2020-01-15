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
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMeshTetVolAlgoV.h>

#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
//#include <Core/Datatypes/Legacy/Matrix/Matrix.h>
// For mapping matrices
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

//STL classes needed
//#include <sci_hash_map.h>
#include <algorithm>
#include <set>

/////////////////////////////////////////////////////
// Refine elements for a TetVol
using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;

RefineMeshTetVolAlgoV::RefineMeshTetVolAlgoV()
{

}

bool
RefineMeshTetVolAlgoV::runImpl(FieldHandle input, FieldHandle& output,
                      const std::string& select, double isoval) const
{
  FieldInformation fi(input);

  fi.make_tetvolmesh();

  output = CreateField(fi);

  if (!output)
  {
    error("Could not create an output field");
    return (false);
  }

  VField* field   = input->vfield();
  VMesh*  mesh    = input->vmesh();
  VMesh*  refined = output->vmesh();
  VField* rfield  = output->vfield();

  VMesh::Node::array_type onodes(4);

  mesh->synchronize(Mesh::EDGES_E);

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
      error("Unknown region selection method encountered");
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

      p = (p0 + p1).asPoint()*0.5;

      enodes[*be] = refined->add_point(p);
      if(field->basis_order() == 1)
      {
        ivalues.push_back(0.5*(ivalues[nodes[0]]+ivalues[nodes[1]]));
      }
    }
    ++be;
  }


  VMesh::Edge::array_type oedges(6);
  VMesh::Node::array_type nnodes(4);

  VMesh::Elem::iterator bi, ei;
  mesh->begin(bi); mesh->end(ei);

  unsigned int cnt = 0;
  unsigned int loopcnt = 0;

  VMesh::Elem::size_type sz; mesh->size(sz);


  while (bi != ei)
  {
			cnt++; if (cnt == 100) { loopcnt +=cnt; cnt = 0;}// algo->update_progress(loopcnt,sz);  }
    mesh->get_nodes(onodes, *bi);
    mesh->get_edges(oedges, *bi);

    VMesh::index_type i0 = onodes[0];
    VMesh::index_type i1 = onodes[1];
    VMesh::index_type i2 = onodes[2];
    VMesh::index_type i3 = onodes[3];
    VMesh::index_type i4 = enodes[oedges[0]];
    VMesh::index_type i5 = enodes[oedges[1]];
    VMesh::index_type i6 = enodes[oedges[2]];
    VMesh::index_type i7 = enodes[oedges[3]];
    VMesh::index_type i8 = enodes[oedges[4]];
    VMesh::index_type i9 = enodes[oedges[5]];

    if (i4==0 && i5 == 0 && i6 == 0 && i7==0 && i8 == 0 && i9 == 0)
    {
      refined->add_elem(onodes);
      if(field->basis_order() == 0) evalues.push_back(ivalues[*bi]);
    }
    else if (i4 > 0 && i5 > 0 && i6 > 0 && i7 > 0 && i8 > 0 && i9 > 0)
    {
      nnodes[0] =i4; nnodes[1] = i1; nnodes[2] = i5; nnodes[3] = i8;
      refined->add_elem(nnodes);
      nnodes[0] =i4; nnodes[1] = i8; nnodes[2] = i5; nnodes[3] = i7;
      refined->add_elem(nnodes);
      nnodes[0] =i7; nnodes[1] = i8; nnodes[2] = i5; nnodes[3] = i9;
      refined->add_elem(nnodes);
      nnodes[0] =i6; nnodes[1] = i4; nnodes[2] = i5; nnodes[3] = i7;
      refined->add_elem(nnodes);
      nnodes[0] =i6; nnodes[1] = i7; nnodes[2] = i5; nnodes[3] = i9;
      refined->add_elem(nnodes);
      nnodes[0] =i0; nnodes[1] = i4; nnodes[2] = i6; nnodes[3] = i7;
      refined->add_elem(nnodes);
      nnodes[0] =i7; nnodes[1] = i8; nnodes[2] = i9; nnodes[3] = i3;
      refined->add_elem(nnodes);
      nnodes[0] =i6; nnodes[1] = i5; nnodes[2] = i2; nnodes[3] = i9;
      refined->add_elem(nnodes);
      if(field->basis_order() == 0) evalues.insert(evalues.end(),8,ivalues[*bi]);
    }
    else if (i5 == 0 && i8 == 0 && i9 == 0)
    {
      if ( i1 < i2 && i2 <i3)
      { //Checked orientation
        nnodes[0] =i0; nnodes[1] = i4; nnodes[2] = i6; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i1; nnodes[2] = i6; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i1; nnodes[2] = i2; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i1; nnodes[2] = i2; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i1 < i3 && i3 < i2)
      { // checked orientation
        nnodes[0] =i0; nnodes[1] = i4; nnodes[2] = i6; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i1; nnodes[2] = i6; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i1; nnodes[2] = i6; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i1; nnodes[2] = i2; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i2< i1 && i1 < i3)
      { // checked orientation
        nnodes[0] =i0; nnodes[1] = i4; nnodes[2] = i6; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i4; nnodes[2] = i2; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i4; nnodes[2] = i2; nnodes[3] = i1;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i1; nnodes[2] = i2; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i2 < i3 && i3 < i1)
      { // checked orientation
        nnodes[0] =i0; nnodes[1] = i4; nnodes[2] = i6; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i4; nnodes[2] = i2; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i4; nnodes[2] = i2; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i3; nnodes[1] = i4; nnodes[2] = i2; nnodes[3] = i1;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i3 < i1 && i1 < i2)
      { // checked orientation
        nnodes[0] =i0; nnodes[1] = i4; nnodes[2] = i6; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i6; nnodes[2] = i7; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i1; nnodes[1] = i6; nnodes[2] = i4; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i1; nnodes[1] = i2; nnodes[2] = i6; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else
      { // checked orientation
        nnodes[0] =i0; nnodes[1] = i4; nnodes[2] = i6; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i6; nnodes[2] = i7; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i2; nnodes[2] = i6; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i1; nnodes[2] = i2; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
    }
    else if (i4 == 0 && i7 == 0 && i8 == 0)
    {
      if ( i0 < i1 && i1 <i3)
      { //Checked orientation
        nnodes[0] =i2; nnodes[1] = i6; nnodes[2] = i5; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i0; nnodes[2] = i5; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i0; nnodes[2] = i1; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i0; nnodes[2] = i1; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i0 < i3 && i3 < i1)
      { // checked orientation
        nnodes[0] =i2; nnodes[1] = i6; nnodes[2] = i5; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i0; nnodes[2] = i5; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i0; nnodes[2] = i5; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i0; nnodes[2] = i1; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i1< i0 && i0 < i3)
      { // checked orientation
        nnodes[0] =i2; nnodes[1] = i6; nnodes[2] = i5; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i6; nnodes[2] = i1; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i6; nnodes[2] = i1; nnodes[3] = i0;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i0; nnodes[2] = i1; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i1 < i3 && i3 < i0)
      { // checked orientation
        nnodes[0] =i2; nnodes[1] = i6; nnodes[2] = i5; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i6; nnodes[2] = i1; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i6; nnodes[2] = i1; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i3; nnodes[1] = i6; nnodes[2] = i1; nnodes[3] = i0;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i3 < i0 && i0 < i1)
      { // checked orientation
        nnodes[0] =i2; nnodes[1] = i6; nnodes[2] = i5; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i5; nnodes[2] = i9; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i0; nnodes[1] = i5; nnodes[2] = i6; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i0; nnodes[1] = i1; nnodes[2] = i5; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else
      { // checked orientation
        nnodes[0] =i2; nnodes[1] = i6; nnodes[2] = i5; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i5; nnodes[2] = i9; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i1; nnodes[2] = i5; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i0; nnodes[2] = i1; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
    }
    else if (i6 == 0 && i9 == 0 && i7 == 0)
    {
      if ( i2 < i0 && i0 <i3)
      { //Checked orientation
        nnodes[0] =i1; nnodes[1] = i5; nnodes[2] = i4; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i2; nnodes[2] = i4; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i2; nnodes[2] = i0; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i2; nnodes[2] = i0; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i2 < i3 && i3 < i0)
      { // checked orientation
        nnodes[0] =i1; nnodes[1] = i5; nnodes[2] = i4; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i2; nnodes[2] = i4; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i2; nnodes[2] = i4; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i2; nnodes[2] = i0; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i0< i2 && i2 < i3)
      { // checked orientation
        nnodes[0] =i1; nnodes[1] = i5; nnodes[2] = i4; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i5; nnodes[2] = i0; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i5; nnodes[2] = i0; nnodes[3] = i2;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i2; nnodes[2] = i0; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i0 < i3 && i3 < i2)
      { // checked orientation
        nnodes[0] =i1; nnodes[1] = i5; nnodes[2] = i4; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i5; nnodes[2] = i0; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i5; nnodes[2] = i0; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i3; nnodes[1] = i5; nnodes[2] = i0; nnodes[3] = i2;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i3 < i2 && i2 < i0)
      { // checked orientation
        nnodes[0] =i1; nnodes[1] = i5; nnodes[2] = i4; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i4; nnodes[2] = i8; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i2; nnodes[1] = i4; nnodes[2] = i5; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i2; nnodes[1] = i0; nnodes[2] = i4; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else
      { // checked orientation
        nnodes[0] =i1; nnodes[1] = i5; nnodes[2] = i4; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i4; nnodes[2] = i8; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i0; nnodes[2] = i4; nnodes[3] = i3;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i2; nnodes[2] = i0; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
    }
    else if (i5 == 0 && i6 == 0 && i4 == 0)
    {
      if ( i2 < i1 && i1 <i0)
      { //Checked orientation
        nnodes[0] =i3; nnodes[1] = i9; nnodes[2] = i8; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i2; nnodes[2] = i8; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i2; nnodes[2] = i1; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i2; nnodes[2] = i1; nnodes[3] = i0;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i2 < i0 && i0 < i1)
      { // checked orientation
        nnodes[0] =i3; nnodes[1] = i9; nnodes[2] = i8; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i2; nnodes[2] = i8; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i2; nnodes[2] = i8; nnodes[3] = i0;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i2; nnodes[2] = i1; nnodes[3] = i0;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i1< i2 && i2 < i0)
      { // checked orientation
        nnodes[0] =i3; nnodes[1] = i9; nnodes[2] = i8; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i9; nnodes[2] = i1; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i9; nnodes[2] = i1; nnodes[3] = i2;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i2; nnodes[2] = i1; nnodes[3] = i0;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i1 < i0 && i0 < i2)
      { // checked orientation
        nnodes[0] =i3; nnodes[1] = i9; nnodes[2] = i8; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i9; nnodes[2] = i1; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i9; nnodes[2] = i1; nnodes[3] = i0;
        refined->add_elem(nnodes);
        nnodes[0] =i0; nnodes[1] = i9; nnodes[2] = i1; nnodes[3] = i2;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (i0 < i2 && i2 < i1)
      { // checked orientation
        nnodes[0] =i3; nnodes[1] = i9; nnodes[2] = i8; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i8; nnodes[2] = i7; nnodes[3] = i0;
        refined->add_elem(nnodes);
        nnodes[0] =i2; nnodes[1] = i8; nnodes[2] = i9; nnodes[3] = i0;
        refined->add_elem(nnodes);
        nnodes[0] =i2; nnodes[1] = i1; nnodes[2] = i8; nnodes[3] = i0;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else
      { // checked orientation
        nnodes[0] =i3; nnodes[1] = i9; nnodes[2] = i8; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i8; nnodes[2] = i7; nnodes[3] = i0;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i1; nnodes[2] = i8; nnodes[3] = i0;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i2; nnodes[2] = i1; nnodes[3] = i0;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
    }
    else if (i8 == 0)
    {
      if (i1 < i3)
      {
        nnodes[0] =i2; nnodes[1] = i5; nnodes[2] = i9; nnodes[3] = i6;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i1; nnodes[2] = i3; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i5; nnodes[2] = i1; nnodes[3] = i4;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i6; nnodes[2] = i5; nnodes[3] = i4;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i4; nnodes[2] = i1; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i4; nnodes[2] = i6; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i7; nnodes[2] = i6; nnodes[3] = i0;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),7,ivalues[*bi]);
      }
      else
      {
        nnodes[0] =i2; nnodes[1] = i5; nnodes[2] = i9; nnodes[3] = i6;
        refined->add_elem(nnodes);
        nnodes[0] =i3; nnodes[1] = i5; nnodes[2] = i1; nnodes[3] = i4;
        refined->add_elem(nnodes);
        nnodes[0] =i3; nnodes[1] = i5; nnodes[2] = i4; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i5; nnodes[2] = i3; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i5; nnodes[2] = i7; nnodes[3] = i6;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i7; nnodes[2] = i6; nnodes[3] = i4;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i4; nnodes[2] = i7; nnodes[3] = i0;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),7,ivalues[*bi]);
      }
    }
    else if (i9 == 0)
    {
      if (i2 < i3)
      {
        nnodes[0] =i0; nnodes[1] = i6; nnodes[2] = i7; nnodes[3] = i4;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i2; nnodes[2] = i3; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i6; nnodes[2] = i2; nnodes[3] = i5;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i4; nnodes[2] = i6; nnodes[3] = i5;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i5; nnodes[2] = i2; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i5; nnodes[2] = i4; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i8; nnodes[2] = i4; nnodes[3] = i1;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),7,ivalues[*bi]);
      }
      else
      {
        nnodes[0] =i0; nnodes[1] = i6; nnodes[2] = i7; nnodes[3] = i4;
        refined->add_elem(nnodes);
        nnodes[0] =i3; nnodes[1] = i6; nnodes[2] = i2; nnodes[3] = i5;
        refined->add_elem(nnodes);
        nnodes[0] =i3; nnodes[1] = i6; nnodes[2] = i5; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i6; nnodes[2] = i3; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i6; nnodes[2] = i8; nnodes[3] = i4;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i8; nnodes[2] = i4; nnodes[3] = i5;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i5; nnodes[2] = i8; nnodes[3] = i1;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),7,ivalues[*bi]);
      }
    }
    else if (i7 == 0)
    {
      if (i0 < i3)
      {
        nnodes[0] =i1; nnodes[1] = i4; nnodes[2] = i8; nnodes[3] = i5;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i0; nnodes[2] = i3; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i4; nnodes[2] = i0; nnodes[3] = i6;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i5; nnodes[2] = i4; nnodes[3] = i6;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i6; nnodes[2] = i0; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i6; nnodes[2] = i5; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i9; nnodes[2] = i5; nnodes[3] = i2;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),7,ivalues[*bi]);
      }
      else
      {
        nnodes[0] =i1; nnodes[1] = i4; nnodes[2] = i8; nnodes[3] = i5;
        refined->add_elem(nnodes);
        nnodes[0] =i3; nnodes[1] = i4; nnodes[2] = i0; nnodes[3] = i6;
        refined->add_elem(nnodes);
        nnodes[0] =i3; nnodes[1] = i4; nnodes[2] = i6; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i4; nnodes[2] = i3; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i4; nnodes[2] = i9; nnodes[3] = i5;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i9; nnodes[2] = i5; nnodes[3] = i6;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i6; nnodes[2] = i9; nnodes[3] = i2;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),7,ivalues[*bi]);
      }
    }
    else if (i6 == 0)
    {
      if (i2 < i0)
      {
        nnodes[0] =i1; nnodes[1] = i5; nnodes[2] = i4; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i2; nnodes[2] = i0; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i5; nnodes[2] = i2; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i8; nnodes[2] = i5; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i9; nnodes[2] = i2; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i9; nnodes[2] = i8; nnodes[3] = i4;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i7; nnodes[2] = i8; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),7,ivalues[*bi]);
      }
      else
      {
        nnodes[0] =i1; nnodes[1] = i5; nnodes[2] = i4; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i0; nnodes[1] = i5; nnodes[2] = i2; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i0; nnodes[1] = i5; nnodes[2] = i9; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i5; nnodes[2] = i0; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i5; nnodes[2] = i7; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i7; nnodes[2] = i8; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i9; nnodes[2] = i7; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),7,ivalues[*bi]);
      }
    }
    else if (i5 == 0)
    {
      if (i1 < i2)
      {
        nnodes[0] =i0; nnodes[1] = i4; nnodes[2] = i6; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i1; nnodes[2] = i2; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i4; nnodes[2] = i1; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i7; nnodes[2] = i4; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i8; nnodes[2] = i1; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i8; nnodes[2] = i7; nnodes[3] = i6;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i9; nnodes[2] = i7; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),7,ivalues[*bi]);
      }
      else
      {
        nnodes[0] =i0; nnodes[1] = i4; nnodes[2] = i6; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i2; nnodes[1] = i4; nnodes[2] = i1; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i2; nnodes[1] = i4; nnodes[2] = i8; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i4; nnodes[2] = i2; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i4; nnodes[2] = i9; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i4; nnodes[1] = i9; nnodes[2] = i7; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i8; nnodes[2] = i9; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),7,ivalues[*bi]);
      }
    }
    else if (i4 == 0)
    {
      if (i0 < i1)
      {
        nnodes[0] =i2; nnodes[1] = i6; nnodes[2] = i5; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i0; nnodes[2] = i1; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i6; nnodes[2] = i0; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i9; nnodes[2] = i6; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i7; nnodes[2] = i0; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i8; nnodes[1] = i7; nnodes[2] = i9; nnodes[3] = i5;
        refined->add_elem(nnodes);
        nnodes[0] =i7; nnodes[1] = i8; nnodes[2] = i9; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),7,ivalues[*bi]);
      }
      else
      {
        nnodes[0] =i2; nnodes[1] = i6; nnodes[2] = i5; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i1; nnodes[1] = i6; nnodes[2] = i0; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i1; nnodes[1] = i6; nnodes[2] = i7; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i6; nnodes[2] = i1; nnodes[3] = i8;
        refined->add_elem(nnodes);
        nnodes[0] =i5; nnodes[1] = i6; nnodes[2] = i8; nnodes[3] = i9;
        refined->add_elem(nnodes);
        nnodes[0] =i6; nnodes[1] = i8; nnodes[2] = i9; nnodes[3] = i7;
        refined->add_elem(nnodes);
        nnodes[0] =i9; nnodes[1] = i7; nnodes[2] = i8; nnodes[3] = i3;
        refined->add_elem(nnodes);
        if(field->basis_order() == 0) evalues.insert(evalues.end(),7,ivalues[*bi]);
      }
    }
    ++bi;
  }

  rfield->resize_values();
  if (rfield->basis_order() == 0) rfield->set_values(evalues);
  if (rfield->basis_order() == 1) rfield->set_values(ivalues);
  CopyProperties(*input, *output);
  return (true);
}

AlgorithmOutput RefineMeshTetVolAlgoV::run(const AlgorithmInput& input) const
{
  throw "not implemented";
}
