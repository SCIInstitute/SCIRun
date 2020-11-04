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
#include <iostream>
#include <chrono>
#include <future>

#include <Testing/Utils/SCIRunUnitTests.h>

using namespace SCIRun::TestUtils;

/*
 * Class that encapsulates promise and future object and
 * provides API to set exit signal for the thread
 */
class Stoppable
{
    std::promise<void> exitSignal;
    std::future<void> futureObj;
    Stoppable(const Stoppable&) = delete;
public:
    Stoppable() :
            futureObj(exitSignal.get_future())
    {
    }
    Stoppable(Stoppable && obj) : exitSignal(std::move(obj.exitSignal)), futureObj(std::move(obj.futureObj))
    {
        std::cout << "Move Constructor is called" << std::endl;
    }
    Stoppable & operator=(Stoppable && obj)
    {
        std::cout << "Move Assignment is called" << std::endl;
        exitSignal = std::move(obj.exitSignal);
        futureObj = std::move(obj.futureObj);
        return *this;
    }
    // Task need to provide defination  for this function
    // It will be called by thread function
    virtual void run() = 0;
    // Thread function to be executed by thread
    void operator()()
    {
      std::cout << std::this_thread::get_id() << " " << __FUNCTION__ << std::endl;
        run();
    }
    //Checks if thread is requested to stop
    bool stopRequested() const
    {
      std::cout << std::this_thread::get_id() << " " << __FUNCTION__ << std::endl;
        // checks if value in future object is available
        if (futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
            return false;
        return true;
    }
    // Request the thread to stop by setting value in promise object
    void stop()
    {
      std::cout << std::this_thread::get_id() << " " << __FUNCTION__ << std::endl;
      exitSignal.set_value();
    }
};
/*
 * A Task class that extends the Stoppable Task
 */
class MyTask: public Stoppable
{
public:
    // Function to be executed by thread function
    void run()
    {
        std::cout << std::this_thread::get_id() << " Task Start" << std::endl;
        // Check if thread is requested to stop ?
        while (!stopRequested())
        {
            std::cout << std::this_thread::get_id() <<  " Doing Some Work" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        std::cout << std::this_thread::get_id() << " Task End" << std::endl;
    }
};

TEST(StoppableTaskTests, Example)
{
  std::cout << std::this_thread::get_id() << " Creating our Task" << std::endl;
  MyTask task;
  std::cout << std::this_thread::get_id() << " Creating a thread to execute our task" << std::endl;
  std::thread th(std::ref(task));
  std::cout << std::this_thread::get_id() << " sleeping on main thread" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  std::cout << std::this_thread::get_id() << " Asking Task to Stop" << std::endl;
  std::cout << std::this_thread::get_id() << " Stop the Task" << std::endl;
  task.stop();
  std::cout << std::this_thread::get_id() << " Waiting for thread to join" << std::endl;
  th.join();
  std::cout << std::this_thread::get_id() << " Thread Joined" << std::endl;
  std::cout << std::this_thread::get_id() << " Exiting Main Function" << std::endl;

  //FAIL() << "todo";
}
