/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

/// @todo DAN
#if 0
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Core/Datatypes/Mesh/MeshFactory.h>
//#include <Core/Datatypes/Mesh/FieldInformation.h>
//#include <Core/Datatypes/Mesh/VMesh.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

TEST(MeshFactoryTests, CanCreateLatticeVolumeMesh)
{
  FieldInformation lfi("LatVolMesh", LINEARDATA_E, "double");
  int sizex,sizey,sizez;
  sizex = sizey = sizez = 4;
  Point minb(0,0,0);
  Point maxb(4,4,4);
  auto mesh = MeshFactory::Instance().CreateMesh(lfi, MeshConstructionParameters(sizex, sizey, sizez, minb, maxb));
  ASSERT_TRUE(mesh);
}

TEST(MeshFactoryTests, CreateTriSurfMeshWithString)
{
  FieldInformation lfi("TriSurfMesh", LINEARDATA_E, "double");
  auto mesh = MeshFactory::Instance().CreateMesh(lfi.get_mesh_type_id());
  ASSERT_TRUE(mesh);

  auto vmeshHandle = mesh->vmesh();
  ASSERT_TRUE(vmeshHandle);
}


TEST(MeshFactoryTests, CreateTriSurfMeshStringWithFieldInforomation)
{
  FieldInformation lfi("TriSurfMesh", LINEARDATA_E, "double");
  auto mesh = MeshFactory::Instance().CreateMesh(lfi);
  ASSERT_TRUE(mesh);

  auto vmeshHandle = mesh->vmesh();
  ASSERT_TRUE(vmeshHandle);
}
#endif
