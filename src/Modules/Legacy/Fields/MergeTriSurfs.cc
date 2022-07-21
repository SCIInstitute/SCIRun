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


/// @author
///   Michael Callahan,
///   Department of Computer Science,
///   University of Utah
/// @date  July 2004

#include <Modules/Legacy/Fields/MergeTriSurfs.h>
#include <Core/GeometryPrimitives/CompGeom.h>

#include <Core/Utils/Legacy/StringUtil.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;

MODULE_INFO_DEF(MergeTriSurfs, NewField, SCIRun)

//TODO: move to algo layer
namespace detail
{

/// @class MergeTriSurfsAlgo
/// @brief This module self intersects all the triangles in a trisurf with each other so that none overlap.

class MergeTriSurfsAlgo
{
public:

  /// virtual interface.
  void execute(AlgorithmStatusReporter::UpdaterFunc reporter,
               FieldHandle tris,
               std::vector<index_type> &new_nodes,
               std::vector<index_type> &new_elems);
};

void
MergeTriSurfsAlgo::execute(AlgorithmStatusReporter::UpdaterFunc reporter,
                           FieldHandle tris_h,
                           std::vector<index_type> &new_nodes,
                           std::vector<index_type> &new_elems)
{
  VField* tfield = tris_h->vfield();
  VMesh*  tmesh  = tris_h->vmesh();

  tmesh->synchronize(Mesh::EDGES_E | Mesh::ELEM_NEIGHBORS_E
                     | Mesh::FACES_E | Mesh::LOCATE_E);

  VMesh::Elem::iterator abi, aei;
  tmesh->begin(abi);
  tmesh->end(aei);

  VMesh::Elem::array_type candidates;
  VMesh::size_type num_elems = tmesh->num_elems();
  std::vector<Point> newpoints;

  double epsilon = tmesh->get_epsilon();
  int cnt = 0;
  for(VMesh::Elem::index_type idx=0; idx<num_elems;idx++)
  {
    cnt++; if (cnt == 100) { cnt = 0; reporter(static_cast<double>(idx) / num_elems * 2); }

    VMesh::Node::array_type anodes;
    tmesh->get_nodes(anodes, idx);
    Point apoints[3];
    BBox tribox;
    for (int i = 0; i < 3; i++)
    {
      tmesh->get_point(apoints[i], anodes[i]);
      tribox.extend(apoints[i]);
    }

    tribox.extend(epsilon);
    tmesh->locate(candidates, tribox);

    for (size_t p=0; p<candidates.size();p++)
    {
      if (idx != candidates[p])
      {
        VMesh::Node::array_type bnodes;
        tmesh->get_nodes(bnodes,VMesh::Elem::index_type(candidates[p]));
        Point bpoints[3];
        for (int i = 0; i < 3; i++)
        {
          tmesh->get_point(bpoints[i], bnodes[i]);
        }

        TriTriIntersection(apoints[0], apoints[1], apoints[2],
                           bpoints[0], bpoints[1], bpoints[2],
                           newpoints);
      }
    }
  }

  VMesh::Node::index_type newnode;
  VMesh::Elem::array_type newelems;

  cnt = 0;
  for (size_t i = 0; i < newpoints.size(); i++)
  {
    cnt++; if (cnt == 100) { cnt = 0; reporter(static_cast<double>(i) / newpoints.size()); }

    Point closest;
    VMesh::Elem::array_type elem;
    double dist;
    tmesh->find_closest_elems(dist,closest, elem, newpoints[i]);
    for (size_t k = 0; k < elem.size(); k++)
    {
      newelems.clear();
      tmesh->insert_node_into_elem(newelems, newnode, elem[k], closest);
      new_nodes.push_back(newnode);
      for (index_type j = static_cast<index_type>(newelems.size()-1); j >= 0; j--)
      {
        new_elems.push_back(newelems[j]);
      }
    }
  }

  tmesh->synchronize(Mesh::EDGES_E);
  tfield->resize_fdata();
}
}

MergeTriSurfs::MergeTriSurfs() : Module(staticInfo_, false)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
}

void MergeTriSurfs::execute()
{
  auto ifieldhandle = getRequiredInput(InputField);

  FieldInformation fi(ifieldhandle);

  if (!fi.is_trisurfmesh())
  {
    error("This module has only been implemented for Trisurf meshes.");
    return;
  }

  detail::MergeTriSurfsAlgo algo;
  auto output = FieldHandle(ifieldhandle->deep_clone());

  std::vector<index_type> new_nodes;
  std::vector<index_type> new_elems;
  algo.execute(getUpdaterFunc(), output, new_nodes, new_elems);

  sendOutput(OutputField, output);
}
