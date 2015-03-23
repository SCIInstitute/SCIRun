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

///@author
///   Allen R. Sanderson
///   Michael Callahan
///   Department of Computer Science
///   University of Utah
///@date  March 2001

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Fields/ClipMesh/ClipMeshByIsovalue.h>
#include <iostream>

namespace SCIRun {

/// @class ClipVolumeByIsovalue
/// @brief Clip out parts of a field.

class ClipVolumeByIsovalue : public Module
{
  public:
    ClipVolumeByIsovalue(GuiContext* ctx);
    virtual ~ClipVolumeByIsovalue() {}
    virtual void execute();

  private:
    GuiDouble  gui_iso_value_min_;
    GuiDouble  gui_iso_value_max_;
    GuiDouble  gui_iso_value_;
    GuiInt     gui_lte_;
    //gui_update_type_ must be declared after gui_iso_value_max_ which is
    //traced in the tcl code. If gui_update_type_ is set to Auto having it
    //last will prevent the net from executing when it is instantiated.
    GuiString  gui_update_type_;

    SCIRunAlgo::ClipMeshByIsovalueAlgo algo_;    
};


DECLARE_MAKER(ClipVolumeByIsovalue)


ClipVolumeByIsovalue::ClipVolumeByIsovalue(GuiContext* context)
  : Module("ClipVolumeByIsovalue", context, Filter, "NewField", "SCIRun"),
    gui_iso_value_min_(context->subVar("isoval-min"),  0.0),
    gui_iso_value_max_(context->subVar("isoval-max"), 99.0),
    gui_iso_value_(context->subVar("isoval"), 0.0),
    gui_lte_(context->subVar("lte"), 1),
    gui_update_type_(context->subVar("update_type"), "On Release")
{
  algo_.set_progress_reporter(this);
}


void
ClipVolumeByIsovalue::execute()
{
  FieldHandle field_input_handle;
  get_input_handle( "Input", field_input_handle, true );

  // Check to see if the input field has changed.
  if( inputs_changed_ ) 
  {
    update_state(Executing);
    VField* vfield = field_input_handle->vfield();

    if (!vfield->is_scalar()) 
    {
      error("Input field does not contain scalar data.");
      return;
    }

    double minval, maxval;
    vfield->minmax(minval,maxval);
    // extend min and max a little so extreme cases are well supported:
    //   getting back entire input field and clipping away entire input field
    double range = maxval-minval;
    minval -= range/100.0;
    maxval += range/100.0;

    // Check to see if the gui min max are different than the field.
    if( gui_iso_value_min_.get() != minval ||
	      gui_iso_value_max_.get() != maxval ) 
    {
      gui_iso_value_min_.set( minval );
      gui_iso_value_max_.set( maxval );

      std::ostringstream str;
      str << get_id() << " set_min_max ";
      TCLInterface::execute(str.str().c_str());
    }

    if (field_input_handle->basis_order() != 1)
    {
      error("Isoclipping can only be done for fields with data at nodes.  Note: you can insert a ChangeFieldBasis module upstream to push element data to the nodes.");
      return;
    }
  }

  MatrixHandle matrix_input_handle;
  if (get_input_handle("Optional Isovalue", matrix_input_handle, false) )
  {
    if( matrix_input_handle->nrows() == 1 &&
        matrix_input_handle->ncols() == 1 )
    {
      if( matrix_input_handle->get(0, 0) != gui_iso_value_.get() )
      {
        gui_iso_value_.set(matrix_input_handle->get(0, 0));
        gui_iso_value_.reset();

        inputs_changed_ = true;
      }
    }
    else
    {
      error("Input matrix contains more than one value.");
      return;
    }
  }

  if( inputs_changed_ ||
      gui_iso_value_.changed( true ) ||
      gui_lte_.changed( true ) ||
      !oport_cached("Clipped") ||
      !oport_cached("Mapping") )
  {
    update_state(Executing);
  
    MatrixHandle matrix_output_handle;
    FieldHandle field_output_handle; 
    
    algo_.set_scalar("isovalue",gui_iso_value_.get());
    std::string method = "greaterthan";
    if (gui_lte_.get()) method = "lessthan";
    algo_.set_option("method",method);
    if(!(algo_.run(field_input_handle,field_output_handle,matrix_output_handle))) return;
  
    send_output_handle("Clipped",  field_output_handle);
    send_output_handle("Mapping", matrix_output_handle);
  }
}

} // End namespace SCIRun

