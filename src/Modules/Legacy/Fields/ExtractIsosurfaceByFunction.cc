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


///    @file    ExtractIsosurfaceByFunction.h
///    @author  Michael Callahan &&
///             Allen Sanderson
///             SCI Institute
///             University of Utah
///    @date    March 2006

#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Algorithms/Fields/Mapping/ApplyMappingMatrix.h>
#include <Core/Algorithms/Fields/FieldData/ConvertFieldBasisType.h>
#include <Core/Algorithms/Fields/MarchingCubes/MarchingCubes.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixOperations.h>

#include <Core/Util/StringUtil.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {

class SCISHARE ExtractIsosurfaceByFunction : public Module {
  public:
    ExtractIsosurfaceByFunction(GuiContext *context);
    virtual ~ExtractIsosurfaceByFunction() {}

    virtual void execute();
    virtual void presave();
    virtual void post_read();

  private:
    FieldHandle field_transformed_handle_;

    /// GUI variables
    GuiString  gui_function_;

    GuiDouble  gui_slice_value_min_;
    GuiDouble  gui_slice_value_max_;
    GuiDouble  gui_slice_value_;
    GuiDouble  gui_slice_value_typed_;
    GuiInt     gui_slice_value_quantity_;
    GuiString  gui_slice_quantity_range_;
    GuiString  gui_slice_quantity_clusive_;
    GuiDouble  gui_slice_quantity_min_;
    GuiDouble  gui_slice_quantity_max_;
    GuiString  gui_slice_quantity_list_;
    GuiString  gui_slice_value_list_;
    GuiString  gui_slice_matrix_list_;
    GuiInt     gui_use_algorithm_;
    GuiString  gui_active_slice_value_selection_tab_;
    GuiString  gui_active_tab_;

    //gui_update_type_ must be declared after gui_iso_value_max_ which is
    //traced in the tcl code. If gui_update_type_ is set to Auto having it
    //last will prevent the net from executing when it is instantiated.
    GuiString  gui_update_type_;

    /// status variables
    std::vector< double > slicevals_;

    bool old_version_;
    bool mapping_;
};


DECLARE_MAKER(ExtractIsosurfaceByFunction)


ExtractIsosurfaceByFunction::ExtractIsosurfaceByFunction(GuiContext *context)
  : Module("ExtractIsosurfaceByFunction", context, Filter, "NewField", "SCIRun"),

    field_transformed_handle_(0),

    gui_function_(get_ctx()->subVar("function"), "RESULT = sqrt(X*X + Y*Y);"),
    gui_slice_value_min_(context->subVar("slice-value-min"),  0.0),
    gui_slice_value_max_(context->subVar("slice-value-max"), 99.0),
    gui_slice_value_(context->subVar("slice-value"), 0.0),
    gui_slice_value_typed_(context->subVar("slice-value-typed"), 0.0),
    gui_slice_value_quantity_(context->subVar("slice-value-quantity"), 1),
    gui_slice_quantity_range_(context->subVar("quantity-range"), "field"),
    gui_slice_quantity_clusive_(context->subVar("quantity-clusive"), "exclusive"),
    gui_slice_quantity_min_(context->subVar("quantity-min"),   0),
    gui_slice_quantity_max_(context->subVar("quantity-max"), 100),
    gui_slice_quantity_list_(context->subVar("quantity-list"), ""),
    gui_slice_value_list_(context->subVar("slice-value-list"), "No values present."),
    gui_slice_matrix_list_(context->subVar("matrix-list"),"No matrix present - execution needed."),
    gui_use_algorithm_(context->subVar("algorithm"), 0),
    gui_active_slice_value_selection_tab_(context->subVar("active-slice-value-selection-tab"), "0"),
    gui_active_tab_(context->subVar("active_tab"), "0"),
    gui_update_type_(context->subVar("update_type"), "On Release"),
    old_version_(false),
    mapping_(false)
{
}


void
ExtractIsosurfaceByFunction::execute()
{
  /// Get the input field handle from the port.
  FieldHandle input_field_handle;
  FieldHandle field_output_handle;
  std::vector<MatrixHandle> matrices;

  get_input_handle( "Input Field", input_field_handle, true);

  /// Current
  int basis_order = input_field_handle->vfield()->basis_order();
  if( basis_order != 0 && basis_order != 1)
  {
    error( "Can not extract an isosurface with higher order data by function, because currently it is not poosible to build a mapping matrix for the data which is needed." );
    return;
  }
  get_dynamic_input_handles("Array",matrices,false);

  update_state(Executing);

  // update gFunction_ before get.
  TCLInterface::execute(get_id() + " update_text");

  /////////////////////////////////////////////////////////////////////////////
  /// Transform Section

  if( gui_function_.changed( true ) )
    inputs_changed_ = true;

  /// Check to see if the input field has changed.
  if( inputs_changed_ || !field_transformed_handle_.get_rep() )
  {
    NewArrayMathEngine engine;
    engine.set_progress_reporter(this);

    // Create the DATA object for the function
    // DATA is the data on the field
    if( input_field_handle->basis_order() == 1 )
    {
      if(!(engine.add_input_fielddata("DATA", input_field_handle ))) return;
      ///-----------------------
      // Backwards compatibility with version 3.0.2
      if(!(engine.add_input_fielddata("v0",input_field_handle))) return;
      if(!(engine.add_input_fielddata("v",input_field_handle))) return;
      ///-----------------------
    }


    // Create the POS, X,Y,Z, data location objects.

    if(!(engine.add_input_fielddata_location("POS", input_field_handle, 1))) return;
    if(!(engine.add_input_fielddata_coordinates("X","Y","Z", input_field_handle, 1 ))) return;

    ///-----------------------
    // Backwards compatibility with version 3.0.2
    if(!(engine.add_input_fielddata_coordinates("x","y","z", input_field_handle, 1))) return;
    ///-----------------------

    // Create the ELEMENT object describing element properties
    if(!(engine.add_input_fielddata_element("ELEMENT", input_field_handle,1 ))) return;

    std::string function = gui_function_.get();
    bool has_RESULT = true;
    if (function.find("RESULT") != std::string::npos)
    {
      if(!(engine.add_output_fielddata("RESULT",input_field_handle,1,"double"))) return;
    }
    else
    {
      ///-----------------------
      // Backwards compatibility with version 3.0.2
      if(!(engine.add_output_fielddata("result",input_field_handle,1,"double"))) return;
      has_RESULT = false;
      ///-----------------------
    }

    size_t numinputs = matrices.size();
    if (numinputs > 23)
    {
      error("This module cannot handle more than 23 input matrices.");
      return;
    }

    // Add an object for getting the index and size of the array.

    if(!(engine.add_index("INDEX"))) return;
    if(!(engine.add_size("SIZE"))) return;

    // Loop through all matrices and add them to the engine as well
    char mname = 'A';
    std::string matrixname("A");

    for (size_t p = 0; p < numinputs; p++)
    {
      if (matrices[p].get_rep() == 0)
      {
        error("No matrix was found on input port.");
        return;
      }

      matrixname[0] = mname++;
      if (!(engine.add_input_matrix(matrixname,matrices[p]))) return;
    }


    if(!(engine.add_expressions(function))) return;

    // Actual engine call, which does the dynamic compilation, the creation of the
    // code for all the objects, as well as inserting the function and looping
    // over every data point

    if (!(engine.run()))
    {
      ///-----------------------
      // Backwards compatibility with version 3.0.2
      if (old_version_)
      {
        error("This module does not fully support backwards compatibility:");
        error("C++/C functions are not supported in by this module anymore.");
        error("Please review documentation to explore available functionality and grammar of this module.");
        error("We are sorry for this inconvenience, but we do not longer support dynamically compiling in SCIRun.");
      }
      ///-----------------------

      return;
    }

    // Get the result from the engine
    if(has_RESULT)
    {
      engine.get_field("RESULT",field_transformed_handle_);
    }
    else
    {
      engine.get_field("result",field_transformed_handle_);
    }

    if (!(field_transformed_handle_->vfield()->is_scalar()))
    {
      error("Transformed field does not contain scalar data.");
      return;
    }

    double minval, maxval;
    field_transformed_handle_->vfield()->minmax(minval,maxval);

    // Check to see if the gui min max are different than the field.
    if( gui_slice_value_min_.get() != minval ||
        gui_slice_value_max_.get() != maxval )
    {

      gui_slice_value_min_.set( minval );
      gui_slice_value_max_.set( maxval );

      gui_slice_value_min_.reset();
      gui_slice_value_max_.reset();

      std::ostringstream str;
      str << get_id() << " set_min_max ";
      TCLInterface::execute(str.str().c_str());
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Isosurface Section

  std::vector<double> slicevals(0);

  double qmin = gui_slice_value_min_.get();
  double qmax = gui_slice_value_max_.get();

  if (gui_active_slice_value_selection_tab_.get() == "0")
  { // slider / typed
    const double val = gui_slice_value_.get();
    const double valTyped = gui_slice_value_typed_.get();
    if (val != valTyped)
    {
      warning("Typed slice value "+to_string(valTyped)+" was out of range.  Using slice value "+to_string(val)+" instead.");
      gui_slice_value_typed_.set(val);
    }
    if ( qmin <= val && val <= qmax )
    {
      slicevals.push_back(val);
    }
    else
    {
      error("Typed slice value out of range -- skipping slice surfacing.");
      return;
    }
  }
  else if (gui_active_slice_value_selection_tab_.get() == "1")
  { // quantity
    int num = gui_slice_value_quantity_.get();

    if (num < 1)
    {
      error("Slice surface quantity must be at least one -- skipping slice surfacing.");
      return;
    }

    std::string range = gui_slice_quantity_range_.get();

    if (range == "manual")
    {
      qmin = gui_slice_quantity_min_.get();
      qmax = gui_slice_quantity_max_.get();
    } // else we're using "field" and qmax and qmin were set above

    if (qmin >= qmax)
    {
      error("Can't use quantity tab if the minimum and maximum are the same.");
      return;
    }

    std::string clusive = gui_slice_quantity_clusive_.get();
    std::ostringstream str;
    str << get_id() << " set-slice-quant-list \"";

    if (clusive == "exclusive")
    {
      // if the min - max range is 2 - 4, and the user requests 3 slicevals,
      // the code below generates 2.333, 3.0, and 3.666 -- which is nice
      // since it produces evenly spaced slices in torroidal data.

      double di=(qmax - qmin)/(double)num;
      for (int i=0; i<num; i++)
      {
        slicevals.push_back(qmin + ((double)i+0.5)*di);
        str << " " << slicevals[i];
      }
    }
    else if (clusive == "inclusive")
    {
      // if the min - max range is 2 - 4, and the user requests 3 slicevals,
      // the code below generates 2.0, 3.0, and 4.0.

      double di=(qmax - qmin)/(double)(num-1.0);
      for (int i=0; i<num; i++)
      {
        slicevals.push_back(qmin + ((double)i*di));
        str << " " << slicevals[i];
      }
    }

    str << "\"";

    TCLInterface::execute(str.str().c_str());

  }
  else if (gui_active_slice_value_selection_tab_.get() == "2")
  { // list
     multiple_from_string(gui_slice_value_list_.get(),slicevals);
  }
  else if (gui_active_slice_value_selection_tab_.get() == "3")
  { // matrix

    MatrixHandle matrix_input_handle;
    get_input_handle( "Optional Slice values", matrix_input_handle, true );

    std::ostringstream str;
    str << get_id() << " set-slice-matrix-list \"";

    for (index_type i=0; i < matrix_input_handle->nrows(); i++)
    {
      for (index_type j=0; j < matrix_input_handle->ncols(); j++)
      {
        slicevals.push_back(matrix_input_handle->get(i, j));
        str << " " << slicevals[i];
      }
    }

    str << "\"";
    TCLInterface::execute(str.str().c_str());
  }
  else
  {
    error("Bad active_slice_value_selection_tab value");
    return;
  }

  // See if any of the slicevals have changed.
  if( slicevals_.size() != slicevals.size())
  {
    slicevals_.resize( slicevals.size() );
    inputs_changed_ = true;
  }

  for( size_t i=0; i<slicevals.size(); i++ )
  {
    if( slicevals_[i] != slicevals[i] )
    {
      slicevals_[i] = slicevals[i];
      inputs_changed_ = true;
    }
  }

  if( inputs_changed_ || !oport_cached("Output Field") ||
      (mapping_ != oport_connected("Mapping")) )
  {
    SCIRunAlgo::MarchingCubesAlgo mc;
    mc.set_progress_reporter(this);
    mc.set_bool("build_field",true);
    if (input_field_handle->vfield()->basis_order() == 0)
    {
      mc.set_bool("build_node_interpolant",false);
      mc.set_bool("build_elem_interpolant",true);
    }
    else
    {
      mc.set_bool("build_node_interpolant",true);
      mc.set_bool("build_elem_interpolant",false);
    }

    FieldHandle   field_sliced_handle = 0;
    MatrixHandle matrix_sliced_handle = 0;

    if(!(mc.run(field_transformed_handle_,slicevals,
              field_sliced_handle,matrix_sliced_handle)))
    {
      error("MarchingCubes algorithm failed");
      return;
    }

    if (field_sliced_handle.get_rep())
    {
      std::string fldname;
      if (!(field_transformed_handle_->get_property("name", fldname)))
      {
        fldname = "Sliced Surface";
      }
      field_sliced_handle->set_property("name", fldname, false);
    }
    else
    {
      /// No field.
      warning( "No slices found" );
      return;
    }

    // If the basis is not constant make it so.
    if( input_field_handle->vfield()->basis_order() == 0 )
    {
      SCIRunAlgo::ConvertFieldBasisTypeAlgo algo;
      algo.set_progress_reporter(this);
      algo.set_option("basistype", "constant");

      FieldHandle tmp_field_handle = field_sliced_handle;
      field_sliced_handle = 0;
      MatrixHandle mapping_matrix_handle;

      if (!(algo.run(tmp_field_handle,
               field_sliced_handle,
               mapping_matrix_handle))) return;
    }

    if( !field_sliced_handle.get_rep() )
    {
      error( "Can not find the sliced field for clipping" );
      return;
    }
    else if( !matrix_sliced_handle.get_rep() )
    {
      error( "Can not find the matrix for clipping" );
      return;
    }
    else
    {
      remark( "Sliced field and matrix for clipping is present" );
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Apply Mapping Section 1

    /// Apply the matrix to the sliced data.
    FieldHandle field_apply_mapping_handle;

    SCIRunAlgo::ApplyMappingMatrixAlgo mapping_algo;
    mapping_algo.set_progress_reporter(this);

    if(!(mapping_algo.run(input_field_handle,field_sliced_handle,
                     matrix_sliced_handle,field_apply_mapping_handle)))
    {
      error( "Can not find the first mapped field for the second mapping" );
      return;
    }

    field_output_handle = field_apply_mapping_handle;
    if (oport_connected("Mapping"))
    {
      MatrixHandle Mapping = matrix_sliced_handle;
      send_output_handle("Mapping",Mapping);
    }

    mapping_ = oport_connected("Mapping");

  }
  /// Send the data downstream.
  send_output_handle( "Output Field", field_output_handle, true );
}

void
ExtractIsosurfaceByFunction::presave()
{
  // update gui_function_ before saving.
  TCLInterface::execute(get_id() + " update_text");
}

void
ExtractIsosurfaceByFunction::post_read()
{
  // Compatibility with version 3.0.2
  std::string version = get_old_version();
  if (version != get_version())
  {
    old_version_ = true;
  }

  const std::string modName = get_ctx()->getfullname() + "-";
  std::string val;

  ///-----------------------
  // Backwards compatibility with intermediate version....

  if( TCLInterface::get(modName+"mode", val, get_ctx()) )
  {
    TCLInterface::set(modName+"method", val, get_ctx());
  }
  ///-----------------------

  if (old_version_)
  {
    std::string function;
    TCLInterface::get(modName+"function", function, get_ctx());
    if (function.find("return") != std::string::npos)
    {
      // Undo damage intermediate version
      // At some point in time it was required to add a return statement
      // to complete the function, this is imcompatible with newer and older
      // modules, hence this will undo the damage
      // This should fix most cases by stripping the return clause and adding
      // RESULT = expression;
      std::string::size_type loc = function.find("return");
      function = function.substr(0,loc)+function.substr(loc+6);
      function = "RESULT = "+ function;

      TCLInterface::set(modName+"function", function, get_ctx());
    }
  }
}



} // End namespace SCIRun
