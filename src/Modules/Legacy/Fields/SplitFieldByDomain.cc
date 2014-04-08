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

#include <Modules/Legacy/Fields/SplitFieldByDomain.h>
#include <Core/Algorithms/Legacy/Fields/DomainFields/SplitFieldByDomain.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
//#include <Core/Datatypes/MatrixTypeConverter.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
//using namespace SCIRun::Core::Algorithms;
//using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;

ModuleLookupInfo SplitFieldByDomain::staticInfo_("SplitFieldByDomain", "NewField", "SCIRun");


//GuiInt  gui_sort_by_size_;
  //  GuiDouble gui_sort_ascending_;
    //SCIRunAlgo::SplitFieldByDomainAlgo algo_;

SplitFieldByDomain::SplitFieldByDomain() : Module(staticInfo_, false)
//  gui_sort_by_size_(get_ctx()->subVar("sort-by-size"),0),
//  gui_sort_ascending_(get_ctx()->subVar("sort-ascending"),0)
{
}

void SplitFieldByDomain::setStateDefaults()
{
  auto state = get_state();
  state->setValue()
}

void
SplitFieldByDomain::execute()
{
  auto input = getRequiredInput(InputField);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (inputs_changed_ || !oport_cached("All Fields") || !oport_cached("Field1")
    || !oport_cached("Field2") || !oport_cached("Field3") || !oport_cached("Field4")
    || !oport_cached("Field5") || !oport_cached("Field6") || !oport_cached("Field7") 
    || !oport_cached("Field8")  ||gui_sort_by_size_.changed()
    || gui_sort_ascending_.changed())
#endif
  if (needToExecute())
  {
    update_state(Executing);

    algo().set("sort_by_size", gui_sort_by_size_.get());
    algo().set("sort_ascending", gui_sort_ascending_.get());

    if(!(algo_.run(input,output))) 
      return;
    
    BundleHandle boutput(new Bundle);
    for (size_t j=0; j< output.size(); j++)
    {
      std::ostringstream oss;
      oss << "Field" << j;
      boutput->setField(oss.str(),output[j]);
    }

    std::vector<FieldHandle> output;
    FieldHandle nofield;
    
    send_output_handle("All Fields",boutput);

    if (output.size() > 0) 
      send_output_handle("Field1",output[0]); 
    else 
      send_output_handle("Field1",nofield); 

    if (output.size() > 1) 
      send_output_handle("Field2",output[1]);
    else 
      send_output_handle("Field2",nofield); 
    
    if (output.size() > 2) 
      send_output_handle("Field3",output[2]);
    else 
      send_output_handle("Field3",nofield); 
    
    if (output.size() > 3) 
      send_output_handle("Field4",output[3]);
    else 
      send_output_handle("Field4",nofield); 
    
    if (output.size() > 4) 
      send_output_handle("Field5",output[4]);
    else 
      send_output_handle("Field5",nofield); 
    
    if (output.size() > 5) 
      send_output_handle("Field6",output[5]);
    else 
      send_output_handle("Field6",nofield); 
    
    if (output.size() > 6) 
      send_output_handle("Field7",output[6]);
    else 
      send_output_handle("Field7",nofield); 
    
    if (output.size() > 7) 
      send_output_handle("Field8",output[7]);
    else 
      send_output_handle("Field8",nofield); 
  }
}
