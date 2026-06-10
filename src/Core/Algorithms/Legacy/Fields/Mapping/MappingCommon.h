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

#ifndef CORE_ALGORITHMS_LEGACY_FIELDS_MAPPING_MAPPINGCOMMON_H
#define CORE_ALGORITHMS_LEGACY_FIELDS_MAPPING_MAPPINGCOMMON_H

// Shared template helper for the "single-destination" parallel mapping pattern
// used by BuildMappingMatrixSingleDestinationPAlgo and
// MapFieldDataFromSourceToDestinationSingleDestinationPAlgo.
//
// Both algorithms iterate over source elements/nodes, find the closest
// destination element/node, and store the result in a per-source index array.
// The array type differs (raw pointer vs std::vector), so CCArray is templated.

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Geometry/Point.h>

namespace SCIRun { namespace Core { namespace Algorithms { namespace Fields {

/// For each source element/node in [start, end), find the closest destination
/// element/node and record its index in cc[idx] (or -1 when beyond maxdist).
/// CCArray may be index_type* or std::vector<index_type> — both expose operator[].
template <typename CCArray>
void fillSingleDestinationMapping(
    VField* sfield, VField* dfield,
    VMesh* smesh, VMesh* dmesh,
    double maxdist,
    VField::index_type start, VField::index_type end,
    CCArray& cc,
    int proc, const AlgorithmBase* algo)
{
  using SCIRun::Core::Geometry::Point;
  int cnt = 0;
  if (sfield->basis_order() == 0 && dfield->basis_order() == 0)
  {
    Point p, r;
    VMesh::coords_type coords;
    VMesh::Elem::index_type didx;
    for (VMesh::Elem::index_type idx = start; idx < end; idx++)
    {
      smesh->get_center(p, idx);
      double dist;
      if (dmesh->find_closest_elem(dist, r, coords, didx, p))
      {
        if (maxdist < 0.0 || dist < maxdist)
          cc[idx] = didx;
        else
          cc[idx] = -1;
      }
      if (proc == 0) { cnt++; if (cnt == 200) { cnt = 0; algo->update_progress_max(idx, end); } }
    }
  }
  else if (sfield->basis_order() == 1 && dfield->basis_order() == 0)
  {
    Point p, r;
    VMesh::coords_type coords;
    VMesh::Elem::index_type didx;
    for (VMesh::Node::index_type idx = start; idx < end; idx++)
    {
      smesh->get_center(p, idx);
      double dist;
      if (dmesh->find_closest_elem(dist, r, coords, didx, p))
      {
        if (maxdist < 0.0 || dist < maxdist)
          cc[idx] = didx;
        else
          cc[idx] = -1;
      }
      if (proc == 0) { cnt++; if (cnt == 200) { cnt = 0; algo->update_progress_max(idx, end); } }
    }
  }
  else if (sfield->basis_order() == 0 && dfield->basis_order() == 1)
  {
    Point p, r;
    VMesh::Node::index_type didx;
    for (VMesh::Elem::index_type idx = start; idx < end; idx++)
    {
      smesh->get_center(p, idx);
      double dist;
      if (dmesh->find_closest_node(dist, r, didx, p))
      {
        if (maxdist < 0.0 || dist < maxdist)
          cc[idx] = didx;
        else
          cc[idx] = -1;
      }
      if (proc == 0) { cnt++; if (cnt == 200) { cnt = 0; algo->update_progress_max(idx, end); } }
    }
  }
  else if (sfield->basis_order() == 1 && dfield->basis_order() == 1)
  {
    Point p, r;
    VMesh::Node::index_type didx;
    for (VMesh::Node::index_type idx = start; idx < end; idx++)
    {
      smesh->get_center(p, idx);
      double dist;
      if (dmesh->find_closest_node(dist, r, didx, p))
      {
        if (maxdist < 0.0 || dist < maxdist)
          cc[idx] = didx;
        else
          cc[idx] = -1;
      }
      if (proc == 0) { cnt++; if (cnt == 200) { cnt = 0; algo->update_progress_max(idx, end); } }
    }
  }
}

}}}} // namespace SCIRun::Core::Algorithms::Fields

#endif // CORE_ALGORITHMS_LEGACY_FIELDS_MAPPING_MAPPINGCOMMON_H
