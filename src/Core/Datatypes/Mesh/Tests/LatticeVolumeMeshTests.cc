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

TEST(LatticeVolumeMeshTests, BasicCubeTests)
{
  int basisOrder = 1;
  FieldInformation lfi("LatVolMesh", basisOrder, "double");
  int sizex,sizey,sizez;
  sizex = sizey = sizez = 2;
  Point minb(0,0,0);
  Point maxb(1,1,1);
  MeshHandle mesh = MeshFactory::Instance().CreateMesh(lfi, MeshConstructionParameters(sizex, sizey, sizez, minb, maxb));
  ASSERT_TRUE(mesh);

  ASSERT_TRUE(mesh->vmesh());
  
  auto latVolVMesh = mesh->vmesh();

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