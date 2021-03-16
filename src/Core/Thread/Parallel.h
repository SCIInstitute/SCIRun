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


#ifndef CORE_THREAD_PARLLEL_H
#define CORE_THREAD_PARLLEL_H

#include <boost/noncopyable.hpp>
#include <thread>
#include <vector>
#include <functional>
#include <Core/Thread/share.h>

namespace SCIRun
{
namespace Core
{
namespace Thread
{
  class SCISHARE Parallel : public boost::noncopyable
  {
  public:
    typedef std::function<void(int)> IndexedTask;
    static void RunTasks(IndexedTask task, int numProcs);
    static unsigned int NumCores();
    static void SetMaximumCores(unsigned int max);
  private:
    static unsigned int maximumCoresSetByUser_;
    static unsigned int capByUserCoreCount(unsigned int numProcs);
  };

  class SCISHARE ThreadGroup : public boost::noncopyable
  {
  public:
    template <typename ...Args>
    void create_thread(Args&&... args) { threads_.emplace_back(args...); }

    void join_all();
    void clear() { threads_.clear(); }
  private:
    std::vector<std::thread> threads_;
  };


}}}

#endif
