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


#include <Core/Algorithms/Fields/FieldData/ConvertMappingMatrixToFieldData.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

namespace SCIRun {

class SwapFieldDataWithMatrixEntries : public Module
{
  public:
    SwapFieldDataWithMatrixEntries(GuiContext*);
    virtual ~SwapFieldDataWithMatrixEntries() {}
    virtual void execute();

  private:
    GuiInt gui_keepscalartype_;
    SCIRunAlgo::GetFieldDataAlgo get_algo_;
    SCIRunAlgo::SetFieldDataAlgo set_algo_;

};


DECLARE_MAKER(SwapFieldDataWithMatrixEntries)
SwapFieldDataWithMatrixEntries::SwapFieldDataWithMatrixEntries(GuiContext* ctx)
  : Module("SwapFieldDataWithMatrixEntries", ctx, Source, "ChangeFieldData", "SCIRun"),
  gui_keepscalartype_(ctx->subVar("preserve-scalar-type"), 0)
{
  get_algo_.set_progress_reporter(this);
  set_algo_.set_progress_reporter(this);
}

void
SwapFieldDataWithMatrixEntries::execute()
{
  FieldHandle field_input_handle;
  MatrixHandle matrix_input_handle;

  if(!(get_input_handle("Input Field",field_input_handle,true))) return;

  get_input_handle("Input Matrix",matrix_input_handle,false);

  bool need_field  = oport_connected("Output Field");
  bool need_matrix = oport_connected("Output Matrix");

  if (inputs_changed_ ||
      gui_keepscalartype_.changed() ||
      (need_field && !oport_cached("Output Field")) ||
      (need_matrix && !oport_cached("Output Matrix")))
  {
    update_state(Executing);

    // Get the data.
    if( need_matrix )
    {
      MatrixHandle matrix_output_handle;
      if(!(get_algo_.run(field_input_handle,matrix_output_handle))) return;
      send_output_handle("Output Matrix", matrix_output_handle);
    }

    // Set the data.
    if( need_field )
    {
      FieldHandle field_output_handle;

      if (matrix_input_handle.get_rep())
      {
        if (gui_keepscalartype_.get())
          set_algo_.set_option("scalardatatype",field_input_handle->vfield()->get_data_type());


        if(!(set_algo_.run(field_input_handle,matrix_input_handle,
			       field_output_handle))) return;

        field_output_handle->copy_properties(field_input_handle.get_rep());
      }
      else
      {
        warning("No input matrix passing the field through");
        field_output_handle = field_input_handle;
      }

      send_output_handle("Output Field", field_output_handle);
    }
  }
}

} // End namespace SCIRun
