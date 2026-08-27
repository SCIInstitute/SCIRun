/*
	 For more information, please see: http://software.sci.utah.edu

	 The MIT License

	 Copyright (c) 2022 Scientific Computing and Imaging Institute,
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


#include <gtest/gtest.h>

#include <fstream>
#include <queue>
#include <future>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Thread/Mutex.h>
#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Thread;

std::string testFileForStreaming()
{
	return (TestResources::rootDir() / "TransientOutput" / "danSmallMatrix0.txt").string();
}

TEST(BigVectorFieldFile, DISABLED_MakeIt)
{
  const size_t dim = 2;
  const size_t rows = 3 * dim * dim * dim;
	const size_t cols = 6;
  DenseMatrix m(rows, cols);
  for (int r = 0; r < rows; ++r)
  {
    for (int c = 0; c < cols; ++c)
    {
      m(r, c) = (r+1.0) * (c+1.5);
    }
  }
  std::ofstream file(testFileForStreaming());
  file << m;
}

TEST(BigVectorFieldFile, DISABLED_ReadIt)
{
  std::ifstream file((TestResources::rootDir() / "danBigMatrix.txt").string());
  DenseMatrix m;
  file >> m;
  EXPECT_EQ(6, m.cols());
  EXPECT_EQ(3000, m.rows());
}

TEST(GenerateLargeFileForStreamTesting, DISABLED_WriteItOut)
{
	std::ofstream file(testFileForStreaming());
	const int numDoubles = 100000;
	const int numDoublesPerLine = 10;
	const int numLines = numDoubles / numDoublesPerLine;
	for (int i = 0; i < numDoubles; ++i)
	{
		file << std::setprecision(8) << static_cast<double>(i) / numDoubles << " ";
		if (i != 0 && i % numDoublesPerLine == 0)
			file << '\n';
	}
	file << std::endl;
}

#if 0
void writeDataAsync()
{
	logInfo("__WRITE__ starting writer thread");
	
	const int numDoubles = 1000000;
	const int numDoublesPerLine = 10;
	const int numLines = numDoubles / numDoublesPerLine;
	for (int i = 0; i < numDoubles; ++i)
	{
		logInfo("__WRITE__ <<< writing line {}", i);
		std::ostringstream buffer;
		buffer << std::setprecision(8) << static_cast<double>(i) / numDoubles << " ";
		if (i != 0 && i % numDoublesPerLine == 0)
		{
			std::ofstream file(testFileForStreaming());
			file << buffer.str() << std::endl;
			logInfo("__WRITE__ <<< writer resting ");
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	}
}
#endif

using DataChunk = std::vector<double>;
//TODO: need thread-safe container to share
using DataStream = std::queue<DataChunk>;
static Mutex dataMutex("test");

class StreamAppender 
{
public:
	StreamAppender(const std::string& filename, DataStream& stream) : file_(filename), stream_(stream), hasData_(true) {}

	bool hasData() const
	{	
		auto ret = hasData_ || !stream_.empty();
		logInfo("__SR__ hasData returns {}", ret);
		return ret;
	}

	int numDataAppended() const { return appendCount_; }
	
	void start()
	{
		logInfo("__SR__ ........starting streaming reader");
		
    std::string line;

		while (getline(file_, line))
    {
      if (!line.empty())
      {
        DataChunk values;
        logInfo("__SR__ >>>>> reading line\n\t{}", line);
        std::istringstream istr(line);
        std::copy(std::istream_iterator<double>(istr), std::istream_iterator<double>(), std::back_inserter(values));
        logInfo("__SR__ >>> pushing new data object: [{}-{}]", values.front(), values.back());
        {
          Guard g(dataMutex.get());
          stream_.push(values);
					appendCount_++;
        }
      }
      logInfo("__SR__ : waiting for {} ms", appendWaitTime_);
      std::this_thread::sleep_for(std::chrono::milliseconds(appendWaitTime_));
    }
    hasData_ = false;
  }
private:
  std::ifstream file_;
	DataStream& stream_;
	bool hasData_;
	const int appendWaitTime_ = 500;
	int appendCount_{ 0 };
};

class StreamListener
{
public:
	explicit StreamListener(DataStream& stream) : stream_(stream), maxTries_(4) {}

	int numDataRead() const { return readCount_; }

	void listen()
	{
		while (tries_ <= maxTries_)
		{
			if (!stream_.empty())
			{
				//this will be the remaining body of execute
				//wait for result. 
				logInfo("__MAIN__ Execute called");

				//once data is available, output to ports 
				{
					auto data = stream_.front();
					{
						Guard g(dataMutex.get());
						stream_.pop();
						readCount_++;
					}
					logInfo("__MAIN__ Received data: [{}-{}] outputting matrices.", data.front(), data.back());
					logInfo("__MAIN__ Processing data: waiting for {} ms", processWaitTime_);
					std::this_thread::sleep_for(std::chrono::milliseconds(processWaitTime_));
					logInfo("__MAIN__ Enqueue execute again");
				}
			}

			while (stream_.empty() && tries_++ < maxTries_)
			{
				logInfo("__MAIN__ Waiting for data...attempt {} out of {}", tries_, maxTries_);
				std::this_thread::sleep_for(std::chrono::milliseconds(readWaitTime_));
			}
			//this artificial loop represents the network execution loop via enqueueExecuteAgain
		}
	}
private:
	DataStream& stream_;
	const int maxTries_;
	int tries_{ 0 };
	int readCount_{ 0 };
	const int readWaitTime_ = 500;
	const int processWaitTime_ = 1000;
};

TEST(StreamingReaderTests, DISABLED_CanReadAndLoadChunksSync)
{
	SCIRun::Core::Logging::GeneralLog::Instance().setVerbose(true);

	//this is the shared data buffer
	DataStream stream;

	//this will be launched on a separate thread
	StreamAppender sr(testFileForStreaming(), stream);
	sr.start();

	StreamListener sl(stream);
	sl.listen();

	ASSERT_EQ(sr.numDataAppended(), sl.numDataRead());
}

TEST(StreamingReaderTests, DISABLED_CanLoadFileAndOutputChunksAsync)
{
	SCIRun::Core::Logging::GeneralLog::Instance().setVerbose(true);

	//this is the shared data buffer
	DataStream stream;
	
	//this will be launched on a separate thread
	StreamAppender sr(testFileForStreaming(), stream);
	auto t = std::async([&sr]() { sr.start(); });

	StreamListener sl(stream);
	sl.listen();
	
	logInfo("__MAIN__ End of test func");

	ASSERT_EQ(sr.numDataAppended(), sl.numDataRead());
}

#if 0
namespace Cond
{

	std::mutex g_mutex;
	std::condition_variable g_cv;

	bool g_ready = false;
	DataStream dataStr;
	//int g_data = 0;

	void produceData() 
	{
		logInfo("__WRITE__ starting writer thread");

		const int numDoubles = 100;
		const int numDoublesPerLine = 10;
		const int numLines = numDoubles / numDoublesPerLine;
		for (int i = 0; i < numDoubles; ++i)
		{
			//logInfo("__WRITE__ <<< writing line {}", i);
			std::ostringstream buffer;
			buffer << std::setprecision(8) << static_cast<double>(i) / numDoubles << " ";
			if (i != 0 && i % numDoublesPerLine == 0)
			{
				std::ofstream file(testFileForStreaming());
				file << buffer.str() << std::endl;
				//logInfo("__WRITE__ <<< writer resting ");
				//std::this_thread::sleep_for(std::chrono::seconds(5));
			}
		}
		logInfo("__WRITE__ <<< done");
	}

	void consumeData() 
	{ 
		std::ifstream in(testFileForStreaming());
		std::string line;
		while (getline(in, line))
		{
			logInfo("__SR__ :{}", __LINE__);
			if (!line.empty())
			{
				logInfo("__SR__ :{}", __LINE__);
				DataChunk values;
				logInfo("__SR__ >>>>> reading line\n\t{}", line);
				std::istringstream istr(line);
				std::copy(std::istream_iterator<double>(istr), std::istream_iterator<double>(), std::back_inserter(values));
				logInfo("__SR__ >>> pushing new data object: [{}-{}]", values.front(), values.back());
				dataStr.push(values);
			}
		}
	}

	void consumer(int n) {
		int data = 0;
		int i = 0;
		while (true) 
		{
			std::unique_lock<std::mutex> ul(g_mutex);
			g_cv.wait(ul, []() { return g_ready; });
			consumeData();
			g_ready = false;
			ul.unlock();
			g_cv.notify_one();
		}
	}

	void producer(int n)
	{
		int i = 0;
		while (i++ < n)
		{
			std::unique_lock<std::mutex> ul(g_mutex);
			produceData();
			g_ready = true;
			ul.unlock();
			g_cv.notify_one();
			ul.lock();
			g_cv.wait(ul, []() { return !g_ready; });
		}
	}

	void consumerThread(int n) { consumer(n); }

	void producerThread(int n) { producer(n); }

}

TEST(StreamingReaderTests, UsingConditionVariable)
{
	SCIRun::Core::Logging::GeneralLog::Instance().setVerbose(true);

	using namespace Cond;

	logInfo("DataStr starts with size {}", dataStr.size());

	int times = 100;
	std::thread t1(consumerThread, times);
	std::thread t2(producerThread, times);
	t1.join();
	t2.join();

	logInfo("DataStr now has size {}", dataStr.size());
}
#endif