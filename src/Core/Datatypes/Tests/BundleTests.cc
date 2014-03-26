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
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using ::testing::NotNull;

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
  FAIL() << "todo";
}

TEST(BundleTests, CanStoreStrings)
{
  FAIL() << "todo";
}

TEST(BundleTests, CanStoreMatrices)
{
  FAIL() << "todo";
}

TEST(BundleTests, CanStoreMultipleTypes)
{
  FAIL() << "todo";
}

TEST(BundleTests, IsSortedCaseInsensitively)
{
  FAIL() << "todo";
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
  FAIL() << "todo";
}

TEST(BundleTests, CanOverwriteValuesOfSameNameCaseInsensitive)
{
  FAIL() << "todo";
}

TEST(BundleTests, CanRemoveValuesByName)
{
  FAIL() << "todo";
}