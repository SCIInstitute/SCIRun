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
#include <gmock/gmock.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using ::testing::NotNull;
using namespace ::testing;

TEST(BundleTests, CanDefaultConstruct)
{
  Bundle b;
  EXPECT_TRUE(b.empty());
  EXPECT_EQ(0, b.size());
}

TEST(BundleTests, CanStoreFields)
{
  Bundle b;
  FieldHandle f(new NullField);
  b.set("foo", f);
  EXPECT_TRUE(b.isField("foo"));
  ASSERT_THAT(b.get("foo"), NotNull());
}

TEST(BundleTests, CanStoreStrings)
{
  Bundle b;
  StringHandle s(new String("foo"));
  b.set("foo", s);
  EXPECT_TRUE(b.isString("foo"));
  ASSERT_THAT(b.get("foo"), NotNull());
}

TEST(BundleTests, CanStoreMatrices)
{
  Bundle b;
  MatrixHandle m(new DenseMatrix(1,2));
  b.set("foo", m);
  EXPECT_TRUE(b.isMatrix("foo"));
  ASSERT_THAT(b.get("foo"), NotNull());
}

TEST(BundleTests, CanStoreMultipleTypes)
{
  Bundle b;
  StringHandle s(new String("foo"));
  b.set("str", s);
  EXPECT_TRUE(b.isString("str"));
  MatrixHandle m(new DenseMatrix(1,2));
  b.set("mat", m);
  EXPECT_TRUE(b.isMatrix("mat"));
  FieldHandle f(new NullField);
  b.set("field", f);
  EXPECT_TRUE(b.isField("field"));

  EXPECT_EQ(3, b.size());
}

TEST(BundleTests, CanCountMultipleTypes)
{
  Bundle b;
  StringHandle s(new String("foo"));
  b.set("str", s);
  MatrixHandle m1(new DenseMatrix(1,2));
  b.set("mat1", m1);
  MatrixHandle m2(new DenseMatrix(1,2));
  b.set("mat2", m2);
  FieldHandle f1(new NullField);
  b.set("field1", f1);
  FieldHandle f2(new NullField);
  b.set("field2", f2);
  FieldHandle f3(new NullField);
  b.set("field3", f3);

  EXPECT_EQ(6, b.size());
  EXPECT_EQ(1, b.numStrings());
  EXPECT_EQ(2, b.numMatrices());
  EXPECT_EQ(3, b.numFields());
}

TEST(BundleTests, CanCopyConstruct)
{
  Bundle b1;
  MatrixHandle m(new DenseMatrix(1,2));
  b1.set("foo", m);
  EXPECT_EQ(1, b1.size());
  Bundle b2(b1);
  EXPECT_EQ(1, b2.size());
  b1.remove("foo");
  EXPECT_EQ(0, b1.size());
  EXPECT_EQ(1, b2.size());
}

TEST(BundleTests, CanClone)
{
  Bundle b1;
  MatrixHandle m(new DenseMatrix(1,2));
  b1.set("foo", m);
  EXPECT_EQ(1, b1.size());
  BundleHandle b2(b1.clone());
  EXPECT_EQ(1, b2->size());
  b1.remove("foo");
  EXPECT_EQ(0, b1.size());
  EXPECT_EQ(1, b2->size());
}

// change from v4. MD saw no reason to restrict names by case
TEST(BundleTests, IsSortedCaseSensitively)
{
  Bundle b;
  MatrixHandle m1(new DenseMatrix(1,2));
  b.set("foo", m1);
  MatrixHandle m2(new DenseMatrix(1,2));
  b.set("Foo", m2);
  EXPECT_EQ(2, b.size());
}

TEST(BundleTests, CanGetElementsByName)
{
  Bundle bundle;
  const std::string name = "foo";
  FieldHandle f(new NullField(name));
  bundle.set(name, f);
  ASSERT_TRUE(bundle.isField(name));
  auto field = bundle.getField(name);
  ASSERT_THAT(field, NotNull());
  EXPECT_EQ(field, f);
}

TEST(BundleTests, CanOverwriteValuesOfSameName)
{
  Bundle b;
  MatrixHandle m1(new DenseMatrix(1,2));
  b.set("foo", m1);
  EXPECT_EQ(1, b.size());
  MatrixHandle m2(new DenseMatrix(2,2));
  b.set("foo", m2);
  EXPECT_EQ(1, b.size());
}

TEST(BundleTests, CanRemoveValuesByName)
{
  Bundle b;
  MatrixHandle m1(new DenseMatrix(1,2));
  b.set("foo", m1);
  EXPECT_EQ(1, b.size());
  EXPECT_TRUE(b.remove("foo"));
  EXPECT_TRUE(b.empty());
  EXPECT_EQ(0, b.size());
  EXPECT_FALSE(b.remove("foo"));
  MatrixHandle m2(new DenseMatrix(2,2));
  b.set("foo", m2);
  EXPECT_EQ(1, b.size());
}

TEST(BundleTests, CanIterateOverEachType)
{
  Bundle b;
  StringHandle s(new String("foo"));
  b.set("str", s);
  MatrixHandle m1(new DenseMatrix(1,2));
  b.set("mat1", m1);
  MatrixHandle m2(new DenseMatrix(1,2));
  b.set("mat2", m2);
  FieldHandle f1(new NullField);
  b.set("field1", f1);
  FieldHandle f2(new NullField);
  b.set("field2", f2);
  FieldHandle f3(new NullField);
  b.set("field3", f3);

  auto mats = b.getMatrices();
  EXPECT_THAT(mats, ElementsAre(m1, m2));
  auto strings = b.getStrings();
  EXPECT_THAT(strings, ElementsAre(s));
  auto fields = b.getFields();
  EXPECT_THAT(fields, ElementsAre(f1, f2, f3));
}

TEST(BundleTests, CanRetrieveNamesByType)
{
  Bundle b;
  StringHandle s(new String("foo"));
  b.set("str", s);
  MatrixHandle m1(new DenseMatrix(1,2));
  b.set("mat1", m1);
  MatrixHandle m2(new DenseMatrix(1,2));
  b.set("mat2", m2);
  FieldHandle f1(new NullField);
  b.set("field1", f1);
  FieldHandle f2(new NullField);
  b.set("field2", f2);
  FieldHandle f3(new NullField);
  b.set("field3", f3);

  auto matNames = b.getMatrixNames();
  EXPECT_THAT(matNames, ElementsAre("mat1", "mat2"));
  auto fieldNames = b.getFieldNames();
  EXPECT_THAT(fieldNames, ElementsAre("field1", "field2", "field3"));
}
