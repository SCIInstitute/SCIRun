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


#include <Core/Algorithms/Fields/FindNodes/FindClosestNodeByValue.h>
#include <float.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
FindClosestNodeByValueAlgo::run(FieldHandle input, FieldHandle points, std::vector<index_type>& output)
{
  algo_start("FindClosestNodeByValue");

  if (input.get_rep() == 0)
  {
    error("Could not obtain input field");
    algo_end(); return (false);
  }
  VField* ifield = input->vfield();
  VMesh* imesh =   input->vmesh();

  if (points.get_rep() == 0)
  {
    error("Could not obtain input field with node locations");
    algo_end(); return (false);
  }

  VMesh*  pmesh = points->vmesh();

  VMesh::Node::size_type nnodes = pmesh->num_nodes();

  if (nnodes == 0)
  {
    error("No nodes locations are given in node mesh");
    algo_end(); return (false);
  }

  output.resize(nnodes);

  VMesh::Node::iterator pit, pit_end;
  VMesh::Node::index_type idx;

  double dist = DBL_MAX;
  double dist2;

  Point p, q;
  size_t m = 0;

  double valuemin = get_scalar("valuemin");
  double valuemax = get_scalar("valuemax");

  if (ifield->basis_order() == 0)
  {
    VMesh::Elem::iterator it, it_end;
    VMesh::Node::array_type nodes;
    double ival;

    pmesh->begin(pit);
    pmesh->end(pit_end);
    while (pit != pit_end)
    {
      dist = DBL_MAX;
      pmesh->get_center(p,*pit);
      imesh->begin(it);
      imesh->end(it_end);
      while (it != it_end)
      {
        ifield->get_value(ival,*it);
        if (ival >=valuemin && ival <= valuemax)
        {
          imesh->get_nodes(nodes,*it);
          for (size_t k=0; k<nodes.size(); k++)
          {
            imesh->get_center(q,nodes[k]);
            dist2 = Vector(p-q).length2();
            if (dist2 < dist)
            {
              idx = nodes[k];
              dist = dist2;
            }
          }
        }
        ++it;
      }
      ++pit;
      output[m] = idx;
      m++;
    }
  }
  else
  {
    VMesh::Node::iterator it, it_end;
    double ival;

    pmesh->begin(pit);
    pmesh->end(pit_end);
    while (pit != pit_end)
    {
      dist = DBL_MAX;
      pmesh->get_center(p,*pit);
      imesh->begin(it);
      imesh->end(it_end);
      while (it != it_end)
      {
        ifield->value(ival,*it);
        if (ival >=valuemin && ival <= valuemax)
        {
          imesh->get_center(q,*it);
          dist2 = Vector(p-q).length2();
          if (dist2 < dist)
          {
            idx = *it;
            dist = dist2;
          }
        }
        ++it;
      }
      ++pit;
      output[m] = idx;
      m++;
    }
  }

  algo_end(); return (true);
}


} // end namespace
