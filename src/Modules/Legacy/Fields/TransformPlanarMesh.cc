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


///
/// @file  TransformPlanarMesh.cc
///
/// @brief Rotate and flip field to get it into "standard" view
///
/// @author
///    Allen Sanderson
///    Scientific Computing and Imaging Institute
///    University of Utah
/// @date  April 2006
///

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Algorithms/Fields/TransformMesh/TransformPlanarMesh.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>


namespace SCIRun {

/// @class TransformPlanarMesh
/// @brief Non-interactive geometric transform of a field.

class TransformPlanarMesh : public Module
{
  public:
    TransformPlanarMesh(GuiContext* ctx);
    virtual ~TransformPlanarMesh() {}
    virtual void execute();

  private:
    GuiInt gui_axis_;
    GuiInt gui_invert_;
    GuiDouble gui_trans_x_;
    GuiDouble gui_trans_y_;

    SCIRunAlgo::TransformPlanarMeshAlgo algo_;
};


DECLARE_MAKER(TransformPlanarMesh)

TransformPlanarMesh::TransformPlanarMesh(GuiContext* context)
  : Module("TransformPlanarMesh", context, Filter, "ChangeMesh", "SCIRun"),
    gui_axis_(context->subVar("axis"), 2),
    gui_invert_(context->subVar("invert"), 0),
    gui_trans_x_(context->subVar("trans_x"), 0.0),
    gui_trans_y_(context->subVar("trans_y"), 0.0)
{
  algo_.set_progress_reporter(this);
}

void
TransformPlanarMesh::execute()
{
  // Get the input field.
  FieldHandle field_input_handle, field_output_handle;
  get_input_handle( "Input Field", field_input_handle, true );

  // Get a handle to the optional index matrix port.
  MatrixHandle matrix_input_handle;
  get_input_handle( "Index Matrix", matrix_input_handle, false );

  if( matrix_input_handle.get_rep() )
  {
    /// Check to see what index has been selected and if it matches
    /// the gui index.a
    if( gui_trans_x_.get() != matrix_input_handle->get(0, 0) ||
        gui_trans_y_.get() != matrix_input_handle->get(1, 0) )
    {

      gui_trans_x_.set( (int) matrix_input_handle->get(0, 0) );
      gui_trans_y_.set( (int) matrix_input_handle->get(1, 0) );

      inputs_changed_ = true;
    }
  }

  /// If no data or an input change recreate the field. I.e Only
  /// execute when neeed.
  if (inputs_changed_  ||
      gui_axis_.changed( true ) ||
      gui_invert_.changed( true ) ||
      gui_trans_x_.changed( true ) ||
      gui_trans_y_.changed( true ) ||
      !oport_cached("Transformed Field"))
  {
    update_state(Executing);

    int axis = gui_axis_.get();

    if (axis == 0) algo_.set_option("axis","x");
    else if (axis == 1) algo_.set_option("axis","y");
    else if (axis == 2) algo_.set_option("axis","z");

    algo_.set_bool("invert",gui_invert_.get());
    algo_.set_scalar("translate_x",gui_trans_x_.get());
    algo_.set_scalar("translate_y",gui_trans_y_.get());

    if(!(algo_.run(field_input_handle, field_output_handle))) return;

    // Send the data downstream.
    send_output_handle( "Transformed Field",  field_output_handle, true );
  }
}

} // End namespace SCIRun
