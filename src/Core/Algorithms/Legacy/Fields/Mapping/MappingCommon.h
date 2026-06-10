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
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
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

/// Shared validation and output-field setup for MapFieldDataOntoElems and
/// MapFieldDataOntoNodes. Called after the caller has set the basis order on
/// fo (make_constantdata / make_lineardata). Returns false (and logs an error)
/// on any invalid combination; on success creates output and returns true.
inline bool configureOutputField(
    FieldHandle source,
    FieldHandle destination,
    FieldHandle weights,
    const FieldInformation& fi,
    FieldInformation& fo,
    const std::string& quantity,
    const std::string& mappingModel,
    FieldHandle& output,
    const AlgorithmBase* algo)
{
  if (weights)
  {
    FieldInformation wfi(weights);
    if (mappingModel == "closestnodedata")
    {
      if (!wfi.is_lineardata())
      {
        algo->error("Closest node data only works for weights data located at the nodes.");
        return false;
      }
    }
    if (wfi.is_nodata())
    {
      algo->error("No data in weights field.");
      return false;
    }
  }

  // Make sure output equals quantity to be computed
  if (quantity == "value")
  {
    fo.set_data_type(fi.get_data_type());
  }
  else if (quantity == "gradient")
  {
    if (!fi.is_scalar())
    {
      algo->error("Gradient can only be calculated on a scalar field.");
      return false;
    }
    fo.make_vector();
  }
  else if (quantity == "gradientnorm")
  {
    if (!fi.is_scalar())
    {
      algo->error("Gradient can only be calculated on a scalar field.");
      return false;
    }
    fo.make_double();
  }
  else if (quantity == "flux")
  {
    if (!fi.is_scalar())
    {
      algo->error("Flux can only be calculated on a scalar field.");
      return false;
    }
    if (!fo.is_surface())
    {
      algo->error("Flux can only be computed for surfaces meshes as destination");
      return false;
    }
    fo.make_double();
  }

  // Incorporate the weights and alter the datatype to reflect that
  if (weights)
  {
    FieldInformation wfi(weights);
    if ((!wfi.is_tensor()) && (!wfi.is_scalar()))
    {
      algo->error("Weights field needs to be a scalar or a tensor.");
      return false;
    }
    if (fo.is_scalar() && wfi.is_tensor())
    {
      fo.make_tensor();
    }
    if (fo.is_tensor() && wfi.is_tensor())
    {
      algo->error("Weights and source field cannot be both tensor data.");
      return false;
    }
  }

  // Create new output field
  output = CreateField(fo, destination->mesh());
  output->vfield()->resize_values();

  if (!output)
  {
    algo->error("Could not allocate output field");
    return false;
  }

  return true;
}

}}}} // namespace SCIRun::Core::Algorithms::Fields

#endif // CORE_ALGORITHMS_LEGACY_FIELDS_MAPPING_MAPPINGCOMMON_H
