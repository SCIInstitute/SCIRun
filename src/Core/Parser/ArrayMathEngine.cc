/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Parser/ArrayMathFunctionCatalog.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>

#include <sci_debug.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

bool
NewArrayMathEngine::add_input_fielddata(const std::string& name,
                                        FieldHandle field)
{
  std::string error_str;
  if (!field)
  {
    error_str = "No input field '"+name+"'.";
    pr_->error(error_str);
    return (false);
  }

  VField* vfield = field->vfield();
  VField::size_type num_values = vfield->num_values();

  // Check whether size is OK
  if (num_values > 1)
  {
    if (array_size_ == 1) array_size_ = num_values;
    if (array_size_ != num_values)
    {
      error_str = "The number of data values of field '"+name+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+name;

  if (vfield->is_scalar())
  {
    pre_expression_ += name+"=get_scalar("+tname+");";
  }
  else if (vfield->is_vector())
  {
    pre_expression_ += name+"=get_vector("+tname+");";
  }
  else if (vfield->is_tensor())
  {
    pre_expression_ += name+"=get_tensor("+tname+");";
  }

  int flags = 0;
  if (num_values > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_fielddata_source(mprogram_,tname,field,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"FD",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}


bool
NewArrayMathEngine::add_input_fielddata_location(const std::string& name,
                                        FieldHandle field)
{
  std::string error_str;

  if (!field)
  {
    error_str = "No input field '"+name+"'.";
    pr_->error(error_str);
    return (false);
  }

  VField* vfield = field->vfield();
  VField::size_type num_values = vfield->num_values();

  // Check whether size is OK
  if (num_values > 1)
  {
    if (array_size_ == 1) array_size_ = num_values;
    if (array_size_ != num_values)
    {
      error_str = "The number of data values of field '"+name+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+name;

  if (vfield->basis_order() == -1)
  {
    error_str = "The input field does not contain any values.";
    pr_->error(error_str);
    return (false);
  }
  else if (vfield->basis_order() == 0)
  {
    pre_expression_ += name+"=get_element_location("+tname+");";
  }
  else if (vfield->basis_order() == 1)
  {
    pre_expression_ += name+"=get_node_location("+tname+");";
  }
  else
  {
    error_str = "Quadratic/Cubic fields are not yet supported.";
    pr_->error(error_str);
    return (false);
  }

  int flags = 0;
  if (num_values > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_fieldmesh_source(mprogram_,tname,field,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"FM",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }

  return (true);
}


bool
NewArrayMathEngine::add_input_fielddata_location(const std::string& name,
                                        FieldHandle field,
                                        int basis_order)
{
  std::string error_str;
  if (!field)
  {
    error_str = "No input field '"+name+"'.";
    pr_->error(error_str);
    return (false);
  }

  VMesh* vmesh = field->vmesh();
  VMesh::size_type num_values;
  if (basis_order == 0)
  {
    num_values = vmesh->num_elems();
  }
  else
  {
    num_values = vmesh->num_nodes();
  }

  // Check whether size is OK
  if (num_values > 1)
  {
    if (array_size_ == 1) array_size_ = num_values;
    if (array_size_ != num_values)
    {
      error_str = "The number of data values of field '"+name+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+name;

  if (basis_order == -1)
  {
    error_str = "The input field does not contain any values.";
    pr_->error(error_str);
    return (false);
  }
  else if (basis_order == 0)
  {
    pre_expression_ += name+"=get_element_location("+tname+");";
  }
  else if (basis_order == 1)
  {
    pre_expression_ += name+"=get_node_location("+tname+");";
  }
  else
  {
    error_str = "Quadratic/Cubic fields are not yet supported.";
    pr_->error(error_str);
    return (false);
  }

  int flags = 0;
  if (num_values > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_fieldmesh_source(mprogram_,tname,field,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"FM",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}


bool
NewArrayMathEngine::add_input_fielddata_coordinates(const std::string& xname,
                                        const std::string& yname,
                                        const std::string& zname,
                                        FieldHandle field)
{
  std::string error_str;
  if (!field)
  {
    error_str = "No input field '"+xname+"'.";
    pr_->error(error_str);
    return (false);
  }

  VField* vfield = field->vfield();
  VField::size_type num_values = vfield->num_values();

  // Check whether size is OK
  if (num_values > 1)
  {
    if (array_size_ == 1) array_size_ = num_values;
    if (array_size_ != num_values)
    {
      error_str = "The number of data values of field '"+xname+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+xname;

  if (vfield->basis_order() == -1)
  {
    error_str = "The input field does not contain any values.";
    pr_->error(error_str);
    return (false);
  }
  else if (vfield->basis_order() == 0)
  {
    pre_expression_ += xname+"=get_element_x("+tname+");";
    pre_expression_ += yname+"=get_element_y("+tname+");";
    pre_expression_ += zname+"=get_element_z("+tname+");";
  }
  else if (vfield->basis_order() == 1)
  {
    pre_expression_ += xname+"=get_node_x("+tname+");";
    pre_expression_ += yname+"=get_node_y("+tname+");";
    pre_expression_ += zname+"=get_node_z("+tname+");";
  }
  else
  {
    error_str = "Quadratic/Cubic fields are not yet supported.";
    pr_->error(error_str);
    return (false);
  }

  int flags = 0;
  if (num_values > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_fieldmesh_source(mprogram_,tname,field,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"FM",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}

bool
NewArrayMathEngine::add_input_fielddata_coordinates(const std::string& xname,
                                        const std::string& yname,
                                        const std::string& zname,
                                        FieldHandle field,
                                        int basis_order)
{
  std::string error_str;
  if (!field)
  {
    error_str = "No input field '"+xname+"'.";
    pr_->error(error_str);
    return (false);
  }

  VMesh* vmesh = field->vmesh();
  VMesh::size_type num_values;
  if (basis_order == 0)
  {
    num_values = vmesh->num_elems();
  }
  else
  {
    num_values = vmesh->num_nodes();
  }

  // Check whether size is OK
  if (num_values > 1)
  {
    if (array_size_ == 1) array_size_ = num_values;
    if (array_size_ != num_values)
    {
      error_str = "The number of data values of field '"+xname+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+xname;

  if (basis_order == -1)
  {
    error_str = "The input field does not contain any values.";
    pr_->error(error_str);
    return (false);
  }
  else if (basis_order == 0)
  {
    pre_expression_ += xname+"=get_element_x("+tname+");";
    pre_expression_ += yname+"=get_element_y("+tname+");";
    pre_expression_ += zname+"=get_element_z("+tname+");";
  }
  else if (basis_order == 1)
  {
    pre_expression_ += xname+"=get_node_x("+tname+");";
    pre_expression_ += yname+"=get_node_y("+tname+");";
    pre_expression_ += zname+"=get_node_z("+tname+");";
  }
  else
  {
    error_str = "Quadratic/Cubic fields are not yet supported.";
    pr_->error(error_str);
    return (false);
  }

  int flags = 0;
  if (num_values > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_fieldmesh_source(mprogram_,tname,field,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"FM",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}


bool
NewArrayMathEngine::add_input_fielddata_element(const std::string& name,
                                        FieldHandle field)
{
  std::string error_str;

  if (!field)
  {
    error_str = "No input field '"+name+"'.";
    pr_->error(error_str);
    return (false);
  }

  VField* vfield = field->vfield();
  VField::size_type num_values = vfield->num_values();

  // Check whether size is OK
  if (num_values > 1)
  {
    if (array_size_ == 1) array_size_ = num_values;
    if (array_size_ != num_values)
    {
      error_str = "The number of data values of field '"+name+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  std::string type;

  if (vfield->basis_order() == -1)
  {
    error_str = "The input field does not contain any values.";
    pr_->error(error_str);
    return (false);
  }
  else if (vfield->basis_order() == 0)
  {
    type = "FE";
  }
  else if (vfield->basis_order() == 1)
  {
    type = "FN";
  }
  else
  {
    error_str = "Quadratic/Cubic fields are not yet supported.";
    pr_->error(error_str);
    return (false);
  }

  int flags = 0;
  if (num_values > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_fieldmesh_source(mprogram_,name,field,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,name,type,flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}

bool
NewArrayMathEngine::add_input_fielddata_element(const std::string& name,
                                        FieldHandle field,
                                        int basis_order)
{
  std::string error_str;

  if (!field)
  {
    error_str = "No input field '"+name+"'.";
    pr_->error(error_str);
    return (false);
  }

  VMesh* vmesh = field->vmesh();
  VMesh::size_type num_values;
  if (basis_order == 0)
  {
    num_values = vmesh->num_elems();
  }
  else
  {
    num_values = vmesh->num_nodes();
  }

  // Check whether size is OK
  if (num_values > 1)
  {
    if (array_size_ == 1) array_size_ = num_values;
    if (array_size_ != num_values)
    {
      error_str = "The number of data values of field '"+name+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  std::string type;

  if (basis_order == -1)
  {
    error_str = "The input field does not contain any values.";
    pr_->error(error_str);
    return (false);
  }
  else if (basis_order == 0)
  {
    type = "FE";
  }
  else if (basis_order == 1)
  {
    type = "FN";
  }
  else
  {
    error_str = "Quadratic/Cubic fields are not yet supported.";
    pr_->error(error_str);
    return (false);
  }

  int flags = 0;
  if (num_values > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_fieldmesh_source(mprogram_,name,field,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,name,type,flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}


bool
NewArrayMathEngine::add_input_fieldnodes(const std::string& name,
                                        FieldHandle field)
{
  std::string error_str;
  if (!field)
  {
    error_str = "No input field '"+name+"'.";
    pr_->error(error_str);
    return (false);
  }

  VMesh* vmesh = field->vmesh();
  VMesh::size_type num_values = vmesh->num_nodes();

  // Check whether size is OK
  if (num_values > 1)
  {
    if (array_size_ == 1) array_size_ = num_values;
    if (array_size_ != num_values)
    {
      error_str = "The number of nodes of field '"+name+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+name;
  pre_expression_ += name+"=get_node_location("+tname+");";

  int flags = 0;
  if (num_values > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_fieldmesh_source(mprogram_,tname,field,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"FM",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}

bool
NewArrayMathEngine::add_input_fieldnodes_coordinates(const std::string& xname,
                                        const std::string& yname,
                                        const std::string& zname,
                                        FieldHandle field)
{
  std::string error_str;
  if (!field)
  {
    error_str = "No input field '"+xname+"'.";
    pr_->error(error_str);
    return (false);
  }

  VMesh* vmesh = field->vmesh();
  VMesh::size_type num_values = vmesh->num_nodes();

  // Check whether size is OK
  if (num_values > 1)
  {
    if (array_size_ == 1) array_size_ = num_values;
    if (array_size_ != num_values)
    {
      error_str = "The number of nodes of field '"+xname+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+xname;
  pre_expression_ += xname+"=get_node_x("+tname+");";
  pre_expression_ += yname+"=get_node_y("+tname+");";
  pre_expression_ += zname+"=get_node_z("+tname+");";

  int flags = 0;
  if (num_values > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_fieldmesh_source(mprogram_,tname,field,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"FM",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}


bool
NewArrayMathEngine::add_input_matrix(const std::string& name,
                                     MatrixHandle matrix)
{
  std::string error_str;
  if (!matrix)
  {
    error_str = "No input matrix '"+name+"'.";
    pr_->error(error_str);
    return (false);
  }

  size_type m = matrix->nrows();
  size_type n = matrix->ncols();

  // Check whether size is OK
  if (m > 1)
  {
    if (array_size_ == 1) array_size_ = m;
    if (array_size_ != m)
    {
      // Fix around issue in SCIRun that translates Point and Tensors in Column
      // matrices instead of row matrices. This is a quick and dirty hack around
      // this issue
      if ((m == 3 || m == 6)&&(n==1))
      {
        // pseudo transpose
        n = m; m = 1;
      }
      else
      {
        error_str = "The number of matrix rows '"+name+"' does not match the size of the other objects.";
        pr_->error(error_str);
        return (false);
      }
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+name;

  if (n == 1)
  {
    pre_expression_ += name+"=get_scalar("+tname+");";
  }
  else if (n == 3)
  {
    pre_expression_ += name+"=get_vector("+tname+");";
  }
  else if ((n==6)||(n==9))
  {
    pre_expression_ += name+"=get_tensor("+tname+");";
  }

  int flags = 0;
  if (m > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_matrix_source(mprogram_,tname,matrix,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"M",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}


bool
NewArrayMathEngine::add_input_fullmatrix(const std::string& name,
                                     MatrixHandle matrix)
{
  std::string error_str;
  if (!matrix)
  {
    error_str = "No input matrix '"+name+"'.";
    pr_->error(error_str);
    return (false);
  }

  size_type size = matrix->ncols()*matrix->nrows();

  // Check whether size is OK
  if (size > 1)
  {
    if (array_size_ == 1) array_size_ = size;
    if (array_size_ != size)
    {
      error_str = "The number of matrix rows '"+name+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+name;

  pre_expression_ += name+"=get_matrix_element("+tname+");";

  int flags = 0;
  if (size > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_matrix_source(mprogram_,tname,matrix,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"M",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}


bool
NewArrayMathEngine::add_input_bool_array(const std::string& name,
                                     std::vector<bool>* array)
{
  std::string error_str;
  if (array->size() == 0)
  {
    error_str = "Input boolean array '"+name+"' is empty.";
    pr_->error(error_str);
    return (false);
  }

  SCIRun::size_type size = array->size();

  // Check whether size is OK
  if (size > 1)
  {
    if (array_size_ == 1) array_size_ = size;
    if (array_size_ != size)
    {
      error_str = "The number of array elements in '"+name+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+name;

  pre_expression_ += name+"=get_scalar("+tname+");";

  int flags = 0;
  if (size > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_bool_array_source(mprogram_,tname,array,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"AB",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}

bool
NewArrayMathEngine::add_input_int_array(const std::string& name,
                                     std::vector<int>* array)
{
  std::string error_str;
  if (array->size() == 0)
  {
    error_str = "Input boolean array '"+name+"' is empty.";
    pr_->error(error_str);
    return (false);
  }

  SCIRun::size_type size = array->size();

  // Check whether size is OK
  if (size > 1)
  {
    if (array_size_ == 1) array_size_ = size;
    if (array_size_ != size)
    {
      error_str = "The number of array elements in '"+name+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+name;

  pre_expression_ += name+"=get_scalar("+tname+");";

  int flags = 0;
  if (size > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_int_array_source(mprogram_,tname,array,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"AI",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}

bool
NewArrayMathEngine::add_input_double_array(const std::string& name,
                                     std::vector<double>* array)
{
  std::string error_str;
  if (array->size() == 0)
  {
    error_str = "Input double array '"+name+"' is empty.";
    pr_->error(error_str);
    return (false);
  }

  SCIRun::size_type size = array->size();

  // Check whether size is OK
  if (size > 1)
  {
    if (array_size_ == 1) array_size_ = size;
    if (array_size_ != size)
    {
      error_str = "The number of array elements in '"+name+"' does not match the size of the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+name;

  pre_expression_ += name+"=get_scalar("+tname+");";

  int flags = 0;
  if (size > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  // Add the variable to the interpreter
  if(!(add_double_array_source(mprogram_,tname,array,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"AD",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  return (true);
}


bool
NewArrayMathEngine::add_index(const std::string& name)
{
  pre_expression_ += name+"=index();";
  return (true);
}

bool
NewArrayMathEngine::add_size(const std::string& name)
{
  std::ostringstream ostr;
  ostr << name << "=" << array_size_ << ";";
  pre_expression_ += ostr.str();
  return (true);
}


bool
NewArrayMathEngine::add_output_fielddata(const std::string& name,
                              FieldHandle field,
                              int basis_order,
                              const std::string& output_datatype)
{
  std::string error_str;
  if (!field)
  {
    error_str = "No input field '"+name+"'.";
    pr_->error(error_str);
    return (false);
  }

  if (basis_order < 0)
  {
    error_str = "Basis order needs to be 0 or 1.";
    pr_->error(error_str);
    return (false);
  }

  if (basis_order > 1)
  {
    error_str = "Basis order needs to be 0 or 1.";
    pr_->error(error_str);
    return (false);
  }

  int flags = 0;
  VMesh* vmesh = field->vmesh();

  size_type size;

  if (basis_order == 0)
  {
    if (array_size_ != vmesh->num_elems() && vmesh->num_elems() != 1 && vmesh->num_elems() != 0)
    {
      error_str = "The output field '"+name+"' does not have the same number of elements as the other objects.";
      pr_->error(error_str);
      return (false);
    }
    size = vmesh->num_elems();
  }
  else
  {
    if (array_size_ != vmesh->num_nodes() && vmesh->num_nodes() != 1 && vmesh->num_nodes() != 0)
    {
      error_str = "The output field '"+name+"' does not have the same number of nodes as the other objects.";
      pr_->error(error_str);
      return (false);
    }
    size = vmesh->num_nodes();
  }

  std::string tname =  "__"+name;

  if (size == 0)
  {
    OutputFieldData fd;
    fd.array_name_ = name;
    fd.field_name_ = tname;

    FieldInformation fi(field);
    if (output_datatype == "Scalar")
      fi.set_data_type("double");
    else
      fi.set_data_type(output_datatype);
    fi.set_basis_type(basis_order);
    fd.field_ = CreateField(fi);

    fielddata_.push_back(fd);
    return (true);
  }

  if (size > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  post_expression_ += tname+"=to_fielddata("+name+");";

  // Add the variable to the parser
  if(!(add_output_variable(pprogram_,tname,"FD",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  if(!(add_output_variable(pprogram_,name,"U",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }

  // Store information for processing after parsing has succeeded
  OutputFieldData fd;
  fd.array_name_ = name;
  fd.field_name_ = tname;
  fd.field_ = field;
  fd.output_datatype_ = output_datatype;
  fd.output_basis_order_ = basis_order;
  fielddata_.push_back(fd);

  return (true);
}

bool
NewArrayMathEngine::add_output_fielddata(const std::string& name,
                              FieldHandle field)
{
  std::string error_str;
  if (!field)
  {
    error_str = "No input field '"+name+"'.";
    pr_->error(error_str);
    return (false);
  }

  int basis_order = field->vfield()->basis_order();
  if (basis_order < 0) basis_order = 1;
  if (basis_order > 1) basis_order = 1;

  int flags = 0;
  VMesh* vmesh = field->vmesh();

  size_type size;

  if (basis_order == 0)
  {
    if (array_size_ != vmesh->num_elems() && vmesh->num_elems() != 1 && vmesh->num_elems() != 0)
    {
      if (array_size_ == vmesh->num_nodes())
      {
        basis_order = 1;
        size = vmesh->num_nodes();
      }
      else
      {
        error_str = "The output field '"+name+"' does not have the same number of elements as the other objects.";
        pr_->error(error_str);
        return (false);
      }
    }
    else
    {
      size = vmesh->num_elems();
    }
  }
  else
  {
    if (array_size_ != vmesh->num_nodes() && vmesh->num_nodes() != 1 && vmesh->num_nodes() != 0)
    {
      if (array_size_ == vmesh->num_elems())
      {
        basis_order = 0;
        size = vmesh->num_elems();
      }
      else
      {
        error_str = "The output field '"+name+"' does not have the same number of nodes as the other objects.";
        pr_->error(error_str);
        return (false);
      }
    }
    else
    {
      size = vmesh->num_nodes();
    }
  }

  if (size > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  std::string tname =  "__"+name;

  if (size == 0)
  {
    OutputFieldData fd;
    fd.array_name_ = name;
    fd.field_name_ = tname;

    FieldInformation fi(field);
    fd.field_ = CreateField(fi);

    fielddata_.push_back(fd);
    return (true);
  }

  post_expression_ += tname+"=to_fielddata("+name+");";

  // Add the variable to the parser
  if(!(add_output_variable(pprogram_,tname,"FD",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }

  // Store information for processing after parsing has succeeded
  OutputFieldData fd;
  fd.array_name_ = name;
  fd.field_name_ = tname;
  fd.field_ = field;
  fd.output_datatype_ = "same as input";
  fd.output_basis_order_ = basis_order;
  fielddata_.push_back(fd);

  return (true);
}


bool
NewArrayMathEngine::add_output_fieldnodes(const std::string& name,
                                          FieldHandle field)
{
  std::string error_str;

  if (!field)
  {
    error_str = "No input field '"+name+"'.";
    pr_->error(error_str);
    return (false);
  }

  int flags = 0;
  VMesh* vmesh = field->vmesh();

  size_type size;

  if (array_size_ != vmesh->num_nodes() && vmesh->num_nodes() != 1 && vmesh->num_nodes() != 0)
  {
    error_str = "The output field '"+name+"' does not have the same number of nodes as the other objects.";
    pr_->error(error_str);
    return (false);
  }
  else
  {
    size = vmesh->num_nodes();
  }

  if (size > 1) flags = SCRIPT_SEQUENTIAL_VAR_E;

  std::string tname =  "__"+name;

  if (size == 0)
  {
    OutputFieldData fd;
    fd.array_name_ = name;
    fd.field_name_ = tname;

    FieldInformation fi(field);
    fd.field_ = CreateField(fi);

    fielddata_.push_back(fd);
    return (true);
  }

  post_expression_ += tname+"=to_fieldnode("+name+");";

  // Add the variable to the parser
  if(!(add_output_variable(pprogram_,tname,"FM",flags)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  // Store information for processing after parsing has succeeded
  OutputFieldMesh fd;
  fd.array_name_ = name;
  fd.field_name_ = tname;
  fd.field_ = field;
  fieldmesh_.push_back(fd);

  return (true);
}


bool
NewArrayMathEngine::add_output_matrix(const std::string& name)
{
  std::string tname =  "__"+name;

  post_expression_ += tname+"=to_matrix("+name+");";

  // Add the variable to the parser
  if(!(add_output_variable(pprogram_,name,"U",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  if(!(add_output_variable(pprogram_,tname,"M",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  // Store information for processing after parsing has succeeded
  OutputMatrix m;
  m.array_name_ = name;
  m.matrix_name_ = tname;
  m.matrix_ = 0;
  m.size_ = 0;
  matrixdata_.push_back(m);

  return (true);
}

bool
NewArrayMathEngine::add_output_matrix(const std::string& name,
                                      size_type size)
{
  std::string error_str;
  std::string tname =  "__"+name;

  post_expression_ += tname+"=to_matrix("+name+");";

  // Add the variable to the parser
  if(!(add_output_variable(pprogram_,name,"U",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  if(!(add_output_variable(pprogram_,tname,"M",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }

  if (size > 1 && array_size_ > 1)
  {
    if (size != array_size_)
    {
      error_str = "The output matrix '"+name+"' does not have the same number of rows as the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Store information for processing after parsing has succeeded
  OutputMatrix m;
  m.array_name_ = name;
  m.matrix_name_ = tname;
  m.matrix_ = 0;
  m.size_ = 0;
  matrixdata_.push_back(m);

  return (true);
}


bool
NewArrayMathEngine::add_output_fullmatrix(const std::string& name,
                                      MatrixHandle matrix)
{
  std::string error_str;
  std::string tname =  "__"+name;

  post_expression_ += tname+"=set_matrix_element("+name+");";

  // Add the variable to the parser
  if(!(add_output_variable(pprogram_,name,"S",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  if(!(add_output_variable(pprogram_,tname,"M",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }

  size_type size = matrix->ncols()*matrix->nrows();

  if (size > 1 && array_size_ > 1)
  {
    if (size != array_size_)
    {
      error_str = "The output matrix '"+name+"' does not have the same number of rows as the other objects.";
      pr_->error(error_str);
      return (false);
    }
  }

  // Store information for processing after parsing has succeeded
  OutputMatrix m;
  m.array_name_ = name;
  m.matrix_name_ = tname;
  m.matrix_ = matrix;
  m.size_ = size;
  matrixdata_.push_back(m);

  return (true);
}

bool
NewArrayMathEngine::add_output_bool_array(const std::string& name,
                                     std::vector<bool>* array)
{
  std::string error_str;
  std::string tname =  "__"+name;

  post_expression_ += tname+"=to_bool_array("+name+");";

  // Add the variable to the parser
  if(!(add_output_variable(pprogram_,name,"S",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  if(!(add_output_variable(pprogram_,tname,"AB",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }

  array->resize(array_size_);

  // Store information for processing after parsing has succeeded
  OutputBoolArray m;
  m.array_name_ = name;
  m.bool_array_name_ = tname;
  m.bool_array_ = array;

  boolarraydata_.push_back(m);

  return (true);
}

bool
NewArrayMathEngine::add_output_int_array(const std::string& name,
                                     std::vector<int>* array)
{
  std::string error_str;
  std::string tname =  "__"+name;

  post_expression_ += tname+"=to_int_array("+name+");";

  // Add the variable to the parser
  if(!(add_output_variable(pprogram_,name,"S",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  if(!(add_output_variable(pprogram_,tname,"AB",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }

  array->resize(array_size_);

  // Store information for processing after parsing has succeeded
  OutputIntArray m;
  m.array_name_ = name;
  m.int_array_name_ = tname;
  m.int_array_ = array;

  intarraydata_.push_back(m);

  return (true);
}

bool
NewArrayMathEngine::add_output_double_array(const std::string& name,
                                     std::vector<double>* array)
{
  std::string error_str;
  std::string tname =  "__"+name;

  post_expression_ += tname+"=to_double_array("+name+");";

  // Add the variable to the parser
  if(!(add_output_variable(pprogram_,name,"S",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }
  if(!(add_output_variable(pprogram_,tname,"AB",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }

  array->resize(array_size_);

  // Store information for processing after parsing has succeeded
  OutputDoubleArray m;
  m.array_name_ = name;
  m.double_array_name_ = tname;
  m.double_array_ = array;

  doublearraydata_.push_back(m);

  return (true);
}


bool
NewArrayMathEngine::add_expressions(const std::string& expressions)
{
  expression_ += expressions;
  return (true);
}


bool
NewArrayMathEngine::get_field(const std::string& name, FieldHandle& field)
{
  for (size_t j = 0; j<fielddata_.size(); j++)
  {
    if (fielddata_[j].array_name_ == name)
    {
      field = fielddata_[j].field_;
      return (true);
    }
  }

  for (size_t j = 0; j<fieldmesh_.size(); j++)
  {
    if (fieldmesh_[j].array_name_ == name)
    {
      field = fieldmesh_[j].field_;
      return (true);
    }
  }

  return (false);
}

bool
NewArrayMathEngine::get_matrix(const std::string& name, MatrixHandle& matrix)
{
  for (size_t j = 0; j<matrixdata_.size(); j++)
  {
    if (matrixdata_[j].array_name_ == name)
    {
      matrix = matrixdata_[j].matrix_;
      return (true);
    }
  }

  return (false);
}

bool
NewArrayMathEngine::run()
{
  std::string error_str;

  // Link everything together
  std::string full_expression = pre_expression_+";"+expression_+";"+post_expression_;

  // Parse the full expression
  if(!(parse(pprogram_,full_expression,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }

  // Get the catalog with all possible functions
  ParserFunctionCatalogHandle catalog = ArrayMathFunctionCatalog::get_catalog();

  // Validate the expressions
  if (!(validate(pprogram_,catalog,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }

  // Optimize the expressions
  if (!(optimize(pprogram_,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }

  // DEBUG CALL
#ifdef DEBUG
//  pprogram_->print();
#endif

  // Allocate output objects
  // Create output fields (edit field data)
  for(size_t j=0; j< fielddata_.size(); j++)
  {
    FieldHandle field = fielddata_[j].field_;
    std::string type;

    get_output_variable_type(pprogram_,fielddata_[j].array_name_,type);
    FieldInformation fi(field);
    if (type == "S")
    {
      std::string output_datatype = fielddata_[j].output_datatype_;
      if (output_datatype == "Scalar") output_datatype = "double";
      if (output_datatype != "same as input" && output_datatype != "Same as input") fi.set_data_type(output_datatype);
      if (fi.is_vector() || fi.is_tensor()) fi.make_double();
    }
    else if (type == "V")
    {
      fi.make_vector();
    }
    else if (type == "T")
    {
      fi.make_tensor();
    }

    if (fielddata_[j].output_basis_order_ > -1)
    {
      fi.set_basis_type(fielddata_[j].output_basis_order_);
    }

    MeshHandle mesh = field->mesh();
    field = CreateField(fi,mesh);
    if (!field)
    {
      error_str = "Could not allocate output field.";
      pr_->error(error_str);
      return (false);
    }

    std::string fieldname = fielddata_[j].field_name_;
    // Add the variable to the interpreter
    if(!(add_fielddata_sink(mprogram_,fieldname,field,error_str)))
    {
      pr_->error(error_str);
      return (false);
    }
    fielddata_[j].field_ = field;
  }

  // Create output meshes (edit nodes)
  for(size_t j=0; j< fieldmesh_.size(); j++)
  {
    FieldHandle field = fieldmesh_[j].field_;
    FieldInformation fi(field);
    MeshHandle mesh;
    if (fi.is_regularmesh())
    {
      VMesh* vmesh = field->vmesh();
      VMesh::dimension_type dims;
      vmesh->get_dimensions(dims);
      if (fi.is_latvolmesh())
      {
        fi.make_structhexvolmesh();
        mesh = CreateMesh(fi,dims[0],dims[1],dims[2]);
      }
      else if (fi.is_imagemesh())
      {
        fi.make_structquadsurfmesh();
        mesh = CreateMesh(fi,dims[0],dims[1]);
      }
      else if (fi.is_scanlinemesh())
      {
        fi.make_structcurvemesh();
        mesh = CreateMesh(fi,dims[0]);
      }

      if (!mesh)
      {
        error_str = "Could not allocate output mesh.";
        pr_->error(error_str);
        return (false);
      }

      // Copy the node locations
      VMesh* omesh = mesh->vmesh();
      VMesh::size_type num_nodes = omesh->num_nodes();
      for (VMesh::Node::index_type idx=0; idx<num_nodes;idx++)
      {
        Point pnt;
        vmesh->get_point(pnt,idx);
        omesh->set_point(pnt,idx);
      }
    }
    else
    {
      mesh.reset(field->mesh()->clone());
      if (!mesh)
      {
        error_str = "Could not allocate output mesh.";
        pr_->error(error_str);
        return (false);
      }
    }

    field = CreateField(fi,mesh);
    if (!field)
    {
      error_str = "Could not allocate output field.";
      pr_->error(error_str);
      return (false);
    }

    // Insert a copy of all values
    field->vfield()->copy_values(fieldmesh_[j].field_->vfield());

    std::string fieldname = fieldmesh_[j].field_name_;
    // Add the variable to the interpreter
    if(!(add_fieldmesh_sink(mprogram_,fieldname,field,error_str)))
    {
      pr_->error(error_str);
      return (false);
    }
    fieldmesh_[j].field_ = field;
  }

  // Create output fields (edit field data)
  for(size_t j=0; j< matrixdata_.size(); j++)
  {
    if (matrixdata_[j].matrix_)
    {
      std::string matrixname = matrixdata_[j].matrix_name_;
      if(!(add_matrix_sink(mprogram_,matrixname,matrixdata_[j].matrix_,error_str)))
      {
        pr_->error(error_str);
        return (false);
      }
    }
    else
    {
      std::string type;
      int flags = 0;
      get_output_variable_type(pprogram_,matrixdata_[j].array_name_,type,flags);
      size_type n = 0, m = 0;

      size_type size = matrixdata_[j].size_;
      if (size > 0)
      {
        if (flags & SCRIPT_SEQUENTIAL_VAR_E)
        {
          if (size != array_size_)
          {
            error_str = "Output matrix '"+matrixdata_[j].array_name_+"' size does not match the input object sizes.";
            pr_->error(error_str);
            return (false);
          }
          m = array_size_;
        }
        else
        {
          if (size != 1)
          {
            error_str = "Output matrix '"+matrixdata_[j].array_name_+"' size does not match the input object sizes.";
            pr_->error(error_str);
            return (false);
          }
          m = 1;
        }
      }
      else
      {
        if (flags & SCRIPT_SEQUENTIAL_VAR_E)
        {
          m = array_size_;
        }
        else
        {
          m = 1;
        }
      }
      if (type == "S")
      {
        n = 1;
      }
      else if (type == "V")
      {
        n = 3;
      }
      else if (type == "T")
      {
        n = 6;
      }

      MatrixHandle matrix(new DenseMatrix(m,n));
      if (!matrix)
      {
        error_str = "Could not allocate output matrix.";
        pr_->error(error_str);
        return (false);
      }

      std::string matrixname = matrixdata_[j].matrix_name_;
      // Add the variable to the interpreter
      if(!(add_matrix_sink(mprogram_,matrixname,matrix,error_str)))
      {
        pr_->error(error_str);
        return (false);
      }
      matrixdata_[j].matrix_ = matrix;
    }
  }

  for(size_t j=0; j< boolarraydata_.size(); j++)
  {
    if (boolarraydata_[j].bool_array_)
    {
      std::string arrayname = boolarraydata_[j].bool_array_name_;
      if(!(add_bool_array_sink(mprogram_,arrayname,boolarraydata_[j].bool_array_,error_str)))
      {
        pr_->error(error_str);
        return (false);
      }
    }
  }

  for(size_t j=0; j< intarraydata_.size(); j++)
  {
    if (intarraydata_[j].int_array_)
    {
      std::string arrayname = intarraydata_[j].int_array_name_;
      if(!(add_int_array_sink(mprogram_,arrayname,intarraydata_[j].int_array_,error_str)))
      {
        pr_->error(error_str);
        return (false);
      }
    }
  }

  for(size_t j=0; j< doublearraydata_.size(); j++)
  {
    if (doublearraydata_[j].double_array_)
    {
      std::string arrayname = doublearraydata_[j].double_array_name_;
      if(!(add_double_array_sink(mprogram_,arrayname,doublearraydata_[j].double_array_,error_str)))
      {
        pr_->error(error_str);
        return (false);
      }
    }
  }
  // Translate the code
  if (!(translate(pprogram_,mprogram_,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Set the final array size
  if(!(set_array_size(mprogram_,array_size_)))
  {
    pr_->error("Could not set array size.");
    return (false);
  }
  // Run the program
  if (!(ArrayMathInterpreter::run(mprogram_,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  return (true);
}



void
NewArrayMathEngine::clear()
{
  // Reset all values
  pprogram_ = 0;
  mprogram_ = 0;

  pre_expression_.clear();
  expression_.clear();
  post_expression_.clear();
  array_size_ = 1;

  fielddata_.clear();
  fieldmesh_.clear();
  matrixdata_.clear();
}
