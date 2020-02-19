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

   Author:              Moritz Dannhauer
   Last Modification:   September 17 2014
*/


#include <gtest/gtest.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldData.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/GetFieldData.h>
#include <Core/Algorithms/BrainStimulator/SimulateForwardMagneticFieldAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Utils/StringUtil.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Algorithms/DataIO/ReadMatrix.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Algorithms::DataIO;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms::BrainStimulator;

FieldHandle LoadFieldFirstModuleInput()
{
  return loadFieldFromFile(TestResources::rootDir() / "Fields/simulateforwardmagneticfield/first.fld");
}

FieldHandle LoadFieldSecondModuleInput()
{
  return loadFieldFromFile(TestResources::rootDir() / "Fields/simulateforwardmagneticfield/second.fld");
}

FieldHandle LoadFieldThirdModuleInput()
{
  return loadFieldFromFile(TestResources::rootDir() / "Fields/simulateforwardmagneticfield/third.fld");
}

FieldHandle LoadFieldFourthModuleInput()
{
  return loadFieldFromFile(TestResources::rootDir() / "Fields/simulateforwardmagneticfield/fourth.fld");
}

FieldHandle LoadFieldMagneticFieldResult()
{
  return loadFieldFromFile(TestResources::rootDir() / "Fields/simulateforwardmagneticfield/result_magnetic_field.fld");
}

FieldHandle LoadFieldMagneticFieldMagnitudesResult()
{
  return loadFieldFromFile(TestResources::rootDir() / "Fields/simulateforwardmagneticfield/result_magnetic_field_magnitudes.fld");
}


TEST(SimulateForwardMagneticFieldAlgoTest, TestOnLatVol)
{
  FieldHandle first = LoadFieldFirstModuleInput();
  FieldHandle second = LoadFieldSecondModuleInput();
  FieldHandle third = LoadFieldThirdModuleInput();
  FieldHandle fourth = LoadFieldFourthModuleInput();
  long nr_nodes = (long)second->vmesh()->num_nodes();
  DenseMatrixHandle tensor_matrix(boost::make_shared<DenseMatrix>(nr_nodes,9));

  for (long i=0;i<nr_nodes;i++)
  {
   for (int j=0;j<7;j++)
   {
    if (j==0 || j==4 || j==8)
      (*tensor_matrix)(i,j)=1;
    else
      (*tensor_matrix)(i,j)=0;
   }
  }

  SetFieldDataAlgo algo;

  FieldHandle second_with_tensor = algo.runImpl(second, tensor_matrix);

  SimulateForwardMagneticFieldAlgo algo2;

  FieldHandle MField, MFieldMagnitudes;
  boost::tie(MField,MFieldMagnitudes) = algo2.run(first,second_with_tensor,third,fourth);

  GetFieldDataAlgo algo3;

  DenseMatrixHandle MField_matrix = algo3.runMatrix(MField);
  DenseMatrixHandle MFieldMagnitudes_matrix = algo3.runMatrix(MFieldMagnitudes);

  FieldHandle MField_expected = LoadFieldMagneticFieldResult();
  FieldHandle MFieldMagnitudes_expected = LoadFieldMagneticFieldMagnitudesResult();

  DenseMatrixHandle MField_expected_matrix = algo3.runMatrix(MField_expected);
  DenseMatrixHandle MFieldMagnitudes_expected_matrix = algo3.runMatrix(MFieldMagnitudes_expected);

  EXPECT_MATRIX_EQ_TOLERANCE(*MField_matrix, *MField_expected_matrix, 1e-16);
  EXPECT_MATRIX_EQ_TOLERANCE(*MFieldMagnitudes_matrix, *MFieldMagnitudes_expected_matrix, 1e-16);
}
