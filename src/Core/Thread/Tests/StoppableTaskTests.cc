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
#include <Core/Thread/Interruptible.h>
#include <Testing/Utils/SCIRunUnitTests.h>

using namespace SCIRun::Core::Thread;

/*
 * A Task class that extends the Stoppable Task
 */
class MyTask : public Stoppable
{
public:
    // Function to be executed by thread function
    void operator()()
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

TEST(StoppableTaskTests, DISABLED_Example)
{
  std::cout << std::this_thread::get_id() << " Creating our Task" << std::endl;
  MyTask task;
  std::cout << std::this_thread::get_id() << " Creating a thread to execute our task" << std::endl;
  std::thread th(std::ref(task));
  std::cout << std::this_thread::get_id() << " sleeping on main thread" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  std::cout << std::this_thread::get_id() << " Asking Task to Stop" << std::endl;
  std::cout << std::this_thread::get_id() << " Stop the Task" << std::endl;
  task.sendStopRequest();
  std::cout << std::this_thread::get_id() << " Waiting for thread to join" << std::endl;
  th.join();
  std::cout << std::this_thread::get_id() << " Thread Joined" << std::endl;
  std::cout << std::this_thread::get_id() << " Exiting Main Function" << std::endl;
}

TEST(StoppableTaskTests, DISABLED_CanResetTaskToRunAgain)
{
  std::cout << std::this_thread::get_id() << " Creating our Task" << std::endl;
  MyTask task;
  std::cout << std::this_thread::get_id() << " Creating a thread to execute our task" << std::endl;
  std::thread th1(std::ref(task));
  std::cout << std::this_thread::get_id() << " sleeping on main thread" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  std::cout << std::this_thread::get_id() << " Asking Task to Stop" << std::endl;
  std::cout << std::this_thread::get_id() << " Stop the Task" << std::endl;
  task.sendStopRequest();
  std::cout << std::this_thread::get_id() << " Waiting for thread to join" << std::endl;
  th1.join();
  std::cout << std::this_thread::get_id() << " Thread Joined" << std::endl;
  std::cout << std::this_thread::get_id() << " Exiting Main Function" << std::endl;

  task.resetStoppability();

  std::thread th2(std::ref(task));
  std::cout << std::this_thread::get_id() << " sleeping on main thread" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  std::cout << std::this_thread::get_id() << " Asking Task to Stop" << std::endl;
  std::cout << std::this_thread::get_id() << " Stop the Task" << std::endl;
  task.sendStopRequest();
  std::cout << std::this_thread::get_id() << " Waiting for thread to join" << std::endl;
  th2.join();
  std::cout << std::this_thread::get_id() << " Thread Joined" << std::endl;
  std::cout << std::this_thread::get_id() << " Exiting Main Function" << std::endl;

}
