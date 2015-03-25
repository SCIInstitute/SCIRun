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

#include <Core/Algorithms/Fields/FieldData/SmoothVecFieldMedian.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Module.h>
//#include <Core/Malloc/Allocator.h>

namespace SCIRun {

//using namespace SCIRun;

/// @class SmoothVecFieldMedian
/// @brief This function smoothes vectors assigned to the elements of a mesh
/// using a median filter. 

class SmoothVecFieldMedian : public Module 
{
  public:

    SmoothVecFieldMedian(GuiContext* ctx);
    virtual ~SmoothVecFieldMedian();
    virtual void execute();
    virtual void tcl_command(GuiArgs&, void*);
  
  private:
    SCIRunAlgo::SmoothVecFieldMedianAlgo algo_;

};


DECLARE_MAKER(SmoothVecFieldMedian)

SmoothVecFieldMedian::SmoothVecFieldMedian(GuiContext* ctx) :
  Module("SmoothVecFieldMedian", ctx, Filter, "ChangeFieldData", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

SmoothVecFieldMedian::~SmoothVecFieldMedian(){
}

void
SmoothVecFieldMedian::execute()
{
  FieldHandle input, output;

  get_input_handle( "vector_median_filt", input, true );

  // If no data or a changed recalcute.
  if( inputs_changed_ || !oport_cached("Output_vector_field") )
  {
    update_state(Executing);

    if (!(algo_.run(input,output))) return;
    send_output_handle( "Output_vector_field", output );
  }

}

void
SmoothVecFieldMedian::tcl_command(GuiArgs& args, void* userdata)
{
  Module::tcl_command(args, userdata);
}

} // End namespace SCIRun


