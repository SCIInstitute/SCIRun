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

   Author:              Moritz Dannhauer
   Last Modification:   April 8 2015 (ported from SCIRun4)
*/


#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToTetVolMesh.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

bool ConvertMeshToTetVolMeshAlgo::ConvertHexVolToTetVolV(FieldHandle input, FieldHandle& output) const
{
  VField *ifield = input->vfield();
  VMesh *imesh = ifield->vmesh();

  VField *ofield = output->vfield();
  VMesh* omesh = ofield->vmesh();

  VMesh::Node::size_type numnodes;
  VMesh::Elem::size_type numelems;
  imesh->size(numnodes);
  imesh->size(numelems);

  omesh->node_reserve(numnodes);

  VMesh::Node::iterator nbi, nei;
  VMesh::Elem::iterator ebi, eei;
  VMesh::Node::array_type a(4);

  imesh->begin(nbi);
  imesh->end(nei);
  while (nbi != nei)
  {
    Point point;
    imesh->get_center(point, *nbi);
    omesh->add_node(point);
    ++nbi;
  }

  /// Make sure that get_neighbors works with elements

  imesh->synchronize(Mesh::ELEM_NEIGHBORS_E);
  omesh->elem_reserve(numelems*5);

  std::vector<VMesh::Elem::index_type> elemmap(numelems);
  std::vector<char> visited(numelems, 0);

  VMesh::Elem::iterator bi, ei;
  imesh->begin(bi); imesh->end(ei);

  size_t surfsize = static_cast<size_t>(pow(static_cast<double>(numelems), 2.0 / 3.0));
  std::vector<VMesh::Elem::index_type> buffer;
  buffer.reserve(surfsize);

  while (bi != ei)
  {
    // if list of elements to process is empty ad the next one
    if (buffer.size() == 0)
    {
      if(visited[*bi] == 0) buffer.push_back(*bi);
    }

    if (buffer.size() > 0)
    {
      for (size_t i=0; i< buffer.size(); i++)
      {
        if (visited[buffer[i]] > 0) { continue; }
        VMesh::Elem::array_type neighbors;

        int newtype = 0;

        imesh->get_neighbors(neighbors, buffer[i]);
        for (unsigned int p=0; p<neighbors.size(); p++)
        {
          if(visited[neighbors[p]] > 0)
          {
            if (newtype)
            {
              if (visited[neighbors[p]] != newtype)
              {
                error("Algorithm cannot deal with topology of input field, field cannot by sorted into checker board type of ordering");
                return (false);
              }
            }
            else
            {
              newtype = visited[neighbors[p]];
            }
          }
          else if(visited[neighbors[p]] == 0)
          {
            visited[neighbors[p]] = -1;
            buffer.push_back(neighbors[p]);
          }
        }

        /// For checker board ordering
        if (newtype == 0) newtype = 1;
        if (newtype == 1) newtype = 2; else newtype = 1;

        VMesh::Node::array_type hv;
        imesh->get_nodes(hv, buffer[i]);

        if (newtype == 1)
        {
          /// Add one type of ordering
          a[0] = hv[0]; a[1] = hv[1]; a[2] = hv[2]; a[3] = hv[5];
          elemmap[buffer[i]] = omesh->add_elem(a);

          a[0] = hv[0]; a[1] = hv[2]; a[2] = hv[3]; a[3] = hv[7];
          omesh->add_elem(a);
          a[0] = hv[0]; a[1] = hv[5]; a[2] = hv[2]; a[3] = hv[7];
          omesh->add_elem(a);
          a[0] = hv[0]; a[1] = hv[5]; a[2] = hv[7]; a[3] = hv[4];
          omesh->add_elem(a);
          a[0] = hv[5]; a[1] = hv[2]; a[2] = hv[7]; a[3] = hv[6];
          omesh->add_elem(a);

          visited[buffer[i]] = 1;
        }
        else
        {
          /// Add the other type of ordering
          a[0] = hv[0]; a[1] = hv[1]; a[2] = hv[3]; a[3] = hv[4];
          elemmap[buffer[i]] = omesh->add_elem(a);

          a[0] = hv[1]; a[1] = hv[2]; a[2] = hv[3]; a[3] = hv[6];
          omesh->add_elem(a);
          a[0] = hv[1]; a[1] = hv[3]; a[2] = hv[4]; a[3] = hv[6];
          omesh->add_elem(a);
          a[0] = hv[1]; a[1] = hv[5]; a[2] = hv[6]; a[3] = hv[4];
          omesh->add_elem(a);
          a[0] = hv[3]; a[1] = hv[4]; a[2] = hv[6]; a[3] = hv[7];
          omesh->add_elem(a);

          visited[buffer[i]] = 2;
        }
      }
      buffer.clear();
    }
    ++bi;
  }

  ofield->resize_fdata();

  if (ifield->basis_order() == 0)
  {
    VMesh::size_type sz = ifield->num_values();
    VMesh::index_type idx;

    for (VMesh::index_type r=0; r<sz; r++)
    {
      idx = elemmap[r];
      ofield->copy_value(ifield,r,idx);
      ofield->copy_value(ifield,r,idx+1);
      ofield->copy_value(ifield,r,idx+2);
      ofield->copy_value(ifield,r,idx+3);
      ofield->copy_value(ifield,r,idx+4);
    }
  }

  if (ifield->basis_order() == 1)
  {
    ofield->copy_values(ifield);
  }

  // Copy properties of the property manager
  // output->copy_properties(input.get_rep());

  // Success:
  return true;
}

bool ConvertMeshToTetVolMeshAlgo::ConvertLatVolToTetVolV(FieldHandle input, FieldHandle& output) const
{
  VField *ifield = input->vfield();
  VMesh *imesh = ifield->vmesh();

  VField *ofield = output->vfield();
  VMesh* omesh = ofield->vmesh();

  VMesh::Node::array_type a(4);
  VMesh::Node::size_type numnodes;
  imesh->size(numnodes);
  omesh->node_reserve(numnodes);

  // Copy points directly, assuming they will have the same order.
  VMesh::Node::iterator nbi, nei;
  VMesh::Node::iterator dbi, dei;
  imesh->begin(nbi);
  imesh->end(nei);

  while (nbi != nei)
  {
    Point point;
    imesh->get_center(point, *nbi);
    omesh->add_node(point);
    ++nbi;
  }

  VMesh::Elem::size_type numelems;
  imesh->size(numelems);
  omesh->elem_reserve(numelems*5);

  VMesh::Elem::iterator bi, ei;
  VMesh::Elem::iterator obi, oei;

  VMesh::dimension_type dims;
  imesh->get_dimensions(dims);
  if (dims.size() != 3)
  {
    error("Could not obtain LatVol dimensions");
    return (false);
  }
  const size_type d2 = (dims[0]-1)*(dims[1]-1);
  const size_type d1 = dims[0]-1;

  imesh->begin(bi);
  imesh->end(ei);

  while (bi != ei)
  {
    VMesh::Node::array_type lv;
    imesh->get_nodes(lv, *bi);

    const index_type idx = *bi;
    const index_type k = idx / d2;
    const index_type jk = idx % d2;
    const index_type j = jk / d1;
    const index_type i = jk % d1;

    if (!((i^j^k)&1))
    {
      a[0] = lv[0]; a[1] = lv[1]; a[2] = lv[2]; a[3] = lv[5]; omesh->add_elem(a);
      a[0] = lv[0]; a[1] = lv[2]; a[2] = lv[3]; a[3] = lv[7]; omesh->add_elem(a);
      a[0] = lv[0]; a[1] = lv[5]; a[2] = lv[2]; a[3] = lv[7]; omesh->add_elem(a);
      a[0] = lv[0]; a[1] = lv[5]; a[2] = lv[7]; a[3] = lv[4]; omesh->add_elem(a);
      a[0] = lv[5]; a[1] = lv[2]; a[2] = lv[7]; a[3] = lv[6]; omesh->add_elem(a);
    }
    else
    {
      a[0] = lv[0]; a[1] = lv[1]; a[2] = lv[3]; a[3] = lv[4]; omesh->add_elem(a);
      a[0] = lv[1]; a[1] = lv[2]; a[2] = lv[3]; a[3] = lv[6]; omesh->add_elem(a);
      a[0] = lv[1]; a[1] = lv[3]; a[2] = lv[4]; a[3] = lv[6]; omesh->add_elem(a);
      a[0] = lv[1]; a[1] = lv[5]; a[2] = lv[6]; a[3] = lv[4]; omesh->add_elem(a);
      a[0] = lv[3]; a[1] = lv[4]; a[2] = lv[6]; a[3] = lv[7]; omesh->add_elem(a);
    }
    ++bi;
  }

  ofield->resize_fdata();

  if (ifield->basis_order() == 0)
  {
    VMesh::size_type sz = ifield->num_values();

    VMesh::index_type q = 0;
    for (VMesh::index_type r=0; r<sz; r++)
    {
      ofield->copy_value(ifield,r,q);
      ofield->copy_value(ifield,r,q+1);
      ofield->copy_value(ifield,r,q+2);
      ofield->copy_value(ifield,r,q+3);
      ofield->copy_value(ifield,r,q+4);
      q += 5;
    }
  }

  if (ifield->basis_order() == 1)
  {
    ofield->copy_values(ifield);
  }

  // Copy properties of the property manager
  // output->copy_properties(input.get_rep());

  // Success:
  return true;
}

bool ConvertMeshToTetVolMeshAlgo::run(FieldHandle input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "ConvertMeshToTetVolMeshAlgo");

  if (!input)
  {
    error("No input field");
    return (false);
  }

    // Create information fields and fill them out with the data types of the input
  FieldInformation fi(input);
  FieldInformation fo(input);

  // Ignore non linear cases for now
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    return (false);
  }

  // In case it is already a trisurf skip algorithm
  if (fi.is_tet_element())
  {
    output = input;
    remark("Input is already a TetVolMesh; just copying input to output");
    return (true);
  }

  fo.make_tetvolmesh();

  output = CreateField(fo);
  if (!output)
  {
    error("Could not allocate output field");
     return (false);
  }

  if (fi.is_latvolmesh() || fi.is_structhexvolmesh())
  {
    return (ConvertLatVolToTetVolV(input,output));
  }
  else if (fi.is_hexvolmesh())
  {
    return (ConvertHexVolToTetVolV(input,output));
  }
  else
  {
    error("No algorithm is available to convert this type of mesh");
    return (false);
  }

 return true;
}

AlgorithmInputName ConvertMeshToTetVolMeshAlgo::HexOrLatVol("HexOrLatVol");
AlgorithmOutputName ConvertMeshToTetVolMeshAlgo::TetVol("TetVol");

AlgorithmOutput ConvertMeshToTetVolMeshAlgo::run(const AlgorithmInput& input) const
{
 auto hex_or_latvol = input.get<Field>(HexOrLatVol);

 FieldHandle tetvol;
 if (!run(hex_or_latvol, tetvol))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

 AlgorithmOutput output;
 output[TetVol] = tetvol;

 return output;
}
