/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2011 Scientific Computing and Imaging Institute,
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


/*
 *  BuildBEMatrix.cc:
 *
 *  Written by:
 *   Saeed Babaeizadeh - Northeastern University
 *   Michael Callahan - Department of Computer Science - University of Utah
 *   May, 2003
 *
 *  Updated by:
 *   Burak Erem - Northeastern University
 *   January, 2012
 *
 */

//#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystem.h>

#include <Modules/Legacy/Forward/BuildBEMatrix.h>
#include <Modules/Legacy/Forward/BuildBEMatrixImpl.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Forward;
using namespace SCIRun::Dataflow::Networks;

  //bool
  //validate_gui_var(const std::size_t gui_list_len,
  //                 const std::size_t expected_len,
  //                 const std::string& name);

  //GuiString guiselected_field_;
  //GuiString guiselected_type_;

  //GuiString guifields_;
  //GuiString guifield_type_property_;
  //GuiString guifield_inside_cond_property_;
  //GuiString guifield_outside_cond_property_;
  //GuiString guifield_surface_type_property_;

const ModuleLookupInfo BuildBEMatrix::staticInfo_("BuildBEMatrix", "Forward", "SCIRun");

BuildBEMatrix::BuildBEMatrix():
  Module(staticInfo_)
//     ,
//   old_nodes_generation_( -1 ),
//   old_surface_generation_( -1 ),
//   process_gui_vars_(false),
//   guiselected_field_(get_ctx()->subVar("selected_field", false), ""),
//   guiselected_type_(get_ctx()->subVar("selected_field_type", false), ""),
//   guifields_(get_ctx()->subVar("input_field_list"), ""),
//   guifield_type_property_(get_ctx()->subVar("field_type_list"), ""),
//   guifield_inside_cond_property_(get_ctx()->subVar("inside_cond_list"), ""),
//   guifield_outside_cond_property_(get_ctx()->subVar("outside_cond_list"), ""),
//   guifield_surface_type_property_(get_ctx()->subVar("surface_type_list"), "")
{
  INITIALIZE_PORT(Surface);
  INITIALIZE_PORT(BEM_Forward_Matrix);
}

void BuildBEMatrix::setStateDefaults()
{
  //TODO
}

//bool
//BuildBEMatrix::validate_gui_var(const std::size_t gui_list_len,
//                                const std::size_t expected_len,
//                                const std::string& name)
//{
//  if (gui_list_len != expected_len)
//  {
//    std::ostringstream oss;
//    oss << "Inputs from GUI for " << name << " (" << gui_list_len
//        << ") do not match number of fields (" << expected_len
//        << "). GUI input will be ignored.";
//    warning(oss.str());
//    return false;
//  }
//
//  return true;
//}

void
BuildBEMatrix::execute()
{
  auto inputs = getRequiredDynamicInputs(Surface);

  if (needToExecute())
  {
    BuildBEMatrixImpl impl;

    MatrixHandle transferMatrix = impl.executeImpl(inputs);

    sendOutput(BEM_Forward_Matrix, transferMatrix);
  }
}
