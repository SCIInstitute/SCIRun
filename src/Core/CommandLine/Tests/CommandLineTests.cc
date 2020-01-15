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


#include <gtest/gtest.h>
#include <boost/program_options.hpp>
#include <Core/CommandLine/CommandLine.h>

using namespace SCIRun::Core::CommandLine;
namespace po = boost::program_options;

namespace
{
  po::variables_map readCommandLine(int argc, const char* argv[], const po::options_description& desc)
  {
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    return vm;
  }
}

TEST(CommandLineSpecTest, BoostExampleCode)
{
  // Declare the supported options.
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("compression", po::value<int>(), "set compression level")
    ;

  std::ostringstream ostr;
  ostr << desc;
  EXPECT_EQ("Allowed options:\n  --help                produce help message\n  --compression arg     set compression level\n",  ostr.str());

  {
    const char* argv[] = {"dummy.exe", "--help", "--compression", "4"};
    int argc = sizeof(argv)/sizeof(char*);

    po::variables_map vm = readCommandLine(argc, argv, desc);

    EXPECT_EQ(1, vm.count("help"));
    EXPECT_EQ(1, vm.count("compression"));
    EXPECT_EQ(4, vm["compression"].as<int>());
  }

  {
    const char* argv[] = {"dummy.exe", "--compression", "7"};
    int argc = sizeof(argv)/sizeof(char*);

    po::variables_map vm = readCommandLine(argc, argv, desc);

    EXPECT_EQ(0, vm.count("help"));
    EXPECT_EQ(1, vm.count("compression"));
    EXPECT_EQ(7, vm["compression"].as<int>());
  }

  {
    const char* argv[] = {"dummy.exe", "--help"};
    int argc = sizeof(argv)/sizeof(char*);

    po::variables_map vm = readCommandLine(argc, argv, desc);

    EXPECT_EQ(1, vm.count("help"));
    EXPECT_EQ(0, vm.count("compression"));
    EXPECT_THROW(vm["compression"].as<int>(), boost::bad_any_cast);
  }

  {
    const char* argv[] = {"dummy.exe", "--compression", "7.5"};
    int argc = sizeof(argv)/sizeof(char*);

    //what exception type?
    EXPECT_THROW(readCommandLine(argc, argv, desc), std::exception);
  }
}
