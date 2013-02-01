/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
#include <Core/Datatypes/Mesh/LatticeVolumeMesh.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;
using namespace boost::assign;

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

TEST_F(LatticeVolumeMeshTests, BasicCubeTests)
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

TEST_F(LatticeVolumeMeshTests, CubeIterationTests)
{
  auto latVolVMesh = mesh_->vmesh();

  VirtualMesh::Edge::iterator meshEdgeIter;
  VirtualMesh::Edge::iterator meshEdgeEnd;

  VirtualMesh::Node::array_type nodesFromEdge(2);

  latVolVMesh->end(meshEdgeEnd);

  for (latVolVMesh->begin(meshEdgeIter); meshEdgeIter != meshEdgeEnd; ++meshEdgeIter)
  {
    // get nodes from edge

    VirtualMesh::Edge::index_type edgeID = *meshEdgeIter;
    latVolVMesh->get_nodes(nodesFromEdge, edgeID);
    Point p0, p1;
    latVolVMesh->get_point(p0, nodesFromEdge[0]);
    latVolVMesh->get_point(p1, nodesFromEdge[1]);
    std::cout << "Edge " << edgeID << " nodes=[" << nodesFromEdge[0] << " point=" << p0.get_string()
      << ", " << nodesFromEdge[1] << " point=" << p1.get_string() << "]" << std::endl;
  }

  VirtualMesh::Face::iterator meshFaceIter;
  VirtualMesh::Face::iterator meshFaceEnd;

  VirtualMesh::Edge::array_type edgesFromFace(4);
  VirtualMesh::Node::array_type nodesFromFace(4);

  latVolVMesh->end(meshFaceEnd);

  for (latVolVMesh->begin(meshFaceIter); meshFaceIter != meshFaceEnd; ++meshFaceIter)
  {
    // get edges and nodes from face

    VirtualMesh::Face::index_type faceID = *meshFaceIter;
    latVolVMesh->get_edges(edgesFromFace, faceID);
    std::cout << "Face " << faceID << " edges=[" << join(edgesFromFace) << "]" << std::endl;

    latVolVMesh->get_nodes(nodesFromFace, faceID);
    std::cout << "Face " << faceID << " nodes=[" << join(nodesFromFace) << "]" << std::endl;
  }

  VirtualMesh::Cell::iterator meshCellIter;
  VirtualMesh::Cell::iterator meshCellEnd;

  VirtualMesh::Edge::array_type edgesFromCell(12);
  VirtualMesh::Node::array_type nodesFromCell(8);

  latVolVMesh->end(meshCellEnd);

  for (latVolVMesh->begin(meshCellIter); meshCellIter != meshCellEnd; ++meshCellIter)
  {
    // get edges and nodes from mesh element

    VirtualMesh::Cell::index_type elemID = *meshCellIter;
    latVolVMesh->get_edges(edgesFromCell, elemID);
    std::cout << "Cell " << elemID << " edges=[" << join(edgesFromCell) << "]" << std::endl;

    latVolVMesh->get_nodes(nodesFromCell, elemID);
    std::cout << "Cell " << elemID << " nodes=["<< join(nodesFromCell) << "]" << std::endl;
  }
}