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


#include <Modules/Legacy/Fields/SplitFieldByConnectedRegion.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/SplitByConnectedRegion.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Scalar.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;

MODULE_INFO_DEF(SplitFieldByConnectedRegion, NewField, SCIRun)

SplitFieldByConnectedRegion::SplitFieldByConnectedRegion()
  : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField1);
  INITIALIZE_PORT(OutputField2);
  INITIALIZE_PORT(OutputField3);
  INITIALIZE_PORT(OutputField4);
  INITIALIZE_PORT(OutputField5);
  INITIALIZE_PORT(OutputField6);
  INITIALIZE_PORT(OutputField7);
  INITIALIZE_PORT(OutputField8);
}

void SplitFieldByConnectedRegion::setStateDefaults()
{
 setStateBoolFromAlgo(SplitFieldByConnectedRegionAlgo::SortDomainBySize());
 setStateBoolFromAlgo(SplitFieldByConnectedRegionAlgo::SortAscending());
}

void SplitFieldByConnectedRegion::execute()
{
 auto input_field = getRequiredInput(InputField);

 if (needToExecute())
  {
    setAlgoBoolFromState(SplitFieldByConnectedRegionAlgo::SortDomainBySize());
    setAlgoBoolFromState(SplitFieldByConnectedRegionAlgo::SortAscending());

    auto output = algo().run(make_input((InputField, input_field)));

    sendOutputFromAlgorithm(OutputField1, output);
    sendOutputFromAlgorithm(OutputField2, output);
    sendOutputFromAlgorithm(OutputField3, output);
    sendOutputFromAlgorithm(OutputField4, output);
    sendOutputFromAlgorithm(OutputField5, output);
    sendOutputFromAlgorithm(OutputField6, output);
    sendOutputFromAlgorithm(OutputField7, output);
    sendOutputFromAlgorithm(OutputField8, output);

    #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    boutput = new Bundle;
    for (size_t j=0; j< output.size(); j++)
    {
      std::ostringstream oss;
      oss << "Field" << j;
      boutput->setField(oss.str(),output[j]);
    }
    #endif
  }
}
