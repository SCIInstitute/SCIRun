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
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <Core/Utils/TypeIDTable.h>

using namespace SCIRun::Core::Utility;

struct Dummy
{
  int x;
};

bool operator==(const Dummy& x1, const Dummy& x2)
{
  return x1.x == x2.x;
}

bool operator!=(const Dummy& x1, const Dummy& x2)
{
  return !(x1 == x2);
}

TEST(TypeIDTableTests, CanConstructEmpty)
{
  TypeIDTable<Dummy> table;

  auto ctor = table.findConstructorInfo("LatVolMesh");

  EXPECT_FALSE(ctor);
}

TEST(TypeIDTableTests, CanRegisterObjectForLater)
{
  TypeIDTable<Dummy> table;

  const std::string type = "LatVolMesh";

  Dummy ctorInfo;
  EXPECT_TRUE(table.registerConstructorInfo(type, ctorInfo));

  auto ctor = table.findConstructorInfo(type);

  EXPECT_TRUE(!!ctor);
}

TEST(TypeIDTableTests, CannotRegisterConflictingCtors)
{
  TypeIDTable<Dummy> table;

  const std::string type = "LatVolMesh";

  Dummy ctorInfo1;
  ctorInfo1.x = 1;
  EXPECT_TRUE(table.registerConstructorInfo(type, ctorInfo1));

  Dummy ctorInfo2;
  ctorInfo2.x = 2;
  EXPECT_FALSE(table.registerConstructorInfo(type, ctorInfo2));
  EXPECT_EQ(1, table.size());

  auto ctor = table.findConstructorInfo(type);

  EXPECT_TRUE(!!ctor);
  EXPECT_EQ(1, ctor->x);
}

struct TryRegister
{
  TryRegister(TypeIDTable<Dummy>& table, int& trueCount, int id) : table_(table), trueCount_(trueCount), id_(id) {}
  void operator()()
  {
    const std::string key = "key";
    Dummy x;
    x.x = id_;
    if (table_.registerConstructorInfo(key, x))
      trueCount_++;
  }

  TypeIDTable<Dummy>& table_;
  int& trueCount_;
  int id_;
};

// This test will generate duplicate type exists warnings, since there are
// multiple attempts to insert the same key. This is OK.
TEST(TypeIDTableTests, MultithreadedAccessIsSafe)
{
  TypeIDTable<Dummy> table;

  int trueCount = 0;
  int tryCount = 50;
  std::vector< boost::shared_ptr<boost::thread> > threads;

  for (int i = 0; i < tryCount; ++i)
  {
    TryRegister tr(table, trueCount, i);
    boost::shared_ptr<boost::thread> t(new boost::thread(tr));
    threads.push_back(t);
  }

  for (int i = 0; i < tryCount; ++i)
  {
    threads[i]->join();
  }

  EXPECT_EQ(1, trueCount);
}

TEST(StringFormatting, NewWayMatchesOldWay)
{
  double x = 3.14159265;
  char s[32];
  sprintf(s, "%8.4f", x);
  auto expected = boost::lexical_cast<std::string>(s);

  auto actual = str(boost::format("%8.4f") % x);

  EXPECT_EQ(expected, actual);
}
