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

#include <Core/Math/TrigTable.h>
#include <Core/Math/MiscMath.h>
#include <Core/Exceptions/AssertionFailed.h>
#include <cmath>

using namespace SCIRun;

TEST(SinCosTableTests, TwoSimpleValues)
{
  SinCosTable t(2, 0, 2*M_PI);
  EXPECT_DOUBLE_EQ(0.0, t.sin(0));
  EXPECT_DOUBLE_EQ(1.0, t.cos(0));
  EXPECT_NEAR(0.0, t.sin(1), 1e-15);
  EXPECT_DOUBLE_EQ(1.0, t.cos(1));
}

TEST(SinCosTableTests, ThrowsForSizeOneTable)
{
  EXPECT_THROW(SinCosTable(1, 0, 1), AssertionFailed);
}

TEST(MathTest, LogChangeOfBase)
{
  EXPECT_DOUBLE_EQ(log(34.0) / log(10.0), log10(34.0));
}
