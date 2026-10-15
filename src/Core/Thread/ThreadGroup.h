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


#ifndef CORE_THREAD_THREADGROUP_H
#define CORE_THREAD_THREADGROUP_H

#include <thread>
#include <utility>
#include <vector>

namespace SCIRun {
namespace Core {
namespace Thread {

  /// Workers that borrow state from their launcher -- a raw `this`, a reference to a
  /// local -- must not outlive the call that started them. This owns them and joins
  /// before it goes away, which `Util::launchAsyncThread` (detach) cannot do. #2732
  ///
  /// Declare it *before* any lock the workers need to acquire, so that the destructor
  /// runs after the lock is released; joining while holding that lock deadlocks.
  class JoiningThreadGroup
  {
  public:
    JoiningThreadGroup() = default;
    ~JoiningThreadGroup() { joinAll(); }

    JoiningThreadGroup(const JoiningThreadGroup&) = delete;
    JoiningThreadGroup& operator=(const JoiningThreadGroup&) = delete;

    template <typename Callable>
    void launch(Callable&& callable)
    {
      threads_.emplace_back(std::forward<Callable>(callable));
    }

    void joinAll()
    {
      for (auto& t : threads_)
      {
        if (t.joinable())
          t.join();
      }
      threads_.clear();
    }

    bool empty() const { return threads_.empty(); }

  private:
    std::vector<std::thread> threads_;
  };

}}}

#endif
