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


#ifndef CORE_THREAD_MUTEX_H
#define CORE_THREAD_MUTEX_H

#include <boost/noncopyable.hpp>
#include <thread>
#include <mutex>
#include <Core/Thread/share.h>

namespace SCIRun
{
namespace Core
{
  namespace Thread
  {
    class SCISHARE NamedMutex : public std::mutex, boost::noncopyable
    {
    public:
      explicit NamedMutex(const std::string& name = "[default]");
      std::string name() const;
      NamedMutex& get();
    private:
      std::string name_;
    };

    using Mutex = NamedMutex;

    class SCISHARE LoggedGuard : public std::scoped_lock<std::mutex>
    {
    public:
      LoggedGuard(NamedMutex& mutex, const std::string& log = "");
      ~LoggedGuard();
    private:
      std::string log_;
    };

    SCISHARE LoggedGuard makeLoggedGuard(NamedMutex& mutex, const std::string& name);

    using Guard = LoggedGuard;

    // Boost threads can be created on the stack and automatically detach in the destructor.
    // std::threads halt in the destructor, or throw if still active. This function mimics
    // boost thread creation using std::threads. In the future we can figure out how to use
    // std::async as a nicer replacement.
    class SCISHARE Util : public boost::noncopyable
    {
    public:
      template <typename ...Args>
      static void launchAsyncThread(Args&&... args)
      {
        std::thread t(args...);
        t.detach();
      }
    };
  }
}
}

#endif
