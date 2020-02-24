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


#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Legacy/FiniteElements/BuildMatrix/BuildFEMatrix.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;
using ::testing::NotNull;

namespace FEInputData
{
  boost::filesystem::path feMeshRoot()
  {
    return TestResources::rootDir() / "Fields" / "buildFE" / "inputFields";
  }

  FieldHandle loadTestMesh(const std::string& file)
  {
    return loadFieldFromFile(feMeshRoot() / file);
  }

  boost::filesystem::path expectedOutputRoot()
  {
    return TestResources::rootDir() / "Matrices" / "buildFE" / "v4Output";
  }

  SparseRowMatrixHandle expectedOutput(const std::string& file)
  {
    ReadMatrixAlgorithm reader;
    return castMatrix::toSparse(reader.run((expectedOutputRoot() / file).string()));
  }

  DenseMatrixHandle nullConductivityMatrix()
  {
    return nullptr;
  }
}

TEST(BuildFEMatrixAlgorithmTests, ThrowsForNullMesh)
{
  using namespace FEInputData;
  BuildFEMatrixAlgo algo;

  EXPECT_THROW(algo.run(withInputData((Variables::InputField, nullptr))), AlgorithmProcessingException);
}

TEST(BuildFEMatrixAlgorithmTests, TestMeshSize1e1)
{
  using namespace FEInputData;
  auto mesh = loadTestMesh("fem_1e1_elements.fld");
  ASSERT_THAT(mesh, NotNull());

  EXPECT_EQ(3, mesh->vmesh()->num_elems());

  BuildFEMatrixAlgo algo;
  SparseRowMatrixHandle output;
  ASSERT_NO_THROW(
    auto out = algo.run(withInputData((Variables::InputField, mesh)));
    output = out.get<SparseRowMatrix>(BuildFEMatrixAlgo::Stiffness_Matrix);
  );

  ASSERT_THAT(output, NotNull());

  EXPECT_EQ(7, output->nrows());
  EXPECT_EQ(7, output->ncols());

  EXPECT_DOUBLE_EQ(0.5855738571534416, output->coeff(0,0));
  EXPECT_DOUBLE_EQ(0.3117198873128934, output->coeff(output->nrows() - 1, output->ncols() - 1));

  EXPECT_TRUE(expectedOutput("1e1.mat")->isApprox(*output));
}

TEST(BuildFEMatrixAlgorithmTests, TestMeshSize1e3)
{
  using namespace FEInputData;
  auto mesh = loadTestMesh("fem_1e3_elements.fld");
  ASSERT_THAT(mesh, NotNull());

  EXPECT_EQ(1000, mesh->vmesh()->num_elems());

  BuildFEMatrixAlgo algo;
  SparseRowMatrixHandle output;
  ASSERT_NO_THROW(
    auto out = algo.run(withInputData((Variables::InputField, mesh)));
    output = out.get<SparseRowMatrix>(BuildFEMatrixAlgo::Stiffness_Matrix);
  );

  ASSERT_THAT(output, NotNull());

  EXPECT_EQ(1040, output->nrows());
  EXPECT_EQ(1040, output->ncols());

  EXPECT_DOUBLE_EQ(2, output->coeff(0,0));
  EXPECT_DOUBLE_EQ(1.0/3, output->coeff(output->nrows() - 1, output->ncols() - 1));

  EXPECT_TRUE(expectedOutput("1e3.mat")->isApprox(*output));
}

TEST(BuildFEMatrixAlgorithmTests, TestMeshSize1e4)
{
  using namespace FEInputData;
  auto mesh = loadTestMesh("fem_1e4_elements.fld");
  ASSERT_THAT(mesh, NotNull());

  EXPECT_EQ(10000, mesh->vmesh()->num_elems());

  BuildFEMatrixAlgo algo;
  SparseRowMatrixHandle output;
  ASSERT_NO_THROW(
    auto out = algo.run(withInputData((Variables::InputField, mesh)));
    output = out.get<SparseRowMatrix>(BuildFEMatrixAlgo::Stiffness_Matrix);
  );

  ASSERT_THAT(output, NotNull());

  EXPECT_EQ(10149, output->nrows());
  EXPECT_EQ(10149, output->ncols());

  EXPECT_DOUBLE_EQ(0.2, output->coeff(0,0));
  EXPECT_DOUBLE_EQ(0.016666666666666666, output->coeff(output->nrows() - 1, output->ncols() - 1));

  EXPECT_TRUE(expectedOutput("1e4.mat")->isApprox(*output));
}

// move to nightly: file too big for github unit test repo
TEST(BuildFEMatrixAlgorithmTests, DISABLED_TestMeshSize1e5)
{
  using namespace FEInputData;
  auto mesh = loadTestMesh("fem_1e5_elements.fld");
  ASSERT_THAT(mesh, NotNull());

  EXPECT_EQ(100000, mesh->vmesh()->num_elems());

  BuildFEMatrixAlgo algo;
  SparseRowMatrixHandle output;
  ASSERT_NO_THROW(
    auto out = algo.run(withInputData((Variables::InputField, mesh)));
    output = out.get<SparseRowMatrix>(BuildFEMatrixAlgo::Stiffness_Matrix);
  );

  ASSERT_THAT(output, NotNull());

  EXPECT_EQ(94538, output->nrows());
  EXPECT_EQ(94538, output->ncols());

  EXPECT_DOUBLE_EQ(0.2, output->coeff(0,0));
  EXPECT_DOUBLE_EQ(0.016666666666666666, output->coeff(output->nrows() - 1, output->ncols() - 1));

  EXPECT_TRUE(compare_with_tolerance(*expectedOutput("1e5.mat"), *output));
}

// move to nightly: file too big for github unit test repo
TEST(BuildFEMatrixAlgorithmTests, DISABLED_TestMeshSize1e6)
{
  using namespace FEInputData;
  auto mesh = loadTestMesh("fem_1e6_elements.fld");
  ASSERT_THAT(mesh, NotNull());

  EXPECT_EQ(1000000, mesh->vmesh()->num_elems());

  BuildFEMatrixAlgo algo;
  SparseRowMatrixHandle output;
  ASSERT_NO_THROW(
    auto out = algo.run(withInputData((Variables::InputField, mesh)));
    output = out.get<SparseRowMatrix>(BuildFEMatrixAlgo::Stiffness_Matrix);
  );

  ASSERT_THAT(output, NotNull());

  EXPECT_EQ(812604, output->nrows());
  EXPECT_EQ(812604, output->ncols());

  EXPECT_DOUBLE_EQ(0.2, output->coeff(0,0));
  EXPECT_DOUBLE_EQ(0.066666666666666666, output->coeff(output->nrows() - 1, output->ncols() - 1));

  EXPECT_TRUE(compare_with_tolerance(*expectedOutput("1e6.mat"), *output));
}
