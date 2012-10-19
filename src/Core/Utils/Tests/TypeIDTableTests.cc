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

#include <Core/Utils/TypeIDTable.h>

using namespace SCIRun::Core::Utility;

struct XXX 
{
  int x;
};

bool operator==(const XXX& x1, const XXX& x2)
{
  return x1.x == x2.x;
}

bool operator!=(const XXX& x1, const XXX& x2)
{
  return !(x1 == x2);
}

TEST(TypeIDTableTests, CanConstructEmpty)
{
  TypeIDTable<XXX> table;
  
  auto ctor = table.findConstructorInfo("LatVolMesh");

  EXPECT_FALSE(ctor);
}

TEST(TypeIDTableTests, CanRegisterObjectForLater)
{
  TypeIDTable<XXX> table;

  const std::string type = "LatVolMesh";

  XXX ctorInfo;
  EXPECT_TRUE(table.registerConstructorInfo(type, ctorInfo));

  auto ctor = table.findConstructorInfo(type);

  EXPECT_TRUE(ctor);
}

TEST(TypeIDTableTests, CannotRegisterConflictingCtors)
{
  EXPECT_TRUE(false);
}

TEST(TypeIDTableTests, MultithreadedAccessIsSafe)
{
  EXPECT_TRUE(false);
}