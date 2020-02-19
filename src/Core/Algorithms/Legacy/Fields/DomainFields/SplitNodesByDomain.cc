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


#include <Core/Algorithms/Fields/DomainFields/SplitNodesByDomain.h>

#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
SplitNodesByDomainAlgo::
run( FieldHandle input, FieldHandle& output)
{
  algo_start("SplitNodesByDomain");

  if (input.get_rep() == 0)
  {
    algo_end(); error("No input field");
    return (false);
  }

  FieldInformation fi(input), fo(input);
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements.");
    algo_end(); return (false);
  }

  if (!(fi.is_constantdata()))
  {
    error("This function only works for data located at the elements");
    algo_end(); return (false);
  }

  fo.make_unstructuredmesh();
  fo.make_int();

  VField *field = input->vfield();
  VMesh  *mesh  = input->vmesh();

  if (field == 0 || mesh == 0)
  {
    algo_end(); error("No input field");
    return (false);
  }

  VMesh::Elem::iterator bei, eei;
  VMesh::Node::iterator bni, eni;

  VMesh::size_type num_elems = mesh->num_elems();
  VMesh::size_type num_nodes = mesh->num_nodes();


  std::vector<VMesh::Node::index_type> idxarray(num_nodes);
  std::vector<bool> newidxarray(num_nodes);
  VMesh::Node::array_type nodes;
  VMesh::Node::array_type newnodes;

  int val, minval;
  int eval;
  index_type idx;


  output = CreateField(fo);

  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return(false);
  }

  VField* ofield = output->vfield();
  VMesh *omesh = output->vmesh();

  if (ofield == 0 || omesh == 0)
  {
    error("Could not create output field");
    algo_end(); return(false);
  }

  std::vector<int> newdata(num_elems);

  omesh->elem_reserve(num_elems); // exact number
  omesh->node_reserve(num_nodes); // minimum number of nodes

  double d_minval;
  field->min(d_minval);
  minval = static_cast<int>(d_minval);

  index_type k = 0;

  int cnt = 0;
  while(1)
  {
    val = minval;

    for (size_type p =0; p<num_nodes; p++) newidxarray[p] = true;

    mesh->begin(bei); mesh->end(eei);
    mesh->get_nodes(newnodes,*(bei));

    while (bei != eei)
    {
      field->get_value(eval,*(bei));
      if (eval == val)
      {
        mesh->get_nodes(nodes,*(bei));
        for (size_t p=0; p< nodes.size(); p++)
        {
          idx = nodes[p];
          if (newidxarray[idx])
          {
            Point pt;
            mesh->get_center(pt,nodes[p]);
            idxarray[idx] = omesh->add_point(pt);
            newidxarray[idx] = false;
          }
          newnodes[p] = idxarray[idx];
        }
        omesh->add_elem(newnodes);
        cnt++; if (cnt == 400) { cnt = 0; update_progress(k,num_elems); }
        newdata[k] = eval; k++;
      }
      ++bei;
    }

    eval = val;
    bool foundminval = false;

    mesh->begin(bei);
    mesh->end(eei);
    while (bei != eei)
    {
      field->value(eval,*(bei));
      if (eval > val)
      {
        if (foundminval)
        {
          if (eval < minval) minval = eval;
        }
        else
        {
          minval = eval;
          foundminval = true;
        }
      }
      ++bei;
    }

    if (minval > val)
    {
      val = minval;
    }
    else
    {
      break;
    }
  }

  ofield->resize_values();
  ofield->set_values(newdata);

  algo_end(); return(true);
}

} // namespace SCIRunAlgo
