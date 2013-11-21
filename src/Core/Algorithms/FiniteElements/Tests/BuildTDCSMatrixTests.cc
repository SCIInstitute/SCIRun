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

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/FiniteElements/BuildMatrix/BuildTDCSMatrix.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;

namespace TDCSInputData
{
  SparseRowMatrixHandle stiff()
  {
    //TODO: MORITZ fill in values up to 5x5
    SparseRowMatrixHandle st = MAKE_SPARSE_MATRIX(
      (1,0,0)
      (0,0,0)
      (0,2,0));
    return st;
  }
  
  FieldHandle mesh()
  {
    //TODO: DAN--need to read in file
    return FieldHandle();
  }
  
  DenseMatrixHandle ElectrodeElements()
  {
    //TODO: MORITZ insert values as above.
    return DenseMatrixHandle();
  }
  
  DenseMatrixHandle ElectrodeElementType()
  {
    //TODO: MORITZ insert values as above.
    return DenseMatrixHandle();
  }
  
  DenseMatrixHandle ElectrodeElementDefinition()
  {
    //TODO: MORITZ insert values as above.
    return DenseMatrixHandle();
  }
  
  DenseMatrixHandle contactimpedance()
  {
    //TODO: MORITZ insert values as above.
    return DenseMatrixHandle();
  }
  
  SparseRowMatrixHandle expectedOutput()
  {
    //TODO: MORITZ insert values as above.
    return SparseRowMatrixHandle();
  }
}

TEST(VerifyTDCSInput, IsNotNull)
{
  using namespace TDCSInputData;
  EXPECT_TRUE(stiff() != nullptr);
  EXPECT_TRUE(mesh() != nullptr);
  EXPECT_TRUE(ElectrodeElements() != nullptr);
  EXPECT_TRUE(ElectrodeElementType() != nullptr);
  EXPECT_TRUE(ElectrodeElementDefinition() != nullptr);
  EXPECT_TRUE(contactimpedance() != nullptr);
  EXPECT_TRUE(expectedOutput() != nullptr);
}

TEST(BuildTDCSMatrixAlgorithmTests, ThrowsForNullMesh)
{
  using namespace TDCSInputData;
  BuildTDCSMatrixAlgo algo;
  SparseRowMatrixHandle output;

  //check on BuildTDCSMatrix.cc line 672
  ASSERT_FALSE(algo.run(stiff(), FieldHandle(), ElectrodeElements(), ElectrodeElementType(), ElectrodeElementDefinition(), contactimpedance(), output));
  //TODO: consider throwing an exception instead of returning false
}

TEST(BuildTDCSMatrixAlgorithmTests, ThrowsForNullElectrodeElements)
{
  using namespace TDCSInputData;
  BuildTDCSMatrixAlgo algo;
  SparseRowMatrixHandle output;

  //check on BuildTDCSMatrix.cc line 679
  ASSERT_FALSE(algo.run(stiff(), mesh(), DenseMatrixHandle(), ElectrodeElementType(), ElectrodeElementDefinition(), contactimpedance(), output));
  //TODO: consider throwing an exception instead of returning false
}

//TODO:
//TEST(BuildTDCSMatrixAlgorithmTests, ThrowsForNullElectrodeElementType)  // line 686
//TEST(BuildTDCSMatrixAlgorithmTests, ThrowsForNullElectrodeElementDefinition)  // line 693

//TEST(BuildTDCSMatrixAlgorithmTests, ThrowsForMismatchedElectrodeMatrixRowCount)  // line 563
// etc for checks lines 567-575
// cover as many error cases with tests as you can, there are a lot in TDCSMatrixBuilder::singlethread() but there may be reasonable conceptual groupings to simplify the test code (may also lead to simplifying/refactoring the algorithm code