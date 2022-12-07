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


#include <spire/var-buffer/VarBuffer.hpp>
#include <bserialize/BSerialize.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <cstring>

using namespace spire;

TEST(VarBufferTest, BasicSerialization)
{
  VarBuffer vb;

  // Proceed close to 1024 limit.
  const size_t emptySize = 1020;
  char empty[emptySize];
  std::memset(empty, 0, emptySize);

  vb.writeBytes(empty, emptySize);

  // After this write, we are at 1024.
  uint32_t v1 = 3;
  double v2 = 5.483;
  float v3 = 8293.09f;
  int32_t v4 = -323;
  uint16_t v5 = 8924;
  std::string v6 = "This is a test string.";
  uint8_t v7 = 98;

  vb.write(v1);

  // Continue writing.
  vb.write(v2);
  vb.write(v3);
  vb.write(v4);
  vb.writeNullTermString("Blah!");
  vb.write(v5);
  vb.write(v6);
  vb.write(v7);

  // Now take the buffer and stuff it into BSerialize
  BSerialize sin(vb.getBuffer(), vb.getBufferSize());
  sin.readBytes(emptySize);
  EXPECT_EQ(v1, sin.read<uint32_t>());
  EXPECT_EQ(v2, sin.read<double>());
  EXPECT_EQ(v3, sin.read<float>());
  EXPECT_EQ(v4, sin.read<int32_t>());
  EXPECT_EQ(std::string("Blah!"), std::string(sin.readNullTermString()));
  EXPECT_EQ(v5, sin.read<uint16_t>());
  EXPECT_EQ(v6, sin.read<std::string>());
  EXPECT_EQ(v7, sin.read<uint8_t>());
}

TEST(VarBufferTest, TestPreallocatedSize)
{
  VarBuffer vb(5000);

  EXPECT_EQ(5000, vb.getAllocatedSize());

  // Proceed close to 1024 limit.
  const size_t emptySize = 1020;
  char empty[emptySize];
  std::memset(empty, 0, emptySize);

  vb.writeBytes(empty, emptySize);

  // After this write, we are at 1024.
  uint32_t v1 = 3;
  double v2 = 5.483;
  float v3 = 8293.09f;
  int32_t v4 = -323;
  uint16_t v5 = 8924;
  std::string v6 = "This is a test string.";
  uint8_t v7 = 98;

  vb.write(v1);

  // Continue writing.
  vb.write(v2);
  vb.write(v3);
  vb.write(v4);
  vb.writeNullTermString("Blah!");
  vb.write(v5);
  vb.write(v6);
  vb.write(v7);

  // Now take the buffer and stuff it into BSerialize
  BSerialize sin(vb.getBuffer(), vb.getBufferSize());
  sin.readBytes(emptySize);
  EXPECT_EQ(v1, sin.read<uint32_t>());
  EXPECT_EQ(v2, sin.read<double>());
  EXPECT_EQ(v3, sin.read<float>());
  EXPECT_EQ(v4, sin.read<int32_t>());
  EXPECT_EQ(std::string("Blah!"), std::string(sin.readNullTermString()));
  EXPECT_EQ(v5, sin.read<uint16_t>());
  EXPECT_EQ(v6, sin.read<std::string>());
  EXPECT_EQ(v7, sin.read<uint8_t>());

  // Check to ensure that we are still at size 5000.
  EXPECT_EQ(5000, vb.getAllocatedSize());
}


//takes on the order of 10 second to run becasue of the large allocation size so I dsiabeld it for now
TEST(VarBufferTest, DISABLED_TestLargeAllocation)
{
  size_t size = 1ull << 32;
  EXPECT_GT(size, static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
  VarBuffer vb(size);
  EXPECT_EQ(size, vb.getAllocatedSize());
}
