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

// Include the algorithm
#include <Core/Algorithms/Fields/FieldData/GetFieldData.h>

// The module class
#include <Dataflow/Network/Module.h>

// We need to define the ports used
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/NrrdPort.h>

namespace SCIRun {

/// @class GetFieldData
/// @brief This module will get the data associated with the nodes or the
/// elements of a field and put them in a matrix.

class GetFieldData : public Module {
  public:
    /// constructor and execute function
    GetFieldData(GuiContext*);
    virtual ~GetFieldData() {}
    virtual void execute();
  
  private:
    /// Define algorithms needed
    SCIRunAlgo::GetFieldDataAlgo algo_;
  
};


DECLARE_MAKER(GetFieldData)
GetFieldData::GetFieldData(GuiContext* ctx)
  : Module("GetFieldData", ctx, Source, "ChangeFieldData", "SCIRun")
{
  /// Forward error messages;
  algo_.set_progress_reporter(this);
}

void GetFieldData::execute()
{
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
}

} // End namespace SCIRun

