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


#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/assign.hpp>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;
using namespace boost::assign;

class LatticeVolumeMeshFacadeTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    int basisOrder = 1;
    FieldInformation lfi("LatVolMesh", basisOrder, "double");
    int sizex,sizey,sizez;
    sizex = sizey = sizez = 2;
    Point minb(0,0,0);
    Point maxb(1,1,1);
    mesh_ = CreateMesh(lfi, sizex, sizey, sizez, minb, maxb);
  }

  MeshHandle mesh_;
};

/// @todo: move to utils file (duplicated in TriSurfMeshFacadeTests.cc)
namespace
{
  template <typename T>
  std::string join(const T& list)
  {
    std::ostringstream oss;
    const int SIZE = list.size();
    for (int i = 0; i < SIZE; ++i)
    {
      oss << list[i];
      if (i < SIZE - 1)
        oss << ", ";
    }
    return oss.str();
  }
}

TEST_F(LatticeVolumeMeshFacadeTests, BasicCubeTest)
{
  ASSERT_TRUE(mesh_.get() != nullptr);

  auto facade(mesh_->getFacade());

  EXPECT_EQ(8, facade->numNodes());
  EXPECT_EQ(12, facade->numEdges());
  EXPECT_EQ(6, facade->numFaces());
  EXPECT_EQ(1, facade->numElements());
}

TEST_F(LatticeVolumeMeshFacadeTests, CubeEdgeIterationTest)
{
  auto facade(mesh_->getFacade());

  std::ostringstream ostr;
  for (const auto& edge : facade->edges())
  {
    auto nodesFromEdge = edge.nodeIndices();
    auto nodePoints = edge.nodePoints();
    ostr << "Edge " << edge.index() << " nodes=[" << nodesFromEdge[0] << " point=" << nodePoints[0].get_string()
      << ", " << nodesFromEdge[1] << " point=" << nodePoints[1].get_string() << "]" << std::endl;
  }

  EXPECT_EQ(
    "Edge 0 nodes=[0 point=[0, 0, 0], 1 point=[1, 0, 0]]\n"
    "Edge 1 nodes=[2 point=[0, 1, 0], 3 point=[1, 1, 0]]\n"
    "Edge 2 nodes=[4 point=[0, 0, 1], 5 point=[1, 0, 1]]\n"
    "Edge 3 nodes=[6 point=[0, 1, 1], 7 point=[1, 1, 1]]\n"
    "Edge 4 nodes=[0 point=[0, 0, 0], 2 point=[0, 1, 0]]\n"
    "Edge 5 nodes=[4 point=[0, 0, 1], 6 point=[0, 1, 1]]\n"
    "Edge 6 nodes=[1 point=[1, 0, 0], 3 point=[1, 1, 0]]\n"
    "Edge 7 nodes=[5 point=[1, 0, 1], 7 point=[1, 1, 1]]\n"
    "Edge 8 nodes=[0 point=[0, 0, 0], 4 point=[0, 0, 1]]\n"
    "Edge 9 nodes=[1 point=[1, 0, 0], 5 point=[1, 0, 1]]\n"
    "Edge 10 nodes=[2 point=[0, 1, 0], 6 point=[0, 1, 1]]\n"
    "Edge 11 nodes=[3 point=[1, 1, 0], 7 point=[1, 1, 1]]\n"
    , ostr.str());
}

TEST_F(LatticeVolumeMeshFacadeTests, CubeFaceIterationTest)
{
  auto facade(mesh_->getFacade());

  std::ostringstream ostr;
  for (const auto& face : facade->faces())
  {
    auto faceID = face.index();
    auto edges = face.edgeIndices();
    auto nodes = face.nodeIndices();
    ostr << "Face " << faceID << " edges=[" << join(edges) << "]" << std::endl;
    ostr << "Face " << faceID << " nodes=[" << join(nodes) << "]" << std::endl;
  }

  EXPECT_EQ("Face 0 edges=[0, 1, 4, 6]\n"
    "Face 0 nodes=[0, 1, 3, 2]\n"
    "Face 1 edges=[2, 3, 5, 7]\n"
    "Face 1 nodes=[4, 6, 7, 5]\n"
    "Face 2 edges=[4, 5, 8, 10]\n"
    "Face 2 nodes=[0, 2, 6, 4]\n"
    "Face 3 edges=[6, 7, 9, 11]\n"
    "Face 3 nodes=[1, 5, 7, 3]\n"
    "Face 4 edges=[0, 2, 8, 9]\n"
    "Face 4 nodes=[0, 4, 5, 1]\n"
    "Face 5 edges=[1, 3, 10, 11]\n"
    "Face 5 nodes=[2, 3, 7, 6]\n"
    ,ostr.str());
}

TEST_F(LatticeVolumeMeshFacadeTests, CubeNodeIterationTest)
{
  auto facade(mesh_->getFacade());

  std::ostringstream ostr;
  for (const auto& node : facade->nodes())
  {
    ostr << "Node " << node.index() << " point=" << node.point().get_string() << " edges=[" << join(node.edgeIndices()) << "]" << std::endl;
  }

  EXPECT_EQ("Node 0 point=[0, 0, 0] edges=[0, 4, 8]\n"
    "Node 1 point=[1, 0, 0] edges=[0, 6, 9]\n"
    "Node 2 point=[0, 1, 0] edges=[1, 6, 10]\n"
    "Node 3 point=[1, 1, 0] edges=[1, 8, 11]\n"
    "Node 4 point=[0, 0, 1] edges=[2, 5, 9]\n"
    "Node 5 point=[1, 0, 1] edges=[2, 7, 10]\n"
    "Node 6 point=[0, 1, 1] edges=[3, 7, 11]\n"
    "Node 7 point=[1, 1, 1] edges=[3, 9, 12]\n",
    ostr.str());
}

TEST_F(LatticeVolumeMeshFacadeTests, CubeCellIterationTest)
{
  auto facade(mesh_->getFacade());

  std::ostringstream ostr;
  for (const auto& cell : facade->cells())
  {
    ostr << "Cell " << cell.index() << " center=" << cell.center().get_string() << " edges=[" << join(cell.edgeIndices()) << "]" << std::endl;
  }

  EXPECT_EQ("Cell 0 center=[0.5, 0.5, 0.5] edges=[0, 1, 2, 3, 4, 6, 5, 7, 8, 9, 10, 11]\n",
    ostr.str());

  EXPECT_EQ(1, facade->numCells());
}
