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
#include <boost/foreach.hpp>
#include <Core/Datatypes/Mesh/MeshFactory.h>
#include <Core/Datatypes/Mesh/FieldInformation.h>
#include <Core/Datatypes/Mesh/LatticeVolumeMesh.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>

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
    mesh_ = MeshFactory::Instance().CreateMesh(lfi, MeshConstructionParameters(sizex, sizey, sizez, minb, maxb));
  }

  MeshHandle mesh_;
};

class LatticeVolumeMeshFacade : public MeshFacade
{
public:
  explicit LatticeVolumeMeshFacade(VirtualMeshHandle vmesh) : vmesh_(vmesh) 
  {
    if (!vmesh->is_latvolmesh())
      THROW_INVALID_ARGUMENT("Incorrect mesh type for this facade type.");
  }

  virtual Edges edges() const 
  {
    return Edges(SmartEdgeIterator(vmesh_.get()), SmartEdgeIterator(vmesh_.get(), true));
  }

  virtual size_t numNodes() const
  {
    return vmesh_->num_nodes();
  }

  virtual size_t numEdges() const
  {
    return vmesh_->num_edges();
  }
  
  virtual size_t numFaces() const 
  {
    return vmesh_->num_faces();
  }

  virtual size_t numElements() const 
  {
    return vmesh_->num_elems();
  }
private:
  VirtualMeshHandle vmesh_;
};

TEST_F(LatticeVolumeMeshFacadeTests, BasicCubeTests)
{
  ASSERT_TRUE(mesh_);
  
  LatticeVolumeMeshFacade facade(mesh_->vmesh());

  EXPECT_EQ(8, facade.numNodes());
  EXPECT_EQ(12, facade.numEdges());
  EXPECT_EQ(6, facade.numFaces());
  EXPECT_EQ(1, facade.numElements());
}

TEST_F(LatticeVolumeMeshFacadeTests, CubeIterationTests)
{
  LatticeVolumeMeshFacade facade(mesh_->vmesh());

  std::ostringstream ostr;
  std::cout << "Begin iterating edges" << std::endl;
  BOOST_FOREACH(const EdgeInfo& edge, facade.edges())
  {
    auto nodesFromEdge = edge.nodeIndices();
    auto nodePoints = edge.nodePoints();
    ostr << "Edge " << edge.index() << " nodes=[" << nodesFromEdge[0] << " point=" << nodePoints[0].get_string()
      << ", " << nodesFromEdge[1] << " point=" << nodePoints[1].get_string() << "]" << std::endl;
  }
  std::cout << ostr.str() << std::endl;
  std::cout << "End iterating edges" << std::endl;

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
