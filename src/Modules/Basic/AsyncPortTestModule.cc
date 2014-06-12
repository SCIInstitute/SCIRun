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

#include <Modules/Basic/AsyncPortTestModule.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Field/ReportFieldInfoAlgorithm.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;

ModuleLookupInfo AsyncPortTestModule::staticInfo_("AsyncPortTestModule", "Testing", "SCIRun");

AsyncPortTestModule::AsyncPortTestModule()
  : ModuleWithAsyncDynamicPorts(staticInfo_)
{
  INITIALIZE_PORT(AsyncField);
  LOG_DEBUG("AsyncPortTestModule()");
}

void AsyncPortTestModule::asyncExecute(DatatypeHandle data)
{
  //LOG_DEBUG()
  //TODO
  // auto field = getLatestInput(AsyncField);
  // handle(field);

  std::cout << "Async port test. Latest data is a ";
  auto field = boost::dynamic_pointer_cast<Field>(data);
  if (field)
  {
    ReportFieldInfoAlgorithm algo;
    auto output = algo.run(field);
    std::cout << "field of type " << output.type << "." << std::endl;
  }
  else
  {
    std::cout << "null field." << std::endl;
  }
  
}
