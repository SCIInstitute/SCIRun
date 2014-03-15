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

#include <Modules/Legacy/Fields/JoinFields.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Legacy/Fields/MergeFields/JoinFieldsAlgo.h>
#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToPointCloudMeshAlgo.h>
#endif

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;

  //private:
  //  GuiInt    guiforcepointcloud_;
  //  
  //  SCIRunAlgo::ConvertMeshToPointCloudMeshAlgo calgo_;

ModuleLookupInfo JoinFields::staticInfo_("JoinFields", "NewField", "SCIRun");

JoinFields::JoinFields() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputFields);
  INITIALIZE_PORT(OutputField);
}

void JoinFields::setStateDefaults()
{
  setStateBoolFromAlgo(JoinFieldsAlgo::MergeElems);
  setStateBoolFromAlgo(JoinFieldsAlgo::MergeNodes);
  setStateBoolFromAlgo(JoinFieldsAlgo::MatchNodeValues);
  setStateBoolFromAlgo(JoinFieldsAlgo::MakeNoData);
  setStateDoubleFromAlgo(JoinFieldsAlgo::Tolerance);
  //guiforcepointcloud_(get_ctx()->subVar("force-pointcloud"),0),
}

void JoinFields::execute()
{
  auto fields = getRequiredDynamicInputs(InputFields);

  /*if (inputs_changed_ ||  guitolerance_.changed() ||
      guimergenodes_.changed() || guiforcepointcloud_.changed() ||
      guimatchval_.changed() || guimeshonly_.changed() || guimergeelems_.changed() ||
      !oport_cached("Output Field"))*/
  if (needToExecute())
  {
    update_state(Executing);

    double tolerance = 0.0;
    bool   mergenodes = false;
    bool   mergeelems = false;
    bool   forcepointcloud = false;
    bool   matchval = false;
    bool   meshonly = false;
    
    setAlgoBoolFromState(JoinFieldsAlgo::MergeElems);
    setAlgoBoolFromState(JoinFieldsAlgo::MatchNodeValues);
    setAlgoBoolFromState(JoinFieldsAlgo::MergeNodes);
    setAlgoBoolFromState(JoinFieldsAlgo::MakeNoData);
    setAlgoDoubleFromState(JoinFieldsAlgo::Tolerance);

#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
    if (guiforcepointcloud_.get()) 
      forcepointcloud = true;
#endif
        
    auto output = algo().run_generic(make_input((InputFields, fields)));
    auto outputField = output.get<Field>(Core::Algorithms::AlgorithmParameterName(OutputField));

#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
    if (forcepointcloud)
    {
      if(!(calgo_.run(output,output))) 
        return;
    }
#endif

    sendOutput(OutputField, outputField);
  }
}
