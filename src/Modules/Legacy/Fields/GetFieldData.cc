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

GetFieldDataModule::GetFieldDataModule()
  : Module(ModuleLookupInfo("GetFieldData", "ChangeFieldData", "SCIRun"), false)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputMatrix);
}

void GetFieldDataModule::execute()
{
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    /// Define dataflow handles:
  FieldHandle input;
  MatrixHandle matrixdata(0);
  NrrdDataHandle nrrddata(0);
  
  /// Get data from port:
  if(!(get_input_handle("Field",input,true))) return;

  /// Data is only computed if the output port is connected:
  bool need_matrix_data = oport_connected("Matrix Data");
  bool need_nrrd_data   = oport_connected("Nrrd Data");

  /// Only do work if needed:
  if (inputs_changed_ ||
      (!oport_cached("Matrix Data") && need_matrix_data) ||
      (!oport_cached("Nrrd Data") && need_nrrd_data))
  {    
    update_state(Executing);
    if( need_matrix_data) 
    {
      /// Run algorithm
      if(!(algo_.run(input,matrixdata))) return;
    }

    if(need_nrrd_data )
    {
      /// Run algorithm
      if(!(algo_.run(input,nrrddata))) return;
    }

    /// If port is not connected at time of execute, send down a null handle
    /// send data downstream:
    send_output_handle("Matrix Data", matrixdata);
    send_output_handle("Nrrd Data", nrrddata);

  }
#endif

  ///NO Nrrd support yet !!!
  FieldHandle input = getRequiredInput(InputField);

  ///inputs_changed_ || !oport_cached("Matrix Nodes")
  if (needToExecute())
  {
    update_state(Executing);

    auto output = algo().run_generic(withInputData((InputField, input)));

    sendOutputFromAlgorithm(OutputMatrix, output);
    sendOutputFromAlgorithm(OutputNrrd, output);
  }
}
