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

///TODO Documentation
#include <Core/Algorithms/Legacy/Fields/Mapping/ApplyMappingMatrix.h>
//#include <Core/Algorithms/Fields/Mapping/ApplyMappingMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>


using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

ApplyMappingMatrix::ApplyMappingMatrix()
  : Module(ModuleLookupInfo("ApplyMappingMatrix","Fields", "SCIRun"), false)
{
 INITIALIZE_PORT(Source);
 INITIALIZE_PORT(Destination);
 INITIALIZE_PORT(Mapping);
 INITIALIZE_PORT(Output);
}

void ApplyMappingMatrix::execute()
{
}



#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
namespace SCIRun {

class ApplyMappingMatrix : public Module
{
  public:
    ApplyMappingMatrix(GuiContext* ctx);
    virtual ~ApplyMappingMatrix() {}
    virtual void execute();

  private:
    SCIRunAlgo::ApplyMappingMatrixAlgo algo_;
};


DECLARE_MAKER(ApplyMappingMatrix)

ApplyMappingMatrix::ApplyMappingMatrix(GuiContext* ctx)
  : Module("ApplyMappingMatrix", ctx, Filter, "ChangeFieldData", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

void
ApplyMappingMatrix::execute()
{
  // Get source field.
  FieldHandle source, destination, output;
  MatrixHandle  mapping;
  
  get_input_handle( "Source", source, true);
  get_input_handle( "Destination", destination, true);
  get_input_handle( "Mapping", mapping, true );

  if( inputs_changed_  || !oport_cached("Output") )
  {
    // Inform module that execution started
    update_state(Executing);
    
    // Core algorithm of the module
    if(!(algo_.run(source,destination,mapping,output))) return;
    
    // Send output to output ports
    send_output_handle( "Output",output);
  }
}






} // namespace SCIRun
  #endif
