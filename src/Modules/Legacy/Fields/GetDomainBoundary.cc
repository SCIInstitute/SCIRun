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
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Scalar.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Modules/Legacy/Fields/GetDomainBoundary.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
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
  // Define local handles of data objects:
  auto ifield = getRequiredInput(InputField);
  auto elemLink = getOptionalInput(ElemLink);
  auto minValue = getOptionalInput(MinValue);
  auto maxValue = getOptionalInput(MaxValue);
  
#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
  if (ifield->is_property("ElemLink"))
  {
    ifield->get_property("ElemLink", elemLink);
  }
#endif
  
#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
  // Only reexecute if the input changed. SCIRun uses simple scheduling
  // that executes every module downstream even if no data has changed:    
  if (inputs_changed_ || guiminrange_.changed() ||  
      guimaxrange_.changed() || guivalue_.changed() || 
      guiuserange_.changed() || guiincludeouterboundary_.changed() ||
      guiinnerboundaryonly_.changed() || guinoinnerboundary_.changed() || 
      guidisconnect_.changed() || !oport_cached("Field"))
#endif
  if (needToExecute())
  {
    update_state(Executing);

    if (minValue && *minValue)
    {
      double minrange = (*minValue)->value();
      get_state()->setValue(GetDomainBoundaryAlgo::MinRange, minrange);
      get_state()->setValue(GetDomainBoundaryAlgo::Domain, minrange);  //??
    }
    if (maxValue && *maxValue)
    {
      double maxrange = (*maxValue)->value();
      get_state()->setValue(GetDomainBoundaryAlgo::MaxRange, maxrange);
    }

    auto state = get_state();

    setAlgoIntFromState(GetDomainBoundaryAlgo::MinRange);
    setAlgoIntFromState(GetDomainBoundaryAlgo::MaxRange);
    setAlgoIntFromState(GetDomainBoundaryAlgo::Domain);
    setAlgoBoolFromState(GetDomainBoundaryAlgo::UseRange);
    setAlgoBoolFromState(GetDomainBoundaryAlgo::AddOuterBoundary);
    setAlgoBoolFromState(GetDomainBoundaryAlgo::InnerBoundaryOnly);
    setAlgoBoolFromState(GetDomainBoundaryAlgo::NoInnerBoundary);
    setAlgoBoolFromState(GetDomainBoundaryAlgo::DisconnectBoundaries);

    if (!state->getValue(GetDomainBoundaryAlgo::UseRange).getBool())
    {
      int guiValue = state->getValue(GetDomainBoundaryAlgo::Domain).getInt();
      algo().set(GetDomainBoundaryAlgo::UseRange, true);
      algo().set(GetDomainBoundaryAlgo::MinRange, guiValue);
      algo().set(GetDomainBoundaryAlgo::MaxRange, guiValue);
    }
      
    auto output = algo().run_generic(make_input((InputField, ifield)(ElemLink, optionalAlgoInput(elemLink))));
    
    sendOutputFromAlgorithm(BoundaryField, output);
  }
}
