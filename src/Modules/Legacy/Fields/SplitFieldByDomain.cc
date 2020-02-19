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


/// @todo Documentation Modules/Legacy/Fields/SplitFieldByDomain.cc

#include <Modules/Legacy/Fields/SplitFieldByDomain.h>
#include <Core/Algorithms/Legacy/Fields/DomainFields/SplitFieldByDomainAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;

MODULE_INFO_DEF(SplitFieldByDomain, NewField, SCIRun)

SplitFieldByDomain::SplitFieldByDomain() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(All_Fields);
  INITIALIZE_PORT(Field1);
  INITIALIZE_PORT(Field2);
  INITIALIZE_PORT(Field3);
  INITIALIZE_PORT(Field4);
  INITIALIZE_PORT(Field5);
  INITIALIZE_PORT(Field6);
  INITIALIZE_PORT(Field7);
  INITIALIZE_PORT(Field8);
}

void SplitFieldByDomain::setStateDefaults()
{
  auto state = get_state();
  state->setValue(SplitFieldByDomainAlgo::SortBySize, false);
  state->setValue(SplitFieldByDomainAlgo::SortAscending, false);
}

void SplitFieldByDomain::execute()
{
  auto input = getRequiredInput(InputField);

  if (needToExecute())
  {
    setAlgoBoolFromState(SplitFieldByDomainAlgo::SortBySize);
    setAlgoBoolFromState(SplitFieldByDomainAlgo::SortAscending);

    auto algoOutput = algo().run(withInputData((InputField, input)));

    auto output = algoOutput.getList<Field>(Variables::ListOfOutputFields);

    BundleHandle boutput(new Bundle);
    for (size_t j = 0; j < output.size(); ++j)
    {
      std::ostringstream oss;
      oss << "Field" << j;
      boutput->set(oss.str(), output[j]);
    }

    FieldHandle nofield;

    sendOutput(All_Fields, boutput);

    //TODO: make array of output ports, somehow
    sendOutput(Field1, output.size() > 0 ? output[0] : nofield);
    sendOutput(Field2, output.size() > 1 ? output[1] : nofield);
    sendOutput(Field3, output.size() > 2 ? output[2] : nofield);
    sendOutput(Field4, output.size() > 3 ? output[3] : nofield);
    sendOutput(Field5, output.size() > 4 ? output[4] : nofield);
    sendOutput(Field6, output.size() > 5 ? output[5] : nofield);
    sendOutput(Field7, output.size() > 6 ? output[6] : nofield);
    sendOutput(Field8, output.size() > 7 ? output[7] : nofield);
  }
}
