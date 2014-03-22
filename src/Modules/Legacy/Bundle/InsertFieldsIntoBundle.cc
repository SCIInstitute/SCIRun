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

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Bundle.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/BundlePort.h>
#include <Dataflow/Network/Ports/FieldPort.h>

using namespace SCIRun;

class InsertFieldsIntoBundle : public Module {
public:
  InsertFieldsIntoBundle(GuiContext*);
  virtual void execute();

private:
  GuiString     guifield1name_;
  GuiString     guifield2name_;
  GuiString     guifield3name_;
  GuiString     guifield4name_;
  GuiString     guifield5name_;
  GuiString     guifield6name_;
  
  GuiInt        guireplace1_;
  GuiInt        guireplace2_;
  GuiInt        guireplace3_;
  GuiInt        guireplace4_;
  GuiInt        guireplace5_;
  GuiInt        guireplace6_;
  
  GuiString     guibundlename_;
};


DECLARE_MAKER(InsertFieldsIntoBundle)
  InsertFieldsIntoBundle::InsertFieldsIntoBundle(GuiContext* ctx)
    : Module("InsertFieldsIntoBundle", ctx, Filter, "Bundle", "SCIRun"),
      guifield1name_(get_ctx()->subVar("field1-name"), "field1"),
      guifield2name_(get_ctx()->subVar("field2-name"), "field2"),
      guifield3name_(get_ctx()->subVar("field3-name"), "field3"),
      guifield4name_(get_ctx()->subVar("field4-name"), "field4"),
      guifield5name_(get_ctx()->subVar("field5-name"), "field5"),
      guifield6name_(get_ctx()->subVar("field6-name"), "field6"),
      guireplace1_(get_ctx()->subVar("replace1"),1),
      guireplace2_(get_ctx()->subVar("replace2"),1),
      guireplace3_(get_ctx()->subVar("replace3"),1),
      guireplace4_(get_ctx()->subVar("replace4"),1),
      guireplace5_(get_ctx()->subVar("replace5"),1),
      guireplace6_(get_ctx()->subVar("replace6"),1),
      guibundlename_(get_ctx()->subVar("bundlename",false), "")
{
}

void InsertFieldsIntoBundle::execute()
{
  BundleHandle  handle;
  FieldHandle field1, field2, field3;
  FieldHandle field4, field5, field6;

  get_input_handle("bundle",handle,false);
  get_input_handle("field1",field1,false);
  get_input_handle("field2",field2,false);
  get_input_handle("field3",field3,false);
  get_input_handle("field4",field4,false);
  get_input_handle("field5",field5,false);
  get_input_handle("field6",field6,false);
  
  if (inputs_changed_ || guifield1name_.changed() || 
      guifield2name_.changed() || guifield3name_.changed() ||
      guifield4name_.changed() || guifield5name_.changed() ||
      guifield6name_.changed() || 
      guireplace1_.changed() || guireplace2_.changed() ||
      guireplace3_.changed() || guireplace4_.changed() ||
      guireplace5_.changed() || guireplace6_.changed() ||
      guibundlename_.changed() || !oport_cached("bundle"))
  {
    update_state(Executing);
  
    std::string field1name = guifield1name_.get();
    std::string field2name = guifield2name_.get();
    std::string field3name = guifield3name_.get();
    std::string field4name = guifield4name_.get();
    std::string field5name = guifield5name_.get();
    std::string field6name = guifield6name_.get();
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
    
    if (field1.get_rep()
        &&(guireplace1_.get()||!(handle->isField(field1name)))) 
      handle->setField(field1name,field1);

    if (field2.get_rep()
        &&(guireplace2_.get()||!(handle->isField(field2name)))) 
      handle->setField(field2name,field2);
      
    if (field3.get_rep()
        &&(guireplace3_.get()||!(handle->isField(field3name)))) 
      handle->setField(field3name,field3);

    if (field4.get_rep()
        &&(guireplace4_.get()||!(handle->isField(field4name)))) 
      handle->setField(field4name,field4);

    if (field5.get_rep()
        &&(guireplace5_.get()||!(handle->isField(field5name)))) 
      handle->setField(field5name,field5);

    if (field6.get_rep()
        &&(guireplace6_.get()||!(handle->isField(field6name)))) 
      handle->setField(field6name,field6);
 
    if (bundlename != "")
    {
      handle->set_property("name",bundlename,false);
    }

    send_output_handle("bundle", handle);
  }
}

