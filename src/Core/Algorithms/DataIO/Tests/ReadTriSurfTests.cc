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

/// @todo DAN
#if 0

#include <cmath>
#include <fstream>
#include <boost/math/constants/constants.hpp>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Algorithms/DataIO/TextToTriSurfField.h>

#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Mesh/FieldFwd.h>
#include <Core/Datatypes/Mesh/VMesh.h>

using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

namespace
{
  boost::filesystem::path testPath = SCIRun::TestUtils::TestResources::rootDir() / "convert-examples";
}

TEST(ReadTriSurfTests, ReadFromFilePoints)
{
  TextToTriSurfFieldAlgorithm algo;
  auto filename = testPath / "simple_triangle_trisurf.pts";
  auto mesh = algo.run(filename.string());
  ASSERT_TRUE(mesh);

  VirtualMeshHandle vmesh = mesh->vmesh();
  ASSERT_TRUE(vmesh);
  ASSERT_TRUE(vmesh->is_trisurfmesh());
  /// @todo: code needs to be enabled
  //ASSERT_TRUE(vmesh->is_unstructuredmesh());
}

TEST(ReadTriSurfTests, ReadFromFileFaces)
{
  TextToTriSurfFieldAlgorithm algo;
  auto filename = testPath / "simple_triangle_trisurf.fac";
  auto mesh = algo.run(filename.string());
  ASSERT_TRUE(mesh);

  VirtualMeshHandle vmesh = mesh->vmesh();
  ASSERT_TRUE(vmesh);
  ASSERT_TRUE(vmesh->is_trisurfmesh());
  /// @todo: code needs to be enabled
  //ASSERT_TRUE(vmesh->is_unstructuredmesh());
}


TEST(ReadTriSurfTests, ReadInvalidPointsFile)
{
  TextToTriSurfFieldAlgorithm algo;
  const std::string filename("not_a_valid_file.pts");
  auto mesh = algo.run(filename);
  ASSERT_FALSE(mesh);
}


TEST(ReadTriSurfTests, ReadInvalidFacesFile)
{
  TextToTriSurfFieldAlgorithm algo;
  const std::string filename("not_a_valid_file.fac");
  auto mesh = algo.run(filename);
  ASSERT_FALSE(mesh);
}

struct MakePointsOnSphere
{
  MakePointsOnSphere(unsigned int seed = 1)
  {
    srand(seed);
  }

  Point operator()() const
  {
    const double pi = boost::math::constants::pi<double>();
    double random1 = (double)rand() / RAND_MAX;
    double phi = pi * random1;
    double random2 = (double)rand() / RAND_MAX;
    double theta = 2 * pi * random2;
    return Point(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
  }
};

TEST(GenerateSphereOfPoints, DISABLED_Run)
{
  int n = 10000;

  std::vector<Point> points;
  std::generate_n(std::back_inserter(points), n, MakePointsOnSphere());

  std::ofstream filePts("C:\\Dev\\scirun_unit_test_data\\SCIRunUnitTestData\\SCIRun5\\UnitTests\\convert-examples\\sphereOfPoints.pts");
  std::for_each(points.begin(), points.end(), [&](const Point& p) { filePts << p.x() << " " << p.y() << " " << p.z() << std::endl; });
  std::ofstream fileFac("C:\\Dev\\scirun_unit_test_data\\SCIRunUnitTestData\\SCIRun5\\UnitTests\\convert-examples\\sphereOfPoints.fac");
  for (int i = 0; i < n; ++i)
  {
    fileFac << i << " " << (i + 1) << " " << (i + 2) << std::endl;
  }
}
#endif
