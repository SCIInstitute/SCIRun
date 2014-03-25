
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

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#include <Core/Datatypes/Bundle.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/BundlePort.h>
#include <Dataflow/Network/Ports/FieldPort.h>

using namespace SCIRun;

class GetFieldsFromBundle : public Module {

public:
  GetFieldsFromBundle(GuiContext*);
  virtual void execute();
  
private:
  GuiString             guifield1name_;
  GuiString             guifield2name_;
  GuiString             guifield3name_;
  GuiString             guifield4name_;
  GuiString             guifield5name_;
  GuiString             guifield6name_;
  GuiString             guifields_;
};


DECLARE_MAKER(GetFieldsFromBundle)
  GetFieldsFromBundle::GetFieldsFromBundle(GuiContext* ctx)
    : Module("GetFieldsFromBundle", ctx, Filter, "Bundle", "SCIRun"),
      guifield1name_(get_ctx()->subVar("field1-name"), "field1"),
      guifield2name_(get_ctx()->subVar("field2-name"), "field2"),
      guifield3name_(get_ctx()->subVar("field3-name"), "field3"),
      guifield4name_(get_ctx()->subVar("field4-name"), "field4"),
      guifield5name_(get_ctx()->subVar("field5-name"), "field5"),
      guifield6name_(get_ctx()->subVar("field6-name"), "field6"),
      guifields_(get_ctx()->subVar("field-selection",false), "")
{
}



void GetFieldsFromBundle::execute()
{
  // Define input handle:
  BundleHandle handle;
  
  // Get data from input port:
  if (!(get_input_handle("bundle",handle,true))) return;
  
  if (inputs_changed_ || guifield1name_.changed() || 
      guifield2name_.changed() || guifield3name_.changed() ||
      guifield4name_.changed() || guifield5name_.changed() ||
      guifield6name_.changed() || !oport_cached("bundle") || 
      !oport_cached("field1")  || !oport_cached("field2") || 
      !oport_cached("field3")  || !oport_cached("field4") ||
      !oport_cached("field5")  || !oport_cached("field6"))
  {
    update_state(Executing);

    FieldHandle fhandle;
    std::string field1name = guifield1name_.get();
    std::string field2name = guifield2name_.get();
    std::string field3name = guifield3name_.get();
    std::string field4name = guifield4name_.get();
    std::string field5name = guifield5name_.get();
    std::string field6name = guifield6name_.get();
    std::string fieldlist;
    
    int numFields = handle->numFields();
    for (int p = 0; p < numFields; p++)
    {
      fieldlist += "{" + handle->getFieldName(p) + "} ";
    }

    guifields_.set(fieldlist);
    get_ctx()->reset();  
  
    // Send field1 if we found one that matches the name:
    if (handle->isField(field1name))
    {
      fhandle = handle->getField(field1name);
      send_output_handle("field1",fhandle);
    } 
    
    // Send field2 if we found one that matches the name:
    if (handle->isField(field2name))
    {
      fhandle = handle->getField(field2name);
      send_output_handle("field2",fhandle);
    } 
    
    // Send field3 if we found one that matches the name:
    if (handle->isField(field3name))
    {
      fhandle = handle->getField(field3name);
      send_output_handle("field3",fhandle);
    } 

    // Send field4 if we found one that matches the name:
    if (handle->isField(field4name))
    {
      fhandle = handle->getField(field4name);
      send_output_handle("field4",fhandle);
    } 
    
    // Send field5 if we found one that matches the name:
    if (handle->isField(field5name))
    {
      fhandle = handle->getField(field5name);
      send_output_handle("field5",fhandle);
    } 
    
    // Send field6 if we found one that matches the name:
    if (handle->isField(field6name))
    {
      fhandle = handle->getField(field6name);
      send_output_handle("field6",fhandle);
    } 
        
    send_output_handle("bundle",handle);
  }
}





#endif