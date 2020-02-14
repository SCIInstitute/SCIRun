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
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Modules/Basic/NeedToExecuteTester.h>

using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

MODULE_INFO_DEF(NeedToExecuteTester, Testing, SCIRun)

NeedToExecuteTester::NeedToExecuteTester() : Module(staticInfo_, false), expensiveComputationDone_(false), executeCalled_(false)
{
  INITIALIZE_PORT(TestMatrixIn);
  INITIALIZE_PORT(TestMatrixOut);
  INITIALIZE_PORT(TestFieldIn);
  INITIALIZE_PORT(TestFieldOut);
}

void NeedToExecuteTester::setStateDefaults()
{

}

void NeedToExecuteTester::execute()
{
  //std::cout << "NTET::execute()" << std::endl;
  executeCalled_ = true;
  //std::cout << "NTET::execute() executeCalled true" << std::endl;

  std::cout << "\t\tNTET::execute: getInput Matrix" << std::endl;
  auto in0 = getOptionalInput(TestMatrixIn);
  std::cout << "\t\tNTET::execute: getInput Field" << std::endl;
  auto in1 = getOptionalInput(TestFieldIn);
  //std::cout << "NTET::execute() getInput" << std::endl;
  if (needToExecute())
  {
    std::cout << "!!!\t\tNTET::execute() needToExecute is true" << std::endl;
    expensiveComputationDone_ = true;
    if (in0)
      sendOutput(TestMatrixOut, *in0);
    if (in1)
      sendOutput(TestFieldOut, *in1);
  }
}

void NeedToExecuteTester::resetFlags()
{
  executeCalled_ = expensiveComputationDone_ = false;
}
