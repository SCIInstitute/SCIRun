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


// A simple program that builds a sqrt table
#include <cstdio>
#include <cmath>
#include <string>
#include <Modules/Factory/Generator/MakeModuleFactory.h>
#include <Modules/Factory/Generator/ModuleFactoryGenerator.h>
#include <fstream>

int SCIRun::Modules::Factory::MakeGeneratedModuleFactoryCode(const std::string& sourcePath, const std::string& generatedFilename)
{
  auto code = SCIRun::Modules::Factory::Generator::GenerateModuleCodeFileFromSourcePath(sourcePath);
  std::ofstream out(generatedFilename);
  out << code;
  return 0;
}

int main(int argc, char *argv[])
{
  // make sure we have enough arguments
  if (argc < 3)
  {
    return 1;
  }

  return SCIRun::Modules::Factory::MakeGeneratedModuleFactoryCode(argv[1], argv[2]);
}
