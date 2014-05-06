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

#include <Modules/Legacy/Fields/MapFieldDataFromElemToNode.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class MapFieldDataFromElemToNodeModuleTests : public ModuleTest
{

};

TEST_F(MapFieldDataFromElemToNodeModuleTests, ThrowsForNullInput)
{
  auto test = makeModule("MapFieldDataFromElemToNode");
  FieldHandle nullField;
  stubPortNWithThisData(test, 0, nullField);
  stubPortNWithThisData(test, 1, nullField);
	stubPortNWithThisData(test, 2, nullField);
  stubPortNWithThisData(test, 3, nullField);

  EXPECT_THROW(test->execute(), NullHandleOnPortException);
}


TEST_F(MapFieldDataFromElemToNodeModuleTests, ThrowForDenseMatrixInPort)
{
  auto test = makeModule("MapFieldDataFromElemToNode");
	DenseMatrixHandle m (boost::make_shared<DenseMatrix>(3,1));
	for (int i=0; i<3; i++)
		(*m)(i, 0) = 1;
  stubPortNWithThisData(test, 0, m);
	EXPECT_THROW(test->execute(), WrongDatatypeOnPortException);
}

TEST_F(MapFieldDataFromElemToNodeModuleTests, ThrowForSparseRowMatrixInPort)
{
  auto test = makeModule("MapFieldDataFromElemToNode");
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
  stubPortNWithThisData(test, 0, m);
	EXPECT_THROW(test->execute(), WrongDatatypeOnPortException);
}

TEST_F(MapFieldDataFromElemToNodeModuleTests, ThrowForDenseColumnMatrixInPort)
{
  auto test = makeModule("MapFieldDataFromElemToNode");
	DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));
	m->setZero();
  stubPortNWithThisData(test, 0, m);
	EXPECT_THROW(test->execute(), WrongDatatypeOnPortException);
}