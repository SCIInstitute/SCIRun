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


#include <Modules/Legacy/Math/AddKnownsToLinearSystem.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/SCIRunUnitTests.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::Modules;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::TestUtils;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class AddKnownsToLinearSystemTests : public ModuleTest
{

};

namespace
{
  FieldHandle CreateTriSurfScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.fld");
  }
  FieldHandle CreateTetMeshScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }
  FieldHandle CreatePointCloudeScalar()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/point_cloud/scalar/pts_scalar.fld");
  }

	// x vector [1; 2; ... n]
	DenseMatrixHandle x_num(int rows)
	{
		DenseMatrixHandle m (boost::make_shared<DenseMatrix>(rows,1));
		for (int i = 0; i < m->rows(); ++ i)
				(*m)(i, 0) = i+1;
		return m;
	}
	// x vector [NaN; NaN; NaN;]
	DenseMatrixHandle x_all_nan ()
	{
		DenseMatrixHandle m (boost::make_shared<DenseMatrix>(3,1));
		(*m)(0, 0) = std::numeric_limits<double>::quiet_NaN();
		(*m)(1, 0) = std::numeric_limits<double>::quiet_NaN();
		(*m)(2, 0) = std::numeric_limits<double>::quiet_NaN();
		return m;
	}

	// symmetric LHS (stiff) matrix
	SparseRowMatrixHandle lhs()
	{
		SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
		m->insert(0,0) = 1;
		m->insert(0,1) = 7;
		m->insert(0,2) = 3;
		m->insert(1,0) = 7;
		m->insert(1,1) = 4;
		m->insert(1,2) = -5;
		m->insert(2,0) = 3;
		m->insert(2,1) = -5;
		m->insert(2,2) = 6;
		m->makeCompressed();
		return m;
	}

	// RHS vector of zeros
	DenseMatrixHandle rhs_zero(int rows)
	{
		DenseMatrixHandle m(boost::make_shared<DenseMatrix>(rows,1));
		m->setZero();
		return m;
	}
	// RHS vector [1; 2; ... n]
	DenseMatrixHandle rhs_num(int rows)
	{
		DenseMatrixHandle m(boost::make_shared<DenseMatrix>(rows,1));
    for (int i=0; i<rows; i++)
      (*m)(i,0) = 1+i;
		return m;
	}
}

TEST_F(AddKnownsToLinearSystemTests, ValidInputOnAllPorts_KnownsHaveNoNaN)
{
  auto test = makeModule("AddKnownsToLinearSystem");
  SparseRowMatrixHandle   stiff = lhs();
  DenseMatrixHandle bvect = rhs_num(3);
  DenseMatrixHandle       known = x_num(3);
  stubPortNWithThisData(test, 0, stiff);
  stubPortNWithThisData(test, 1, bvect);
  stubPortNWithThisData(test, 2, known);
  EXPECT_NO_THROW(test->execute());
}

TEST_F(AddKnownsToLinearSystemTests, ValidInputOnAllPorts_KnownsHaveAllNaN)
{
  auto test = makeModule("AddKnownsToLinearSystem");
  SparseRowMatrixHandle   stiff = lhs();
  DenseMatrixHandle bvect = rhs_num(3);
  DenseMatrixHandle       known = x_all_nan();
  stubPortNWithThisData(test, 0, stiff);
  stubPortNWithThisData(test, 1, bvect);
  stubPortNWithThisData(test, 2, known);
  EXPECT_NO_THROW(test->execute());
}

TEST_F(AddKnownsToLinearSystemTests, ValidInputOnPortZeroAndTwoNothingOnPortOne)
{
  auto test = makeModule("AddKnownsToLinearSystem");
  SparseRowMatrixHandle   stiff = lhs();
  DenseMatrixHandle       known = x_num(3);
  stubPortNWithThisData(test, 0, stiff);
  stubPortNWithThisData(test, 2, known);
  EXPECT_NO_THROW(test->execute());
}

TEST_F(AddKnownsToLinearSystemTests, ThrowsForFieldInputOnPortZero)
{
  auto test = makeModule("AddKnownsToLinearSystem");
  FieldHandle             stiff = CreatePointCloudeScalar();
  DenseMatrixHandle bvect = rhs_zero(3);
  DenseMatrixHandle       known = x_num(3);

  stubPortNWithThisData(test, 0, stiff);
  stubPortNWithThisData(test, 1, bvect);
  stubPortNWithThisData(test, 2, known);

  EXPECT_THROW(test->execute(), DataPortException);
}

TEST_F(AddKnownsToLinearSystemTests, ThrowsForFieldInputOnPortOne)
{
  auto test = makeModule("AddKnownsToLinearSystem");
  SparseRowMatrixHandle   stiff = lhs();
  FieldHandle             bvect = CreateTetMeshScalarOnNode();
  DenseMatrixHandle       known = x_num(3);

  stubPortNWithThisData(test, 0, stiff);
  stubPortNWithThisData(test, 1, bvect);
  stubPortNWithThisData(test, 2, known);

  EXPECT_THROW(test->execute(), DataPortException);
}

TEST_F(AddKnownsToLinearSystemTests, ThrowsForFieldInputOnPortTwo)
{
  auto test = makeModule("AddKnownsToLinearSystem");
  SparseRowMatrixHandle   stiff = lhs();
  DenseMatrixHandle bvect = rhs_zero(3);
  FieldHandle             known = CreateTriSurfScalarOnNode();

  stubPortNWithThisData(test, 0, stiff);
  stubPortNWithThisData(test, 1, bvect);
  stubPortNWithThisData(test, 2, known);

  EXPECT_THROW(test->execute(), DataPortException);
}

TEST_F(AddKnownsToLinearSystemTests, ThrowsForNullInput)
{
  auto csdf = makeModule("AddKnownsToLinearSystem");
  FieldHandle nullField;
  stubPortNWithThisData(csdf, 0, nullField);
  stubPortNWithThisData(csdf, 1, nullField);
  stubPortNWithThisData(csdf, 2, nullField);
  stubPortNWithThisData(csdf, 3, nullField);

  EXPECT_THROW(csdf->execute(), NullHandleOnPortException);
}
