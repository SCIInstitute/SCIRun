
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

#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Bundle.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/BundlePort.h>
#include <Dataflow/Network/Ports/StringPort.h>

using namespace SCIRun;

/// @class GetStringsFromBundle
/// @brief This module retrieves a string object from a bundle.

class GetStringsFromBundle : public Module {
public:
  GetStringsFromBundle(GuiContext*);
  virtual void execute();
  
private:
  GuiString             guistring1name_;
  GuiString             guistring2name_;
  GuiString             guistring3name_;
  GuiString             guistring4name_;
  GuiString             guistring5name_;
  GuiString             guistring6name_;
  GuiString             guistrings_;
};


DECLARE_MAKER(GetStringsFromBundle)

GetStringsFromBundle::GetStringsFromBundle(GuiContext* ctx)
  : Module("GetStringsFromBundle", ctx, Filter, "Bundle", "SCIRun"),
    guistring1name_(get_ctx()->subVar("string1-name"), "string1"),
    guistring2name_(get_ctx()->subVar("string2-name"), "string2"),
    guistring3name_(get_ctx()->subVar("string3-name"), "string3"),
    guistring4name_(get_ctx()->subVar("string4-name"), "string4"),
    guistring5name_(get_ctx()->subVar("string5-name"), "string5"),
    guistring6name_(get_ctx()->subVar("string6-name"), "string6"),
    guistrings_(get_ctx()->subVar("string-selection",false), "")
{
}

void
GetStringsFromBundle::execute()
{
  // Define input handle:
  BundleHandle handle;
  
  // Get data from input port:
  get_input_handle("bundle",handle,true);
  
  if (inputs_changed_ || guistring1name_.changed() || 
      guistring2name_.changed() || guistring3name_.changed() || 
      guistring4name_.changed() || guistring5name_.changed() ||
      guistring6name_.changed() ||!oport_cached("bundle") || 
      !oport_cached("string1") || !oport_cached("string2") || 
      !oport_cached("string3") || !oport_cached("string4") ||
      !oport_cached("string5") || !oport_cached("string6"))
  {
    update_state(Executing);

    StringHandle fhandle;
    std::string string1name = guistring1name_.get();
    std::string string2name = guistring2name_.get();
    std::string string3name = guistring3name_.get();
    std::string string4name = guistring4name_.get();
    std::string string5name = guistring5name_.get();
    std::string string6name = guistring6name_.get();
    std::string stringlist;
        
    int numStrings = handle->numStrings();
    for (int p = 0; p < numStrings; p++)
    {
      stringlist += "{" + handle->getStringName(p) + "} ";
    }

    guistrings_.set(stringlist);
    get_ctx()->reset();
 
    // Send string1 if we found one that matches the name:
    if (handle->isString(string1name))
    {
      fhandle = handle->getString(string1name);
      send_output_handle("string1",fhandle);
    }


    // Send string2 if we found one that matches the name:
    if (handle->isString(string2name))
    {
      fhandle = handle->getString(string2name);
      send_output_handle("string2",fhandle);
    } 

    
    // Send string3 if we found one that matches the name:
    if (handle->isString(string3name))
    {
      fhandle = handle->getString(string3name);
      send_output_handle("string3",fhandle);
    } 

    
    // Send string4 if we found one that matches the name:
    if (handle->isString(string4name))
    {
      fhandle = handle->getString(string4name);
      send_output_handle("string4",fhandle);
    }


    // Send string5 if we found one that matches the name:
    if (handle->isString(string5name))
    {
      fhandle = handle->getString(string5name);
      send_output_handle("string5",fhandle);
    } 

    
    // Send string6 if we found one that matches the name:
    if (handle->isString(string6name))
    {
      fhandle = handle->getString(string6name);
      send_output_handle("string6",fhandle);
    } 

    send_output_handle("bundle",handle);    
  }      
}



