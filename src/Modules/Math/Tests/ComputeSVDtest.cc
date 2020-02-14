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


//ComputeSVD module test.
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Modules/Legacy/Math/ComputeSVD.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;

class ComputeSVDtest : public ModuleTest
{
};

//Checks for null input.
TEST_F(ComputeSVDtest, CheckInputNull)
{
    auto svdMod = makeModule("ComputeSVD");
    MatrixHandle nullMatrix;
    stubPortNWithThisData(svdMod, 0, nullMatrix);

    EXPECT_THROW(svdMod -> execute(), NullHandleOnPortException);
}

//Checks for dense input.
TEST_F(ComputeSVDtest, CheckInputDense)
{
    auto svdMod = makeModule("ComputeSVD");
    MatrixHandle denseMatrix = MAKE_DENSE_MATRIX_HANDLE((1,2,5,6)(3,4,7,9)(7,8,9,1)(2,3,5,9));
    stubPortNWithThisData(svdMod, 0, denseMatrix);

    EXPECT_NO_THROW(svdMod -> execute());
}

//ComputePCA currently only supports dense input, but we will leave these if we want to change it later.
//There is a check for this in the algorithm.
//Checks for column input.
TEST_F(ComputeSVDtest, CheckInputColumn)
{
    auto svdMod = makeModule("ComputeSVD");
    DenseColumnMatrixHandle columnInput(new DenseColumnMatrix(5));
    (*columnInput) << 1,2,3,4,5;
    stubPortNWithThisData(svdMod, 0, columnInput);

    EXPECT_NO_THROW(svdMod -> execute());
}

//Checks for sparse input.
TEST_F(ComputeSVDtest, CheckInputSparse)
{
    auto svdMod = makeModule("ComputeSVD");
    MatrixHandle sparseMatrix = MAKE_SPARSE_MATRIX_HANDLE((0,0,0,0)(5,8,0,0)(0,0,3,0)(0,6,0,0));
    stubPortNWithThisData(svdMod, 0, sparseMatrix);

    EXPECT_NO_THROW(svdMod -> execute());
}
