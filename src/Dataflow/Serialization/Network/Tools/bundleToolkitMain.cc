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


#include <iostream>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <fstream>
#include <boost/filesystem/operations.hpp>


int printHelp()
{
  std::cout << "Usage: bundle_toolkit OUTPUT_FILE [DIRECTORY_TO_SCAN]\nIf no directory specified, current directory is scanned." << std::endl;
  return 0;
}

int main(int argc, const char* argv[])
{
  if (argc < 2)
  {
    return printHelp();
  }

  std::string filename(argv[1]);

  if (filename.find("help") != std::string::npos ||
      filename.find("-h") != std::string::npos ||
      (!filename.empty() && filename[0] == '-'))
  {
    return printHelp();
  }

  std::string directoryToScan;
  if (argc < 3)
  {
    directoryToScan = boost::filesystem::current_path().string();
  }
  else
    directoryToScan = argv[2];

  std::cout << "Scanning directory: " << directoryToScan << std::endl;

  std::ofstream f(filename + ".toolkit");
  SCIRun::Dataflow::Networks::makeToolkitFromDirectory(directoryToScan).save(f);
  std::cout << "Saved toolkit file: " << filename << ".toolkit" << std::endl;
  return 0;
}
