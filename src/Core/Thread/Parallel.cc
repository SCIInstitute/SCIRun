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


#include <Core/Thread/Parallel.h>
#include <Core/Logging/Log.h>
#include <boost/thread/thread.hpp>
#include <vector>
#include <iostream>

using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Logging;

void Parallel::RunTasks(IndexedTask task, int numProcs)
{
  boost::thread_group threads;

  for (int i = 0; i < capByUserCoreCount(numProcs); ++i)
  {
    threads.create_thread(boost::bind(task, i));
  }

  try
  {
    threads.join_all();
  }
  catch (boost::thread_interrupted&)
  {
    threads.interrupt_all();
    throw;
  }
}

unsigned int Parallel::NumCores()
{
  return capByUserCoreCount(boost::thread::hardware_concurrency());
}

void Parallel::SetMaximumCores(unsigned int max)
{
  if (max == 0)
  {
    max = std::numeric_limits<unsigned int>::max();
    logWarning("Maximum cores available for parallel algorithms set to unlimited");
  }
  else
  {
    logWarning("Maximum cores available for parallel algorithms set to {}", max);
  }
  maximumCoresSetByUser_ = max;
}

unsigned int Parallel::capByUserCoreCount(unsigned int numProcs)
{
  return std::min(numProcs, maximumCoresSetByUser_);
}

unsigned int Parallel::maximumCoresSetByUser_(std::numeric_limits<unsigned int>::max());
