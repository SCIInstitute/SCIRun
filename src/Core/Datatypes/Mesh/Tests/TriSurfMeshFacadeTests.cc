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

//#include <Core/Datatypes/Legacy/Field/MeshFactory.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/TriSurfMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <Core/GeometryPrimitives/Point.h>

#include <iostream>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

using ::testing::_;
using ::testing::DefaultValue;
using ::testing::Return;
using namespace boost::assign;

class TriSurfMeshFacadeTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    {
      FieldInformation fi("TriSurfMesh", LINEARDATA_E, "double");
      basicTriangleMesh_ = CreateMesh(fi);
      auto triangleVMesh = basicTriangleMesh_->vmesh();
      triangleVMesh->add_point(Point(0.0, 0.0, 0.0));
      triangleVMesh->add_point(Point(1.0, 0.0, 0.0));
      triangleVMesh->add_point(Point(0.5, 1.0, 0.0));

      VMesh::Node::array_type vdata;
      vdata += 0, 1, 2;
      triangleVMesh->add_elem(vdata);
    }

    {
      FieldInformation fi("TriSurfMesh", LINEARDATA_E, "double");
      cubeMesh_ = CreateMesh(fi);
      auto cubeVMesh = cubeMesh_->vmesh();
      cubeVMesh->add_point(Point(0.0, 1.0, 0.0));
      cubeVMesh->add_point(Point(0.0, 0.0, 0.0));
      cubeVMesh->add_point(Point(1.0, 1.0, 0.0));
      cubeVMesh->add_point(Point(1.0, 0.0, 0.0));
      cubeVMesh->add_point(Point(1.0, 0.0, -1.0));
      cubeVMesh->add_point(Point(1.0, 1.0, -1.0));
      cubeVMesh->add_point(Point(0.0, 1.0, -1.0));
      cubeVMesh->add_point(Point(0.0, 0.0, -1.0));

      VMesh::Node::array_type vdata1;
      vdata1 += 0, 1, 7;
      cubeVMesh->add_elem(vdata1);
      VMesh::Node::array_type vdata2;
      vdata2 += 0, 7, 6;
      cubeVMesh->add_elem(vdata2);
      VMesh::Node::array_type vdata3;
      vdata3 += 1, 0, 2;
      cubeVMesh->add_elem(vdata2);
      VMesh::Node::array_type vdata4;
      vdata4 += 1, 3, 2;
      cubeVMesh->add_elem(vdata4);
      VMesh::Node::array_type vdata5;
      vdata5 += 2, 3, 4;
      cubeVMesh->add_elem(vdata5);
      VMesh::Node::array_type vdata6;
      vdata6 += 2, 4, 5;
      cubeVMesh->add_elem(vdata6);
      VMesh::Node::array_type vdata7;
      vdata7 += 4, 7, 1;
      cubeVMesh->add_elem(vdata7);
      VMesh::Node::array_type vdata8;
      vdata8 += 4, 3, 1;
      cubeVMesh->add_elem(vdata8);
      VMesh::Node::array_type vdata9;
      vdata9 += 5, 6, 0;
      cubeVMesh->add_elem(vdata9);
      VMesh::Node::array_type vdata10;
      vdata10 += 5, 2, 0;
      cubeVMesh->add_elem(vdata10);
      VMesh::Node::array_type vdata11;
      vdata11 += 7, 6, 5;
      cubeVMesh->add_elem(vdata11);
      VMesh::Node::array_type vdata12;
      vdata12 += 7, 4, 5;
      cubeVMesh->add_elem(vdata12);
    }

    {
      FieldInformation fi("TriSurfMesh", LINEARDATA_E, "double");
      tetrahedronMesh_ = CreateMesh(fi);
      auto tetrahedronVMesh = tetrahedronMesh_->vmesh();
      tetrahedronVMesh->add_point(Point(1.0, 0.0, -0.707));
      tetrahedronVMesh->add_point(Point(-1.0, 0.0, -0.707));
      tetrahedronVMesh->add_point(Point(0.0, 1.0, 0.707));
      tetrahedronVMesh->add_point(Point(0.0, -1.0, 0.707));

      VMesh::Node::array_type vdata1;
      vdata1 += 0, 1, 2;
      tetrahedronVMesh->add_elem(vdata1);
      VMesh::Node::array_type vdata2;
      vdata2 += 0, 1, 3;
      tetrahedronVMesh->add_elem(vdata2);
      VMesh::Node::array_type vdata3;
      vdata3 += 1, 2, 3;
      tetrahedronVMesh->add_elem(vdata2);
      VMesh::Node::array_type vdata4;
      vdata4 += 0, 2, 3;
      tetrahedronVMesh->add_elem(vdata4);
    }
  }

  MeshHandle basicTriangleMesh_;
  MeshHandle cubeMesh_;
  MeshHandle tetrahedronMesh_;
};

/// @todo: move to utils file (duplicated in LatticeVolumeMeshFacadeTests.cc)
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

TEST_F(TriSurfMeshFacadeTests, BasicTriangleTest)
{
  ASSERT_TRUE(basicTriangleMesh_.get() != nullptr);

  auto facade(basicTriangleMesh_->getFacade());

  EXPECT_EQ(3, facade->numNodes());
  EXPECT_EQ(1, facade->numFaces());
  EXPECT_EQ(1, facade->numElements());
  EXPECT_EQ(3, facade->numEdges());
}

TEST_F(TriSurfMeshFacadeTests, CubeTest)
{
  ASSERT_TRUE(cubeMesh_.get() != nullptr);

  auto facade(cubeMesh_->getFacade());

  EXPECT_EQ(8, facade->numNodes());
  EXPECT_EQ(12, facade->numFaces());
  EXPECT_EQ(12, facade->numElements());
  EXPECT_EQ(18, facade->numEdges());
}

TEST_F(TriSurfMeshFacadeTests, TetrahedronTest)
{
  ASSERT_TRUE(tetrahedronMesh_.get() != nullptr);

  auto facade(tetrahedronMesh_->getFacade());

  EXPECT_EQ(4, facade->numNodes());
  EXPECT_EQ(4, facade->numFaces());
  EXPECT_EQ(4, facade->numElements());
  EXPECT_EQ(6, facade->numEdges());
}

TEST_F(TriSurfMeshFacadeTests, BasicTriangleEdgeIterationTest)
{
  auto facade(basicTriangleMesh_->getFacade());

  std::ostringstream ostr;
  for (const auto& edge : facade->edges())
  {
    auto nodesFromEdge = edge.nodeIndices();
    auto nodePoints = edge.nodePoints();
    ostr << "Edge " << edge.index() << " nodes=[" << nodesFromEdge[0] << " point=" << nodePoints[0].get_string()
    << ", " << nodesFromEdge[1] << " point=" << nodePoints[1].get_string() << "]" << std::endl;
  }

  EXPECT_EQ(
            "Edge 0 nodes=[2 point=[0.5, 1, 0], 0 point=[0, 0, 0]]\n"
            "Edge 1 nodes=[1 point=[1, 0, 0], 2 point=[0.5, 1, 0]]\n"
            "Edge 2 nodes=[0 point=[0, 0, 0], 1 point=[1, 0, 0]]\n"
            , ostr.str());
}

TEST_F(TriSurfMeshFacadeTests, BasicTriangleFaceIterationTest)
{
  auto facade(basicTriangleMesh_->getFacade());
  ASSERT_TRUE(facade.get() != nullptr);
  std::ostringstream ostr;
  for (const FaceInfo<VMesh>& face : facade->faces())
  {
    auto faceID = face.index();
    auto edges = face.edgeIndices();
    auto nodes = face.nodeIndices();
    ostr << "Face " << faceID << " edges=[" << join(edges) << "]" << std::endl;
    ostr << "Face " << faceID << " nodes=[" << join(nodes) << "]" << std::endl;
  }

  EXPECT_EQ("Face 0 edges=[2, 1, 0]\n"
            "Face 0 nodes=[0, 1, 2]\n"
            ,ostr.str());
}

TEST_F(TriSurfMeshFacadeTests, BasicTriangleNodeIterationTest)
{
  auto facade(basicTriangleMesh_->getFacade());
  ASSERT_TRUE(facade.get() != nullptr);
  std::ostringstream ostr;
  for (const NodeInfo<VMesh>& node : facade->nodes())
  {
    // special case, since this is essentially a 2D mesh with a single element,
    // the last edge value is not filled
    auto edges = node.edgeIndices();
    ostr << "Node " << node.index() << " point=" << node.point().get_string() << " edges=[" << join(edges) << "]" << std::endl;
  }

  EXPECT_EQ("Node 0 point=[0, 0, 0] edges=[2, 1, 0]\n"
            "Node 1 point=[1, 0, 0] edges=[2, 1, 0]\n"
            "Node 2 point=[0.5, 1, 0] edges=[2, 1, 0]\n"
            ,ostr.str());
}
