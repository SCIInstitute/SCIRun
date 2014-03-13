/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

///TODO Documentation

#include <Core/Thread/Parallel.h>

#include <boost/foreach.hpp>
#include <boost/thread/thread.hpp>
#include <vector>

using namespace SCIRun::Core::Thread;

typedef boost::shared_ptr<boost::thread> threadPointer;

void Parallel::RunTasks(IndexedTask task, int numProcs)
{
  std::vector<threadPointer> threads;
  
  for (int i = 0; i < numProcs; ++i)
  {
    threadPointer t(new boost::thread(task, i));
    threads.push_back(t);
  }
  
  BOOST_FOREACH(threadPointer& t, threads)
  {
    t->join();
  }
}

unsigned int Parallel::NumCores()
{
  return boost::thread::hardware_concurrency();
}
