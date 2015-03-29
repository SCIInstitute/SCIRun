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

// Include all code for the dynamic engine
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Field.h>
#include <Core/Parser/ArrayMathEngine.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/StringPort.h>

namespace SCIRun {

/// @class CalculateFieldData3
/// @brief Calculate new data for the field given a function that uses field 
/// data, node location and element properties as input, with a second and 
/// third input port for two additional fields. 

class CalculateFieldData3 : public Module 
{
  public:
    CalculateFieldData3(GuiContext*);
    virtual ~CalculateFieldData3() {}

    virtual void execute();
    virtual void presave();
    virtual void post_read();
    virtual void tcl_command(GuiArgs& args, void* userdata);
    
  private:
    GuiString guifunction_;     // function code
    GuiString guiformat_;       // scalar, vector, or tensor ?
    GuiInt    gui_cache_;
    GuiInt    gui_count_;
    
  private:
    bool old_version_;   // Is this converted from an old version

    MatrixHandle count_;
    FieldHandle  cache_;    
};


DECLARE_MAKER(CalculateFieldData3)
CalculateFieldData3::CalculateFieldData3(GuiContext* ctx)
  : Module("CalculateFieldData3", ctx, Source, "ChangeFieldData", "SCIRun"),
  guifunction_(get_ctx()->subVar("function")),
  guiformat_(get_ctx()->subVar("format")),
  gui_cache_(get_ctx()->subVar("cache"), 0),
  gui_count_(get_ctx()->subVar("count", 0), 0),
  old_version_(false)  
{
  count_ = new DenseMatrix(0.0);
}

void CalculateFieldData3::execute()
{
  // Define input handles:
  FieldHandle field, field2, field3;
  StringHandle func;
  std::vector<MatrixHandle> matrices;
  
  // Get data from input ports:
  get_input_handle("Field1",field,true);
  get_input_handle("Field2",field2,false);
  get_input_handle("Field3",field3,false);
  if (get_input_handle("Function",func,false))
  {
    if (func.get_rep())
    {
      guifunction_.set(func->get());
      get_ctx()->reset();  
    }
  }
  get_dynamic_input_handles("Array",matrices,false);

  // Force TCL to update variables:
  TCLInterface::eval(get_id()+" update_text");
  
  if (inputs_changed_ || guifunction_.changed() || 
      guiformat_.changed() || !oport_cached("Field"))
  {
    // Inform module that execution started
    update_state(Executing);

    // If not caching set the field and count to zero.
    if( !(gui_cache_.get()) ) 
    {
      cache_ = 0;
      gui_count_.set(0);
      gui_count_.reset();
    }
    
    if (cache_.get_rep() == 0)
    {
      // Create a dummy field with one value as default
      FieldInformation fi(field);
      FieldInformation fid("PointCloudMesh",1,fi.get_data_type());
      if (guiformat_.get() != "Same as Input") fid.set_data_type(guiformat_.get());
      cache_ = CreateField(fid);
      cache_->vmesh()->add_point(Point(0,0,0));
      cache_->vfield()->resize_values();
    }
    
    count_->put(0,0,static_cast<double>(gui_count_.get()));
    
    // Get number of matrix ports with data (the last one is always empty)
    size_t numinputs = matrices.size();
    
    if (numinputs > 23)
    {
      error("This module cannot handle more than 23 input matrices.");
      return;
    }

    NewArrayMathEngine engine;
    engine.set_progress_reporter(this);

    // Create the DATA object for the function
    // DATA is the data on the field
    if(!(engine.add_input_fielddata("DATA",field))) return;
    if(!(engine.add_input_fielddata("DATA1",field))) return;

    //-----------------------
    // Backwards compatibility with version 3.0.2
    if(!(engine.add_input_fielddata("v0",field))) return;
    if(!(engine.add_input_fielddata("v",field))) return;
    ///-----------------------

    // Create the POS, X,Y,Z, data location objects.  

    if(!(engine.add_input_fielddata_location("POS",field))) return;
    if(!(engine.add_input_fielddata_location("POS1",field))) return;
    if(!(engine.add_input_fielddata_coordinates("X","Y","Z",field))) return;
    if(!(engine.add_input_fielddata_coordinates("X1","Y1","Z1",field))) return;

    //-----------------------
    // Backwards compatibility with version 3.0.2
    if(!(engine.add_input_fielddata_coordinates("x","y","z",field))) return;
    //-----------------------

    // Create the ELEMENT object describing element properties
    if(!(engine.add_input_fielddata_element("ELEMENT",field))) return;
    if(!(engine.add_input_fielddata_element("ELEMENT1",field))) return;

    // Caching method
    if(!(engine.add_input_matrix("COUNT",count_))) return;
    if(!(engine.add_input_fielddata("RESULT",cache_))) return;
    //-----------------------
    // Backwards compatibility with intermediate version between 3.0.2 and 4.0
    if(!(engine.add_input_matrix("count",count_))) return;
    if(!(engine.add_input_fielddata("result",cache_))) return;    
    //-----------------------

    if (field2.get_rep())
    {
      if(!(engine.add_input_fielddata("DATA2",field2))) return;
      if(!(engine.add_input_fielddata("v1",field2))) return;

      // Create the POS, X,Y,Z, data location objects.  

      if(!(engine.add_input_fielddata_location("POS2",field2))) return;
      if(!(engine.add_input_fielddata_coordinates("X2","Y2","Z2",field2))) return;

      // Create the ELEMENT object describing element properties
      if(!(engine.add_input_fielddata_element("ELEMENT2",field2))) return;
    }

    if (field3.get_rep())
    {
      if(!(engine.add_input_fielddata("DATA3",field3))) return;
      if(!(engine.add_input_fielddata("v2",field3))) return;

      // Create the POS, X,Y,Z, data location objects.  

      if(!(engine.add_input_fielddata_location("POS3",field3))) return;
      if(!(engine.add_input_fielddata_coordinates("X3","Y3","Z3",field3))) return;

      // Create the ELEMENT object describing element properties
      if(!(engine.add_input_fielddata_element("ELEMENT3",field3))) return;
    }
    
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

    int basis_order = field->vfield()->basis_order();
    
    std::string format = guiformat_.get();
    if (format == "") format = "double";

    std::string function = guifunction_.get();    
    bool has_RESULT = true;
    if (function.find("RESULT") != std::string::npos)
    {
      if(!(engine.add_output_fielddata("RESULT",field,basis_order,format))) return;
    }
    else
    {
      //-----------------------
      // Backwards compatibility with version 3.0.2
      if(!(engine.add_output_fielddata("result",field,basis_order,format))) return;
      has_RESULT = false;
      //-----------------------
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
      //-----------------------
      // Backwards compatibility with version 3.0.2
      if (old_version_)
      {
        error("This module does not fully support backwards compatibility:");
        error("C++/C functions are not supported in by this module anymore.");
        error("Please review documentation to explore available functionality and grammar of this module.");
        error("We are sorry for this inconvenience, but we do not longer support dynamically compiling code in SCIRun.");
      }
      //-----------------------
      
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
      //-----------------------
      // Backwards compatibility with version 3.0.2
      engine.get_field("result",ofield);
      //-----------------------
    }

    // send new output if there is any: 
    send_output_handle("Field", ofield);
    
    if( (gui_cache_.get()) )
    {
      int count = static_cast<int>(count_->get(0,0)); count++;
      gui_count_.set(count);
      gui_count_.reset();
      cache_ = ofield;
    }
    else
    {
      cache_ = 0;
    }

  }
}

void 
CalculateFieldData3::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("CalculateFieldData needs a minor command");
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
CalculateFieldData3::presave()
{
  // update gui_function_ before saving.
  TCLInterface::execute(get_id() + " update_text");
}

void
CalculateFieldData3::post_read()
{
  // Compatibility with version 3.0.2
  std::string old_module_name = get_old_modulename();
  if (old_module_name == "CalculateFieldDataCompiled3" || old_module_name == "TransformData3")
  {
    old_version_ = true;
  }

  const std::string modName = get_ctx()->getfullname() + "-";
  std::string val;
  
  //-----------------------
  // Backwards compatibility with intermediate version

  if( TCLInterface::get(modName+"outputdatatype", val, get_ctx()) )
  {
    if (val == "input 0 port") val = "same as input";
    if (val == "input 1 port") val = "same as input";

      // Set the current values for the new names
    TCLInterface::set(modName+"format", val, get_ctx());
  }
  //-----------------------

}

} // End namespace SCIRun

