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


/// @todo Documentation Modules/Basic/DynamicPortTester.cc

#include <iostream>
#include <Modules/Basic/DynamicPortTester.h>
#include <Core/Datatypes/Scalar.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

DynamicPortTester::DynamicPortTester()
  : Module(ModuleLookupInfo("DynamicPortTester", "Testing", "SCIRun"), false)
{
  INITIALIZE_PORT(DynamicMatrix);
  INITIALIZE_PORT(DynamicField);
  INITIALIZE_PORT(DynamicString);
  INITIALIZE_PORT(NumMatrices);
  INITIALIZE_PORT(NumFields);
  INITIALIZE_PORT(NumStrings);
}

void DynamicPortTester::execute()
{
  auto matrices = getRequiredDynamicInputs(DynamicMatrix);
  auto fields = getRequiredDynamicInputs(DynamicField);
  auto strings  = getRequiredDynamicInputs(DynamicString);

  std::cout << "Dynamic port test. Connected are " << matrices.size() << " matrices, " << fields.size()
    << " fields, and " << strings.size() << " strings." << std::endl;

  sendOutput(NumMatrices, boost::make_shared<Int32>(static_cast<int>(matrices.size())));
  sendOutput(NumFields, boost::make_shared<Int32>(static_cast<int>(fields.size())));
  sendOutput(NumStrings, boost::make_shared<Int32>(static_cast<int>(strings.size())));
}
