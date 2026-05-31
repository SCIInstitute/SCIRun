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

#include <Modules/Basic/AsyncPortTestModule.h>
#include <Modules/Basic/DynamicPortTester.h>
#include <Modules/Basic/LoggingTester.h>
#include <Modules/Basic/NeedToExecuteTester.h>
#include <Modules/Basic/PortFeedbackTestModules.h>
#include <Modules/Basic/ReceiveComplexScalar.h>
#include <Modules/Basic/SendComplexScalar.h>
#include <Modules/DataIO/ReadField.h>
#include <Modules/DataIO/ReadMatrixClassic.h>
#include <Modules/DataIO/WriteField.h>
#include <Modules/DataIO/WriteMatrix.h>
#include <Modules/Factory/ModuleDescriptionLookup.h>
#include <Modules/Legacy/Bundle/GetFieldsFromBundle.h>
#include <Modules/Legacy/Matlab/DataIO/ImportDatatypesFromMatlab.h>
#include <Modules/Visualization/CreateTestingArrow.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Modules::DataIO;
using namespace SCIRun::Modules::Matlab::DataIO;


void ModuleDescriptionLookup::addEssentialModules()
{
  addModuleDesc<ReadMatrix>("ReadMatrix", "DataIO", "SCIRun", "Redo of ReadMatrix", "...");
  addModuleDesc<WriteMatrix>("WriteMatrix", "DataIO", "SCIRun",
      "Functional, outputs text files or binary .mat only.", "...");
  addModuleDesc<ReadField>(
      "ReadField", "DataIO", "SCIRun", "Functional, needs GUI and algorithm work.", "...");
  addModuleDesc<WriteField>(
      "WriteField", "DataIO", "SCIRun", "Functional, outputs binary .fld only.", "...");







}

void ModuleDescriptionLookup::addTestingModules()
{
  addModuleDesc<SendComplexScalar>("SendComplexMatrix", "Testing", "SCIRun", "...", "...");
  addModuleDesc<ReceiveComplexScalar>("ReceiveComplexMatrix", "Testing", "SCIRun", "...", "...");
  addModuleDesc<DynamicPortTester>("DynamicPortTester", "Testing", "SCIRun", "...", "...");
  addModuleDesc<AsyncPortTestModule>("...", "ViewScene clone");
  addModuleDesc<NeedToExecuteTester>("...", "...");
  addModuleDesc<PortFeedbackReceiver>("...", "...");
  addModuleDesc<PortFeedbackSender>("...", "...");
  addModuleDesc<LoggingTester>("...", "...");
  addModuleDesc<CreateTestingArrow>("...", "...");

}
