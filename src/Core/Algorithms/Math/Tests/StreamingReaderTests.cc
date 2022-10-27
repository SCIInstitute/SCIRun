
TEST(BigVectorFieldFile, DISABLED_MakeIt)
{
  const size_t dim = 10;
  const size_t cols = 3 * dim * dim * dim;
  DenseMatrix m(6, cols);
  size_t col = 0;
  for (int d = 0; d < 3; ++d)
  {
    for (int i = 0; i < dim; ++i)
    {
      for (int j = 0; j < dim; ++j)
      {
        for (int k = 0; k < dim; ++k)
        {
          m(0, col) = (col % 3) == 0;
          m(1, col) = (col % 3) == 1;
          m(2, col) = (col % 3) == 2;
          m(3, col) = i;
          m(4, col) = j;
          m(5, col) = k;
          col++;
        }
      }
    }
  }
  std::ofstream file((TestResources::rootDir() / "danBigMatrix.txt").string());
  file << m;
}

TEST(BigVectorFieldFile, DISABLED_ReadIt)
{
  std::ifstream file((TestResources::rootDir() / "danBigMatrix.txt").string());
  DenseMatrix m;
  file >> m;
  EXPECT_EQ(6, m.rows());
  EXPECT_EQ(3000, m.cols());
}

std::string testFileForStreaming()
{
	return (TestResources::rootDir() / "TransientOutput" / "danBigNumberFile.txt").string();
}

TEST(GenerateLargeFileForStreamTesting, WriteItOut)
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

using DataChunk = std::vector<double>;
//TODO: need thread-safe container to share
using DataStream = std::queue<DataChunk>;
static Mutex dataMutex("test");

class StreamingReader 
{
public:
	StreamingReader(const std::string& filename, DataStream& stream) : file_(filename), stream_(stream), hasData_(true) {}
	bool hasData() const
	{	
		auto ret = hasData_ || !stream_.empty();
		std::cout << "hasData returns " << std::boolalpha << ret << std::endl;
		return ret;
	}
	void start()
	{
		std::cout << "starting streaming reader" << std::endl;
		
		
		
		std::string line;

		
      while( getline(file_, line, '\n'))
      {
		  
        if (line.size() > 0)
		{
			DataChunk values;
			std::cout << ">>>>> reading line\n\t" << line << std::endl;
			std::istringstream istr(line);
			std::copy(std::istream_iterator<double>(istr), std::istream_iterator<double>(), std::back_inserter(values));
			std::cout << ">>> pushing new data object: [" << values.front() << "-" << values.back() << "]" << std::endl;
			{
				Guard g(dataMutex.get());
				stream_.push(values);
			}
		}
		
	  }
		hasData_ = false;
		
	}
private:
	std::ifstream file_;
	DataStream& stream_;
	bool hasData_;
};

TEST(StreamingReaderTests, CanLoadFileAndOutputChunks)
{
	//this is the shared data buffer
	DataStream str;
	
	//this will be launched on a separate thread
	StreamingReader sr(testFileForStreaming(), str);
	auto t = std::async([&sr]() { sr.start(); });
	
	//this artificial loop represents the network execution loop via enqueueExecuteAgain
	while (sr.hasData())
	{
		//this will be the remaining body of execute
		//wait for result. 
		std::cout << "Execute called" << std::endl;
		while (str.empty())
		{
			std::cout << "\tWaiting for data..." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			//thread sleep
		}
		//once data is available, output to ports 
		{
			
			auto data = str.front();
			{
				Guard g(dataMutex.get());
				str.pop();
			}
			std::cout << "Received data: [" << data.front() << "-" << data.back() << "] outputting matrices." << std::endl;
			std::cout << "Enqueue execute again" << std::endl;
		}
	}
}

