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
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMeshHexVolAlgoV.h>

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/GeometryPrimitives/Point.h>
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

int pattern_table[256][2];
SCIRun::Core::Geometry::Point hcoords[8];

Point RIinterpolate(VMesh *refined,
                        VMesh::Node::array_type& onodes,
                        const Point &coordsp)  {
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
        result += (p * w[i]);
      }

      return result;
    };

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
    };

unsigned int iedge(unsigned int a, unsigned int b)
    {
      return (1<<(7-a)) | (1<<(7-b));
    };

unsigned int iface(unsigned int a, unsigned int b,
                       unsigned int c, unsigned int d)
    {
      return iedge(a, b) | iedge(c, d);
    };

unsigned int iface3(unsigned int a, unsigned int b, unsigned int c)
    {
      return (1<<(7-a)) | (1<<(7-b)) | (1<<(7-c));
    };

int hex_reorder_table[14][8] = {
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


double hcoords_double[8][3] = {
  { 0.0, 0.0, 0.0},
  { 1.0, 0.0, 0.0},
  { 1.0, 1.0, 0.0},
  { 0.0, 1.0, 0.0},
  { 0.0, 0.0, 1.0},
  { 1.0, 0.0, 1.0},
  { 1.0, 1.0, 1.0},
  { 0.0, 1.0, 1.0}
};

void set_table(int i, int pattern, int reorder)
    {
      pattern_table[i][0] = pattern;
      pattern_table[i][1] = reorder;
    };

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
    };

VMesh::Node::index_type RefineMeshHexVolAlgoV::add_point(VMesh *refined,
                                      VMesh::Node::array_type &nodes,
                                      const int *reorder,
                                      unsigned int a,
                                      unsigned int b,
                                      double factor,
                                      std::vector<double>& ivalues,
                                      int basis_order) const
    {
      Point coordsp;

      unsigned int ra = reorder[a];
      unsigned int rb = reorder[b];
      coordsp = Interpolate(hcoords[ra], hcoords[rb], factor);

      const Point inbetween = RIinterpolate(refined, nodes, coordsp);
      if (basis_order == 1) ivalues.push_back(RIinterpolateV(ivalues,nodes,coordsp));
      return refined->add_point(inbetween);
    };

VMesh::Node::index_type RefineMeshHexVolAlgoV::add_point_convex(VMesh *refined,
                                      VMesh::Node::array_type &nodes,
                                      const int *reorder,
                                      VMesh::index_type a,
                                      VMesh::index_type b,
                                      std::vector<double>& ivalues,
                                      int basis_order) const
    {
      VMesh::index_type ra = reorder[a];
      VMesh::index_type rb = reorder[b];
      Point coordsp = Interpolate(hcoords[ra], hcoords[rb], 1.0/3.0);

      const Point inbetween = RIinterpolate(refined, nodes, coordsp);

      if (basis_order == 1) ivalues.push_back(RIinterpolateV(ivalues,nodes,coordsp));
      return refined->add_point(inbetween);
    };


VMesh::Node::index_type RefineMeshHexVolAlgoV::add_point_convex(VMesh *refined,
                                      VMesh::Node::array_type &nodes,
                                      const Point& coordsp,
                                      std::vector<double>& ivalues,
                                      int basis_order) const
    {
      const Point inbetween = RIinterpolate(refined, nodes, coordsp);
      if (basis_order == 1) ivalues.push_back(RIinterpolateV(ivalues,nodes,coordsp));
      return refined->add_point(inbetween);
    };


VMesh::Node::index_type RefineMeshHexVolAlgoV::lookup(VMesh *refined,
                                   edge_hash_type &edgemap,
                                   VMesh::Node::array_type &nodes,
                                   const int *reorder,
                                   VMesh::index_type a,
                                   VMesh::index_type b,
                                   double factor,
                                   std::vector<double>& ivalues,
                                   int basis_order) const
    {
      edgepair_t ep;
      ep.first = nodes[reorder[a]];
      ep.second = nodes[reorder[b]];
      const edge_hash_type::iterator loc = edgemap.find(ep);
      if (loc == edgemap.end())
      {
        const VMesh::Node::index_type newnode =
          add_point(refined, nodes, reorder, a, b, factor, ivalues, basis_order);
        edgemap[ep] = newnode;
        return newnode;
      }
      else
      {
        return (*loc).second;
      }
    };

VMesh::Node::index_type RefineMeshHexVolAlgoV::lookup_convex(VMesh *refined,
                                   edge_hash_type &edgemap,
                                   VMesh::Node::array_type &onodes,
                                   const int *reorder,
                                   VMesh::index_type a,
                                   VMesh::index_type b,
                                   std::vector<double>& ivalues,
                                   int basis_order) const
    {
      edgepair_t ep;
      ep.first = onodes[reorder[a]];
      ep.second = onodes[reorder[b]];
      const edge_hash_type::iterator loc = edgemap.find(ep);
      if (loc == edgemap.end())
      {
        const VMesh::Node::index_type newnode =
          add_point_convex(refined, onodes, reorder, a, b, ivalues, basis_order);
        edgemap[ep] = newnode;
        return newnode;
      }
      else
      {
        return (*loc).second;
      }
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
                           int basis_order)const
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



RefineMeshHexVolAlgoV::RefineMeshHexVolAlgoV()
{

}
bool
RefineMeshHexVolAlgoV::
runImpl(FieldHandle input, FieldHandle& output, bool convex,
               const std::string& select, double isoval) const
{
  // Obtain information on what type of input field we have
  FieldInformation fi(input);
  // Alter the input so it will become a QuadSurf
  fi.make_hexvolmesh();
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

  //maxnode = mesh->num_nodes();
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
      error("Unknown region selection method encountered");
      return (false);
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
				cnt++; if (cnt == 100) { loopcnt +=cnt; cnt = 0; this->update_progress_max(loopcnt,sz);  }

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
				cnt++; if (cnt == 100) { loopcnt +=cnt; cnt = 0; this->update_progress_max(loopcnt,sz);  }

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
  CopyProperties(*input, *output);
	return (true);
}

AlgorithmOutput RefineMeshHexVolAlgoV::run(const AlgorithmInput& input) const
{
  throw "not implemented";
}
