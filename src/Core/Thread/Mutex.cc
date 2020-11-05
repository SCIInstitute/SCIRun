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


#include <thread>
//#include <iostream>
#include <chrono>
#include <Core/Thread/Mutex.h>
#include <Core/Thread/Interruptible.h>

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
  //TODO: rewrite
  //boost::this_thread::interruption_point();
  //#ifdef WIN32 // this is working on Mac, but not Windows.
  //std::cout << "trying to interrupt_point in thread " << boost::this_thread::get_id() << std::endl;
  //std::cout << "interruption enabled? " << boost::this_thread::interruption_enabled() << std::endl;
  //std::cout << "interruption requested? " << boost::this_thread::interruption_requested() << std::endl;
  //#endif
}

Stoppable::Stoppable() :
  exitSignal(new std::promise<void>),
        futureObj(exitSignal->get_future())
{
}
Stoppable::Stoppable(Stoppable && obj) : exitSignal(std::move(obj.exitSignal)), futureObj(std::move(obj.futureObj))
{
    //std::cout << "Move Constructor is called" << std::endl;
}
Stoppable& Stoppable::operator=(Stoppable && obj)
{
    //std::cout << "Move Assignment is called" << std::endl;
    exitSignal = std::move(obj.exitSignal);
    futureObj = std::move(obj.futureObj);
    return *this;
}

void Stoppable::reset()
{
  exitSignal.reset(new std::promise<void>);
  futureObj = exitSignal->get_future();
}
// Thread function to be executed by thread
// void Stoppable::operator()()
// {
//   std::cout << std::this_thread::get_id() << " " << __FUNCTION__ << std::endl;
//     run();
// }
//Checks if thread is requested to stop
bool Stoppable::stopRequested() const
{
  //std::cout << std::this_thread::get_id() << " " << __FUNCTION__ << "?";

  auto timedout = futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
  //std::cout << "\t" << std::boolalpha << !timedout << std::endl;
    // checks if value in future object is available
    if (timedout)
        return false;
    return true;
}
// Request the thread to stop by setting value in promise object
void Stoppable::stop()
{
  //std::cout << std::this_thread::get_id() << " " << __FUNCTION__ << std::endl;
  exitSignal->set_value();
}
