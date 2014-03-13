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
//#include <Core/Algorithms/Fields/MergeFields/JoinFields.h>
//#include <Core/Algorithms/Fields/ConvertMeshType/ConvertMeshToPointCloudMesh.h>

#include <vector>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;

  //private:
  //  GuiInt    guiclear_;
  //  GuiDouble guitolerance_;
  //  GuiInt    guimergenodes_;
  //  GuiInt    guimergeelems_;
  //  GuiInt    guiforcepointcloud_;
  //  GuiInt    guimatchval_;
  //  GuiInt    guimeshonly_;
  //  
  //  SCIRunAlgo::JoinFieldsAlgo algo_;
  //  SCIRunAlgo::ConvertMeshToPointCloudMeshAlgo calgo_;

ModuleLookupInfo JoinFields::staticInfo_("JoinFields", "NewField", "SCIRun");

JoinFields::JoinFields() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
}

void JoinFields::setStateDefaults()
{
  //guiclear_(get_ctx()->subVar("clear", false), 0),  
  //guitolerance_(get_ctx()->subVar("tolerance"), 0.0001),
  //guimergenodes_(get_ctx()->subVar("force-nodemerge"),1),
  //guimergeelems_(get_ctx()->subVar("force-elemmerge"),0),
  //guiforcepointcloud_(get_ctx()->subVar("force-pointcloud"),0),
  //guimatchval_(get_ctx()->subVar("matchval"),0),
  //guimeshonly_(get_ctx()->subVar("meshonly"),0)  
}

void JoinFields::execute()
{
#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
  std::vector<SCIRun::FieldHandle> fields;

  get_dynamic_input_handles("Field",fields,true);

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
    
    algo_.set_scalar("tolerance",guitolerance_.get());
    algo_.set_bool("merge_nodes",guimergenodes_.get());
    algo_.set_bool("merge_elems",guimergeelems_.get());
    algo_.set_bool("match_node_values",guimatchval_.get());
    algo_.set_bool("make_no_data",guimeshonly_.get());

    if (guiforcepointcloud_.get()) 
      forcepointcloud = true;

    std::vector<FieldHandle> ffields;
    for (size_t j=0; j<fields.size(); j++)
    {
      if (fields[j].get_rep()) ffields.push_back(fields[j]);
      else remark("One of the field inputs is empty");
    }
    
    if(!(algo_.run(fields,output))) 
      return;

    if (forcepointcloud)
    {
      if(!(calgo_.run(output,output))) 
        return;
    }

    send_output_handle("Output Field", output);
  }
#endif
}
