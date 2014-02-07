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

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/Fields/DomainFields/GetDomainBoundaryAlgo.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;
using ::testing::NotNull;


class GetDomainBoundaryTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    SCIRun::Core::Logging::Log::get().setVerbose(true);
  }

  void runTest(bool includeOuterBoundary, bool useRange, int domainValue,
    int expectedBoundaryNodes, int expectedBoundaryElements)
  {
    FieldHandle latVol = loadFieldFromFile(TestResources::rootDir() / "latVolWithNormData.fld");
    ASSERT_TRUE(latVol->vmesh()->is_latvolmesh());

    GetDomainBoundaryAlgo algo;

    // How to set parameters on an algorithm (that come from the GUI)
    
    algo.set(GetDomainBoundaryAlgo::AddOuterBoundary, includeOuterBoundary);
    
    // TODO: this logic matches the wacky module behavior
    algo.set(GetDomainBoundaryAlgo::UseRange, useRange);
    if (!useRange)
    {
      algo.set(GetDomainBoundaryAlgo::Domain, domainValue);
      algo.set(GetDomainBoundaryAlgo::MinRange, domainValue);
      algo.set(GetDomainBoundaryAlgo::MaxRange, domainValue);
      algo.set(GetDomainBoundaryAlgo::UseRange, true);
    }
    

    FieldHandle boundary;
    SparseRowMatrixHandle unused;
    algo.runImpl(latVol, unused, boundary);

    ASSERT_THAT(boundary, NotNull());

    EXPECT_EQ(expectedBoundaryNodes, boundary->vmesh()->num_nodes());
    EXPECT_EQ(expectedBoundaryElements, boundary->vmesh()->num_elems());
    EXPECT_TRUE(boundary->vmesh()->is_quadsurfmesh());
  }
};

// manual, undesirable way to test many different value combinations.
// includeOuterBoundary = Bool()
// useRange = Bool()
// domainValue = Values(0,1,2,3,4)

// expected values of -1 need to be figured out from the v4 GUI.

TEST_F(GetDomainBoundaryTests, LatVolBoundary_False_True_1)
{
  runTest(false, true, 1, -1, -1);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_False_False_1)
{
  runTest(false, false, 1, 8680, 8334);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_True_False_1)
{
  runTest(true, false, 1, -1, -1);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_True_True_1)
{
  runTest(true, true, 1, -1, -1);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_False_False_4)
{
  runTest(false, false, 4, -1, -1);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_True_False_4)
{
  runTest(true, false, 4, 0, 0);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_False_True_4)
{
  runTest(false, true, 4, -1, -1);
}

TEST_F(GetDomainBoundaryTests, LatVolBoundary_True_True_4)
{
  runTest(true, true, 4, -1, -1);
}

TEST_F(GetDomainBoundaryTests, CanLogErrorMessage)
{
  GetDomainBoundaryAlgo algo;

  FieldHandle input, output;
  SparseRowMatrixHandle elemLink;

  EXPECT_FALSE(algo.runImpl(input, elemLink, output));
}