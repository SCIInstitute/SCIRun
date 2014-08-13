/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
/// @todo Documentation Modules/Legacy/Fields/SetFieldData.cc

#include <Modules/Legacy/Fields/SetFieldData.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldData.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

ModuleLookupInfo SetFieldDataModule::staticInfo_("SetFieldData", "ChangeFieldData", "SCIRun");

SetFieldDataModule::SetFieldDataModule() :  Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputField);
}

void SetFieldDataModule::setStateDefaults()
{
  setStateBoolFromAlgo(SetFieldDataAlgo::keepTypeCheckBox);
}

void SetFieldDataModule::execute()
{
  auto input_field = getRequiredInput(InputField);
  auto input_matrix = getRequiredInput(InputMatrix);
  ///NO Nrrd support yet !!!

  ///inputs_changed_ || !oport_cached("Matrix Nodes")
  if (needToExecute())
  {
    update_state(Executing);

    auto output = algo().run_generic(make_input((InputField, input_field)(InputMatrix, input_matrix)));

    sendOutputFromAlgorithm(OutputField, output);
  }
}
