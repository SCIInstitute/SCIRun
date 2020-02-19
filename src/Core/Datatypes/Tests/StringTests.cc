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
#include <boost/any.hpp>

TEST(AnyTests, CanDefaultConstruct)
{
  boost::any a;
  EXPECT_TRUE(a.empty());
}

namespace {

  boost::any empty()
  { return boost::any(); }

  const boost::any const_empty()
  { return boost::any(); }
}

TEST(AnyTests, CanCopyConstructEmpty)
{
  boost::any a(empty());
  EXPECT_TRUE(a.empty());

  boost::any a2((boost::any()));
  EXPECT_TRUE(a2.empty());

  boost::any a3(2);
  EXPECT_FALSE(a3.empty());

  boost::any a4((boost::any(2)));
  EXPECT_FALSE(a4.empty());
}

TEST(AnyTests, DISABLED_CanCopyConstructConstEmpty_SEGFAULTSWITHBOOST154)
{
  boost::any a(const_empty());
  EXPECT_TRUE(a.empty());
}
