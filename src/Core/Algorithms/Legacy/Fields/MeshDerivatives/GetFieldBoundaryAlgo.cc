/*/*
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


#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundaryAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/PropertyManagerExtensions.h>

#include <boost/unordered_map.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

AlgorithmOutputName GetFieldBoundaryAlgo::BoundaryField("BoundaryField");
AlgorithmOutputName GetFieldBoundaryAlgo::MappingMatrix("Mapping");

GetFieldBoundaryAlgo::GetFieldBoundaryAlgo()
{
  addOption(AlgorithmParameterName("mapping"),"auto","auto|node|elem|none");
}

struct IndexHash
{
  size_t operator()(const index_type &idx) const
    { return (static_cast<size_t>(idx)); }
};

bool
GetFieldBoundaryAlgo::run(FieldHandle input, FieldHandle& output, MatrixHandle& mapping) const
{
  ScopedAlgorithmStatusReporter asr(this, "GetFieldBoundary");

  /// Define types we need for mapping
  using hash_map_type = boost::unordered_map<index_type,index_type,IndexHash>;

  hash_map_type node_map;
  hash_map_type elem_map;

  /// Check whether we have an input field
  if (!input)
  {
    error("No input field");
    return (false);
  }

  /// Figure out what the input type and output type have to be
  FieldInformation fi(input);
  FieldInformation fo(input);

  /// We do not yet support Quadratic and Cubic Meshes here
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    return (false);
  }

  /// Figure out which type of field the output is:
  auto found_method = false;
  if (fi.is_hex_element())    { fo.make_quadsurfmesh(); found_method = true; }
  if (fi.is_prism_element())  { fo.make_quadsurfmesh(); found_method = true; }
  if (fi.is_tet_element())    { fo.make_trisurfmesh(); found_method = true; }
  if (fi.is_quad_element())   { fo.make_curvemesh(); found_method = true; }
  if (fi.is_tri_element())    { fo.make_curvemesh(); found_method = true; }
  if (fi.is_pnt_element())
  {
    remark("The field boundary of a point cloud is the same point cloud");
    output = input;
    return (true);
  }

  /// Check whether we could make a conversion
  if (!found_method)
  {
    error("No method available for mesh of type: " + fi.get_mesh_type());
    return (false);
  }

  /// Create the output field
  output = CreateField(fo);
  if (!output)
  {
    error("Could not create output field");
    return (false);
  }

  /// Get the virtual interfaces:
  auto imesh = input->vmesh();
  auto omesh = output->vmesh();
  auto ifield = input->vfield();
  auto ofield = output->vfield();

  imesh->synchronize(Mesh::DELEMS_E | Mesh::ELEM_NEIGHBORS_E);

  /// These are all virtual iterators, virtual index_types and array_types
  VMesh::Elem::iterator be, ee;
  VMesh::Elem::index_type nci, ci;
  VMesh::DElem::array_type delems;
  VMesh::Node::array_type inodes;
  VMesh::Node::array_type onodes;
  VMesh::Node::index_type a;

  inodes.clear();
  onodes.clear();
  Point point;

  /// This algorithm was copy from the original dynamic compiled version
  /// and was slightly adapted to work here:

  imesh->begin(be);
  imesh->end(ee);

  while (be != ee)
  {
    checkForInterruption();
    ci = *be;
    imesh->get_delems(delems, ci);
    for (size_t p = 0; p < delems.size(); p++)
    {
      auto includeface = false;

      if (!(imesh->get_neighbor(nci, ci, delems[p]))) includeface = true;

      if (includeface)
      {
        imesh->get_nodes(inodes, delems[p]);
        onodes.resize(inodes.size());

        for (size_t q = 0; q < inodes.size(); q++)
        {
          a = inodes[q];
          auto it = node_map.find(a);
          if (it == node_map.end())
          {
            imesh->get_center(point, a);
            onodes[q] = omesh->add_node(point);
            node_map[a] = onodes[q];
          }
          else
          {
            onodes[q] = node_map[a];
          }
        }
        elem_map[omesh->add_elem(onodes)] = ci;
      }
    }
    ++be;
  }

  mapping.reset();

  ofield->resize_fdata();

  if (
    (
    (ifield->basis_order() == 0)
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
      && checkOption("mapping","auto")
      )
      ||
       checkOption("mapping","elem")
#else
    )
#endif
      )
  {
    VMesh::Elem::size_type isize;
    VMesh::Elem::size_type osize;
    imesh->size(isize);
    omesh->size(osize);

    size_type nrows = osize;
    size_type ncols = isize;

    typedef SparseRowMatrix::Triplet T;
    std::vector<T> tripletList;
    tripletList.reserve(nrows);

    hash_map_type::iterator it, it_end;
    it = elem_map.begin();
    it_end = elem_map.end();

    while (it != it_end)
    {
      tripletList.push_back(T(it->first, it->second, 1));
      ++it;
    }
    SparseRowMatrixHandle mat(new SparseRowMatrix(nrows, ncols));
    mat->setFromTriplets(tripletList.begin(), tripletList.end());
    mapping = mat;
  }
  else if (
    ((ifield->basis_order() == 1)
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    && checkOption("mapping","auto"))
    ||
      checkOption("mapping","node")
#else
    )
#endif
      )
  {
    VMesh::Node::size_type isize;
    VMesh::Node::size_type osize;
    imesh->size(isize);
    omesh->size(osize);

    size_type nrows = osize;
    size_type ncols = isize;

    typedef Eigen::Triplet<double> T;
    std::vector<T> tripletList;
    tripletList.reserve(nrows);

    hash_map_type::iterator it, it_end;
    it = node_map.begin();
    it_end = node_map.end();

    while (it != it_end)
    {
      tripletList.push_back(T(it->second, it->first, 1));
      ++it;
    }
    SparseRowMatrixHandle mat(new SparseRowMatrix(nrows, ncols));
    mat->setFromTriplets(tripletList.begin(), tripletList.end());
    mapping = mat;
  }

  if (ifield->basis_order() == 0)
  {
    hash_map_type::iterator it, it_end;
    it = elem_map.begin();
    it_end = elem_map.end();

    while (it != it_end)
    {
      checkForInterruption();
      VMesh::Elem::index_type idx1((*it).second);
      VMesh::Elem::index_type idx2((*it).first);

      /// Copying values
      ofield->copy_value(ifield,idx1,idx2);
      ++it;
    }
  }
  else if (input->basis_order() == 1)
  {
    hash_map_type::iterator it, it_end;
    it = node_map.begin();
    it_end = node_map.end();

    while (it != it_end)
    {
      checkForInterruption();
      VMesh::Node::index_type idx1((*it).first);
      VMesh::Node::index_type idx2((*it).second);
      ofield->copy_value(ifield,idx1,idx2);
      ++it;
    }
  }

  CopyProperties(*input, *output);

  return (true);
}


/// A copy of the algorithm without creating the mapping matrix.
/// Need this for the various algorithms that only use the boundary to
/// project nodes on.

bool
GetFieldBoundaryAlgo::run(FieldHandle input, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "GetFieldBoundary");

  /// Define types we need for mapping
  using hash_map_type = boost::unordered_map<index_type,index_type,IndexHash>;

  hash_map_type node_map;
  hash_map_type elem_map;

  /// Check whether we have an input field
  if (!input)
  {
    error("No input field");
    return (false);
  }

  /// Figure out what the input type and output type have to be
  FieldInformation fi(input);
  FieldInformation fo(input);

  /// We do not yet support Quadratic and Cubic Meshes here
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    return (false);
  }

  /// Figure out which type of field the output is:
  auto found_method = false;
  if (fi.is_hex_element())    { fo.make_quadsurfmesh(); found_method = true; }
  if (fi.is_prism_element())  { fo.make_quadsurfmesh(); found_method = true; }
  if (fi.is_tet_element())    { fo.make_trisurfmesh(); found_method = true; }
  if (fi.is_quad_element())   { fo.make_curvemesh(); found_method = true; }
  if (fi.is_tri_element())    { fo.make_curvemesh(); found_method = true; }
  if (fi.is_pnt_element())
  {
    remark("The field boundary of a point cloud is the same point cloud");
    output = input;
    return (true);
  }

  /// Check whether we could make a conversion
  if (!found_method)
  {
    error("No method available for mesh of type: " + fi.get_mesh_type());
    return (false);
  }

  /// Create the output field
  output = CreateField(fo);
  if (!output)
  {
    error("Could not create output field");
    return (false);
  }

  /// Get the virtual interfaces:
  auto imesh = input->vmesh();
  auto omesh = output->vmesh();
  auto ifield = input->vfield();
  auto ofield = output->vfield();

  imesh->synchronize(Mesh::DELEMS_E|Mesh::ELEM_NEIGHBORS_E);

  /// These are all virtual iterators, virtual index_types and array_types
  VMesh::Elem::iterator be, ee;
  VMesh::Elem::index_type nci, ci;
  VMesh::DElem::array_type delems;
  VMesh::Node::array_type inodes;
  VMesh::Node::array_type onodes;
  VMesh::Node::index_type a;

  inodes.clear();
  onodes.clear();
  Point point;

  /// This algorithm was copy from the original dynamic compiled version
  /// and was slightly adapted to work here:

  imesh->begin(be);
  imesh->end(ee);

  while (be != ee)
  {
    checkForInterruption();
    ci = *be;
    imesh->get_delems(delems, ci);
    for (size_t p = 0; p < delems.size(); p++)
    {
      auto includeface = false;

      if (!(imesh->get_neighbor(nci, ci, delems[p]))) includeface = true;

      if (includeface)
      {
        imesh->get_nodes(inodes, delems[p]);
        if (onodes.size() == 0) onodes.resize(inodes.size());
        for (size_t q = 0; q < onodes.size(); q++)
        {
          a = inodes[q];
          auto it = node_map.find(a);
          if (it == node_map.end())
          {
            imesh->get_center(point, a);
            onodes[q] = omesh->add_node(point);
            node_map[a] = onodes[q];
          }
          else
          {
            onodes[q] = node_map[a];
          }
        }
        elem_map[omesh->add_elem(onodes)] = ci;
      }
    }
    ++be;
  }

  ofield->resize_fdata();

  if (ifield->basis_order() == 0)
  {
    hash_map_type::iterator it, it_end;
    it = elem_map.begin();
    it_end = elem_map.end();

    while (it != it_end)
    {
      VMesh::Elem::index_type idx1((*it).second);
      VMesh::Elem::index_type idx2((*it).first);

      /// Copying values
      ofield->copy_value(ifield,idx1,idx2);
      ++it;
    }
  }
  else if (input->basis_order() == 1)
  {
    hash_map_type::iterator it, it_end;
    it = node_map.begin();
    it_end = node_map.end();

    while (it != it_end)
    {
      VMesh::Node::index_type idx1((*it).first);
      VMesh::Node::index_type idx2((*it).second);
      ofield->copy_value(ifield,idx1,idx2);
      ++it;
    }
  }

  CopyProperties(*input, *output);

  return (true);
}

AlgorithmOutput GetFieldBoundaryAlgo::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);

  FieldHandle boundary;
  MatrixHandle mapping;
  if (!run(field, boundary, mapping))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[BoundaryField] = boundary;
  output[MappingMatrix] = mapping;
  return output;
}
