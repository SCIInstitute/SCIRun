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
///@author
///   Allen R. Sanderson
///   Michael Callahan
///   SCI Institute
///   University of Utah
///@date  February 2006
///


#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Field.h>
#include <Core/Parser/ArrayMathEngine.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

namespace SCIRun {

///@class QueryFieldData
///@brief Query field data using a function.

class QueryFieldData : public Module
{
  public:
    QueryFieldData(GuiContext* ctx);
    virtual ~QueryFieldData() {}

    virtual void execute();
    virtual void tcl_command(GuiArgs& args, void* userdata);

    virtual void presave();
    virtual void post_read();

  protected:
    GuiString gui_output_data_type_;
    GuiString gui_function_;
    GuiInt    gui_number_of_datasets_;
    GuiInt    gui_count_;

    FieldHandle  cache_;
    MatrixHandle count_;

    bool old_version_;
};


DECLARE_MAKER(QueryFieldData)


QueryFieldData::QueryFieldData(GuiContext* ctx)
  : Module("QueryFieldData", ctx, Filter,"ChangeFieldData", "SCIRun","2.0"),
    gui_output_data_type_(get_ctx()->subVar("outputdatatype"), "port 0 input"),
    gui_function_(get_ctx()->subVar("function"), "result = v0 + v1 + v2;"),
    gui_number_of_datasets_(get_ctx()->subVar("number_of_datasets"), 1),
    gui_count_(get_ctx()->subVar("count", 0), 0),
    old_version_(false)
{
  count_ = new DenseMatrix(0.0);
}

void
QueryFieldData::execute()
{

  StringHandle string_handle;

  if (get_input_handle( "Function", string_handle, false ))
  {
    gui_function_.set( string_handle->get() );
    gui_function_.reset();

    TCLInterface::execute(get_id() + " set_text");
  }

  // Update gui_function_ before the get.
  TCLInterface::execute(get_id() + " update_text");

  MatrixHandle matrix_handle;

  if (get_input_handle( "Number of Datasets", matrix_handle, false ))
  {
    if( (matrix_handle->nrows() == 1 && matrix_handle->ncols() == 1) )
    {
      gui_number_of_datasets_.set( static_cast<int>( matrix_handle->get(0,0) ));
      gui_number_of_datasets_.reset();
    }
    else
    {
      error( "Input matrix is not a 1x1 matrix" );
      return;
    }
  }

  // If the count is less than the number of data sets then assume
  // something needs to be done.
  if( gui_count_.get() < gui_number_of_datasets_.get() )
  {
    // Check to see if any gui inputs have changed. If they have reset the
    // query.
    if( gui_output_data_type_.changed( true ) ||
        gui_function_.changed( true ) ||
        gui_number_of_datasets_.changed( true ) ||
        !oport_cached("Query Result") )
    {
      inputs_changed_ = true;
      cache_ = 0;

      gui_count_.set(0);
      gui_count_.reset();
      count_->put(0,0,0.0);
    }

    // Get the input field(s).
    std::vector<FieldHandle> field_input_handles;

    if (!get_dynamic_input_handles("Input Field", field_input_handles, true ) )
      return;

    // Check to see if the input field(s) has changed.
    if( inputs_changed_ )
    {
      update_state(Executing);

      if (cache_.get_rep() == 0)
      {
        // Create a dummy field with one value as default
        FieldInformation fi(field_input_handles[0]);
        FieldInformation fid("PointCloudMesh",1,fi.get_data_type());

	if (gui_output_data_type_.get() == "Vector") fid.make_vector();
	else if (gui_output_data_type_.get() == "Tensor") fid.make_tensor();
	else if (gui_output_data_type_.get() == "char") fid.make_char();
	else if (gui_output_data_type_.get() == "unsigned char") fid.make_unsigned_char();
	else if (gui_output_data_type_.get() == "short")  fid.make_short();
	else if (gui_output_data_type_.get() == "unsigned short") fid.make_unsigned_short();
	else if (gui_output_data_type_.get() == "int")  fid.make_int();
	else if (gui_output_data_type_.get() == "unsigned int") fid.make_unsigned_int();
	else if (gui_output_data_type_.get() == "long")  fid.make_long();
	else if (gui_output_data_type_.get() == "unsigned long") fid.make_unsigned_long();
	else if (gui_output_data_type_.get() == "long long")  fid.make_long_long();
	else if (gui_output_data_type_.get() == "unsigned long long") fid.make_unsigned_long_long();
	else if (gui_output_data_type_.get() == "float")  fid.make_float();
	else if (gui_output_data_type_.get() == "double")  fid.make_double();
	else {
	  // Use the input field ...
	}

        cache_ = CreateField(fid);
        cache_->vmesh()->add_point(Point(0,0,0));
        cache_->vfield()->resize_values();
      }

      count_->put(0,0,static_cast<double>(gui_count_.get()));


      std::string format = gui_output_data_type_.get();

      if (format == "") format = "double";
      if (format == "port 0 input") format = "same as input";
      std::string function = gui_function_.get();

      NewArrayMathEngine engine;
      engine.set_progress_reporter(this);

      if(!(engine.add_input_fielddata("DATA",field_input_handles[0]))) return;
      if(!(engine.add_input_fielddata("v",field_input_handles[0]))) return;
      if(!(engine.add_input_fielddata_coordinates("X","Y","Z",field_input_handles[0]))) return;
      if(!(engine.add_input_fielddata_coordinates("x","y","z",field_input_handles[0]))) return;

      for(size_t j=0; j<field_input_handles.size(); j++)
      {
        std::string name1 = "DATA"+to_string(j+1);
        if(!(engine.add_input_fielddata(name1,field_input_handles[j]))) return;

        std::string namex = "X"+to_string(j+1);
        std::string namey = "Y"+to_string(j+1);
        std::string namez = "Z"+to_string(j+1);
        if(!(engine.add_input_fielddata_coordinates(namex,namey,namez,field_input_handles[j]))) return;

        ///-----------------------
        // Backwards compatibility with intermediate version between 3.0.2 and 4.0
        std::string name2 = "v"+to_string(j);
        if(!(engine.add_input_fielddata(name2,field_input_handles[j]))) return;

        std::string namex2 = "x"+to_string(j);
        std::string namey2 = "y"+to_string(j);
        std::string namez2 = "z"+to_string(j);
        if(!(engine.add_input_fielddata_coordinates(namex2,namey2,namez2,field_input_handles[j]))) return;
        ///-----------------------
      }

      // Caching method
      if(!(engine.add_input_matrix("COUNT",count_))) return;
      if(!(engine.add_input_fielddata("RESULT",cache_))) return;
      ///-----------------------
      // Backwards compatibility with intermediate version between 3.0.2 and 4.0
      if(!(engine.add_input_matrix("count",count_))) return;
      if(!(engine.add_input_fielddata("result",cache_))) return;
      ///-----------------------

      int basis_order = field_input_handles[0]->vfield()->basis_order();

      bool has_RESULT = true;
      if (function.find("RESULT") != std::string::npos)
      {
        if(!(engine.add_output_fielddata("RESULT",field_input_handles[0],basis_order,format))) return;
      }
      else
      {
        ///-----------------------
        // Backwards compatibility with version 3.0.2
        if(!(engine.add_output_fielddata("result",field_input_handles[0],basis_order,format))) return;
        has_RESULT = false;
        ///-----------------------
      }

      // Add an object for getting the index and size of the array.

      if(!(engine.add_index("INDEX"))) return;
      if(!(engine.add_size("SIZE"))) return;

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
          error("We are sorry for this inconvenience, but we do not longer support dynamically compiling code in SCIRun.");
        }
        ///-----------------------

        return;
      }

      // Get the result from the engine
      FieldHandle ofield;
      if(has_RESULT)
      {
        engine.get_field("RESULT",ofield);
      }
      else
      {
        ///-----------------------
        // Backwards compatibility with version 3.0.2
        engine.get_field("result",ofield);
        ///-----------------------
      }

      // send new output if there is any:
      send_output_handle("Query Result", ofield, true);


      int count = static_cast<int>(count_->get(0,0)); count++;
      gui_count_.set(count);
      gui_count_.reset();
      cache_ = ofield;
    }
  }
}


void QueryFieldData::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2){
    args.error("QueryFieldData needs a minor command");
    return;
  }

  if (args[1] == "clear")
  {
    gui_count_.set(0);
    gui_count_.reset();

    cache_ = 0;
  }
  else
  {
    Module::tcl_command(args, userdata);
  }
}


void
QueryFieldData::presave()
{
  // update gui_function_ before saving.
  TCLInterface::execute(get_id() + " update_text");
}


void
QueryFieldData::post_read()
{
  if (get_version() != get_old_version())
  {
    old_version_ = true;
  }

  if (gui_output_data_type_.get() == "input")
  {
    gui_output_data_type_.set("port 0 input");
    gui_output_data_type_.reset();
  }
}

} // End namespace SCIRun
