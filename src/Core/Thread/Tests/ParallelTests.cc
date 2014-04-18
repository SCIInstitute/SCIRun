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
#include <numeric>

#include <Core/Thread/Parallel.h>

using namespace SCIRun::Core::Thread;

TEST(ParallelTests, CanDoubleNumberInParallel)
{
  int size = Parallel::NumCores();
  std::vector<int> nums(size);
  int i = 0;
  std::generate(nums.begin(), nums.end(), [&]() {return i++;});
  
  int expectedSum = size * (size-1) / 2;
  EXPECT_EQ(expectedSum, std::accumulate(nums.begin(), nums.end(), 0, std::plus<int>()));

  Parallel::RunTasks([&](int i) {nums[i]*=2;}, Parallel::NumCores());

  EXPECT_EQ(expectedSum * 2, std::accumulate(nums.begin(), nums.end(), 0, std::plus<int>()));
}

/// @todo
#if 0
TEST(ParallelTests, CanDoubleNumberWithParallelForEach)
{
  int size = 1000; //
  std::vector<int> nums(size);
  int i = 0;
  std::generate(nums.begin(), nums.end(), [&]() {return i++;});

  int expectedSum = size * (size-1) / 2;
  EXPECT_EQ(expectedSum, std::accumulate(nums.begin(), nums.end(), 0, std::plus<int>()));

  Parallel::ForEach([&](int i) {nums[i]*=2; std::cout << i << std::endl;}, Parallel::NumCores());

  EXPECT_EQ(expectedSum * 2, std::accumulate(nums.begin(), nums.end(), 0, std::plus<int>()));

  //EXPECT_TRUE(false);
}
#endif