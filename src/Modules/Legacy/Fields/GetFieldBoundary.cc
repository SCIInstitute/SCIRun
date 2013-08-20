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

#include <Modules/Legacy/Fields/GetFieldBoundary.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundary.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Fields;

GetFieldBoundary::GetFieldBoundary()
  : Module(ModuleLookupInfo("GetFieldBoundary", "NewField", "SCIRun"), false)
{
  algo_ = algoFactory_->create(get_module_name(), getLogger());  
}

void
GetFieldBoundary::execute()
{
  FieldHandle field;
  
  get_input_handle("Field",field,true);
  
  // If parameters changed, do algorithm
  if (inputs_changed_ || 
      !oport_cached("BoundaryField") || 
      !oport_cached("Mapping"))
  {
    update_state(Executing);

    FieldHandle ofield;
    MatrixHandle mapping;
    
    if (!(algo_.run(field,ofield,mapping))) return;

    send_output_handle("BoundaryField", ofield);
    send_output_handle("Mapping", mapping);
  }
}
