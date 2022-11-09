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
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
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

namespace SCIRun::Modules::Basic
{
  using DataChunk = DenseMatrixHandle;
  //TODO: need thread-safe container to share
  using DataStream = std::queue<DataChunk>;
  

  class StreamAppender
  {
  public:
    StreamAppender(AsyncStreamingTest* module, DenseMatrixHandle input) : module_(module), input_(input) {}

    bool hasData() const
    {
      return sliceIndex_ < input_->nrows();
    }

    int numDataAppended() const { return sliceIndex_; }

    DataStream& stream() { return stream_; }

    void pushDataToStream()
    {
      logInfo("__SR__ ........starting streaming reader");

      while (hasData())
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
    }

    void beginPushDataAsync()
    {
      f_ = std::async([this]() { pushDataToStream(); });
    }

    void waitAndOutputEach()
    {
      if (hasData())
      {
        //wait for result. 
        while (stream().empty())
        {
          logInfo("__MAIN__ Waiting for data");
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        //once data is available, output to ports 
        auto data = stream().front();
        {
          Guard g(dataMutex.get());
          stream().pop();
        }

        logInfo("__MAIN__ Received data: [{}] outputting matrix.", (*data)(0, 0));
        module_->outputAsyncData(module_->bundleOutputs({ "Slice" }, { data }));

        logInfo("__MAIN__ Enqueue execute again");
        module_->enqueueExecuteAgain(false);
      }
    }

  private:
    AsyncStreamingTest* module_;
    DenseMatrixHandle input_;
    DataStream stream_;
    const int appendWaitTime_ = 2000;
    int sliceIndex_{ 0 };

    std::future<void> f_;
    Mutex dataMutex{ "test" };
  };
}

AsyncStreamingTest::AsyncStreamingTest()
  : Module(staticInfo_, false)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputSlice);
}

AsyncStreamingTest::~AsyncStreamingTest() = default;

void AsyncStreamingTest::execute()
{
  SCIRun::Core::Logging::GeneralLog::Instance().setVerbose(true);

  logCritical("AsyncStreamingTest Execute called");

  auto input = getRequiredInput(InputMatrix);

  if (needToExecute())
  {
    logInfo("__MAIN__ Resetting impl/async thread");
    impl_ = std::make_unique<StreamAppender>(this, castMatrix::toDense(input));
    impl_->beginPushDataAsync();
  }
   
  impl_->waitAndOutputEach();
}

void AsyncStreamingTest::outputAsyncData(BundleHandle data)
{
  sendOutput(OutputSlice, data);
}

Core::Datatypes::BundleHandle AsyncStreamingTest::bundleOutputs(std::initializer_list<std::string> names, std::initializer_list<DatatypeHandle> dataList)
{
  auto bundle = makeShared<Bundle>();
  auto nIter = names.begin();
  auto dIter = dataList.begin();
  for (; nIter != names.end(); ++nIter, ++dIter)
  {
    bundle->set(*nIter, *dIter);
  }
  return bundle;
}
