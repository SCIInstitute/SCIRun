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

// Include all code for the dynamic engine
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Parser/ArrayMathEngine.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/StringPort.h>

namespace SCIRun {

/// @class CalculateMeshNodes
/// @brief Calculate new positions for the node locations of the mesh.

class CalculateMeshNodes : public Module 
{
  public:
    CalculateMeshNodes(GuiContext*);
    virtual ~CalculateMeshNodes() {}
    virtual void execute();

    virtual void tcl_command(GuiArgs&, void*);
    virtual void presave();
    virtual void post_read();
    

  private:
    GuiString guifunction_;     // function code
    
    bool old_version_;
};


DECLARE_MAKER(CalculateMeshNodes)
CalculateMeshNodes::CalculateMeshNodes(GuiContext* ctx)
  : Module("CalculateMeshNodes", ctx, Source, "ChangeMesh", "SCIRun"),
  guifunction_(get_ctx()->subVar("function")),
  old_version_(false)
{
}

void CalculateMeshNodes::execute()
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

  TCLInterface::eval(get_id()+" update_text");

  if (inputs_changed_ || guifunction_.changed() || !oport_cached("Field"))
  {
    update_state(Executing);
    // Get number of matrix ports with data (the last one is always empty)
    size_t numinputs = matrices.size();
    if (numinputs > 23)
    {
      error("This module cannot handle more than 23 input matrices.");
      return;
    }
 
    NewArrayMathEngine engine;
    engine.set_progress_reporter(this);
 
    FieldInformation fi(field);
    if (field->vfield()->basis_order() == 1 || fi.is_pointcloudmesh())
    {
    
      // Create the DATA object for the function
      // DATA is the data on the field
      if(!(engine.add_input_fielddata("DATA",field))) return; 

      //--------------------------
      //Backwards compatiblity with 3.0.2
      if(!(engine.add_input_fielddata("v",field))) return; 
      //--------------------------

    }
    else
    {
      remark("DATA is not defined as data is not located on the nodes");
    }
    
    // Create the POS, X,Y,Z, data location objects.  

    if(!(engine.add_input_fieldnodes("POS",field))) return;
    if(!(engine.add_input_fieldnodes_coordinates("X","Y","Z",field))) return;
    
    //--------------------------
    //Backwards compatiblity with 3.0.2
    if(!(engine.add_input_fieldnodes_coordinates("x","y","z",field))) return;
    //--------------------------
    

    // Create the ELEMENT object describing element properties
    if (field->vfield()->basis_order() == 1)
    {
      if(!(engine.add_input_fielddata_element("ELEMENT",field,field->vfield()->basis_order()))) return;
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

    std::string function = guifunction_.get();
    bool has_NEWPOS = true;
    if (function.find("NEWPOS") != std::string::npos)
    {
      if(!(engine.add_output_fieldnodes("NEWPOS",field))) return;
    }
    else
    {
      ///-----------------------
      // Backwards compatibility with version 3.0.2
      if(!(engine.add_output_fieldnodes("result",field))) return;
      has_NEWPOS = false;
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
    if(has_NEWPOS)
    {
      engine.get_field("NEWPOS",ofield);
    }
    else
    {
      engine.get_field("result",ofield);    
    }
    
    // send new output if there is any: 
    send_output_handle("Field", ofield);
  }
}


void
CalculateMeshNodes::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("ComputeDataField needs a minor command");
    return;
  }

  if( args[1] == "gethelp" )
  {
    return;
  }
  else
  {
    Module::tcl_command(args, userdata);
  }
}

void
CalculateMeshNodes::presave()
{
  // update gui_function_ before saving.
  TCLInterface::execute(get_id() + " update_text");
}



void
CalculateMeshNodes::post_read()
{
  // Compatibility with version 3.0.2
  std::string old_module_name = get_old_modulename();
  if (old_module_name == "TransformMeshWithFunction")
  {
    old_version_ = true;

    const std::string modName = get_ctx()->getfullname() + "-";
    std::string val;
  
    ///-----------------------
    // Backwards compatibility with intermediate version

    if( TCLInterface::get(modName+"function", val, get_ctx()) )
    {
      // Upgrade function so it will still function
      // In the old code, a casting operator needed to be
      // entered, this is ugly and the new code does not need
      // it, it actually does not recognize it, and hence 
      // we need to remove it
      std::string::size_type castop = val.find("(Point)");
      if (castop != std::string::npos)
      {
        // Remove casting operator
        val = val.substr(0,castop)+val.substr(castop+7);
      }
        // Set the current values for the new names
      TCLInterface::set(modName+"function", val, get_ctx());
    }
    ///---------------------
  }
  else if (old_module_name == "MapDataToMeshCoord" || old_module_name == "MapFieldDataToNodeCoordinate")
  {
    old_version_ = true;
    const std::string modName = get_ctx()->getfullname() + "-";
    std::string coord;

    if( TCLInterface::get(modName+"coord", coord, get_ctx()) )
    {
      // Upgrade functions to new module
      if (coord == "0") TCLInterface::set(modName+"function", "NEWPOS = POS + Vector(DATA,0,0);", get_ctx());
      if (coord == "1") TCLInterface::set(modName+"function", "NEWPOS = POS + Vector(0,DATA,0);", get_ctx());
      if (coord == "2") TCLInterface::set(modName+"function", "NEWPOS = POS + Vector(0,0,DATA);", get_ctx());
      if (coord == "3") TCLInterface::set(modName+"function", "NEWPOS = POS + DATA*normal(ELEMENT);", get_ctx());
    }
  }

}


} // End namespace SCIRun


