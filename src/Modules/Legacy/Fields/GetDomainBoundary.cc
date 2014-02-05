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

#include <Core/Algorithms/Legacy/Fields/DomainFields/GetDomainBoundaryAlgo.h>
//#include <Core/Algorithms/Converter/ConverterAlgo.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

#include <Modules/Legacy/Fields/GetDomainBoundary.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;

GetDomainBoundary::GetDomainBoundary() 
  : Module(ModuleLookupInfo("GetDomainBoundary", "NewField", "SCIRun"))
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(BoundaryField);
  INITIALIZE_PORT(MinValue);
  INITIALIZE_PORT(MaxValue);
  INITIALIZE_PORT(ElemLink);
}

void GetDomainBoundary::setStateDefaults()
{
  setStateIntFromAlgo(GetDomainBoundaryAlgo::MinRange);
  setStateIntFromAlgo(GetDomainBoundaryAlgo::MaxRange);
  setStateIntFromAlgo(GetDomainBoundaryAlgo::Domain);

  setStateBoolFromAlgo(GetDomainBoundaryAlgo::UseRange);
  setStateBoolFromAlgo(GetDomainBoundaryAlgo::AddOuterBoundary);
  setStateBoolFromAlgo(GetDomainBoundaryAlgo::InnerBoundaryOnly);
  setStateBoolFromAlgo(GetDomainBoundaryAlgo::NoInnerBoundary);
  setStateBoolFromAlgo(GetDomainBoundaryAlgo::DisconnectBoundaries);
}

void GetDomainBoundary::execute()
{

}

#if SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
namespace SCIRun {

class GetDomainBoundary : public Module {
  public:
    GetDomainBoundary(GuiContext*);
    virtual ~GetDomainBoundary() {}
    virtual void execute();
    
  private:
    GuiInt    guiuserange_;
    GuiDouble guiminrange_;
    GuiDouble guimaxrange_;
    GuiInt    guiusevalue_;
    GuiDouble guivalue_;
    GuiInt    guiincludeouterboundary_;
    GuiInt    guiinnerboundaryonly_;
    GuiInt    guinoinnerboundary_;
    GuiInt    guidisconnect_;
    
    SCIRunAlgo::GetDomainBoundaryAlgo algo_;
};


DECLARE_MAKER(GetDomainBoundary)
GetDomainBoundary::GetDomainBoundary(GuiContext* ctx)
  : Module("GetDomainBoundary", ctx, Source, "NewField", "SCIRun"),
    guiuserange_(get_ctx()->subVar("userange"),255.0),
    guiminrange_(get_ctx()->subVar("minrange"),0.0),
    guimaxrange_(get_ctx()->subVar("maxrange"),0.0),
    guiusevalue_(get_ctx()->subVar("usevalue"),0),
    guivalue_(get_ctx()->subVar("value"),1.0),
    guiincludeouterboundary_(get_ctx()->subVar("includeouterboundary"),1),
    guiinnerboundaryonly_(get_ctx()->subVar("innerboundaryonly"),0),    
    guinoinnerboundary_(get_ctx()->subVar("noinnerboundary"),0),    
    guidisconnect_(get_ctx()->subVar("disconnect"),0)    
{
  algo_.set_progress_reporter(this);
}

void GetDomainBoundary::execute()
{
  // Define local handles of data objects:
  FieldHandle ifield, ofield;
  MatrixHandle ElemLink;
  MatrixHandle MinValue, MaxValue;
 
  // Get the new input data: 
  if(!(get_input_handle("Field",ifield,true))) return;
  get_input_handle("MinValue/Value",MinValue,false);
  get_input_handle("MaxValue",MaxValue,false);
  get_input_handle("ElemLink",ElemLink,false);
  
  if (ifield->is_property("ElemLink"))
  {
    ifield->get_property("ElemLink",ElemLink);
  }
  
  // Only reexecute if the input changed. SCIRun uses simple scheduling
  // that executes every module downstream even if no data has changed:    
  if (inputs_changed_ || guiminrange_.changed() ||  
      guimaxrange_.changed() || guivalue_.changed() || 
      guiuserange_.changed() || guiincludeouterboundary_.changed() ||
      guiinnerboundaryonly_.changed() || guinoinnerboundary_.changed() || 
      guidisconnect_.changed() || !oport_cached("Field"))
  {
    update_state(Executing);

    if (MinValue.get_rep())
    {
      double minrange;
      if (MinValue->get_data_size() > 0) minrange = MinValue->get(0,0);
      guiminrange_.set(minrange);
      guivalue_.set(minrange);
      get_ctx()->reset();
    }
    if (MaxValue.get_rep())
    {
      double maxrange;
      if (MaxValue->get_data_size() > 0) maxrange = MaxValue->get(0,0);
      guimaxrange_.set(maxrange);
      get_ctx()->reset();
    }


    algo_.set_int("min_range",static_cast<int>(guiminrange_.get()));
    algo_.set_int("max_range",static_cast<int>(guimaxrange_.get()));
    algo_.set_int("domain",static_cast<int>(guiminrange_.get()));
    algo_.set_bool("use_range",guiuserange_.get());
    algo_.set_bool("add_outer_boundary",guiincludeouterboundary_.get());
    algo_.set_bool("inner_boundary_only",guiinnerboundaryonly_.get());
    algo_.set_bool("no_inner_boundary",guinoinnerboundary_.get());
    algo_.set_bool("disconnect_boundaries",guidisconnect_.get());

    if (guiusevalue_.get())
    {
      algo_.set_bool("use_range",true);
      algo_.set_int("min_range",static_cast<int>(guivalue_.get()));
      algo_.set_int("max_range",static_cast<int>(guivalue_.get()));    
    }
      
    // The innerworks of the module:
    if(!(algo_.run(ifield,ElemLink,ofield))) return;
    
    // send new output if there is any:        
    send_output_handle("Field", ofield);
  }
}

} // End namespace SCIRun


#endif