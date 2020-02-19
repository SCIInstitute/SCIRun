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


#include <Core/Datatypes/Scalar.h>
#include <Modules/Basic/SendComplexScalar.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Logging/Log.h>
#include <spdlog/fmt/ostr.h>

using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;

SendComplexScalar::SendComplexScalar()
  : Module(ModuleLookupInfo("SendComplexMatrix", "Math", "SCIRun"), false),
  data_(-1)
{
  INITIALIZE_PORT(Scalar);
}

void SendComplexScalar::execute()
{
  if (needToExecute())
  {
    ComplexDenseMatrix c(2,2);
    c << complex(1,2), complex(3,4), complex(-1,-2), complex(-3,-4);
    auto output(boost::make_shared<ComplexDenseMatrix>(c));
    sendOutput(Scalar, output);
  }
  else
  {
    LOG_DEBUG("Executing SendComplexMatrix with old value, not sending anything: {}", data_);
  }
}
