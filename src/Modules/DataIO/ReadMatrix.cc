/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
#include <boost/filesystem.hpp>
#include <Modules/DataIO/ReadMatrix.h>
#include <Algorithms/DataIO/ReadMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/String.h>

using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Algorithms::DataIO;
using namespace SCIRun::Domain::Datatypes;
using namespace SCIRun::Domain::Networks;

ReadMatrixModule::ReadMatrixModule() : Module(ModuleLookupInfo("ReadMatrix", "DataIO", "SCIRun")) {}

void ReadMatrixModule::execute()
{
  filename_ = get_state()->getValue(ReadMatrixAlgorithm::Filename).getString();
  if (!boost::filesystem3::exists(filename_))
  {
    //error()
    std::cout << "File not found: " << filename_ << std::endl;
    return;
  }

  ReadMatrixAlgorithm algo;
  ReadMatrixAlgorithm::Outputs matrix = algo.run(filename_);
  send_output_handle(0, matrix);
  StringHandle file(new String(filename_));
  send_output_handle(1, file);
}
