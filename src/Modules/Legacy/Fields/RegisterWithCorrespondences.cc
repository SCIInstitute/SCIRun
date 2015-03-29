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

#include <Modules/Legacy/Fields/RegisterWithCorrespondences.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Algorithms/Fields/RegisterWithCorrespondences.h>

namespace SCIRun {

using namespace SCIRun;

/// @class RegisterWithCorrespondences
/// @brief This module allows you to morph using a thin plate spline algorithm
/// one point set or mesh to another point set or mesh. 

class RegisterWithCorrespondences : public Module {
public:
  RegisterWithCorrespondences(GuiContext*);

  virtual ~RegisterWithCorrespondences() {}
  virtual void execute();

private:
  SCIRunAlgo::RegisterWithCorrespondencesAlgo algo_;
  GuiString method_;
  GuiString edmethod_;

};


DECLARE_MAKER(RegisterWithCorrespondences)

RegisterWithCorrespondences::RegisterWithCorrespondences(GuiContext* ctx) :
  Module("RegisterWithCorrespondences", ctx, Source, "ChangeFieldData", "SCIRun"),
    method_(ctx->subVar("method")),
    edmethod_(ctx->subVar("ed-method"))
{
  algo_.set_progress_reporter(this);
}

void
RegisterWithCorrespondences::execute()
{
  FieldHandle input, cors1, cors2, output;
  
  get_input_handle("InputField",input,true);
  get_input_handle("Correspondences1",cors1,true);
  get_input_handle("Correspondences2",cors2,true);
  
  if (inputs_changed_ || !oport_cached("Output") || method_.changed() ||
      edmethod_.changed())
  {
    update_state(Executing);
  
    if (method_.get() == "transform")
    {      
      if (edmethod_.get() == "affine")
      {
        std::cout<<"affine"<<std::endl;
        
        if(!(algo_.runA(input,cors1,cors2,output))) return;
      }
      else  if (edmethod_.get() == "morph")
      {
        std::cout<<"morph"<<std::endl;
         if(!(algo_.runM(input,cors1,cors2,output))) return;
      }
      else
      {
        std::cout<<"none"<<std::endl;
         if(!(algo_.runN(input,cors1,cors2,output))) return;    
      }
    }
  
    send_output_handle("Output",output,true);
  }
}

} // End namespace SCIRun


#endif
