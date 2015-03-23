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

#include <Core/Algorithms/Fields/FieldData/ConvertFieldDataType.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
ConvertFieldDataTypeAlgo::run(FieldHandle input, FieldHandle& output)
{
  algo_start("ConvertFieldData");
  
  /// Safety check
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }
  
  /// Get the information of the input field
  FieldInformation fo(input);
  
  std::string datatype;
  get_option("datatype",datatype);
  
  fo.set_data_type(datatype);
  
  output = CreateField(fo,input->mesh());
  
  if (output.get_rep() == 0)
  {
    error("Could no create output field");
    algo_end(); return (false);
  }
  
  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  
  ofield->resize_values();
  ofield->copy_values(ifield);
  ofield->copy_properties(ifield);
  
  /// Support for quadratic fields
  if (ofield->basis_order() == 2)
  {
    ofield->copy_evalues(ifield);
  }
  
  algo_end(); return (true);
}

} // end namespace SCIRunAlgo
