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


#include <Core/Thread/Mutex.h>
#include <Core/Thread/Interruptible.h>
#include <boost/thread.hpp>

using namespace SCIRun::Core::Thread;

Mutex::Mutex(const std::string& name) : name_(name)
{
}

void Mutex::lock()
{
  impl_.lock();
}

void Mutex::unlock()
{
  impl_.unlock();
}

void Interruptible::checkForInterruption()
{
  boost::this_thread::interruption_point();
  //#ifdef WIN32 // this is working on Mac, but not Windows.
  //std::cout << "trying to interrupt_point in thread " << boost::this_thread::get_id() << std::endl;
  //std::cout << "interruption enabled? " << boost::this_thread::interruption_enabled() << std::endl;
  //std::cout << "interruption requested? " << boost::this_thread::interruption_requested() << std::endl;
  //#endif
}
