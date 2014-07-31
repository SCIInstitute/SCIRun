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

#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Bundle.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/BundlePort.h>
#include <Dataflow/Network/Ports/StringPort.h>

using namespace SCIRun;

/// @class InsertStringIntoBundle
/// @brief This module inserts a string object into a bundle. 

class InsertStringsIntoBundle : public Module {
public:
  InsertStringsIntoBundle(GuiContext*);
  virtual void execute();
  
private:
  GuiString     guistring1name_;
  GuiString     guistring2name_;
  GuiString     guistring3name_;
  GuiString     guistring4name_;
  GuiString     guistring5name_;
  GuiString     guistring6name_;
  
  GuiInt        guireplace1_;
  GuiInt        guireplace2_;
  GuiInt        guireplace3_;
  GuiInt        guireplace4_;
  GuiInt        guireplace5_;
  GuiInt        guireplace6_;
  
  GuiString     guibundlename_;
};


DECLARE_MAKER(InsertStringsIntoBundle)

InsertStringsIntoBundle::InsertStringsIntoBundle(GuiContext* ctx)
  : Module("InsertStringsIntoBundle", ctx, Filter, "Bundle", "SCIRun"),
    guistring1name_(get_ctx()->subVar("string1-name"), "string1"),
    guistring2name_(get_ctx()->subVar("string2-name"), "string2"),
    guistring3name_(get_ctx()->subVar("string3-name"), "string3"),
    guistring4name_(get_ctx()->subVar("string4-name"), "string4"),
    guistring5name_(get_ctx()->subVar("string5-name"), "string5"),
    guistring6name_(get_ctx()->subVar("string6-name"), "string6"),
    guireplace1_(get_ctx()->subVar("replace1"),1),
    guireplace2_(get_ctx()->subVar("replace2"),1),
    guireplace3_(get_ctx()->subVar("replace3"),1),
    guireplace4_(get_ctx()->subVar("replace4"),1),
    guireplace5_(get_ctx()->subVar("replace5"),1),
    guireplace6_(get_ctx()->subVar("replace6"),1),
    guibundlename_(get_ctx()->subVar("bundlename",false))
{
}

void InsertStringsIntoBundle::execute()
{
  BundleHandle  handle;
  StringHandle string1, string2, string3;
  StringHandle string4, string5, string6;

  get_input_handle("bundle",handle,false);
  get_input_handle("string1",string1,false);
  get_input_handle("string2",string2,false);
  get_input_handle("string3",string3,false);
  get_input_handle("string4",string4,false);
  get_input_handle("string5",string5,false);
  get_input_handle("string6",string6,false);

  
  if (inputs_changed_ || guistring1name_.changed() || 
      guistring2name_.changed() || guistring3name_.changed() || 
      guistring4name_.changed() || guistring5name_.changed() || 
      guistring6name_.changed() ||
      guireplace1_.changed() || guireplace2_.changed() ||
      guireplace3_.changed() || guireplace4_.changed() ||
      guireplace5_.changed() || guireplace6_.changed() ||       
      guibundlename_.changed() || !oport_cached("bundle"))
  {
    update_state(Executing);
  
    std::string string1Name = guistring1name_.get();
    std::string string2Name = guistring2name_.get();
    std::string string3Name = guistring3name_.get();
    std::string string4Name = guistring4name_.get();
    std::string string5Name = guistring5name_.get();
    std::string string6Name = guistring6name_.get();
    std::string bundlename = guibundlename_.get();

    if (handle.get_rep())
    {
      handle.detach();
    }
    else
    {
      handle = new Bundle();
      if (handle.get_rep() == 0)
      {
        error("Could not allocate new bundle");
        return;
      }
    }

    if (string1.get_rep()
        &&(guireplace1_.get()||!(handle->isString(string1Name)))) 
      handle->setString(string1Name,string1);

    if (string2.get_rep()
        &&(guireplace2_.get()||!(handle->isString(string2Name)))) 
      handle->setString(string2Name,string2);

    if (string3.get_rep()
        &&(guireplace3_.get()||!(handle->isString(string3Name)))) 
      handle->setString(string3Name,string3);

    if (string4.get_rep()
        &&(guireplace4_.get()||!(handle->isString(string4Name)))) 
      handle->setString(string4Name,string4);
                      
    if (string5.get_rep()
        &&(guireplace5_.get()||!(handle->isString(string5Name)))) 
      handle->setString(string5Name,string5);

    if (string6.get_rep()
        &&(guireplace6_.get()||!(handle->isString(string6Name)))) 
      handle->setString(string6Name,string6);

    if (bundlename != "")
    {
      handle->set_property("name",bundlename,false);
    }

    send_output_handle("bundle",handle);
  }
}

