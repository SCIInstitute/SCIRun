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

#include <Core/Datatypes/Legacy/Base/TypeName.h>

using namespace SCIRun;

TEST(TypeNameTests, BuiltIn)
{
  char c;
  EXPECT_EQ("char", find_type_name(&c));

  int i;
  EXPECT_EQ("int", find_type_name(&i));

  bool b;
  EXPECT_EQ("bool", find_type_name(&b));

  long l;
  EXPECT_EQ("long", find_type_name(&l));

  unsigned int ui;
  EXPECT_EQ("unsigned_int", find_type_name(&ui));

  double d;
  EXPECT_EQ("double", find_type_name(&d));

  float f;
  EXPECT_EQ("float", find_type_name(&f));

  long long ll;
  EXPECT_EQ("long_long", find_type_name(&ll));
}

TEST(TypeNameTests, Vectors)
{
  std::vector<int> vi;
  EXPECT_EQ("vector<int>", find_type_name(&vi));

  std::vector<double> vd;
  EXPECT_EQ("vector<double>", find_type_name(&vd));

  std::vector<std::vector<double>> vvd;
  EXPECT_EQ("vector<vector<double>>", find_type_name(&vvd));
}

TEST(TypeNameTests, Strings)
{
  std::string s;
  EXPECT_EQ("string", find_type_name(&s));
}

class S
{
public:
  static std::string type_name(int/*unused*/) { return "S"; }
};

TEST(TypeNameTests, CustomClass)
{
  S s;
  EXPECT_EQ("S", find_type_name(&s));
}
