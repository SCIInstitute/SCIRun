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


#include <Testing/Utils/SCIRunUnitTests.h>
#include <fstream>

using namespace SCIRun::TestUtils;

TEST(BinaryMatrixReaderTests, CanReadECGSimBinaryFile)
{
  boost::filesystem::path file(TestResources::rootDir() / "Matrices" / "ecgsim_matrix" / "ventricles2Thorax.mat");

  {
    auto fp_ = fopen(file.string().c_str(), "rb");

    const int sizeCount = 2;
    unsigned int hdr[sizeCount];
    if (!fread(hdr, sizeof(unsigned int), sizeCount, fp_))
    {
      FAIL() << "Header read failed.";
    }
    else
    {
      std::cout << "first " << sizeCount << " ints read (size): " << hdr[0] << ", " << hdr[1] << std::endl;
    }

    std::vector<float> values(hdr[0] * hdr[1]);
    if (!fread(&values[0], sizeof(float), values.size(), fp_))
    {
      FAIL() << "values read failed.";
    }
    else
    {
      std::cout << "Read " << values.size() << " floats. Here are first 10: " << std::endl;
      for (auto i = 0; i < 10; ++i)
        std::cout << values[i] << std::endl;
    }

    fclose(fp_);
  }
}

TEST(BinaryMatrixReaderTests, CanReadECGSimBinaryFileStreamClass)
{
  boost::filesystem::path file(TestResources::rootDir() / "Matrices" / "ecgsim_matrix" / "ventricles2Thorax.mat");

  {
    std::ifstream fp(file.string(), std::ifstream::binary);
    if (!fp)
      FAIL() << "file load failed";

    const int sizeCount = 2;
    unsigned int hdr[sizeCount];
    fp.read(reinterpret_cast<char*>(hdr), sizeCount * sizeof(unsigned int));
    if (!fp)
    {
      FAIL() << "Header read failed.";
    }
    else
    {
      std::cout << "first " << sizeCount << " ints read (size): " << hdr[0] << ", " << hdr[1] << std::endl;
    }
    unsigned int rows = hdr[0], cols = hdr[1];
    std::vector<float> values(rows*cols);
    fp.read(reinterpret_cast<char*>(&values[0]), values.size() * sizeof(float));
    if (!fp)
    {
      FAIL() << "values read failed.";
    }
    else
    {
      std::cout << "Read " << values.size() << " floats. Here are first 10: " << std::endl;
      for (auto i = 0; i < 10; ++i)
        std::cout << values[i] << std::endl;
    }
  }
}

TEST(BinaryMatrixReaderTests, DISABLED_CanReadECGSimBinaryFile2)
{
  //BinaryPiostream stream()
  std::string file("C:\\Dev\\ecgsim_matrix\\test.ECGsimsource");
  {
    auto fp_ = fopen(file.c_str(), "rb");

    const int sizeCount = 2;
    unsigned int hdr[sizeCount];
    if (!fread(hdr, sizeof(unsigned int), sizeCount, fp_))
    {
      FAIL() << "Header read failed.";
    }
    else
    {
      std::cout << "first " << sizeCount << " ints read (size): " << hdr[0] << ", " << hdr[1] << std::endl;
    }

    std::vector<float> values(hdr[0] * hdr[1]);
    if (!fread(&values[0], sizeof(float), values.size(), fp_))
    {
      FAIL() << "values read failed.";
    }
    else
    {
      std::cout << "Read " << values.size() << " floats. Here are first 10: " << std::endl;
      for (auto i = 0; i < 10; ++i)
        std::cout << values[i] << std::endl;
    }

    fclose(fp_);
  }


  FAIL() << "todo";
}
