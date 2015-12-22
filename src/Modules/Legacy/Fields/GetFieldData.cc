/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

/// @brief This module will get the data associated with the nodes or the
/// elements of a field and put them in a matrix.

#include <Modules/Legacy/Fields/GetFieldData.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/GetFieldData.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

const ModuleLookupInfo GetFieldDataModule::staticInfo_("GetFieldData", "ChangeFieldData", "SCIRun");

GetFieldDataModule::GetFieldDataModule()
  : Module(staticInfo_, false)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputMatrix);
  INITIALIZE_PORT(OutputNrrd);
}

void GetFieldDataModule::execute()
{
  auto input = getRequiredInput(InputField);

  bool need_matrix_data = oport_connected(OutputMatrix);
  bool need_nrrd_data = oport_connected(OutputNrrd);

  //TODO: need to integrate "output port connection status changed" into needToExecute()
  if (needToExecute() || need_nrrd_data)
  {
    update_state(Executing);

    algo().set(Parameters::CalcMatrix, need_matrix_data);
    algo().set(Parameters::CalcNrrd, need_nrrd_data);

    auto output = algo().run_generic(withInputData((InputField, input)));

    sendOutputFromAlgorithm(OutputMatrix, output);
    sendOutputFromAlgorithm(OutputNrrd, output);
  }
}
