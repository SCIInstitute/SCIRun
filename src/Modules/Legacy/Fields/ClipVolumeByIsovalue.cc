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


///@author
///   Allen R. Sanderson
///   Michael Callahan
///   Department of Computer Science
///   University of Utah
///@date  March 2001

#include <Modules/Legacy/Fields/ClipVolumeByIsovalue.h>
#include <Core/Algorithms/Legacy/Fields/ClipMesh/ClipMeshByIsovalue.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 #include <Core/Algorithms/Fields/ClipMesh/ClipMeshByIsovalue.h>
 #include <iostream>
#endif

using namespace SCIRun;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Algorithms;

/// @class ClipVolumeByIsovalue
/// @brief Clip out parts of a field.

MODULE_INFO_DEF(ClipVolumeByIsovalue, NewField, SCIRun)

ClipVolumeByIsovalue::ClipVolumeByIsovalue() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(Isovalue);
  INITIALIZE_PORT(OutputField);
}

void ClipVolumeByIsovalue::setStateDefaults()
{
  auto state = get_state();
  setStateDoubleFromAlgo(ClipMeshByIsovalueAlgo::ScalarIsoValue);
  setStateIntFromAlgo(ClipMeshByIsovalueAlgo::LessThanIsoValue);
}


void ClipVolumeByIsovalue::execute()
{
  auto input = getRequiredInput(InputField);
  auto isovalueOption = getOptionalInput(Isovalue);

  if (needToExecute())
  {
    double iso=0;

    // GUI inputs have less priority than isovalue - second module input
    iso = get_state()->getValue(ClipMeshByIsovalueAlgo::ScalarIsoValue).toDouble();

    if (isovalueOption && *isovalueOption && !(*isovalueOption)->empty())
    {
      iso = (*isovalueOption)->get(0,0);
    }

    algo().set(ClipMeshByIsovalueAlgo::ScalarIsoValue, iso);

    auto gui_LessThanIsoValue = (get_state()->getValue(ClipMeshByIsovalueAlgo::LessThanIsoValue)).toInt();

    if (gui_LessThanIsoValue==-1)
       gui_LessThanIsoValue=0;

    algo().set(ClipMeshByIsovalueAlgo::LessThanIsoValue, gui_LessThanIsoValue);

    auto output = algo().run(withInputData((InputField, input)));

    sendOutputFromAlgorithm(OutputField, output);
  }

  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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
    algo_.setOption("method",method);
    if(!(algo_.run(field_input_handle,field_output_handle,matrix_output_handle))) return;

    send_output_handle("Clipped",  field_output_handle);
    send_output_handle("Mapping", matrix_output_handle);
  }
 #endif
}
