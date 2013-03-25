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

#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Algorithms/DataIO/TextToTriSurfField.h>

#include <Core/Datatypes/Mesh/FieldFwd.h>
#include <Core/Datatypes/Mesh/VMesh.h>

using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::Core::Datatypes;

namespace
{
  boost::filesystem::path testPath = SCIRun::TestUtils::TestResources::rootDir() / "convert-examples";
}

TEST(ReadTriSurfTests, ReadFromFilePoints)
{
  TextToTriSurfFieldAlgorithm algo;
  auto filename = testPath / "simple_triangle_trisurf.pts";
  MeshHandle mesh = algo.run(filename.string());
  ASSERT_TRUE(mesh);
  
  VirtualMeshHandle vmesh = mesh->vmesh();
  ASSERT_TRUE(vmesh);
  ASSERT_TRUE(vmesh->is_trisurfmesh());
  // TODO: code needs to be enabled
  //ASSERT_TRUE(vmesh->is_unstructuredmesh());
}

TEST(ReadTriSurfTests, ReadFromFileFaces)
{
  TextToTriSurfFieldAlgorithm algo;
  auto filename = testPath / "simple_triangle_trisurf.fac";
  MeshHandle mesh = algo.run(filename.string());
  ASSERT_TRUE(mesh);
  
  VirtualMeshHandle vmesh = mesh->vmesh();
  ASSERT_TRUE(vmesh);
  ASSERT_TRUE(vmesh->is_trisurfmesh());
  // TODO: code needs to be enabled
  //ASSERT_TRUE(vmesh->is_unstructuredmesh());
}


TEST(ReadTriSurfTests, ReadInvalidPointsFile)
{
  TextToTriSurfFieldAlgorithm algo;
  const std::string filename("not_a_valid_file.pts");
  MeshHandle mesh = algo.run(filename);
  ASSERT_FALSE(mesh);
}


TEST(ReadTriSurfTests, ReadInvalidFacesFile)
{
  TextToTriSurfFieldAlgorithm algo;
  const std::string filename("not_a_valid_file.fac");
  MeshHandle mesh = algo.run(filename);
  ASSERT_FALSE(mesh);
}