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
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Algorithms/Fields/MergeFields/JoinFields.h>
#include <Core/Algorithms/Fields/ConvertMeshType/ConvertMeshToPointCloudMesh.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

#include <vector>

namespace SCIRun {

class JoinFields : public Module {
  public:
    JoinFields(GuiContext*);
    virtual ~JoinFields() {}
    virtual void execute();

  private:
    GuiInt    guiclear_;
    GuiDouble guitolerance_;
    GuiInt    guimergenodes_;
    GuiInt    guimergeelems_;
    GuiInt    guiforcepointcloud_;
    GuiInt    guimatchval_;
    GuiInt    guimeshonly_;
    
    SCIRunAlgo::JoinFieldsAlgo algo_;
    SCIRunAlgo::ConvertMeshToPointCloudMeshAlgo calgo_;
};


DECLARE_MAKER(JoinFields)
JoinFields::JoinFields(GuiContext* ctx)
  : Module("JoinFields", ctx, Source, "NewField", "SCIRun"),
  guiclear_(get_ctx()->subVar("clear", false), 0),  
  guitolerance_(get_ctx()->subVar("tolerance"), 0.0001),
  guimergenodes_(get_ctx()->subVar("force-nodemerge"),1),
  guimergeelems_(get_ctx()->subVar("force-elemmerge"),0),
  guiforcepointcloud_(get_ctx()->subVar("force-pointcloud"),0),
  guimatchval_(get_ctx()->subVar("matchval"),0),
  guimeshonly_(get_ctx()->subVar("meshonly"),0)  
{
  algo_.set_progress_reporter(this);
  calgo_.set_progress_reporter(this);
}

void JoinFields::execute()
{
  // Define local handles of data objects:
  std::vector<SCIRun::FieldHandle> fields;
  FieldHandle output;


  // Some stuff for old power apps
  if (guiclear_.get())
  {
    guiclear_.set(0);

    // Sending 0 does not clear caches.
    FieldInformation fi("PointCloudMesh",0,"double");
    FieldHandle handle = CreateField(fi);

    send_output_handle("Output Field", handle);
    return;
  }

  // Get the new input data:  
  get_dynamic_input_handles("Field",fields,true);

  // Only reexecute if the input changed. SCIRun uses simple scheduling
  // that executes every module downstream even if no data has changed: 

  if (inputs_changed_ ||  guitolerance_.changed() ||
      guimergenodes_.changed() || guiforcepointcloud_.changed() ||
      guimatchval_.changed() || guimeshonly_.changed() || guimergeelems_.changed() ||
      !oport_cached("Output Field"))
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

    if (guiforcepointcloud_.get()) forcepointcloud = true;

    std::vector<FieldHandle> ffields;
    for (size_t j=0; j<fields.size(); j++)
    {
      if (fields[j].get_rep()) ffields.push_back(fields[j]);
      else remark("One of the field inputs is empty");
    }
    
    if(!(algo_.run(fields,output))) return;
    // This option is here to be compatible with the old GatherFields module:
    // This is a separate algorithm now

    if (forcepointcloud)
    {
      if(!(calgo_.run(output,output))) return;
    }

    // send new output if there is any:        
    send_output_handle("Output Field", output);
  }
}

} // End namespace SCIRun

