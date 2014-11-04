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
#include <Core/Algorithms/Legacy/Fields/FieldData/GetFieldData.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldDataToConstantValue.h>
#include <Core/Algorithms/Field/Tests/LoadFieldsForAlgoCoreTests.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Utils/StringUtil.h>

using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;

TEST(SetFieldDataToConstantValueAlgoTest, TODO)
{
  SetFieldDataToConstantValueAlgo algo;

  FieldHandle tetmesh = LoadTet();

  FieldHandle result;
  ASSERT_TRUE(algo.runImpl(tetmesh, result));

  GetFieldDataAlgo algo1;

  DenseMatrixHandle resultmatrix = algo1.run(result);
  FAIL() << "todo: compare matrices, should be a constant value matrix";

  //EXPECT_MATRIX_EQ_TOLERANCE(*resultmatrix, *matrix, 1e-16);
}
