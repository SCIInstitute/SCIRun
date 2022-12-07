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


#include <Core/Algorithms/Fields/TransformMesh/PolarProjectionTools.h>

#include <Core/Geometry/Transform.h>

#include <Core/Datatypes/VField.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/FieldInformation.h>


namespace SCIRunAlgo {

using namespace SCIRun;

bool GenerateEdgeProjection(VMesh* vmesh,
                            VMesh::Node::index_type start_node,
                            Vector normal,
                            Vector axis,
                            double maxdist,
                            VMesh::Node::index_type final_node,
                            Point& fpoint,
                            double &cum_dist)
{
  // check whether node is valid
  if (start_node < 0 || start_node >= vmesh->num_nodes()) return (false);

  // Make sure we have a valid normal
  if (normal.length() == 0) return (false);

  Point po, p;
  std::vector<Point> ps;
  Point point1, point2;
  // add first point to list
  vmesh->get_center(po,start_node);
  point1 = po;
  point2 = po;

  // initialize search algorithm
  VMesh::Node::index_type  fnode = start_node;
  VMesh::Node::index_type  fnode2 = -1;
  VMesh::DElem::index_type fedge = -1;
  VMesh::Elem::index_type  felem = -1;
  VMesh::Elem::index_type  felem2 = -1;

  VMesh::Elem::array_type selem(1);
  VMesh::Node::array_type nodes;
  VMesh::DElem::array_type delems;

  bool found_end = false;
  bool found_next = false;

  double epsilon = vmesh->get_epsilon();
  double epsilon2 = epsilon*epsilon;

  cum_dist = 0.0;

  while (!found_end)
  {

    if (fnode >= 0)
    {
      // we are passing through a node
      vmesh->get_elems(selem,fnode);
      // mark out the element we just passed through
      for (size_t j=0;j<selem.size();j++)
      {
        if (selem[j] == felem) selem[j] = -1;
        if (selem[j] == felem2) selem[j] = -1;
      }
    }
    else
    {
      // we are passing through an edge
      selem.resize(1);
      vmesh->get_neighbor(selem[0],felem,fedge);
      if (selem[0] == felem) selem[0] = -1;
      if (selem[0] == felem2) selem[0] = -1;
    }

    found_next = false;
    // Now search through list of possible elements
    for (size_t j =0; j<selem.size(); j++)
    {
      if (selem[j] >= 0)
      {
        // check nodes
        vmesh->get_nodes(nodes,selem[j]);
        // search through node list to see if one of the nodes is on the list
        for (size_t i=0; i<nodes.size(); i++)
        {
          // ignore the current point and the previous point if there are any
          if ((nodes[i] ==  fnode)||(nodes[i] == fnode2)) continue;

          vmesh->get_center(p,nodes[i]);

          Vector pop = (po-p); pop.normalize();
          if (Abs(Dot(pop,normal)) < epsilon2 && Dot(pop,Cross(normal,axis)) > 0)
          {
            // found next node
            fnode2 = fnode;
            felem2 = felem;
            fnode = nodes[i];
            fedge = -1;
            felem = selem[j];
            point1 = point2;
            point2 = p;
            found_next = true;
            break;
          }
        }

        if (found_next) break;
      }
    }

    if (!found_next)
    {
      for (size_t j =0; j<selem.size(); j++)
      {
        if (selem[j] >= 0)
        {
          // search through edges
          vmesh->get_delems(delems,selem[j]);

          for (size_t i=0; i<delems.size(); i++)
          {
            if (delems[i] == fedge) continue;
            vmesh->get_nodes(nodes,delems[i]);
            if (nodes.size() != 2) continue;
            if (nodes[0] == fnode || nodes[1] == fnode) continue;

            vmesh->get_centers(ps,nodes);

            double alpha1 = Dot(normal,po-ps[0]);
            double alpha2 = Dot(normal,po-ps[1]);

            if (alpha1*alpha2 < epsilon2)
            {
              p = ps[0] + Abs(alpha1)/(Abs(alpha1)+Abs(alpha2))*(ps[1]-ps[0]);
              if (Dot(po-p,Cross(normal,axis)) > -epsilon)
              {
                fnode2 = fnode;
                felem2 = felem;
                fnode = -1;
                fedge = delems[i];
                felem = selem[j];
                point1 = point2;
                point2 = p;
                found_next = true;

                break;
              }
            }
            if (found_next) break;
          }
        }
        if (found_next) break;
      }
    }

    if (!found_next)
    {
      return (false);
    }

    double prev_dist = cum_dist;
    cum_dist += (point2 - point1).length();

    if (maxdist > 0.0)
    {
      if (cum_dist > maxdist)
      {
        double alpha = (maxdist-prev_dist)/(cum_dist-prev_dist);
        fpoint = ((alpha)*point2 + (1.0-alpha)*point1).asPoint();
        cum_dist = maxdist;
        found_end = true;
      }
    }
    else if (final_node >= 0)
    {
      if (fnode == final_node)
      {
        found_end = true;
        fpoint = point2;
      }
    }
    else if (found_end)
    {
      fpoint = point2;
    }
  }

  return (true);
}

} // End namespace SCIRunAlgo
