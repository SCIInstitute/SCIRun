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


/// @todo Documentation Modules/Legacy/Fields/GetMeshQualityField.cc
//Reports the quality of each element in the mesh based on the metric that you choose.

#include <Core/Algorithms/Legacy/Fields/MeshData/GetMeshQualityFieldAlgo.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Fields/GetMeshQualityField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms::Fields::Parameters;

MODULE_INFO_DEF(GetMeshQualityField, MiscField, SCIRun)

GetMeshQualityField::GetMeshQualityField() : Module(staticInfo_)
{
    //Initialize all ports.
    INITIALIZE_PORT(InputField);
    INITIALIZE_PORT(OutputField);
}

void GetMeshQualityField::setStateDefaults()
{
    setStateStringFromAlgoOption(Metric);
}

void GetMeshQualityField::execute()
{
  auto input = getRequiredInput(InputField);

  if (needToExecute())
  {
    setAlgoOptionFromState(Metric);

    auto output = algo().run(withInputData((InputField,input)));

    sendOutputFromAlgorithm(OutputField,output);

  }
}
