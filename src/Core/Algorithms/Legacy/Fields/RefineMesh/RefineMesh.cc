/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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


#include <Core/Algorithms/Fields/RefineMesh/RefineMesh.h>

//! For mapping matrices
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>

//! STL classes needed
#include <sci_hash_map.h>
#include <algorithm>
#include <set>

namespace SCIRunAlgo {


///////////////////////////////////////////////////////
// Refine elements for a TetVol 

bool  
RefineMeshTetVolAlgoV(AlgoBase* algo, FieldHandle input, FieldHandle& output,
                      std::string select, double isoval)
{
  //! Obtain information on what type of input field we have
  FieldInformation fi(input);
  
  //! Alter the input so it will become a tetvol
  fi.make_tetvolmesh();
  
  //! Create the output field
  output = CreateField(fi);
  
  if (output.get_rep() == 0)
  {
    algo->error("Could not create an output field");
    algo->algo_end(); return (false);
  }

  VField* field   = input->vfield();
  VMesh*  mesh    = input->vmesh();
  VMesh*  refined = output->vmesh();
  VField* rfield  = output->vfield();

  VMesh::Node::array_type onodes(4);
  
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
      algo->error("Unknown region selection method encountered");
      algo->algo_end(); return (false);
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
      algo->error("RefineMesh: Unknown region selection method encountered");
      algo->algo_end(); return (false);
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
    
      p = (p0.asVector() + p1.asVector()).asPoint()*0.5;
    
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
    cnt++; if (cnt == 100) { loopcnt +=cnt; cnt = 0; algo->update_progress(loopcnt,sz);  }
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
  rfield->copy_properties(field);

  algo->algo_end(); return (true);
}



///////////////////////////////////////////////////////
// Refine elements for a CurveMesh

bool  
RefineMeshCurveAlgoV(AlgoBase* algo, FieldHandle input, FieldHandle& output,
                       std::string select, double isoval)
{
  //! Obtain information on what type of input field we have
  FieldInformation fi(input);
  
  //! Alter the input so it will become a QuadSurf
  fi.make_curvemesh();
  output = CreateField(fi);
  
  if (output.get_rep() == 0)
  {
    algo->error("RefineMesh: Could not create an output field");
    algo->algo_end(); return (false);
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
      algo->error("RefineMesh: Unknown region selection method encountered");
      algo->algo_end(); return (false);
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
      algo->error("RefineMesh: Unknown region selection method encountered");
      algo->algo_end(); return (false);
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
    
      p = (p0.asVector() + p1.asVector()).asPoint()*0.5;
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
  rfield->copy_properties(field);

  algo->algo_end(); return (true);
}



///////////////////////////////////////////////////////
// Refine elements for a TriSurf 

bool  
RefineMeshTriSurfAlgoV(AlgoBase* algo, FieldHandle input, FieldHandle& output,
                       std::string select, double isoval)
{
  //! Obtain information on what type of input field we have
  FieldInformation fi(input);
  
  //! Alter the input so it will become a QuadSurf
  fi.make_trisurfmesh();
  output = CreateField(fi);
  
  if (output.get_rep() == 0)
  {
    algo->error("RefineMesh: Could not create an output field");
    algo->algo_end(); return (false);
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
      algo->error("RefineMesh: Unknown region selection method encountered");
      algo->algo_end(); return (false);
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
      algo->error("RefineMesh: Unknown region selection method encountered");
      algo->algo_end(); return (false);
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
    
      p = (p0.asVector() + p1.asVector()).asPoint()*0.5;
    
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
    cnt++; if (cnt == 100) { loopcnt +=cnt; cnt = 0; algo->update_progress(loopcnt,sz);  }

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
  rfield->copy_properties(field);

  algo->algo_end(); return (true);
}




///////////////////////////////////////////////////////
// Refine elements for a QuadSurf 

class RefineMeshQuadSurfAlgoV 
{
  public:  

    bool  run(AlgoBase* algo, FieldHandle input, FieldHandle& output,
                                     std::string select, double isoval);

  private:
  
    // Functions need for hashmap
    struct edgepair_t
    {
      VMesh::index_type first;
      VMesh::index_type second;
    };

    struct edgepairequal
    {
      bool operator()(const edgepair_t &a, const edgepair_t &b) const
      {
        return a.first == b.first && a.second == b.second;
      }
    };

    struct edgepairless
    {
      bool operator()(const edgepair_t &a, const edgepair_t &b)
      {
        return less(a, b);
      }
      static bool less(const edgepair_t &a, const edgepair_t &b)
      {
        return a.first < b.first || a.first == b.first && a.second < b.second;
      }
    };

    #ifdef HAVE_HASH_MAP
      struct edgepairhash
      {
        unsigned int operator()(const edgepair_t &a) const
        {
        #if defined(__ECC) || defined(_MSC_VER)
          hash_compare<unsigned int> h;
        #else
          hash<unsigned int> h;
        #endif
          return h((a.first<<3) ^ a.second);
        }
        
        #if defined(__ECC) || defined(_MSC_VER)

        // These are particularly needed by ICC's hash stuff
        static const size_t bucket_size = 4;
        static const size_t min_buckets = 8;
        
        // This is a less than function.
        bool operator()(const edgepair_t & a, const edgepair_t & b) const 
        {
          return edgepairless::less(a,b);
        }
        #endif // endif ifdef __ICC
      };

    #if defined(__ECC) || defined(_MSC_VER)
      typedef hash_map<edgepair_t, 
                       VMesh::Node::index_type, 
                       edgepairhash> edge_hash_type;
    #else
      typedef hash_map<edgepair_t,
                       VMesh::Node::index_type,
                       edgepairhash,
                       edgepairequal> edge_hash_type;
    #endif
    
    #else
      typedef std::map<edgepair_t,
                  VMesh::Node::index_type,
                  edgepairless> edge_hash_type;
    #endif


  VMesh::Node::index_type lookup(VMesh *refined,
                                 edge_hash_type &edgemap,
                                 VMesh::Node::index_type a,
                                 VMesh::Node::index_type b,
                                 double factor,
                                 std::vector<double>& ivalues)
  {
    edgepair_t ep;
    ep.first = a; ep.second = b;
    const edge_hash_type::iterator loc = edgemap.find(ep);
    if (loc == edgemap.end())
    {
      Point pa, pb;
      refined->get_point(pa, a);
      refined->get_point(pb, b);
      const Point inbetween = ((1.0-factor)*pa + (factor)*pb).asPoint();
      const VMesh::Node::index_type newnode = refined->add_point(inbetween);
      ivalues.push_back(((1.0-factor)*ivalues[a]+(factor)*ivalues[b]));
      edgemap[ep] = newnode;
      return newnode;
    }
    else
    {
      return (*loc).second;
    }
  }

  Point RIinterpolate(VMesh *refined,
                      VMesh::Node::array_type& onodes,
                      double coords[2])
  {
    Point result(0.0, 0.0, 0.0);
    
    double w[4];
    const double x = coords[0], y = coords[1];  
    w[0] = (-1 + x) * (-1 + y);
    w[1] = -x * (-1 + y);
    w[2] = x * y;
    w[3] = -(-1 + x) * y;

    Point p;
    for (int i = 0; i < 4; i++)
    {
      refined->get_point(p, onodes[i]);
      result += (p * w[i]).asVector();
    }
    return result;  
  }


  double RIinterpolateV(std::vector<double>& ivalues,
                        VMesh::Node::array_type& onodes,
                        double coords[2])
  {
    double w[4];
    const double x = coords[0], y = coords[1];  
    w[0] = (-1 + x) * (-1 + y);
    w[1] = -x * (-1 + y);
    w[2] = x * y;
    w[3] = -(-1 + x) * y;

    return(w[0]*ivalues[onodes[0]] + w[1]*ivalues[onodes[1]] + 
             w[2]*ivalues[onodes[2]] + w[3]*ivalues[onodes[3]]);
  }
                      
  void dice(VMesh *refined, edge_hash_type &emap,
             VMesh::Node::array_type onodes,
             VMesh::index_type index, 
             VMesh::mask_type mask,
             VMesh::size_type maxnode,
             std::vector<double>& ivalues,
             std::vector<double>& evalues,
             double vv,
             int basis_order);

};


void
RefineMeshQuadSurfAlgoV::dice(VMesh *refined,
                         edge_hash_type &emap,
                         VMesh::Node::array_type onodes,
                         VMesh::index_type index,
                         VMesh::mask_type mask,
                         VMesh::size_type maxnode,
                         std::vector<double>& ivalues,
                         std::vector<double>& evalues,
                         double vv,
                         int basis_order)
{
  const VMesh::index_type i0 = onodes[index];
  const VMesh::index_type i1 = onodes[(index+1)%4];
  const VMesh::index_type i2 = onodes[(index+2)%4];
  const VMesh::index_type i3 = onodes[(index+3)%4];

  const double f13 = 1.0/3.0;
  const double f12 = 1.0/2.0;

  const int tab[4][2] = {{0,0}, {1, 0}, {1, 1}, {0, 1}};
  double coords[2];
  VMesh::Node::array_type nnodes(4);

  if (mask == (0xF & ~(1<<index)))
  {
    const VMesh::index_type i4 = lookup(refined, emap, i0, i1, f13, ivalues);
    const VMesh::index_type i5 = lookup(refined, emap, i4, i1, f12, ivalues);

    const VMesh::index_type i6 = lookup(refined, emap, i1, i2, f13, ivalues);
    const VMesh::index_type i7 = lookup(refined, emap, i6, i2, f12, ivalues);

    const VMesh::index_type i8 = lookup(refined, emap, i2, i3, f13, ivalues);
    const VMesh::index_type i9 = lookup(refined, emap, i8, i3, f12, ivalues);

    const VMesh::index_type i10 = lookup(refined, emap, i3, i0, f13, ivalues);
    const VMesh::index_type i11 = lookup(refined, emap, i10, i0, f12, ivalues);
    
    coords[0] = tab[index][0] * f13 + f13;
    coords[1] = tab[index][1] * f13 + f13;
    Point p12 = RIinterpolate(refined, onodes, coords);
    if (basis_order==1) ivalues.push_back(RIinterpolateV(ivalues,onodes,coords));

    coords[0] = tab[(index+1)%4][0] * f13 + f13;
    coords[1] = tab[(index+1)%4][1] * f13 + f13;
    Point p13 = RIinterpolate(refined, onodes, coords);
    if (basis_order==1) ivalues.push_back(RIinterpolateV(ivalues,onodes,coords));

    coords[0] = tab[(index+2)%4][0] * f13 + f13;
    coords[1] = tab[(index+2)%4][1] * f13 + f13;
    Point p14 = RIinterpolate(refined, onodes, coords);
    if (basis_order==1) ivalues.push_back(RIinterpolateV(ivalues,onodes,coords));

    coords[0] = tab[(index+3)%4][0] * f13 + f13;
    coords[1] = tab[(index+3)%4][1] * f13 + f13;
    Point p15 = RIinterpolate(refined, onodes, coords);
    if (basis_order==1) ivalues.push_back(RIinterpolateV(ivalues,onodes,coords));

    const VMesh::Node::index_type i12 = refined->add_point(p12);
    const VMesh::Node::index_type i13 = refined->add_point(p13);
    const VMesh::Node::index_type i14 = refined->add_point(p14);
    const VMesh::Node::index_type i15 = refined->add_point(p15);

    nnodes[0] = i0; nnodes[1] = i4; nnodes[2] = i12; nnodes[3] = i11;
    refined->add_elem(nnodes);
    
    nnodes[0] = i4; nnodes[1] = i5; nnodes[2] = i13; nnodes[3] = i12;
    refined->add_elem(nnodes);

    nnodes[0] = i5; nnodes[1] = i1; nnodes[2] = i6; nnodes[3] = i13;
    refined->add_elem(nnodes);

    nnodes[0] = i11; nnodes[1] = i12; nnodes[2] = i15; nnodes[3] = i10;
    refined->add_elem(nnodes);
    
    nnodes[0] = i12; nnodes[1] = i13; nnodes[2] = i14; nnodes[3] = i15;
    refined->add_elem(nnodes);
    
    nnodes[0] = i13; nnodes[1] = i6; nnodes[2] = i7; nnodes[3] = i14;
    refined->add_elem(nnodes);

    nnodes[0] = i10; nnodes[1] = i15; nnodes[2] = i9; nnodes[3] = i3;
    refined->add_elem(nnodes);
    
    nnodes[0] = i15; nnodes[1] = i14; nnodes[2] = i8; nnodes[3] = i9;
    refined->add_elem(nnodes);
    
    nnodes[0] = i14; nnodes[1] = i7; nnodes[2] = i2; nnodes[3] = i8;
    refined->add_elem(nnodes);
    
    if (basis_order == 0)
    {
      evalues.push_back(vv);
      evalues.push_back(vv);
      evalues.push_back(vv);
      evalues.push_back(vv);
      evalues.push_back(vv);
      evalues.push_back(vv);
      evalues.push_back(vv);
      evalues.push_back(vv);
      evalues.push_back(vv);
    }
    
    return;
  }
 
  
  if (mask & (1<<((index+2)%4)))
  {
    coords[0] = tab[index][0] * f13 + f13;
    coords[1] = tab[index][1] * f13 + f13;
  }
  else
  {
    coords[0] = f12;
    coords[1] = f12;
  }
  
  Point interior = RIinterpolate(refined, onodes, coords);
  const VMesh::Node::index_type interior_node = refined->add_point(interior);
  if (refined->basis_order() == 1) ivalues.push_back(RIinterpolateV(ivalues,onodes,coords));
  
  nnodes[0] = i0;
  if (mask & (1<<((index+1)%4)))
    nnodes[1] = lookup(refined, emap, i0, i1, f13, ivalues);
  else
    nnodes[1] = lookup(refined, emap, i0, i1, f12, ivalues);
  nnodes[2] = interior_node;
  
  if (mask & (1<<((index+3)%4)))
    nnodes[3] = lookup(refined, emap, i0, i3, f13, ivalues);
  else
    nnodes[3] = lookup(refined, emap, i0, i3, f12, ivalues);
    
  refined->add_elem(nnodes);
  if (basis_order == 0) evalues.push_back(vv);

  if (mask & (1<<((index+1)%4)))
    nnodes[0] = lookup(refined, emap, i0, i1, f13, ivalues);
  else
    nnodes[0] = lookup(refined, emap, i0, i1, f12, ivalues);
  
  nnodes[1] = i1;
  nnodes[2] = i2;
  nnodes[3] = interior_node;
  
  if ((mask & (1<<((index+1)%4))) && (mask & (1<<((index+2)%4))))
  {
    if (i1 < i2)
    {
      dice(refined, emap, nnodes, 1, 1<<2, maxnode,ivalues,evalues,vv,basis_order);
    }
    else
    {
      dice(refined, emap, nnodes, 2, 1<<1, maxnode,ivalues,evalues,vv,basis_order);
    }
  }
  else if (mask & (1<<((index+1)%4)))
  {
    dice(refined, emap, nnodes, 1, 0, maxnode,ivalues,evalues,vv,basis_order);
  }
  else if (mask & (1<<((index+2)%4)))
  {
    dice(refined, emap, nnodes, 2, 0, maxnode,ivalues,evalues,vv,basis_order);
  }
  else
  {
    refined->add_elem(nnodes);
    if (basis_order == 0) evalues.push_back(vv);
  }

  if (mask & (1<<((index+3)%4)))
    nnodes[0] = lookup(refined, emap, i0, i3, f13, ivalues);
  else
    nnodes[0] = lookup(refined, emap, i0, i3, f12, ivalues);
  
  nnodes[1] = interior_node;
  nnodes[2] = i2;
  nnodes[3] = i3;
  
  if ((mask & (1<<((index+2)%4))) && (mask & (1<<((index+3)%4))))
  {
    if (i2 < i3)
    {
      dice(refined, emap, nnodes, 2, 1<<3, maxnode,ivalues,evalues,vv,basis_order);
    }
    else
    {
      dice(refined, emap, nnodes, 3, 1<<2, maxnode,ivalues,evalues,vv,basis_order);
    }
  }
  else if (mask & (1<<((index+2)%4)))
  {
    dice(refined, emap, nnodes, 2, 0, maxnode,ivalues,evalues,vv,basis_order);
  }
  else if (mask & (1<<((index+3)%4)))
  {
    dice(refined, emap, nnodes, 3, 0, maxnode,ivalues,evalues,vv,basis_order);
  }
  else
  {
    refined->add_elem(nnodes);
    if (basis_order == 0) evalues.push_back(vv);
  }
}


bool
RefineMeshQuadSurfAlgoV::
run(AlgoBase* algo, FieldHandle input, FieldHandle& output,
                  std::string select, double isoval)
{
  // Obtain information on what type of input field we have
  FieldInformation fi(input);
  // Alter the input so it will become a QuadSurf
  fi.make_quadsurfmesh();
  output = CreateField(fi);
  
  if (output.get_rep() == 0)
  {
    algo->error("Could not create an output field");
    algo->algo_end(); return (false);
  }

  VField* field   = input->vfield();
  VMesh*  mesh    = input->vmesh();
  VMesh*  refined = output->vmesh();
  VField* rfield  = output->vfield();

  edge_hash_type emap;
  VMesh::Node::array_type onodes(4);
 
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
      algo->error("Unknown region selection method encountered");
      algo->algo_end(); return (false);
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
      algo->error("Unknown region selection method encountered");
      algo->algo_end(); return (false);
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

  VMesh::size_type maxnode = mesh->num_nodes();

  VMesh::Elem::iterator bi, ei;
  mesh->begin(bi); mesh->end(ei);
  while (bi != ei)
  {
    mesh->get_nodes(onodes, *bi);

    // Get the values and compute an inside/outside mask.
    VMesh::mask_type inside = 0;
    for (size_t i = 0; i < onodes.size(); i++)
    {
      if (values[onodes[i]] == true)
      {
        inside |= (1<<i);
      }
    }

    if (inside == 0)
    {
      // Nodes are the same order, so just add the element.
      refined->add_elem(onodes);
      if (rfield->basis_order() == 0) evalues.push_back(ivalues[*bi]);
    }
    else
    {
      int which = -1;
      for (int i = 0; i < 4; i++)
      {
        if (inside & (1 << i))
        {
          if (which < 0 || onodes[which] > onodes[i])
          {
            which = i;
          }
        }
      }
      if (which >= 0)
      {
        double vv = 0.0;
        if (rfield->basis_order() == 0) vv = ivalues[*bi];
        dice(refined, emap, onodes, which, inside & ~(1<<which),maxnode,ivalues,evalues,vv, rfield->basis_order());
      }
    }
    ++bi;
  }  

  rfield->resize_values();
  if (rfield->basis_order() == 0) rfield->set_values(evalues);
  if (rfield->basis_order() == 1) rfield->set_values(ivalues);
  rfield->copy_properties(field);

  algo->algo_end(); return (true);
}


/////////////////////////////////////////////
// Refinement code for hexahedral elements II


class RefineMeshHexVolAlgoV
{
  public:  

    bool run(AlgoBase* algo, FieldHandle input, FieldHandle& output,
               bool convex, std::string select, double isoval);

  protected:
  
    struct edgepair_t
    {
      VMesh::index_type first;
      VMesh::index_type second;
    };

    struct edgepairequal
    {
      bool operator()(const edgepair_t &a, const edgepair_t &b) const
      {
        return a.first == b.first && a.second == b.second;
      }
    };

    struct edgepairless
    {
      bool operator()(const edgepair_t &a, const edgepair_t &b)
      {
        return less(a, b);
      }
      static bool less(const edgepair_t &a, const edgepair_t &b)
      {
        return a.first < b.first || a.first == b.first && a.second < b.second;
      }
    };

    #ifdef HAVE_HASH_MAP
      struct edgepairhash
      {
        unsigned int operator()(const edgepair_t &a) const
        {
        #if defined(__ECC) || defined(_MSC_VER)
          hash_compare<unsigned int> h;
        #else
          hash<unsigned int> h;
        #endif
          return h((a.first<<3) ^ a.second);
        }
        
        #if defined(__ECC) || defined(_MSC_VER)
          // These are particularly needed by ICC's hash stuff
          static const size_t bucket_size = 4;
          static const size_t min_buckets = 8;
          
          // This is a less than function.
          bool operator()(const edgepair_t & a, const edgepair_t & b) const 
          {
            return edgepairless::less(a,b);
          }
        #endif // endif ifdef __ICC
      };

      #if defined(__ECC) || defined(_MSC_VER)
        typedef hash_map<edgepair_t,
                         VMesh::index_type,
                         edgepairhash> edge_hash_type;
      #else
        typedef hash_map<edgepair_t,
                         VMesh::index_type,
                         edgepairhash,
                         edgepairequal> edge_hash_type;
      #endif
    #else
      typedef std::map<edgepair_t,
                VMesh::Node::index_type,
                edgepairless> edge_hash_type;
    #endif

                                     
    Point RIinterpolate(VMesh *refined,
                        VMesh::Node::array_type& onodes,
                        const Point &coordsp)
    {
      Point result(0.0, 0.0, 0.0);
    
      double w[8];
      const double x = coordsp.x(), y = coordsp.y(), z = coordsp.z();
      w[0] = -((-1 + x)*(-1 + y)*(-1 + z));
      w[1] = x*(-1 + y)*(-1 + z);
      w[2] = -(x*y*(-1 + z));
      w[3] = (-1 + x)*y*(-1 + z);
      w[4] = (-1 + x)*(-1 + y)*z;
      w[5] = -(x*(-1 + y)*z);
      w[6] = x*y*z;
      w[7] = -((-1 + x)*y*z);

      Point p;
      for (int i = 0; i < 8; i++)
      {
        refined->get_point(p, onodes[i]);
        result += (p * w[i]).asVector();
      }
      
      return result;
    }


    double RIinterpolateV(std::vector<double>& ivalues,
                        VMesh::Node::array_type& onodes,
                        const Point &coordsp)
    {
      double w[8];
      const double x = coordsp.x(), y = coordsp.y(), z = coordsp.z();
      w[0] = -((-1 + x)*(-1 + y)*(-1 + z));
      w[1] = x*(-1 + y)*(-1 + z);
      w[2] = -(x*y*(-1 + z));
      w[3] = (-1 + x)*y*(-1 + z);
      w[4] = (-1 + x)*(-1 + y)*z;
      w[5] = -(x*(-1 + y)*z);
      w[6] = x*y*z;
      w[7] = -((-1 + x)*y*z);

      return (w[0]*ivalues[onodes[0]] + w[1]*ivalues[onodes[1]] +
        w[2]*ivalues[onodes[2]] + w[3]*ivalues[onodes[3]] +
        w[4]*ivalues[onodes[4]] + w[5]*ivalues[onodes[5]] +
        w[6]*ivalues[onodes[6]] + w[7]*ivalues[onodes[7]]);
    }

    VMesh::Node::index_type add_point(VMesh *refined,
                                      VMesh::Node::array_type &nodes,
                                      const int *reorder, 
                                      unsigned int a, 
                                      unsigned int b,
                                      double factor,
                                      std::vector<double>& ivalues,
                                      int basis_order)
    {
      Point coordsp;
      
      unsigned int ra = reorder[a];
      unsigned int rb = reorder[b];
      coordsp = Interpolate(hcoords[ra], hcoords[rb], factor);
      
      const Point inbetween = RIinterpolate(refined, nodes, coordsp);
      if (basis_order == 1) ivalues.push_back(RIinterpolateV(ivalues,nodes,coordsp));
      return refined->add_point(inbetween);
    }

    VMesh::Node::index_type add_point_convex(VMesh *refined,
                                      VMesh::Node::array_type &nodes,
                                      const int *reorder, 
                                      VMesh::index_type a, 
                                      VMesh::index_type b,
                                      std::vector<double>& ivalues,
                                      int basis_order)
    {
      VMesh::index_type ra = reorder[a];
      VMesh::index_type rb = reorder[b];
      Point coordsp = Interpolate(hcoords[ra], hcoords[rb], 1.0/3.0);
      
      const Point inbetween = RIinterpolate(refined, nodes, coordsp);

      if (basis_order == 1) ivalues.push_back(RIinterpolateV(ivalues,nodes,coordsp));
      return refined->add_point(inbetween);
    }


    VMesh::Node::index_type add_point_convex(VMesh *refined,
                                      VMesh::Node::array_type &nodes,
                                      Point coordsp,
                                      std::vector<double>& ivalues,
                                      int basis_order)
    {
      const Point inbetween = RIinterpolate(refined, nodes, coordsp);
      if (basis_order == 1) ivalues.push_back(RIinterpolateV(ivalues,nodes,coordsp));
      return refined->add_point(inbetween);
    }


    VMesh::Node::index_type lookup(VMesh *refined,
                                   edge_hash_type &edgemap,
                                   VMesh::Node::array_type &nodes,
                                   const int *reorder, 
                                   VMesh::index_type a, 
                                   VMesh::index_type b,
                                   double factor,
                                   std::vector<double>& ivalues,
                                   int basis_order)
    {
      edgepair_t ep;
      ep.first = nodes[reorder[a]]; 
      ep.second = nodes[reorder[b]];
      const edge_hash_type::iterator loc = edgemap.find(ep);
      if (loc == edgemap.end())
      {
        const VMesh::Node::index_type newnode =
          add_point(refined, nodes, reorder, a, b, factor,ivalues,basis_order);
        edgemap[ep] = newnode;
        return newnode;
      }
      else
      {
        return (*loc).second;
      }
    }

    VMesh::Node::index_type lookup_convex(VMesh *refined,
                                   edge_hash_type &edgemap,
                                   VMesh::Node::array_type &onodes,
                                   const int *reorder, 
                                   VMesh::index_type a, 
                                   VMesh::index_type b,
                                   std::vector<double>& ivalues,
                                   int basis_order)
    {
      edgepair_t ep;
      ep.first = onodes[reorder[a]]; 
      ep.second = onodes[reorder[b]];
      const edge_hash_type::iterator loc = edgemap.find(ep);
      if (loc == edgemap.end())
      {
        const VMesh::Node::index_type newnode =
          add_point_convex(refined, onodes, reorder, a, b,ivalues,basis_order);
        edgemap[ep] = newnode;
        return newnode;
      }
      else
      {
        return (*loc).second;
      }
    }


    inline unsigned int iedge(unsigned int a, unsigned int b)
    {
      return (1<<(7-a)) | (1<<(7-b));
    }

    inline unsigned int iface(unsigned int a, unsigned int b,
                       unsigned int c, unsigned int d)
    {
      return iedge(a, b) | iedge(c, d);
    }

    inline unsigned int iface3(unsigned int a, unsigned int b, unsigned int c)
    {
      return (1<<(7-a)) | (1<<(7-b)) | (1<<(7-c));
    }

    inline void set_table(int i, int pattern, int reorder)
    {
      pattern_table[i][0] = pattern;
      pattern_table[i][1] = reorder;
    }

    void init_pattern_table()
    {
      for (int i = 0; i < 256; i++)
      {
        set_table(i, -1, 0);
      }

      set_table(0, 0, 0);

      // Add corners
      set_table(1, 1, 7);
      set_table(2, 1, 6);
      set_table(4, 1, 5);
      set_table(8, 1, 4);
      set_table(16, 1, 3);
      set_table(32, 1, 2);
      set_table(64, 1, 1);
      set_table(128, 1, 0);

      // Add edges
      set_table(iedge(0, 1), 2, 0);
      set_table(iedge(1, 2), 2, 1);
      set_table(iedge(2, 3), 2, 2);
      set_table(iedge(3, 0), 2, 3);
      set_table(iedge(4, 5), 2, 5);
      set_table(iedge(5, 6), 2, 6);
      set_table(iedge(6, 7), 2, 7);
      set_table(iedge(7, 4), 2, 4);
      set_table(iedge(0, 4), 2, 8);
      set_table(iedge(1, 5), 2, 9);
      set_table(iedge(2, 6), 2, 10);
      set_table(iedge(3, 7), 2, 11);

      set_table(iface(0, 1, 2, 3), 4, 0);
      set_table(iface(0, 1, 5, 4), 4, 12);
      set_table(iface(1, 2, 6, 5), 4, 9);
      set_table(iface(2, 3, 7, 6), 4, 13);
      set_table(iface(3, 0, 4, 7), 4, 8);
      set_table(iface(4, 5, 6, 7), 4, 7);

      set_table(iface3(0, 1, 2), -4, 0);
      set_table(iface3(0, 1, 3), -4, 0);
      set_table(iface3(0, 2, 3), -4, 0);
      set_table(iface3(1, 2, 3), -4, 0);
      set_table(iedge(0, 2), -4, 0);
      set_table(iedge(1, 3), -4, 0);

      set_table(iface3(0, 1, 5), -4, 12);
      set_table(iface3(0, 1, 4), -4, 12);
      set_table(iface3(0, 5, 4), -4, 12);
      set_table(iface3(1, 5, 4), -4, 12);
      set_table(iedge(0, 5), -4, 0);
      set_table(iedge(1, 4), -4, 0);

      set_table(iface3(1, 2, 6), -4, 9);
      set_table(iface3(1, 2, 5), -4, 9);
      set_table(iface3(1, 6, 5), -4, 9);
      set_table(iface3(2, 6, 5), -4, 9);
      set_table(iedge(1, 6), -4, 0);
      set_table(iedge(2, 5), -4, 0);

      set_table(iface3(2, 3, 7), -4, 13);
      set_table(iface3(2, 3, 6), -4, 13);
      set_table(iface3(2, 7, 6), -4, 13);
      set_table(iface3(3, 7, 6), -4, 13);
      set_table(iedge(2, 7), -4, 0);
      set_table(iedge(3, 6), -4, 0);

      set_table(iface3(3, 0, 4), -4, 8);
      set_table(iface3(3, 0, 7), -4, 8);
      set_table(iface3(3, 4, 7), -4, 8);
      set_table(iface3(0, 4, 7), -4, 8);
      set_table(iedge(3, 4), -4, 0);
      set_table(iedge(0, 7), -4, 0);

      set_table(iface3(4, 5, 6), -4, 7);
      set_table(iface3(4, 5, 7), -4, 7);
      set_table(iface3(4, 6, 7), -4, 7);
      set_table(iface3(5, 6, 7), -4, 7);
      set_table(iedge(4, 6), -4, 0);
      set_table(iedge(5, 7), -4, 0);
                       
      set_table(255, 8, 0);

      for (int i = 0; i < 8; i++)
      {
        hcoords[i] = Point(hcoords_double[i][0],
                           hcoords_double[i][1],
                           hcoords_double[i][2]);
      }
    }

    void dice(VMesh *refined, 
              edge_hash_type &emap,
              VMesh::Node::array_type nodes,
              VMesh::index_type index, 
              VMesh::mask_type mask,
              std::vector<double>& ivalues,
              std::vector<double>& evalues,
              double vv,
              int basis_order);


  private:
  
    Point hcoords[8];
    int pattern_table[256][2];
    
    static int hex_reorder_table[14][8];
    static double hcoords_double[8][3];
    
    VMesh::index_type maxnode;
};



int RefineMeshHexVolAlgoV::hex_reorder_table[14][8] = {
  {0, 1, 2, 3, 4, 5, 6, 7},
  {1, 2, 3, 0, 5, 6, 7, 4},
  {2, 3, 0, 1, 6, 7, 4, 5},
  {3, 0, 1, 2, 7, 4, 5, 6},

  {4, 7, 6, 5, 0, 3, 2, 1},
  {5, 4, 7, 6, 1, 0, 3, 2},
  {6, 5, 4, 7, 2, 1, 0, 3},
  {7, 6, 5, 4, 3, 2, 1, 0},

  {4, 0, 3, 7, 5, 1, 2, 6},
  {1, 5, 6, 2, 0, 4, 7, 3},
  {6, 2, 1, 5, 7, 3, 0, 4},
  {3, 7, 4, 0, 2, 6, 5, 1},

  {1, 0, 4, 5, 2, 3, 7, 6},
  {3, 2, 6, 7, 0, 1, 5, 4}
};


double RefineMeshHexVolAlgoV::hcoords_double[8][3] = {
  { 0.0, 0.0, 0.0},
  { 1.0, 0.0, 0.0},
  { 1.0, 1.0, 0.0},
  { 0.0, 1.0, 0.0},
  { 0.0, 0.0, 1.0},
  { 1.0, 0.0, 1.0},
  { 1.0, 1.0, 1.0},
  { 0.0, 1.0, 1.0}
};




void
RefineMeshHexVolAlgoV::dice(VMesh *refined,
                           edge_hash_type &emap,
                           VMesh::Node::array_type nodes,
                           VMesh::index_type index,
                           VMesh::mask_type mask,
                           std::vector<double>& ivalues,
                           std::vector<double>& evalues,
                           double vv,
                           int basis_order)
{
  const int *ro = hex_reorder_table[index];

  const double f1 = 1/3.0;
  const double f2 = 1/2.0;
  
  double m1 = f2; if (mask & (1 << ro[1])) m1 = f1;
  double m2 = f2; if (mask & (1 << ro[2])) m2 = f1;
  double m3 = f2; if (mask & (1 << ro[3])) m3 = f1;
  double m4 = f2; if (mask & (1 << ro[4])) m4 = f1;
  double m5 = f2; if (mask & (1 << ro[5])) m5 = f1;
  double m6 = f2; if (mask & (1 << ro[6])) m6 = f1;
  double m7 = f2; if (mask & (1 << ro[7])) m7 = f1;
  
  const VMesh::Node::index_type i06node =
    add_point(refined, nodes, ro, 0, 6, m6,ivalues,basis_order);

  VMesh::Node::array_type nnodes(8);



  // Add this corner.
  nnodes[0] = nodes[ro[0]];
  nnodes[1] = lookup(refined, emap, nodes, ro, 0, 1,m1,ivalues,basis_order);
  nnodes[2] = lookup(refined, emap, nodes, ro, 0, 2,m2,ivalues,basis_order);
  nnodes[3] = lookup(refined, emap, nodes, ro, 0, 3,m3,ivalues,basis_order);
  nnodes[4] = lookup(refined, emap, nodes, ro, 0, 4,m4,ivalues,basis_order);
  nnodes[5] = lookup(refined, emap, nodes, ro, 0, 5,m5,ivalues,basis_order);
  nnodes[6] = i06node;
  nnodes[7] = lookup(refined, emap, nodes, ro, 0, 7,m7,ivalues,basis_order);
  refined->add_elem(nnodes);
  if (basis_order == 0) evalues.push_back(vv);

  // Add the other three pieces.
  nnodes[0] = lookup(refined, emap, nodes, ro, 0, 1,m1,ivalues,basis_order);
  nnodes[1] = nodes[ro[1]];
  nnodes[2] = nodes[ro[2]];
  nnodes[3] = lookup(refined, emap, nodes, ro, 0, 2,m2,ivalues,basis_order);
  nnodes[4] = lookup(refined, emap, nodes, ro, 0, 5,m5,ivalues,basis_order);
  nnodes[5] = nodes[ro[5]];
  nnodes[6] = nodes[ro[6]];
  nnodes[7] = i06node;
  
  {
    const bool a = (mask & (1 << ro[1]))?1:0;
    const bool b = (mask & (1 << ro[2]))?1:0;
    const bool c = (mask & (1 << ro[5]))?1:0;
    const bool d = (mask & (1 << ro[6]))?1:0;
    unsigned int mask1 = (a << 1) | (b << 2) | (c << 5) | (d << 6);
    int which = -1;
    for (int i = 0; i < 8; i++)
    {
      if (mask1 & (1 << i))
      {
        if (which < 0 || nnodes[which] > nnodes[i])
        {
          which = i;
        }
      }
    }
    if (which >= 0)
    {
      dice(refined, emap, nnodes, which, mask1,ivalues,evalues,vv,basis_order);
    }
    else
    {
      refined->add_elem(nnodes);
      if (basis_order == 0) evalues.push_back(vv);
    }
  }

  nnodes[0] = lookup(refined, emap, nodes, ro, 0, 3,m3,ivalues,basis_order);
  nnodes[1] = lookup(refined, emap, nodes, ro, 0, 2,m2,ivalues,basis_order);
  nnodes[2] = nodes[ro[2]];
  nnodes[3] = nodes[ro[3]];
  nnodes[4] = lookup(refined, emap, nodes, ro, 0, 7,m7,ivalues,basis_order);
  nnodes[5] = i06node;
  nnodes[6] = nodes[ro[6]];
  nnodes[7] = nodes[ro[7]];
  {
    const bool a = (mask & (1 << ro[2]))?1:0;
    const bool b = (mask & (1 << ro[3]))?1:0;
    const bool c = (mask & (1 << ro[6]))?1:0;
    const bool d = (mask & (1 << ro[7]))?1:0;
    unsigned int mask1 = (a << 2) | (b << 3) | (c << 6) | (d << 7);
    int which = -1;
    for (int i = 0; i < 8; i++)
    {
      if (mask1 & (1 << i))
      {
        if (which < 0 || nnodes[which] > nnodes[i])
        {
          which = i;
        }
      }
    }
    if (which >= 0)
    {
      dice(refined, emap, nnodes, which, mask1,ivalues,evalues,vv,basis_order);
    }
    else
    {
      refined->add_elem(nnodes);
      if (basis_order == 0) evalues.push_back(vv);
    }
  }
      
  nnodes[0] = lookup(refined, emap, nodes, ro, 0, 4,m4,ivalues,basis_order);
  nnodes[1] = lookup(refined, emap, nodes, ro, 0, 5,m5,ivalues,basis_order);
  nnodes[2] = i06node;
  nnodes[3] = lookup(refined, emap, nodes, ro, 0, 7,m7,ivalues,basis_order);
  nnodes[4] = nodes[ro[4]];
  nnodes[5] = nodes[ro[5]];
  nnodes[6] = nodes[ro[6]];
  nnodes[7] = nodes[ro[7]];
  {
    const bool a = (mask & (1 << ro[4]))?1:0;
    const bool b = (mask & (1 << ro[5]))?1:0;
    const bool c = (mask & (1 << ro[6]))?1:0;
    const bool d = (mask & (1 << ro[7]))?1:0;
    unsigned int mask1 = (a << 4) | (b << 5) | (c << 6) | (d << 7);
    int which = -1;
    for (int i = 0; i < 8; i++)
    {
      if (mask1 & (1 << i))
      {
        if (which < 0 || nnodes[which] > nnodes[i])
        {
          which = i;
        }
      }
    }
    if (which >= 0)
    {
      dice(refined, emap, nnodes, which, mask1,ivalues,evalues,vv,basis_order);
    }
    else
    {
      refined->add_elem(nnodes);
      if (basis_order == 0) evalues.push_back(vv);
    }
  }
}



bool
RefineMeshHexVolAlgoV::
run(AlgoBase* algo, FieldHandle input, FieldHandle& output,
               bool convex, std::string select, double isoval)
{
  // Obtain information on what type of input field we have
  FieldInformation fi(input);
  // Alter the input so it will become a QuadSurf
  fi.make_hexvolmesh();
  output = CreateField(fi);
  
  if (output.get_rep() == 0)
  {
    algo->error("Could not create an output field");
    algo->algo_end(); return (false);
  }

  VField* field   = input->vfield();
  VMesh*  mesh    = input->vmesh();
  VMesh*  refined = output->vmesh();
  VField* rfield  = output->vfield();

  edge_hash_type emap;
  VMesh::Node::array_type onodes(8);
  VMesh::Node::array_type nnodes(8);
  
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

  std::vector<double> ivalues;
  std::vector<double> evalues;

  maxnode = mesh->num_nodes();

  init_pattern_table();

  // get all values, make computation easier
  VMesh::size_type num_nodes = mesh->num_nodes();
  VMesh::size_type num_elems = mesh->num_elems();

  // get all values, make computation easier
  std::vector<bool> values(num_nodes,false);

 
  // Deal with data stored at different locations
  // If data is on the elements make sure that all nodes
  // of that element pass requirement.
  
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
      algo->error("Unknown region selection method encountered");
      algo->algo_end(); return (false);
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
      algo->error("Unknown region selection method encountered");
      algo->algo_end(); return (false);
    }
  }
  else
  {
    for (size_t j=0;j<values.size();j++) values[j] = true;
  }


  if (convex)
  {
    int basis_order = rfield->basis_order();
    bool changed;
    do 
    {
      changed = false;
      VMesh::Elem::iterator bi, ei;
      mesh->begin(bi); mesh->end(ei);
      while (bi != ei)
      {
        mesh->get_nodes(onodes, *bi);
      
        // Get the values and compute an inside/outside mask.
        unsigned int inside = 0;
        for (unsigned int i = 0; i < onodes.size(); i++)
        {
          inside = inside << 1;
          if (values[onodes[i]])
          {
            inside |= 1;
          }
        }

        const int pattern = pattern_table[inside][0];
        const int which = pattern_table[inside][1];

        if (pattern == -1)
        {
          changed = true;
          for (unsigned int i = 0; i < onodes.size(); i++)
          {
            values[onodes[i]] = true;
          }
        }
        else if (pattern == -4)
        {
          changed = true;
          const int *ro = hex_reorder_table[which];

          for (unsigned int i = 0; i < 4; i++)
          {
            values[onodes[ro[i]]] = true;
          }
        }

        ++bi;
      }
    } 
    while (changed);  
  
  
    VMesh::Elem::iterator bi, ei;
    mesh->begin(bi); mesh->end(ei);
  
    unsigned int cnt = 0;
    unsigned int loopcnt = 0;

    VMesh::Elem::size_type sz; mesh->size(sz);
    
    while (bi != ei)
    {
      cnt++; if (cnt == 100) { loopcnt +=cnt; cnt = 0; algo->update_progress(loopcnt,sz);  }

      mesh->get_nodes(onodes, *bi);
    
      // Get the values and compute an inside/outside mask.
      unsigned int inside = 0;
      unsigned int inside_count = 0;
      for (unsigned int i = 0; i < onodes.size(); i++)
      {
        inside = inside << 1;
        if (values[onodes[i]])
        {
          inside |= 1;
          inside_count++;
        }
      }

      const int pattern = pattern_table[inside][0];
      const int which = pattern_table[inside][1];
  
      if (pattern == 0)
      {
        // Nodes are the same order, so just add the element.
        refined->add_elem(onodes);
        if (basis_order == 0) evalues.push_back(ivalues[*bi]);
      }
      else if (pattern == 1)
      {
        const int *ro = hex_reorder_table[which];
        
        VMesh::Node::index_type i06node =
            add_point_convex(refined, onodes, ro, 0, 6,ivalues,basis_order);

        // Add this corner.
        nnodes[0] = onodes[ro[0]];
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 0, 1,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 0, 3,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 0, 4,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[6] = i06node;
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        refined->add_elem(nnodes);

        // Add the other three pieces.
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 1,ivalues,basis_order);
        nnodes[1] = onodes[ro[1]];
        nnodes[2] = onodes[ro[2]];
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[5] = onodes[ro[5]];
        nnodes[6] = onodes[ro[6]];
        nnodes[7] = i06node;
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 3,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[2] = onodes[ro[2]];
        nnodes[3] = onodes[ro[3]];
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        nnodes[5] = i06node;
        nnodes[6] = onodes[ro[6]];
        nnodes[7] = onodes[ro[7]];
        refined->add_elem(nnodes);
      
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 4,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[2] = i06node;
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        nnodes[4] = onodes[ro[4]];
        nnodes[5] = onodes[ro[5]];
        nnodes[6] = onodes[ro[6]];
        nnodes[7] = onodes[ro[7]];
        refined->add_elem(nnodes);
        if (basis_order == 0) evalues.insert(evalues.end(),4,ivalues[*bi]);
      }
      else if (pattern == 2)
      {
        const int *ro = hex_reorder_table[which];

       VMesh::Node::index_type i06node =
          add_point_convex(refined, onodes, ro, 0, 6,ivalues,basis_order);
       VMesh::Node::index_type i17node =
          add_point_convex(refined, onodes, ro, 1, 7,ivalues,basis_order);
       VMesh::Node::index_type i60node =
          add_point_convex(refined, onodes, ro, 6, 0,ivalues,basis_order);
       VMesh::Node::index_type i71node =
          add_point_convex(refined, onodes, ro, 7, 1,ivalues,basis_order);

        // Leading edge.
        nnodes[0] = onodes[ro[0]];
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 0, 1,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 0, 3,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 0, 4,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[6] = i06node;
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 1,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 0,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 1, 3,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 1, 4,ivalues,basis_order);
        nnodes[6] = i17node;
        nnodes[7] = i06node;
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 1, 0,ivalues,basis_order);
        nnodes[1] = onodes[ro[1]];
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 1, 2,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 1, 3,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 1, 4,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 1, 5,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 1, 6,ivalues,basis_order);
        nnodes[7] = i17node;
        refined->add_elem(nnodes);

        // Top center
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 3,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 3, 1,ivalues,basis_order);
        nnodes[3] = onodes[ro[3]];
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        nnodes[5] = i06node;
        nnodes[6] = i71node;
        nnodes[7] = onodes[ro[7]];
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 3,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 0,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 1,ivalues,basis_order);
        nnodes[4] = i06node;
        nnodes[5] = i17node;
        nnodes[6] = i60node;
        nnodes[7] = i71node;
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 1, 3,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 2,ivalues,basis_order);
        nnodes[2] = onodes[ro[2]];
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 2, 0,ivalues,basis_order);
        nnodes[4] = i17node;
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 1, 6,ivalues,basis_order);
        nnodes[6] = onodes[ro[6]];
        nnodes[7] = i60node;
        refined->add_elem(nnodes);

        // Front Center
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 4,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[2] = i06node;
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        nnodes[4] = onodes[ro[4]];
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 4, 1,ivalues,basis_order);
        nnodes[6] = i71node;
        nnodes[7] = onodes[ro[7]];
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 4,ivalues,basis_order);
        nnodes[2] = i17node;
        nnodes[3] = i06node;
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 4, 1,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 5, 0,ivalues,basis_order);
        nnodes[6] = i60node;
        nnodes[7] = i71node;
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 1, 4,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 5,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 1, 6,ivalues,basis_order);
        nnodes[3] = i17node;
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 5, 0,ivalues,basis_order);
        nnodes[5] = onodes[ro[5]];
        nnodes[6] = onodes[ro[6]];
        nnodes[7] = i60node;
        refined->add_elem(nnodes);

        // Outside wedges
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 3, 1,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 2, 0,ivalues,basis_order);
        nnodes[2] = onodes[ro[2]];
        nnodes[3] = onodes[ro[3]];
        nnodes[4] = i71node;
        nnodes[5] = i60node;
        nnodes[6] = onodes[ro[6]];
        nnodes[7] = onodes[ro[7]];
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 4, 1,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 5, 0,ivalues,basis_order);
        nnodes[2] = i60node;
        nnodes[3] = i71node;
        nnodes[4] = onodes[ro[4]];
        nnodes[5] = onodes[ro[5]];
        nnodes[6] = onodes[ro[6]];
        nnodes[7] = onodes[ro[7]];
        refined->add_elem(nnodes);
        
        if (basis_order == 0) evalues.insert(evalues.end(),11,ivalues[*bi]);
      }
      else if (pattern == 4)
      {
        const int *ro = hex_reorder_table[which];

        // Interior
       VMesh::Node::index_type i06node =
          add_point_convex(refined, onodes, ro, 0, 6,ivalues,basis_order);
       VMesh::Node::index_type i17node =
          add_point_convex(refined, onodes, ro, 1, 7,ivalues,basis_order);
       VMesh::Node::index_type i24node =
          add_point_convex(refined, onodes, ro, 2, 4,ivalues,basis_order);
       VMesh::Node::index_type i35node =
          add_point_convex(refined, onodes, ro, 3, 5,ivalues,basis_order);

        
        const Point i06 = Interpolate(hcoords[ro[0]], hcoords[ro[6]], 1.0/3.0);
        const Point i17 = Interpolate(hcoords[ro[1]], hcoords[ro[7]], 1.0/3.0);
        const Point i24 = Interpolate(hcoords[ro[2]], hcoords[ro[4]], 1.0/3.0);
        const Point i35 = Interpolate(hcoords[ro[3]], hcoords[ro[5]], 1.0/3.0);
        const Point i42a = Interpolate(hcoords[ro[4]], hcoords[ro[2]], 1.0/3.0);
        const Point i53a = Interpolate(hcoords[ro[5]], hcoords[ro[3]], 1.0/3.0);
        const Point i60a = Interpolate(hcoords[ro[6]], hcoords[ro[0]], 1.0/3.0);
        const Point i71a = Interpolate(hcoords[ro[7]], hcoords[ro[1]], 1.0/3.0);
        const Point i42 = Interpolate(i06, i42a, 0.5);
        const Point i53 = Interpolate(i17, i53a, 0.5);
        const Point i60 = Interpolate(i24, i60a, 0.5);
        const Point i71 = Interpolate(i35, i71a, 0.5);
        
       VMesh::Node::index_type i42node =
          add_point_convex(refined, onodes, i42,ivalues,basis_order);
       VMesh::Node::index_type i53node =
          add_point_convex(refined, onodes, i53,ivalues,basis_order);
       VMesh::Node::index_type i60node =
          add_point_convex(refined, onodes, i60,ivalues,basis_order);
       VMesh::Node::index_type i71node =
          add_point_convex(refined, onodes, i71,ivalues,basis_order);

        // Top Front
        nnodes[0] = onodes[ro[0]];
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 0, 1,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 0, 3,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 0, 4,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[6] = i06node;
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 1,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 0,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 1, 3,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 1, 4,ivalues,basis_order);
        nnodes[6] = i17node;
        nnodes[7] = i06node;
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 1, 0,ivalues,basis_order);
        nnodes[1] = onodes[ro[1]];
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 1, 2,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 1, 3,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 1, 4,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 1, 5,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 1, 6,ivalues,basis_order);
        nnodes[7] = i17node;
        refined->add_elem(nnodes);

        // Top Center
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 3,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 3, 1,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 0,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        nnodes[5] = i06node;
        nnodes[6] = i35node;
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 3, 4,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 3,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 0,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 1,ivalues,basis_order);
        nnodes[4] = i06node;
        nnodes[5] = i17node;
        nnodes[6] = i24node;
        nnodes[7] = i35node;
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 1, 3,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 2,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 1,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 2, 0,ivalues,basis_order);
        nnodes[4] = i17node;
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 1, 6,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 2, 5,ivalues,basis_order);
        nnodes[7] = i24node;
        refined->add_elem(nnodes);

        // Top Back
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 3, 0,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 3, 1,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 3, 2,ivalues,basis_order);
        nnodes[3] = onodes[ro[3]];
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 3, 4,ivalues,basis_order);
        nnodes[5] = i35node;
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 3, 6,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 3, 7,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 3, 1,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 2, 0,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 3,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 2,ivalues,basis_order);
        nnodes[4] = i35node;
        nnodes[5] = i24node;
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 2, 7,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 3, 6,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 2, 0,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 2, 1,ivalues,basis_order);
        nnodes[2] = onodes[ro[2]];
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 2, 3,ivalues,basis_order);
        nnodes[4] = i24node;
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 2, 5,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 2, 6,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 2, 7,ivalues,basis_order);
        refined->add_elem(nnodes);

        // Front
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 4,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[2] = i06node;
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        nnodes[4] = onodes[ro[4]];
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 4, 1,ivalues,basis_order);
        nnodes[6] = i42node;
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 4, 3,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 4,ivalues,basis_order);
        nnodes[2] = i17node;
        nnodes[3] = i06node;
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 4, 1,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 5, 0,ivalues,basis_order);
        nnodes[6] = i53node;
        nnodes[7] = i42node;
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 1, 4,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 5,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 1, 6,ivalues,basis_order);
        nnodes[3] = i17node;
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 5, 0,ivalues,basis_order);
        nnodes[5] = onodes[ro[5]];
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 5, 2,ivalues,basis_order);
        nnodes[7] = i53node;
        refined->add_elem(nnodes);

        // Center
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        nnodes[1] = i06node;
        nnodes[2] = i35node;
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 4,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 4, 3,ivalues,basis_order);
        nnodes[5] = i42node;
        nnodes[6] = i71node;
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 7, 0,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = i06node;
        nnodes[1] = i17node;
        nnodes[2] = i24node;
        nnodes[3] = i35node;
        nnodes[4] = i42node;
        nnodes[5] = i53node;
        nnodes[6] = i60node;
        nnodes[7] = i71node;
        refined->add_elem(nnodes);
 
        nnodes[0] = i17node;
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 6,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 5,ivalues,basis_order);
        nnodes[3] = i24node;
        nnodes[4] = i53node;
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 5, 2,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 6, 1,ivalues,basis_order);
        nnodes[7] = i60node;
        refined->add_elem(nnodes);

        // Back
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 3, 4,ivalues,basis_order);
        nnodes[1] = i35node;
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 3, 6,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 7,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 7, 0,ivalues,basis_order);
        nnodes[5] = i71node;
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 7, 2,ivalues,basis_order);
        nnodes[7] = onodes[ro[7]];
        refined->add_elem(nnodes);

        nnodes[0] = i35node;
        nnodes[1] = i24node;
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 7,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 6,ivalues,basis_order);
        nnodes[4] = i71node;
        nnodes[5] = i60node;
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 6, 3,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 7, 2,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = i24node;
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 2, 5,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 6,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 2, 7,ivalues,basis_order);
        nnodes[4] = i60node;
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 6, 1,ivalues,basis_order);
        nnodes[6] = onodes[ro[6]];
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 6, 3,ivalues,basis_order);
        refined->add_elem(nnodes);

        // Bottom Center
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 4, 3,ivalues,basis_order);
        nnodes[1] = i42node;
        nnodes[2] = i71node;
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 7, 0,ivalues,basis_order);
        nnodes[4] = onodes[ro[4]];
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 4, 1,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 7, 2,ivalues,basis_order);
        nnodes[7] = onodes[ro[7]];
        refined->add_elem(nnodes);

        nnodes[0] = i42node;
        nnodes[1] = i53node;
        nnodes[2] = i60node;
        nnodes[3] = i71node;
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 4, 1,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 5, 0,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 6, 3,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 7, 2,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = i53node;
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 5, 2,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 6, 1,ivalues,basis_order);
        nnodes[3] = i60node;
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 5, 0,ivalues,basis_order);
        nnodes[5] = onodes[ro[5]];
        nnodes[6] = onodes[ro[6]];
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 6, 3,ivalues,basis_order);
        refined->add_elem(nnodes);

        // Bottom
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 4, 1,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 5, 0,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 6, 3,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 7, 2,ivalues,basis_order);
        nnodes[4] = onodes[ro[4]];
        nnodes[5] = onodes[ro[5]];
        nnodes[6] = onodes[ro[6]];
        nnodes[7] = onodes[ro[7]];
        refined->add_elem(nnodes);
        if (basis_order == 0) evalues.insert(evalues.end(),22,ivalues[*bi]);
      }
      else if (pattern == 8)
      {
        const int *ro = hex_reorder_table[which];

        // Interior
       VMesh::Node::index_type i06node =
          add_point_convex(refined, onodes, ro, 0, 6,ivalues,basis_order);
       VMesh::Node::index_type i17node =
          add_point_convex(refined, onodes, ro, 1, 7,ivalues,basis_order);
       VMesh::Node::index_type i24node =
          add_point_convex(refined, onodes, ro, 2, 4,ivalues,basis_order);
       VMesh::Node::index_type i35node =
          add_point_convex(refined, onodes, ro, 3, 5,ivalues,basis_order);
       VMesh::Node::index_type i42node =
          add_point_convex(refined, onodes, ro, 4, 2,ivalues,basis_order);
       VMesh::Node::index_type i53node =
          add_point_convex(refined, onodes, ro, 5, 3,ivalues,basis_order);
       VMesh::Node::index_type i60node =
          add_point_convex(refined, onodes, ro, 6, 0,ivalues,basis_order);
       VMesh::Node::index_type i71node =
          add_point_convex(refined, onodes, ro, 7, 1,ivalues,basis_order);

        // Top Front
        nnodes[0] = onodes[ro[0]];
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 0, 1,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 0, 3,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 0, 4,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[6] = i06node;
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 1,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 0,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 1, 3,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 1, 4,ivalues,basis_order);
        nnodes[6] = i17node;
        nnodes[7] = i06node;
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 1, 0,ivalues,basis_order);
        nnodes[1] = onodes[ro[1]];
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 1, 2,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 1, 3,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 1, 4,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 1, 5,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 1, 6,ivalues,basis_order);
        nnodes[7] = i17node;
        refined->add_elem(nnodes);

        // Top Center
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 3,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 3, 1,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 0,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        nnodes[5] = i06node;
        nnodes[6] = i35node;
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 3, 4,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 2,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 3,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 0,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 1,ivalues,basis_order);
        nnodes[4] = i06node;
        nnodes[5] = i17node;
        nnodes[6] = i24node;
        nnodes[7] = i35node;
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 1, 3,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 2,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 1,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 2, 0,ivalues,basis_order);
        nnodes[4] = i17node;
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 1, 6,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 2, 5,ivalues,basis_order);
        nnodes[7] = i24node;
        refined->add_elem(nnodes);

        // Top Back
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 3, 0,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 3, 1,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 3, 2,ivalues,basis_order);
        nnodes[3] = onodes[ro[3]];
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 3, 4,ivalues,basis_order);
        nnodes[5] = i35node;
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 3, 6,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 3, 7,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 3, 1,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 2, 0,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 3,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 2,ivalues,basis_order);
        nnodes[4] = i35node;
        nnodes[5] = i24node;
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 2, 7,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 3, 6,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 2, 0,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 2, 1,ivalues,basis_order);
        nnodes[2] = onodes[ro[2]];
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 2, 3,ivalues,basis_order);
        nnodes[4] = i24node;
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 2, 5,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 2, 6,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 2, 7,ivalues,basis_order);
        refined->add_elem(nnodes);

        // Front
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 4,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[2] = i06node;
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 4, 0,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 4, 1,ivalues,basis_order);
        nnodes[6] = i42node;
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 4, 3,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 5,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 4,ivalues,basis_order);
        nnodes[2] = i17node;
        nnodes[3] = i06node;
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 4, 1,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 5, 0,ivalues,basis_order);
        nnodes[6] = i53node;
        nnodes[7] = i42node;
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 1, 4,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 5,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 1, 6,ivalues,basis_order);
        nnodes[3] = i17node;
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 5, 0,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 5, 1,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 5, 2,ivalues,basis_order);
        nnodes[7] = i53node;
        refined->add_elem(nnodes);

        // Center
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 0, 7,ivalues,basis_order);
        nnodes[1] = i06node;
        nnodes[2] = i35node;
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 4,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 4, 3,ivalues,basis_order);
        nnodes[5] = i42node;
        nnodes[6] = i71node;
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 7, 0,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = i06node;
        nnodes[1] = i17node;
        nnodes[2] = i24node;
        nnodes[3] = i35node;
        nnodes[4] = i42node;
        nnodes[5] = i53node;
        nnodes[6] = i60node;
        nnodes[7] = i71node;
        refined->add_elem(nnodes);

        nnodes[0] = i17node;
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 1, 6,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 5,ivalues,basis_order);
        nnodes[3] = i24node;
        nnodes[4] = i53node;
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 5, 2,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 6, 1,ivalues,basis_order);
        nnodes[7] = i60node;
        refined->add_elem(nnodes);

        // Back
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 3, 4,ivalues,basis_order);
        nnodes[1] = i35node;
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 3, 6,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 7,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 7, 0,ivalues,basis_order);
        nnodes[5] = i71node;
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 7, 2,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 7, 3,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = i35node;
        nnodes[1] = i24node;
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 7,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 3, 6,ivalues,basis_order);
        nnodes[4] = i71node;
        nnodes[5] = i60node;
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 6, 3,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 7, 2,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = i24node;
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 2, 5,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 2, 6,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 2, 7,ivalues,basis_order);
        nnodes[4] = i60node;
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 6, 1,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 6, 2,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 6, 3,ivalues,basis_order);
        refined->add_elem(nnodes);

        // Bottom Front
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 4, 0,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 4, 1,ivalues,basis_order);
        nnodes[2] = i42node;
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 4, 3,ivalues,basis_order);
        nnodes[4] = onodes[ro[4]];
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 4, 5,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 4, 6,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 4, 7,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 4, 1,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 5, 0,ivalues,basis_order);
        nnodes[2] = i53node;
        nnodes[3] = i42node;
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 4, 5,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 5, 4,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 5, 7,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 4, 6,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 5, 0,ivalues,basis_order);
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 5, 1,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 5, 2,ivalues,basis_order);
        nnodes[3] = i53node;
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 5, 4,ivalues,basis_order);
        nnodes[5] = onodes[ro[5]];
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 5, 6,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 5, 7,ivalues,basis_order);
        refined->add_elem(nnodes);

        // Bottom Center
        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 4, 3,ivalues,basis_order);
        nnodes[1] = i42node;
        nnodes[2] = i71node;
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 7, 0,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 4, 7,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 4, 6,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 7, 5,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 7, 4,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = i42node;
        nnodes[1] = i53node;
        nnodes[2] = i60node;
        nnodes[3] = i71node;
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 4, 6,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 5, 7,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 6, 4,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 7, 5,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = i53node;
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 5, 2,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 6, 1,ivalues,basis_order);
        nnodes[3] = i60node;
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 5, 7,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 5, 6,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 6, 5,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 6, 4,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = lookup_convex(refined, emap, onodes, ro, 7, 0,ivalues,basis_order);
        nnodes[1] = i71node;
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 7, 2,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 7, 3,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 7, 4,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 7, 5,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 7, 6,ivalues,basis_order);
        nnodes[7] = onodes[ro[7]];
        refined->add_elem(nnodes);

        nnodes[0] = i71node;
        nnodes[1] = i60node;
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 6, 3,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 7, 2,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 7, 5,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 6, 4,ivalues,basis_order);
        nnodes[6] = lookup_convex(refined, emap, onodes, ro, 6, 7,ivalues,basis_order);
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 7, 6,ivalues,basis_order);
        refined->add_elem(nnodes);

        nnodes[0] = i60node;
        nnodes[1] = lookup_convex(refined, emap, onodes, ro, 6, 1,ivalues,basis_order);
        nnodes[2] = lookup_convex(refined, emap, onodes, ro, 6, 2,ivalues,basis_order);
        nnodes[3] = lookup_convex(refined, emap, onodes, ro, 6, 3,ivalues,basis_order);
        nnodes[4] = lookup_convex(refined, emap, onodes, ro, 6, 4,ivalues,basis_order);
        nnodes[5] = lookup_convex(refined, emap, onodes, ro, 6, 5,ivalues,basis_order);
        nnodes[6] = onodes[ro[6]];
        nnodes[7] = lookup_convex(refined, emap, onodes, ro, 6, 7,ivalues,basis_order);
        refined->add_elem(nnodes);
        if (basis_order == 0) evalues.insert(evalues.end(),27,ivalues[*bi]);
      }
      else
      {
        // non convex, emit error.
        std::cout << "Element not convex, cannot replace.\n";
      }
      ++bi;
    }
  }
  else
  {

    VMesh::Elem::iterator bi, ei;
    mesh->begin(bi); mesh->end(ei);

    unsigned int cnt = 0;
    unsigned int loopcnt = 0;

    VMesh::Elem::size_type sz; mesh->size(sz);

    while (bi != ei)
    {
      cnt++; if (cnt == 100) { loopcnt +=cnt; cnt = 0; algo->update_progress(loopcnt,sz);  }

      mesh->get_nodes(onodes, *bi);
      
      // Get the values and compute an inside/outside mask.
      unsigned int inside = 0;
      for (unsigned int i = 0; i < onodes.size(); i++)
      {
        if (values[onodes[i]])
        {
          inside |= (1<<i);
        }
      }

      if (inside == 0)
      {
        // Nodes are the same order, so just add the element.
        refined->add_elem(onodes);
        if (rfield->basis_order() == 0) evalues.push_back(ivalues[*bi]);
      }
      else
      {
        int which = -1;
        for (int i = 0; i < 8; i++)
        {
          if (inside & (1 << i))
          {
            if (which < 0 || onodes[which] > onodes[i])
            {
              which = i;
            }
          }
        }
        if (which >= 0)
        {
          int basis_order = rfield->basis_order();
          double vv = 0.0;
          if (rfield->basis_order() == 0) vv = ivalues[*bi];
          dice(refined, emap, onodes, which, inside & ~(1<<which),ivalues,evalues,vv,basis_order);
        }
      }
      ++bi;
    }
  }
  
  rfield->resize_values();
  if (rfield->basis_order() == 0) rfield->set_values(evalues);
  if (rfield->basis_order() == 1) rfield->set_values(ivalues);
  rfield->copy_properties(field);
  
  algo->algo_end(); return (true);
}



// General access function

bool
RefineMeshAlgo::run(FieldHandle input, FieldHandle& output)
{
  algo_start("RefineMesh");

  if (input.get_rep() == 0)
  {
    error("No input field was given.");
    algo_end(); return (false);
  }

  std::string select; get_option("select",select);
  double isoval; get_scalar("isoval",isoval);

  
  VField* vfield = input->vfield();
  if (vfield->num_values() == 0)
  {
    error("Input field has no data values. The RefineMesh algorithm requires input fields to contain data."); 
    algo_end(); return (false);
  }


  if (select == "none")
  {
    // just passing through
    output = input;
    algo_end(); return (true); 
  }

  FieldInformation fi(input);
  
  if (fi.is_pnt_element() || fi.is_prism_element())
  {
    error("This algorithm does not support point or prism meshes");
    algo_end(); return(false);
  }
    
  if ((!(fi.is_scalar()))&&(select != "all"))
  {
    error("Field data needs to be of scalar type");
    algo_end(); return (false);
  }

  if (fi.is_quad_element())
  {
    RefineMeshQuadSurfAlgoV algo;
    return(algo.run(this,input,output,select,isoval));
  }
  
  if (fi.is_hex_element())
  {
    bool convex; get_bool("hex_convex",convex);
    RefineMeshHexVolAlgoV algo;
    return(algo.run(this,input,output,convex,select,isoval));
  }

  if (fi.is_crv_element())
  {
    return(RefineMeshCurveAlgoV(this,input,output,select,isoval));
  }
  
  if (fi.is_tri_element())
  {
    return(RefineMeshTriSurfAlgoV(this,input,output,select,isoval));
  }
  
  if (fi.is_tet_element())
  {
    return(RefineMeshTetVolAlgoV(this,input,output,select,isoval));
  }

  error("No refinement method has been implemented for this type of mesh");
  algo_end(); return (false);
}                           
     
} // namespace    
