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

/// Tests for VMesh::get_all_neighbors (issue #2434): every element on a delem,
/// without having to hand one of them in.

#include <Testing/Utils/SCIRunFieldSamples.h>

#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <vector>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::TestUtils;

namespace
{
  std::set<VMesh::index_type> toSet(const VMesh::Elem::array_type& elems)
  {
    std::set<VMesh::index_type> s;
    for (const auto& e : elems) s.insert(e);
    return s;
  }

  bool contains(const VMesh::DElem::array_type& delems, VMesh::DElem::index_type d)
  {
    return std::find(delems.begin(), delems.end(), d) != delems.end();
  }

  /// The properties get_all_neighbors has to hold for any mesh type:
  /// every returned element really touches the delem, no duplicates, and the
  /// result agrees with what get_neighbor/get_neighbors say once you pick one
  /// of the elements to stand in as "from".
  void checkDelemNeighborsAgree(VMesh* mesh)
  {
    VMesh::DElem::size_type numDelems;
    mesh->size(numDelems);
    ASSERT_GT(numDelems, 0);

    for (VMesh::index_type d = 0; d < numDelems; ++d)
    {
      const VMesh::DElem::index_type delem(d);

      VMesh::Elem::array_type all;
      mesh->get_all_neighbors(all, delem);

      const auto allSet = toSet(all);
      EXPECT_EQ(allSet.size(), all.size()) << "duplicate elements on delem " << d;
      EXPECT_FALSE(all.empty()) << "delem " << d << " belongs to no element";

      for (const auto& elem : all)
      {
        VMesh::DElem::array_type delems;
        mesh->get_delems(delems, elem);
        EXPECT_TRUE(contains(delems, delem))
          << "elem " << elem << " does not actually touch delem " << d;

        // get_neighbors() is get_all_neighbors() minus the element you asked from.
        VMesh::Elem::array_type others;
        const bool hasNeighbor = mesh->get_neighbors(others, elem, delem);
        auto expected = allSet;
        expected.erase(elem);
        EXPECT_EQ(expected, toSet(others));
        EXPECT_EQ(hasNeighbor, !expected.empty());

        // Same for the single-neighbor call.
        VMesh::Elem::index_type neighbor;
        if (mesh->get_neighbor(neighbor, elem, delem))
          EXPECT_EQ(1u, expected.count(neighbor));
        else
          EXPECT_TRUE(expected.empty());
      }
    }
  }

  /// Number of delems shared by exactly n elements.
  size_t countDelemsWith(VMesh* mesh, size_t n)
  {
    VMesh::DElem::size_type numDelems;
    mesh->size(numDelems);

    size_t count = 0;
    for (VMesh::index_type d = 0; d < numDelems; ++d)
    {
      VMesh::Elem::array_type all;
      mesh->get_all_neighbors(all, VMesh::DElem::index_type(d));
      if (all.size() == n) ++count;
    }
    return count;
  }
}

TEST(VMeshGetAllNeighborsTest, TetVolAgreesWithGetNeighbor)
{
  auto field = CubeTetVolLinearBasis(data_info_type::NONE_E);
  auto mesh = field->vmesh();
  mesh->synchronize(Mesh::FACES_E | Mesh::DELEMS_E | Mesh::ELEM_NEIGHBORS_E);

  checkDelemNeighborsAgree(mesh);
}

TEST(VMeshGetAllNeighborsTest, TetVolCubeSplitsIntoBoundaryAndInteriorFaces)
{
  auto field = CubeTetVolLinearBasis(data_info_type::NONE_E);
  auto mesh = field->vmesh();
  mesh->synchronize(Mesh::FACES_E | Mesh::DELEMS_E | Mesh::ELEM_NEIGHBORS_E);

  // 6 tets, 4 faces each: the 12 cube-surface triangles are used once, the
  // 6 interior faces twice.
  VMesh::DElem::size_type numDelems;
  mesh->size(numDelems);
  EXPECT_EQ(18, numDelems);
  EXPECT_EQ(12u, countDelemsWith(mesh, 1));
  EXPECT_EQ(6u, countDelemsWith(mesh, 2));
  EXPECT_EQ(0u, countDelemsWith(mesh, 3));
}

TEST(VMeshGetAllNeighborsTest, TetVolSingleElementHasOnlyBoundaryFaces)
{
  auto field = TetrahedronTetVolLinearBasis(data_info_type::NONE_E);
  auto mesh = field->vmesh();
  mesh->synchronize(Mesh::FACES_E | Mesh::DELEMS_E | Mesh::ELEM_NEIGHBORS_E);

  VMesh::DElem::size_type numDelems;
  mesh->size(numDelems);
  EXPECT_EQ(4, numDelems);

  for (VMesh::index_type d = 0; d < numDelems; ++d)
  {
    VMesh::Elem::array_type all;
    mesh->get_all_neighbors(all, VMesh::DElem::index_type(d));
    ASSERT_EQ(1u, all.size());
    EXPECT_EQ(0, all[0]);
  }
}

TEST(VMeshGetAllNeighborsTest, TriSurfAgreesWithGetNeighbor)
{
  auto field = CubeTriSurfLinearBasis(data_info_type::NONE_E);
  auto mesh = field->vmesh();
  mesh->synchronize(Mesh::EDGES_E | Mesh::DELEMS_E | Mesh::ELEM_NEIGHBORS_E);

  checkDelemNeighborsAgree(mesh);
}

TEST(VMeshGetAllNeighborsTest, TriSurfClosedSurfaceHasNoBoundaryEdges)
{
  auto field = CubeTriSurfLinearBasis(data_info_type::NONE_E);
  auto mesh = field->vmesh();
  mesh->synchronize(Mesh::EDGES_E | Mesh::DELEMS_E | Mesh::ELEM_NEIGHBORS_E);

  VMesh::DElem::size_type numDelems;
  mesh->size(numDelems);
  EXPECT_EQ(countDelemsWith(mesh, 2), static_cast<size_t>(numDelems));
}

TEST(VMeshGetAllNeighborsTest, TriSurfSingleTriangleIsAllBoundary)
{
  auto field = TriangleTriSurfLinearBasis(data_info_type::NONE_E);
  auto mesh = field->vmesh();
  mesh->synchronize(Mesh::EDGES_E | Mesh::DELEMS_E | Mesh::ELEM_NEIGHBORS_E);

  checkDelemNeighborsAgree(mesh);

  VMesh::DElem::size_type numDelems;
  mesh->size(numDelems);
  EXPECT_EQ(3, numDelems);
  EXPECT_EQ(3u, countDelemsWith(mesh, 1));
}

TEST(VMeshGetAllNeighborsTest, LatVolAgreesWithGetNeighbor)
{
  auto field = CreateEmptyLatVol(3, 3, 3);
  auto mesh = field->vmesh();
  mesh->synchronize(Mesh::FACES_E | Mesh::DELEMS_E | Mesh::ELEM_NEIGHBORS_E);

  checkDelemNeighborsAgree(mesh);
}

TEST(VMeshGetAllNeighborsTest, LatVolInteriorFacesHaveTwoElements)
{
  // 2x2x2 cells: 36 faces, of which 24 are on the outside of the block.
  auto field = CreateEmptyLatVol(3, 3, 3);
  auto mesh = field->vmesh();
  mesh->synchronize(Mesh::FACES_E | Mesh::DELEMS_E | Mesh::ELEM_NEIGHBORS_E);

  VMesh::DElem::size_type numDelems;
  mesh->size(numDelems);
  EXPECT_EQ(36, numDelems);
  EXPECT_EQ(24u, countDelemsWith(mesh, 1));
  EXPECT_EQ(12u, countDelemsWith(mesh, 2));
}

TEST(VMeshGetAllNeighborsTest, PointCloudDelemIsItsOwnElement)
{
  FieldInformation fi(mesh_info_type::POINTCLOUDMESH_E,
                      databasis_info_type::CONSTANTDATA_E, data_info_type::DOUBLE_E);
  auto field = CreateField(fi);
  auto mesh = field->vmesh();
  mesh->add_point(Point(0.0, 0.0, 0.0));
  mesh->add_point(Point(1.0, 0.0, 0.0));
  mesh->add_point(Point(0.0, 1.0, 0.0));

  // A point cloud reports no delems of its own, but node, elem and delem all
  // index the same thing, so a delem index is still answerable.
  VMesh::Elem::size_type numElems;
  mesh->size(numElems);
  ASSERT_EQ(3, numElems);

  // Points have no neighbors, but a point is still the one element on its delem.
  for (VMesh::index_type d = 0; d < numElems; ++d)
  {
    VMesh::Elem::array_type all;
    mesh->get_all_neighbors(all, VMesh::DElem::index_type(d));
    ASSERT_EQ(1u, all.size());
    EXPECT_EQ(d, all[0]);
  }
}
