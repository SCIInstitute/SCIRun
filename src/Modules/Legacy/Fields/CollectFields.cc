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

#include <Modules/Legacy/Fields/CollectFields.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;

const ModuleLookupInfo CollectFields::staticInfo_("CollectFields", "NewField", "SCIRun");

CollectFields::CollectFields() : Module(staticInfo_)
{
  
}


#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Thread/Mutex.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Fields/MergeFields/AppendFields.h>
#include <Core/Algorithms/Converter/ConverterAlgo.h>

#include <Dataflow/Network/Module.h>

namespace SCIRun {

/// @class CollectFields
/// @brief This module collects and joins fields while iterating over a dataflow loop. 

class CollectFields : public Module {
  public:
    CollectFields(GuiContext*);
    ~CollectFields() {}
    virtual void execute();
    virtual void tcl_command(GuiArgs& args, void* userdata);

  private:
    std::list<FieldHandle> buffer_;
    int buffer_length_;
    GuiInt buffersizegui_;
    Mutex bufferlock_;
    int buffersize_;
    
    SCIRunAlgo::AppendFieldsAlgo algo_;
};


DECLARE_MAKER(CollectFields)
CollectFields::CollectFields(GuiContext* ctx)
  : Module("CollectFields", ctx, Source, "NewField", "SCIRun"),
    buffer_length_(0),
    buffersizegui_(ctx->subVar("buffersize")),
    bufferlock_("Lock for internal buffer of module"),
    buffersize_(0)
{
  algo_.set_progress_reporter(this);
}


void
CollectFields::execute()
{
  // Define local handles of data objects:
  FieldHandle Input, Output;
  MatrixHandle BufferSize;
  
  // Get the new input data:  
  get_input_handle("Field",Input,true);
  get_input_handle("BufferSize",BufferSize,false);
  
  // Only reexecute if the input changed. SCIRun uses simple scheduling
  // that executes every module downstream even if no data has changed:  
  if (inputs_changed_ || buffersizegui_.changed() 
      || buffer_.size() != static_cast<size_t>(buffer_length_)
      || !oport_cached("Fields"))
  {
    update_state(Executing);
    buffer_length_ = buffer_.size();
      
    SCIRunAlgo::ConverterAlgo calgo(this);
    
    // Push back data to the GUI if it was overruled by the dataflow input:
    if (BufferSize.get_rep())
    {
      calgo.MatrixToInt(BufferSize,buffersize_);
      buffersizegui_.set(buffersize_);
      get_ctx()->reset(); 
    }
    
    // Get parameters
    buffersize_ = buffersizegui_.get();
    
    bufferlock_.lock();
    buffer_.push_back(Input);
    while (buffer_.size() > static_cast<size_t>(buffersize_)) buffer_.pop_front();
    bufferlock_.unlock();
    
    // Innerworks of module:
    // The lock here protects us from the user wiping out the buffer
    // through the GUI while executing. This could cause a segmentation
    // fault and hence it needs to be protected b y a lock.
    bufferlock_.lock();
    algo_.run(buffer_,Output);
    bufferlock_.unlock();

    // send new output if there is any:      
    send_output_handle("Fields", Output);
  }
}


void
CollectFields::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("CollectFields needs a minor command");
    return;
  }

  if( args[1] == "reset" )
  {
    // Only clear buffer if module is not running 
    bufferlock_.lock();
    buffer_.clear();
    bufferlock_.unlock();

    return;
  }
  else 
  {
    // Relay data to the Module class
    Module::tcl_command(args, userdata);
  }
}


} // End namespace SCIRun


#endif
