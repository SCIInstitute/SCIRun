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
 
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/FiniteElements/BuildMatrix/BuildFEMatrix.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;
using ::testing::NotNull;

namespace FEInputData
{
  boost::filesystem::path feMeshRoot() 
  {
    return TestResources::rootDir() / "buildFE" / "inputFields";
  }

  FieldHandle loadTestMesh(const std::string& file)
  {
    return loadFieldFromFile(feMeshRoot() / file);
  }
  
  FieldHandle meshSize1e3()
  {
    return loadFieldFromFile(feMeshRoot() / "fem_1e3_elements.fld");
  }

  FieldHandle meshSize1e4()
  {
    return loadFieldFromFile(feMeshRoot() / "fem_1e4_elements.fld");
  }

  FieldHandle meshSize1e5()
  {
    return loadFieldFromFile(feMeshRoot() / "fem_1e5_elements.fld");
  }

  FieldHandle meshSize1e6()
  {
    return loadFieldFromFile(feMeshRoot() / "fem_1e6_elements.fld");
  }
  
  SparseRowMatrixHandle expectedOutput()
  {
    //TODO: MORITZ insert values as above.
    return MAKE_SPARSE_MATRIX_HANDLE(
      (1,0)
      (0,1));
  }

  DenseMatrixHandle nullConductivityMatrix()
  {
    return DenseMatrixHandle();
  }
}

TEST(BuildFEMatrixAlgorithmTests, ThrowsForNullMesh)
{
  using namespace FEInputData;
  BuildFEMatrixAlgo algo;
  SparseRowMatrixHandle output;

  EXPECT_FALSE(algo.run(FieldHandle(), nullConductivityMatrix(), output));
  //TODO: consider throwing an exception instead of returning false
}

TEST(BuildFEMatrixAlgorithmTests, TestMeshSize1e1)
{
  using namespace FEInputData;
  auto mesh = loadTestMesh("fem_1e1_elements.fld");
  ASSERT_THAT(mesh, NotNull());

  BuildFEMatrixAlgo algo;
  SparseRowMatrixHandle output;
  ASSERT_TRUE(algo.run(mesh, nullConductivityMatrix(), output));

  ASSERT_THAT(output, NotNull());

  FAIL() << "todo";
}