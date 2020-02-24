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
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMeshQuadSurfAlgoV.h>

#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
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
							  result += (p * w[i]);
							}
							return result;
						}

RefineMeshQuadSurfAlgoV::RefineMeshQuadSurfAlgoV()
{

}

double RefineMeshQuadSurfAlgoV::RIinterpolateV(std::vector<double>& ivalues,
                        VMesh::Node::array_type& onodes,
                        double coords[2])const
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

VMesh::Node::index_type RefineMeshQuadSurfAlgoV::lookup(VMesh *refined,
                                 edge_hash_type &edgemap,
                                 VMesh::Node::index_type a,
                                 VMesh::Node::index_type b,
                                 double factor,
                                 std::vector<double>& ivalues)const
  {
    edgepair_t ep;
    ep.first = a; ep.second = b;
    const edge_hash_type::iterator loc = edgemap.find(ep);
    if (loc == edgemap.end())
    {
      Point pa, pb;
      refined->get_point(pa, a);
      refined->get_point(pb, b);
      const Point inbetween = ((1.0 - factor)*pa + (factor)*pb).asPoint();
      const VMesh::Node::index_type newnode = refined->add_point(inbetween);
      ivalues.push_back(((1.0 - factor)*ivalues[a] + (factor)*ivalues[b]));
      edgemap[ep] = newnode;
      return newnode;
    }
    else
    {
      return (*loc).second;
    }
  }

void RefineMeshQuadSurfAlgoV::dice(VMesh *refined,
                         edge_hash_type &emap,
                         VMesh::Node::array_type onodes,
                         VMesh::index_type index,
                         VMesh::mask_type mask,
                         VMesh::size_type maxnode,
                         std::vector<double>& ivalues,
                         std::vector<double>& evalues,
                         double vv,
                         int basis_order)const
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
runImpl(FieldHandle input, FieldHandle& output,
                  const std::string& select, double isoval) const
{
  // Obtain information on what type of input field we have
  FieldInformation fi(input);
  // Alter the input so it will become a QuadSurf
  fi.make_quadsurfmesh();
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
  CopyProperties(*input, *output);
  return (true);
}

AlgorithmOutput RefineMeshQuadSurfAlgoV::run(const AlgorithmInput& input) const
{
  throw "not implemented";
}
