/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

//class CommandLineData
//{
//public:
//  CommandLineData(int argc, char* argv[])
//  {
//    commandLine_.assign(argv, argv + argc);
//  }
//  
//  int argc() const
//  {
//    return static_cast<int>(commandLine_.size());
//  }
//
//  const char* const* argv() const
//  {
//    if (commandLine_.empty())
//      return 0;
//    return &commandLine_[0];
//  }
//
//private:
//  std::vector<char*> commandLine_;
//};

TEST(CommandLineSpecTest, BoostExampleCode)
{
  // Declare the supported options.
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("compression", po::value<int>(), "set compression level")
    ;

  
  char* argv[] = {"scirun.exe", "--help", "--compression", "4"};
  const int argc = sizeof(argv)/sizeof(char*);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);    

  if (vm.count("help")) 
  {
    std::cout << "help was selected" << std::endl;
    std::cout << desc << "\n";
  }

  if (vm.count("compression")) 
  {
    std::cout << "Compression level was set to " 
      << vm["compression"].as<int>() << ".\n";
  } 
  else 
  {
    std::cout << "Compression level was not set.\n";
  }




  //EXPECT_TRUE(false);
}