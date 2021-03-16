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


#ifndef CORE_THREAD_INTERRUPTIBLE_H
#define CORE_THREAD_INTERRUPTIBLE_H

#include <future>
#include <Core/Utils/Exception.h>
#include <Core/Thread/share.h>

namespace SCIRun
{
namespace Core
{
  namespace Thread
  {
    /*
     * Class that encapsulates promise and future object and
     * provides API to set exit signal for the thread
     */
    class SCISHARE Stoppable
    {
    public:
      Stoppable();
      Stoppable(Stoppable&& obj);
      Stoppable& operator=(Stoppable&& obj);
      void resetStoppability();
      bool stopRequested() const;
      void sendStopRequest();
    private:
      std::unique_ptr<std::promise<void>> exitSignal;
      std::future<void> futureObj;
      Stoppable(const Stoppable&) = delete;
    };

    using Interruptible = Stoppable;

    SCISHARE void checkForInterruption(const Stoppable* stoppable = nullptr);

    struct SCISHARE ThreadStopped : virtual ExceptionBase
    {};
  }
}
}

#endif
