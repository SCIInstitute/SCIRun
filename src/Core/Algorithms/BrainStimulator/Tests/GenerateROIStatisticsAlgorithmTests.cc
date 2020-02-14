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


#include <gtest/gtest.h>
#include <Core/Math/MiscMath.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/BrainStimulator/GenerateROIStatisticsAlgorithm.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Datatypes/String.h>
//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Datatypes;

namespace
{
  FieldHandle CreateGetMeshSolution()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/test_atlas/mesh_sol.fld");
  }

  FieldHandle CreateCoordinatesCubesShiftedBy10()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/test_atlas/coordinates_cubes_shifted_by_10.fld");
  }

  FieldHandle CreateCoordinatesCubes()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/test_atlas/coordinates_cubes.fld");
  }

  FieldHandle CreateAtlas()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/test_atlas/atlas.fld");
  }

  DenseMatrixHandle expected_result_no_roi_spec()
  {
    DenseMatrixHandle expected_result(boost::make_shared<DenseMatrix>(8, 5));

   (*expected_result)(0, 0) = 0.5398800608953688;  (*expected_result)(1, 0) = 0.7546866819823609; (*expected_result)(2, 0) = 0.3784870349158202;  (*expected_result)(3, 0) = 0.5215146855270543;  (*expected_result)(4, 0) = 0.6204598403245096; (*expected_result)(5, 0) = 0.4103867315361441; (*expected_result)(6, 0) = 0.6164145705872455; (*expected_result)(7, 0) = 0.3773992798882833;
   (*expected_result)(0, 1) = 0.2023192248404992;  (*expected_result)(1, 1) = std::numeric_limits<double>::quiet_NaN();  (*expected_result)(2, 1) = 0.270036948169627;  (*expected_result)(3, 1) = 0.2819928747450936;  (*expected_result)(4, 1) = 0.2464919130115238;  (*expected_result)(5, 1) = 0.348358459345021;  (*expected_result)(6, 1) = 0.3382031144990396;  (*expected_result)(7, 1) = 0.1699586184078796;
   (*expected_result)(0, 2) = 0.1868726045543786;  (*expected_result)(1, 2) = 0.7546866819823609; (*expected_result)(2, 2) = 0.1189976815583766; (*expected_result)(3, 2) = 0.223811939491137;  (*expected_result)(4, 2) = 0.2550951154592691;     (*expected_result)(5, 2) = 0.1386244428286791;  (*expected_result)(6, 2) = 0.2435249687249893;  (*expected_result)(7, 2) = 0.1965952504312082;
   (*expected_result)(0, 3) = 0.7951999011370632;  (*expected_result)(1, 3) = 0.7546866819823609;  (*expected_result)(2, 3) = 0.6797026768536748;  (*expected_result)(3, 3) = 0.9597439585160811;  (*expected_result)(4, 3) = 0.8909032525357985;  (*expected_result)(5, 3) = 0.9592914252054443;  (*expected_result)(6, 3) = 0.9292636231872278;  (*expected_result)(7, 3) = 0.6160446761466392;
   (*expected_result)(0, 4) = 9;  (*expected_result)(1, 4) = 1;  (*expected_result)(2, 4) = 5;    (*expected_result)(3, 4) = 5;  (*expected_result)(4, 4) = 5;  (*expected_result)(5, 4) = 5;  (*expected_result)(6, 4) = 5;    (*expected_result)(7, 4) = 5;

    return expected_result;
  }

  DenseMatrixHandle expected_result_roi_spec()
  {
   DenseMatrixHandle expected_result(boost::make_shared<DenseMatrix>(1, 5));
   (*expected_result)(0, 0) = 0.5046221600763444;
   (*expected_result)(0, 1) = 0.2271478773489659;
   (*expected_result)(0, 2) = 0.1626117351946306;
   (*expected_result)(0, 3) = 0.7951999011370632;
   (*expected_result)(0, 4) = 14;

   return expected_result;
  }
}

TEST(GenerateROIStatisticsAlgorithm, ValidInput_TestAllROI)
{
  GenerateROIStatisticsAlgorithm algo;
  FieldHandle atlas = CreateAtlas();
  FieldHandle createcoordcubes = CreateCoordinatesCubes();
  FieldHandle createmeshsolution = CreateGetMeshSolution();
  const std::string atlasMeshLabelsStr("Region1;Region2;Region3;Region4;Region5;Region6;Region7;Region8");
  DenseMatrixHandle specROI(new DenseMatrix(5, 1, 0));
  auto output = algo.run(createmeshsolution,atlas,createcoordcubes,atlasMeshLabelsStr, specROI);
  DenseMatrixHandle outputMatrix = output.get<0>();
  DenseMatrixHandle expected_result = expected_result_no_roi_spec();
  EXPECT_EQ(outputMatrix->rows(),expected_result->rows());
  EXPECT_EQ(outputMatrix->cols(),expected_result->cols());
  for (int i = 0; i < outputMatrix->rows(); i++)
    for (int j = 0; j < outputMatrix->cols(); j++)
      if (i == 1 && j == 1)
      {
        if (!IsNan((*outputMatrix)(i, j)))
        {
          FAIL() << "ERROR: Test on NaN failed!" << std::endl;
        }
      }
      else
      {
        EXPECT_NEAR((*outputMatrix)(i, j), (*expected_result)(i, j), 1e-10);
      }
}

TEST(GenerateROIStatisticsAlgorithm, ValidInput_TestSpecROI)
{
  GenerateROIStatisticsAlgorithm algo;
  FieldHandle atlas = CreateAtlas();
  FieldHandle createcoordcubes = CreateCoordinatesCubes();
  FieldHandle createmeshsolution = CreateGetMeshSolution();
  const std::string& atlasMeshLabelsStr = std::string("Region1;Region2;Region3;Region4;Region5;Region6;Region7;Region8");
  DenseMatrixHandle specROI(new DenseMatrix(5, 1));
  (*specROI)(0, 0) = 0.5;
  (*specROI)(1, 0) = 0.5;
  (*specROI)(2, 0) = 0.5;
  (*specROI)(3, 0) = 0;
  (*specROI)(4, 0) = 1;
  auto output = algo.run(createmeshsolution,atlas,createcoordcubes,atlasMeshLabelsStr, specROI);
  DenseMatrixHandle outputMatrix = output.get<0>();

  DenseMatrixHandle expected_result = expected_result_roi_spec();
  EXPECT_EQ(outputMatrix->rows(),expected_result->rows());
  EXPECT_EQ(outputMatrix->cols(),expected_result->cols());

  for (int i = 0; i < outputMatrix->rows(); i++)
   for (int j = 0; j < outputMatrix->cols(); j++)
          EXPECT_NEAR((*outputMatrix)(i, j), (*expected_result)(i,j), 1e-10);

}

TEST(GenerateROIStatisticsAlgorithm, ValidInput_TestCoordinateSpace)
{
  GenerateROIStatisticsAlgorithm algo;
  FieldHandle atlas = CreateAtlas();
  FieldHandle createcoordcubesshiftby10 = CreateCoordinatesCubesShiftedBy10();
  FieldHandle createmeshsolution = CreateGetMeshSolution();
  std::string atlasMeshLabelsStr("Region1;Region2;Region3;Region4;Region5;Region6;Region7;Region8");
  DenseMatrixHandle specROI(new DenseMatrix(5, 1));
  (*specROI)(0, 0) = 10.5;
  (*specROI)(1, 0) = 10.5;
  (*specROI)(2, 0) = 10.5;
  (*specROI)(3, 0) = 0;
  (*specROI)(4, 0) = 1.0;
  auto output = algo.run(createmeshsolution,atlas,createcoordcubesshiftby10,atlasMeshLabelsStr, specROI);
  DenseMatrixHandle outputMatrix = output.get<0>();

  DenseMatrixHandle expected_result = expected_result_roi_spec();
  EXPECT_EQ(outputMatrix->rows(),expected_result->rows());
  EXPECT_EQ(outputMatrix->cols(),expected_result->cols());

  for (int i = 0; i < outputMatrix->rows(); i++)
   for (int j = 0; j < outputMatrix->cols(); j++)
          EXPECT_NEAR((*outputMatrix)(i, j), (*expected_result)(i,j), 1e-10);

}
