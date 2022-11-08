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

#include <queue>
#include <future>
#include <Modules/Basic/AsyncStreamingTestModule.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Logging/Log.h>
#include <Core/Thread/Mutex.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;

MODULE_INFO_DEF(AsyncStreamingTest, Testing, SCIRun)

namespace detail
{
  using DataChunk = DenseMatrixHandle;
  //TODO: need thread-safe container to share
  using DataStream = std::queue<DataChunk>;
  static Mutex dataMutex("test");

  class StreamAppender
  {
  public:
    explicit StreamAppender(DenseMatrixHandle input) : input_(input), hasData_(true) {}

    bool hasData() const
    {
      //auto ret = hasData_ || !stream_.empty();
      //logInfo("__SR__ hasData returns {}", ret);
      return sliceIndex_ < input_->nrows();
    }

    int numDataAppended() const { return sliceIndex_; }

    DataStream& stream() { return stream_; }

    void start()
    {
      logInfo("__SR__ ........starting streaming reader");

      while (sliceIndex_ < input_->nrows())
      {
        auto value = makeShared<DenseMatrix>(input_->row(sliceIndex_));

        logInfo("__SR__ >>> pushing new data object: [{}]", sliceIndex_);
        {
          Guard g(dataMutex.get());
          stream_.push(value);
          sliceIndex_++;
        }
        logInfo("__SR__ : waiting for {} ms", appendWaitTime_);
        std::this_thread::sleep_for(std::chrono::milliseconds(appendWaitTime_));
      }
      hasData_ = false;
    }
  private:
    DenseMatrixHandle input_;
    DataStream stream_;
    bool hasData_;
    const int appendWaitTime_ = 2000;
    int sliceIndex_{ 0 };
  };


}

namespace SCIRun::Modules::Basic
{
  class Impl
  {
  public:
    Impl(DenseMatrixHandle input) : appender_(input) {}

    std::future<void> f_;

    detail::StreamAppender appender_;

    void start()
    {
      f_ = std::async([this]() { appender_.start(); });
    }
  };
}

AsyncStreamingTest::AsyncStreamingTest()
  : Module(staticInfo_, false),
  counter_(0)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputSlice);
}

AsyncStreamingTest::~AsyncStreamingTest() = default;

void AsyncStreamingTest::execute()
{
  SCIRun::Core::Logging::GeneralLog::Instance().setVerbose(true);


  auto input = getRequiredInput(InputMatrix);
  if (needToExecute())
  {
    logInfo("__MAIN__ Resetting impl/async thread");
    impl_ = std::make_unique<Impl>(castMatrix::toDense(input));
    impl_->start();
  }

  {
    logCritical("AsyncStreamingTest received input");

   


    //this will be launched on a separate thread
  /*  if (!impl_)
    {
      impl_ = std::make_unique<Impl>(castMatrix::toDense(input));
      impl_->start();
    }*/

    if (impl_->appender_.hasData())
    {
      //wait for result. 
      while (impl_->appender_.stream().empty())
      {
        logInfo("__MAIN__ Waiting for data");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      {
        logInfo("__MAIN__ Execute called");

        //once data is available, output to ports 
        {
          auto data = impl_->appender_.stream().front();
          {
            Guard g(detail::dataMutex.get());
            impl_->appender_.stream().pop();
            //readCount_++;
          }
          logInfo("__MAIN__ Received data: [{}] outputting matrix.", (*data)(0,0));
          //logInfo("__MAIN__ Processing data: waiting for {} ms", processWaitTime_);
          //std::this_thread::sleep_for(std::chrono::milliseconds(processWaitTime_));
          sendOutput(OutputSlice, data);
          logInfo("__MAIN__ Enqueue execute again");
          enqueueExecuteAgain(false);
        }
      }

      //while (stream_.empty() && tries_++ < maxTries_)
      //{
      //  logInfo("__MAIN__ Waiting for data...attempt {} out of {}", tries_, maxTries_);
      //  std::this_thread::sleep_for(std::chrono::milliseconds(readWaitTime_));
      //}
      //this artificial loop represents the network execution loop via enqueueExecuteAgain
    }
  }
}
