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
#include <Core/Datatypes/Mesh/MeshFactory.h>
#include <Core/Datatypes/Mesh/FieldInformation.h>
#include <Core/Datatypes/Mesh/TriSurfMesh.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;
using namespace boost::assign;

#if 0
class LatticeVolumeMeshTests : public ::testing::Test
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
    mesh_ = MeshFactory::Instance().CreateMesh(lfi, MeshConstructionParameters(sizex, sizey, sizez, minb, maxb));
  }

  MeshHandle mesh_;
};

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

TEST_F(LatticeVolumeMeshTests, BasicCubeTest)
{
  ASSERT_TRUE(mesh_);

  auto latVolVMesh = mesh_->vmesh();
  ASSERT_TRUE(latVolVMesh);

  VirtualMesh::dimension_type dims;
  latVolVMesh->get_dimensions(dims);

  VirtualMesh::dimension_type expectedDims;
  expectedDims += 2,2,2;
  EXPECT_EQ(expectedDims, dims);

  EXPECT_EQ(8, latVolVMesh->num_nodes());
  EXPECT_EQ(12, latVolVMesh->num_edges());
  EXPECT_EQ(6, latVolVMesh->num_faces());
  EXPECT_EQ(1, latVolVMesh->num_elems());

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  ASSERT_TRUE(latVolVMesh->is_linearmesh());

  ASSERT_FALSE(latVolVMesh->has_normals());
#endif
}

TEST_F(LatticeVolumeMeshTests, CubeIterationTest)
{
  auto latVolVMesh = mesh_->vmesh();

  {
    VirtualMesh::Edge::iterator meshEdgeIter;
    VirtualMesh::Edge::iterator meshEdgeEnd;

    VirtualMesh::Node::array_type nodesFromEdge(2);

    latVolVMesh->end(meshEdgeEnd);

    std::ostringstream ostr;
    for (latVolVMesh->begin(meshEdgeIter); meshEdgeIter != meshEdgeEnd; ++meshEdgeIter)
    {
      // get nodes from edge

      VirtualMesh::Edge::index_type edgeID = *meshEdgeIter;
      latVolVMesh->get_nodes(nodesFromEdge, edgeID);
      Point p0, p1;
      latVolVMesh->get_point(p0, nodesFromEdge[0]);
      latVolVMesh->get_point(p1, nodesFromEdge[1]);
      ostr << "Edge " << edgeID << " nodes=[" << nodesFromEdge[0] << " point=" << p0.get_string()
        << ", " << nodesFromEdge[1] << " point=" << p1.get_string() << "]" << std::endl;
    }
    //std::cout << ostr.str() << std::endl;

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

  {
    VirtualMesh::Face::iterator meshFaceIter;
    VirtualMesh::Face::iterator meshFaceEnd;

    VirtualMesh::Edge::array_type edgesFromFace(4);
    VirtualMesh::Node::array_type nodesFromFace(4);

    latVolVMesh->end(meshFaceEnd);

    std::ostringstream ostr;
    for (latVolVMesh->begin(meshFaceIter); meshFaceIter != meshFaceEnd; ++meshFaceIter)
    {
      // get edges and nodes from face

      VirtualMesh::Face::index_type faceID = *meshFaceIter;
      latVolVMesh->get_edges(edgesFromFace, faceID);
      ostr << "Face " << faceID << " edges=[" << join(edgesFromFace) << "]" << std::endl;

      latVolVMesh->get_nodes(nodesFromFace, faceID);
      ostr << "Face " << faceID << " nodes=[" << join(nodesFromFace) << "]" << std::endl;
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

    //std::cout << ostr.str() << std::endl;
  }

  {
    VirtualMesh::Cell::iterator meshCellIter;
    VirtualMesh::Cell::iterator meshCellEnd;

    VirtualMesh::Edge::array_type edgesFromCell(12);
    VirtualMesh::Node::array_type nodesFromCell(8);

    latVolVMesh->end(meshCellEnd);
    std::ostringstream ostr;

    for (latVolVMesh->begin(meshCellIter); meshCellIter != meshCellEnd; ++meshCellIter)
    {
      // get edges and nodes from mesh element

      VirtualMesh::Cell::index_type elemID = *meshCellIter;
      latVolVMesh->get_edges(edgesFromCell, elemID);
      ostr << "Cell " << elemID << " edges=[" << join(edgesFromCell) << "]" << std::endl;

      latVolVMesh->get_nodes(nodesFromCell, elemID);
      ostr << "Cell " << elemID << " nodes=["<< join(nodesFromCell) << "]" << std::endl;
    }
    //std::cout << ostr.str() << std::endl;

    EXPECT_EQ(
      "Cell 0 edges=[0, 1, 2, 3, 4, 6, 5, 7, 8, 9, 10, 11]\n"
      "Cell 0 nodes=[0, 1, 3, 2, 4, 5, 7, 6]\n",
      ostr.str());
  }

  {
    VirtualMesh::Node::iterator meshNodeIter;
    VirtualMesh::Node::iterator meshNodeEnd;

    VirtualMesh::Edge::array_type edgesFromNode(3);

    latVolVMesh->end(meshNodeEnd);
    std::ostringstream ostr;

    for (latVolVMesh->begin(meshNodeIter); meshNodeIter != meshNodeEnd; ++meshNodeIter)
    {
      // get edges and point from mesh node

      VirtualMesh::Node::index_type nodeID = *meshNodeIter;
      Point p;
      latVolVMesh->get_point(p, nodeID);
      ostr << "Node " << nodeID << " point=" << p.get_string();
      latVolVMesh->get_edges(edgesFromNode, nodeID);
      ostr << " edges=[" << join(edgesFromNode) << "]" << std::endl;
    }
    //std::cout << ostr.str() << std::endl;

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
}
#endif
