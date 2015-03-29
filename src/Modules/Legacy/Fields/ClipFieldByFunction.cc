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
#include <Core/Datatypes/Field.h>
#include <Core/Parser/ArrayMathEngine.h>

#include <Core/Algorithms/Fields/ClipMesh/ClipMeshBySelection.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/StringPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

/// @class ClipFieldByFunction
/// @brief This module selects a subset of a field using a function. 

class ClipFieldByFunction : public Module {
  public:
    ClipFieldByFunction(GuiContext*);

    virtual void execute();
    
  private:
    GuiString guifunction_;  
    GuiString guimethod_;
    SCIRunAlgo::ClipMeshBySelectionAlgo clipping_algo_;
};


DECLARE_MAKER(ClipFieldByFunction)
ClipFieldByFunction::ClipFieldByFunction(GuiContext* ctx)
  : Module("ClipFieldByFunction", ctx, Source, "NewField", "SCIRun","2.0"),
    guifunction_(get_ctx()->subVar("function"),"DATA < 0"),
    guimethod_(get_ctx()->subVar("method"),"onenode")
{
  clipping_algo_.set_progress_reporter(this);
}


void ClipFieldByFunction::execute()
{
  // Define local handles of data objects:
  FieldHandle field;
  StringHandle func;
  std::vector<MatrixHandle> matrices;

  // Get the new input data:  
  get_input_handle("Field",field,true);
  if (get_input_handle("Function",func,false))
  {
    if (func.get_rep())
    {
      guifunction_.set(func->get());
      get_ctx()->reset();  
    }
  }  
  get_dynamic_input_handles("Array",matrices,false);

  // Only do work if needed:
  if (inputs_changed_ || guifunction_.changed() || guimethod_.changed() ||
      !oport_cached("Field") || !oport_cached("Mapping"))
  {  
    update_state(Executing);
    // Get number of matrix ports with data (the last one is always empty)
    size_t numinputs = matrices.size();
    if (numinputs > 23)
    {
      error("This module cannot handle more than 23 input matrices.");
      return;
    }

    std::string method = guimethod_.get();
    if (field->vmesh()->is_pointcloudmesh()) method = "element";
    
    int basis_order = 1;
    if (method == "element") basis_order = 0;
    
    if (field->vmesh()->is_empty())
    {
      warning("Input mesh does not contain any nodes: copying input to output");
      MatrixHandle mapping;
      send_output_handle("Field", field);
      send_output_handle("Mapping", mapping);
      return;
    }
    
    int field_basis_order = field->vfield()->basis_order();

    NewArrayMathEngine engine;
    engine.set_progress_reporter(this);
    
    // Create the DATA object for the function
    // DATA is the data on the field
    if (basis_order == field_basis_order)
    {
      if(!(engine.add_input_fielddata("DATA",field))) return;
    }
    else
    {
      std::ostringstream oss;
      oss << "Input field's basis order (";
      if (field_basis_order == 0)
        oss << "constant";
      else if (field_basis_order == 1)
        oss << "linear";
      else
        oss << "other";
      oss << ") does not match basis order supported by method '" << method << "'. Using DATA as a variable will not be supported.";
      warning(oss.str());
    }
    
    // Create the POS, X,Y,Z, data location objects.  

    if(!(engine.add_input_fielddata_location("POS",field,basis_order))) return;
    if(!(engine.add_input_fielddata_coordinates("X","Y","Z",field,basis_order))) return;

    //-----------------------
    // Backwards compatibility with version 3.0.2
    if(!(engine.add_input_fielddata_coordinates("x","y","z",field,basis_order))) return;
    //-----------------------

    // Create the ELEMENT object describing element properties
    if(!(engine.add_input_fielddata_element("ELEMENT",field,basis_order))) return;

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
    
    std::string function = guifunction_.get();
    if (function.find("RESULT") == std::string::npos && function.find("result") == std::string::npos)
    {
      function = std::string("RESULT = ") + function;   
    }
    
    bool has_RESULT = true;
    if (function.find("RESULT") != std::string::npos)
    {
      if(!(engine.add_output_fielddata("RESULT",field,basis_order,"char"))) return;
    }

    // Add an object for getting the index and size of the array.

    if(!(engine.add_index("INDEX"))) return;
    if(!(engine.add_size("SIZE"))) return;


    // Define the function we are using for clipping:

    if(!(engine.add_expressions(function))) return;

    // Actual engine call, which does the dynamic compilation, the creation of the
    // code for all the objects, as well as inserting the function and looping 
    // over every data point

    if (!(engine.run()))
    {
      return;
    }

    // Get the result from the engine
    FieldHandle  sfield, ofield;    
    MatrixHandle mapping;
    if(has_RESULT)
    {
      engine.get_field("RESULT",sfield);
    }

    clipping_algo_.set_option("method",method);
    if(!(clipping_algo_.run(field,sfield,ofield,mapping))) return;

    send_output_handle("Field", ofield);
    send_output_handle("Mapping", mapping);
  }
}

} // End namespace ModelCreation


