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
#include <Modules/DataIO/WriteMatrix.h>
#include <Algorithms/DataIO/WriteMatrix.h>
#include <Core/Datatypes/Matrix.h>

using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Algorithms::DataIO;
using namespace SCIRun::Domain::Datatypes;
using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Modules::DataIO;

WriteMatrixModule::WriteMatrixModule() : Module(ModuleLookupInfo("WriteMatrix", "DataIO", "SCIRun")) {}

void WriteMatrixModule::execute()
{
  DatatypeHandleOption data = get_input_handle(0);
  if (!data)
  {
    std::cout << "Required input not present, this check should go in the module base class!!!" << std::endl;
    return;
  }

  DenseMatrixHandle matrix = boost::dynamic_pointer_cast<DenseMatrix>(*data);

  filename_ = get_state()->getValue(WriteMatrixAlgorithm::Filename).getString();

  WriteMatrixAlgorithm algo;
  algo.run(matrix, filename_);
}
