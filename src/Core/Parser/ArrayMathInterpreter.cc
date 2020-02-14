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


#include <Core/Parser/ArrayMathInterpreter.h>
#include <Core/Parser/ArrayMathFunctionCatalog.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Thread/Parallel.h>
#include <boost/bind.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Thread;

ArrayMathFunction::ArrayMathFunction(
      ArrayMathFunctionPtr function,
      const std::string& function_id,
      const std::string& function_type,
      int function_flags
    ) : ParserFunction(function_id,function_type,function_flags), function_(function)
{
}

bool
ArrayMathInterpreter::create_program(ArrayMathProgramHandle& mprogram, std::string& error)
{
  if (!mprogram)
  {
    mprogram.reset(new ArrayMathProgram());

    if (!mprogram)
    {
      error = "INTERNAL ERROR - Could not allocate ArrayMathProgram.";
      return (false);
    }
  }
  return (true);
}

// -------------------------------------------------------------------------
// Translate a parser script

bool
ArrayMathInterpreter::translate(ParserProgramHandle& pprogram,
                                ArrayMathProgramHandle& mprogram,
                                std::string& error)
{
  // Create program is needed
  if(!(create_program(mprogram,error))) return (false);
  mprogram->set_parser_program(pprogram);

  // -------------------------------------------------------------------------
  // Process const part of the program

  // Determine the buffer sizes that need to be allocated

  // Get the number of variables/function calls involved
  size_t num_const_variables = pprogram->num_const_variables();
  size_t num_const_functions = pprogram->num_const_functions();
  size_t num_single_variables = pprogram->num_single_variables();
  size_t num_single_functions = pprogram->num_single_functions();
  size_t num_sequential_variables = pprogram->num_sequential_variables();
  size_t num_sequential_functions = pprogram->num_sequential_functions();

  // Reserve space for const part of the program
  mprogram->resize_const_variables(num_const_variables);
  mprogram->resize_const_functions(num_const_functions);
  mprogram->resize_single_variables(num_single_variables);
  mprogram->resize_single_functions(num_single_functions);
  mprogram->resize_sequential_variables(num_sequential_variables);
  mprogram->resize_sequential_functions(num_sequential_functions);

  // Variable part
  ParserScriptVariableHandle chandle;
  ParserScriptVariableHandle vhandle;
  ParserScriptVariableHandle phandle;
  ParserScriptFunctionHandle fhandle;
  int onum, inum, kind;
  std::string type, name;
  int flags;

  size_type buffer_mem = 0;
  // Determine how many space we need to reserve for const variables
  for (size_t j=0; j<num_const_variables; j++)
  {
    pprogram->get_const_variable(j,vhandle);
    std::string type = vhandle->get_type();

    if (type == "S") { buffer_mem += 1; }
    else if (type == "V") { buffer_mem += 3; }
    else if (type == "T") { buffer_mem += 6; }
    else if (type == "FD" || type == "M" || type == "FM" || type == "FE" ||
             type == "FN" || type == "AB" || type == "AI" || type == "AD")
    { buffer_mem += 0; }
    else
    {
      error = "INTERNAL ERROR - Encountered a type that is not supported by ArrayMath functions.";
      return (false);
    }
  }

  // Determine how many space we need to reserve for single variables
  for (size_t j=0; j<num_single_variables; j++)
  {
    pprogram->get_single_variable(j,vhandle);
    std::string type = vhandle->get_type();

    if (type == "S") { buffer_mem += 1; }
    else if (type == "V") { buffer_mem += 3; }
    else if (type == "T") { buffer_mem += 6; }
    else if (type == "FD" || type == "M" || type == "FM" || type == "FE" ||
             type == "FN" || type == "AB" || type == "AI" || type == "AD")
    { buffer_mem += 0; }
    else
    {
      error = "INTERNAL ERROR - Encountered a type that is not supported by ArrayMath functions.";
      return (false);
    }
  }

  // Determine how many space we need to reserve for sequential variables
  auto buffer_size = mprogram->get_buffer_size();
  int num_proc    = mprogram->get_num_proc();

  for (int np=0; np< num_proc; np++)
  {
    for (size_t j=0; j<num_sequential_variables; j++)
    {
      pprogram->get_sequential_variable(j,vhandle);
      std::string type = vhandle->get_type();
      flags = vhandle->get_flags();

      if ((flags & SCRIPT_CONST_VAR_E)&&(np > 0)) continue;
      if (type == "S") { buffer_mem += 1*buffer_size; }
      else if (type == "V") { buffer_mem += 3*buffer_size; }
      else if (type == "T") { buffer_mem += 6*buffer_size; }
      else if (type == "FD" || type == "M" || type == "FM" || type == "FE" ||
               type == "FN" || type == "AB" || type == "AI" || type == "AD")
      { buffer_mem += 0; }
      else
      {
        error = "INTERNAL ERROR - Encountered a type that is not supported by ArrayMath functions.";
        return (false);
      }
    }
  }

  // Now assign buffers to variables

  // Get all the constants in one piece of memory
  // All memory management is inside the ArrayMathProgram
  // We only get the pointer to actually insert all the pieces
  double* buffer = mprogram->create_buffer(buffer_mem);

  if (! buffer)
  {
    error = "INTERNAL ERROR - Empty array math buffer.";
    return false;
  }

  for (size_t j=0; j<num_const_variables; j++)
  {
    // Get the next constant variable
    pprogram->get_const_variable(j,vhandle);

    // Determine the name of the variable
    name = vhandle->get_name();

    // Determine the type to see what we need to do
    type = vhandle->get_type();

    // Determine the type of the variable
    kind = vhandle->get_kind();

    ArrayMathProgramVariableHandle pvhandle;
    if (type == "S")
    {
      // Insert constant variables directly into the buffer
      // This variable should be read only, hence we should be
      // able to store it right away
      if (kind == SCRIPT_CONSTANT_SCALAR_E)
      {
        double val =  vhandle->get_scalar_value();
        buffer[0] = val;
      }

      // Generate a new program variable
      pvhandle.reset(new ArrayMathProgramVariable(name,buffer));
      buffer += 1;
    }
    else if (type == "V")
    {
      // A Vector generated from only constants

      // Generate a new program variable where the value is stored
      pvhandle.reset(new ArrayMathProgramVariable(name,buffer));
      buffer += 3;
    }
    else if (type == "T")
    {
      // A Tensor generated from only constants

      // Generate a new program variable where the value is stored
      pvhandle.reset(new ArrayMathProgramVariable(name,buffer));
      buffer += 6;
    }
    else
    {
      // Generate a new program variable where the value is stored
      // This is a source or sink that does not need memory
      pvhandle.reset(new ArrayMathProgramVariable(name,0));
    }

    // Add this variable to the code
    mprogram->set_const_variable(j,pvhandle);
  }

  for (size_t j=0; j<num_single_variables; j++)
  {
    // Get the next constant variable
    pprogram->get_single_variable(j,vhandle);

    // Determine the name of the variable
    name = vhandle->get_name();

    // Determine the type to see what we need to do
    type = vhandle->get_type();

    // Determine the type of the variable
    kind = vhandle->get_kind();

    ArrayMathProgramVariableHandle pvhandle;
    if (type == "S")
    {
      // Generate a new program variable
      pvhandle.reset(new ArrayMathProgramVariable(name,buffer));
      buffer += 1;
    }
    else if (type == "V")
    {
      // Generate a new program variable where the value is stored
      pvhandle.reset(new ArrayMathProgramVariable(name,buffer));
      buffer += 3;
    }
    else if (type == "T")
    {
      // Generate a new program variable where the value is stored
      pvhandle.reset(new ArrayMathProgramVariable(name,buffer));
      buffer += 6;
    }
    else
    {
      // Generate a new program variable where the value is stored
      // This is a source or sink that does not need memory
      pvhandle.reset(new ArrayMathProgramVariable(name,0));
    }

    // Add this variable to the code
    mprogram->set_single_variable(j,pvhandle);
  }

  for (int np=0; np< num_proc; np++)
  {
    for (size_t j=0; j<num_sequential_variables; j++)
    {
      // Get the next constant variable
      pprogram->get_sequential_variable(j,vhandle);

      // Determine the name of the variable
      name = vhandle->get_name();

      // Determine the type to see what we need to do
      type = vhandle->get_type();

      // Determine the type of the variable
      kind = vhandle->get_kind();

      flags = vhandle->get_flags();
      if ((flags & SCRIPT_CONST_VAR_E)&&(np > 0)) continue;

      ArrayMathProgramVariableHandle pvhandle;
      if (type == "S")
      {
        // Generate a new program variable
        pvhandle.reset(new ArrayMathProgramVariable(name,buffer));
        buffer += 1*buffer_size;
      }
      else if (type == "V")
      {
        // Generate a new program variable where the value is stored
        pvhandle.reset(new ArrayMathProgramVariable(name,buffer));
        buffer += 3*buffer_size;
      }
      else if (type == "T")
      {
        // Generate a new program variable where the value is stored
        pvhandle.reset(new ArrayMathProgramVariable(name,buffer));
        buffer += 6*buffer_size;
      }
      else
      {
        // Generate a new program variable where the value is stored
        // This is a source or sink that does not need memory
        pvhandle.reset(new ArrayMathProgramVariable(name,0));
      }

      // Add this variable to the code
      if (flags & SCRIPT_CONST_VAR_E)
      {
        for (int p=0; p< num_proc; p++)
        {
          mprogram->set_sequential_variable(j,p,pvhandle);
        }
      }
      else
      {
        mprogram->set_sequential_variable(j,np,pvhandle);
      }
    }
  }

  // Function part
  ArrayMathProgramSource ps;

  for (size_t j=0; j<num_const_functions; j++)
  {
    pprogram->get_const_function(j,fhandle);
    // Set the function pointer
    auto func = boost::dynamic_pointer_cast<ArrayMathFunction>(fhandle->get_function());
    ArrayMathProgramCodePtr pcPtr(new ArrayMathProgramCode(func->get_function()));
    ArrayMathProgramCode& pc = *pcPtr;
    pc.set_size(1);
    pc.set_index(0);

    ParserScriptVariableHandle ohandle = fhandle->get_output_var();
    onum = ohandle->get_var_number();
    type = ohandle->get_type();
    name = ohandle->get_name();
    flags = ohandle->get_flags();

    if (type == "S" || type == "V" || type == "T" )
    {
      if (flags & SCRIPT_SEQUENTIAL_VAR_E)
      {
        // These are sequenced variables and hence all multi threaded buffers
        // are equal
        pc.set_variable(0,mprogram->get_sequential_variable(onum,0)->get_data());
        pc.set_size(buffer_size);
      }
      else if (flags & SCRIPT_SINGLE_VAR_E)
      {
        pc.set_variable(0,mprogram->get_single_variable(onum)->get_data());
      }
      else if (flags & SCRIPT_CONST_VAR_E)
      {
        pc.set_variable(0,mprogram->get_const_variable(onum)->get_data());
      }
    }
    else if (type == "FD")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_vfield())
      {
        pc.set_vfield(0,ps.get_vfield());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Field type, but given source is not a field.";
        return (false);
      }
    }
    else if (type == "FM")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_vmesh())
      {
        pc.set_vmesh(0,ps.get_vmesh());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
        return (false);
      }
    }
    else if (type == "FN")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_vmesh())
      {
        pc.set_vmesh(0,ps.get_vmesh());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
        return (false);
      }
    }
    else if (type == "FE")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_vmesh())
      {
        pc.set_vmesh(0,ps.get_vmesh());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
        return (false);
      }
    }
    else if (type == "AB")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_bool_array())
      {
        pc.set_bool_array(0,ps.get_bool_array());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a bool array.";
        return (false);
      }
    }
    else if (type == "AI")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_int_array())
      {
        pc.set_int_array(0,ps.get_int_array());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a int array.";
        return (false);
      }
    }
    else if (type == "AD")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_double_array())
      {
        pc.set_double_array(0,ps.get_double_array());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a double array.";
        return (false);
      }
    }
    else if (type == "M")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_matrix())
      {
        pc.set_matrix(0,ps.get_matrix());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Matrix type, but given source is not a matrix.";
        return (false);
      }
    }
    else
    {
      error = "INTERNAL ERROR - Encountered unknown type.";
      return (false);
    }

    size_t num_input_vars = fhandle->num_input_vars();
    for (size_t i=0; i < num_input_vars; i++)
    {
      ParserScriptVariableHandle ihandle = fhandle->get_input_var(i);
      name = ihandle->get_name();
      inum = ihandle->get_var_number();
      type = ihandle->get_type();
      flags = ihandle->get_flags();
      if (type == "S" || type == "V" || type == "T" )
      {
        if(flags & SCRIPT_SEQUENTIAL_VAR_E)
        {
          pc.set_variable(i+1,mprogram->get_sequential_variable(inum,0)->get_data());
        }
        else if(flags & SCRIPT_SINGLE_VAR_E)
        {
          pc.set_variable(i+1,mprogram->get_single_variable(inum)->get_data());
        }
        else if (flags & SCRIPT_CONST_VAR_E)
        {
          pc.set_variable(i+1,mprogram->get_const_variable(inum)->get_data());
        }

      }
      else if (type == "FD")
      {
        mprogram->find_source(name,ps);
        if (ps.is_vfield())
        {
          pc.set_vfield(i+1,ps.get_vfield());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Field type, but given source is not a field.";
          return (false);
        }
      }
      else if (type == "FM")
      {
        mprogram->find_source(name,ps);
        if (ps.is_vmesh())
        {
          pc.set_vmesh(i+1,ps.get_vmesh());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
          return (false);
        }
      }
      else if (type == "FE")
      {
        mprogram->find_source(name,ps);
        if (ps.is_vmesh())
        {
          pc.set_vmesh(i+1,ps.get_vmesh());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
          return (false);
        }
      }
      else if (type == "FN")
      {
        mprogram->find_source(name,ps);
        if (ps.is_vmesh())
        {
          pc.set_vmesh(i+1,ps.get_vmesh());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
          return (false);
        }
      }
      else if (type == "AB")
      {
        mprogram->find_source(name,ps);
        if (ps.is_bool_array())
        {
          pc.set_bool_array(i+1,ps.get_bool_array());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a bool array.";
          return (false);
        }
      }
      else if (type == "AI")
      {
        mprogram->find_source(name,ps);
        if (ps.is_int_array())
        {
          pc.set_int_array(i+1,ps.get_int_array());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a int array.";
          return (false);
        }
      }
      else if (type == "AD")
      {
        mprogram->find_source(name,ps);
        if (ps.is_double_array())
        {
          pc.set_double_array(i+1,ps.get_double_array());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a double array.";
          return (false);
        }
      }
      else if (type == "M")
      {
        mprogram->find_source(name,ps);
        if (ps.is_matrix())
        {
          pc.set_matrix(i+1,ps.get_matrix());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Matrix type, but given source is not a matrix.";
          return (false);
        }
      }
      else
      {
        error = "INTERNAL ERROR - Encountered unknown type.";
        return (false);
      }
    }
    mprogram->set_const_program_code(j,pcPtr);
  }

  for (size_t j=0; j<num_single_functions; j++)
  {
    pprogram->get_single_function(j,fhandle);

    // Set the function pointer
    auto func = boost::dynamic_pointer_cast<ArrayMathFunction>(fhandle->get_function());
    ArrayMathProgramCodePtr pcPtr(new ArrayMathProgramCode(func->get_function()));
    ArrayMathProgramCode& pc = *pcPtr;

    pc.set_size(1);
    pc.set_index(0);

    ParserScriptVariableHandle ohandle = fhandle->get_output_var();
    onum = ohandle->get_var_number();
    type = ohandle->get_type();
    name = ohandle->get_name();
    flags = ohandle->get_flags();

    if (type == "S" || type == "V" || type == "T" )
    {
      if (flags & SCRIPT_SEQUENTIAL_VAR_E)
      {
        // These are sequenced variables and hence all multi threaded buffers
        // are equal
        pc.set_size(buffer_size);
        pc.set_variable(0,mprogram->get_sequential_variable(onum,0)->get_data());
      }
      else if (flags & SCRIPT_SINGLE_VAR_E)
      {
        pc.set_variable(0,mprogram->get_single_variable(onum)->get_data());
      }
      else if (flags & SCRIPT_CONST_VAR_E)
      {
        pc.set_variable(0,mprogram->get_const_variable(onum)->get_data());
      }

    }
    else if (type == "FD")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_vfield())
      {
        pc.set_vfield(0,ps.get_vfield());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Field type, but given source is not a field.";
        return (false);
      }
    }
    else if (type == "FM")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_vmesh())
      {
        pc.set_vmesh(0,ps.get_vmesh());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
        return (false);
      }
    }
    else if (type == "FN")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_vmesh())
      {
        pc.set_vmesh(0,ps.get_vmesh());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
        return (false);
      }
    }
    else if (type == "FE")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_vmesh())
      {
        pc.set_vmesh(0,ps.get_vmesh());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
        return (false);
      }
    }
    else if (type == "AB")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_bool_array())
      {
        pc.set_bool_array(0,ps.get_bool_array());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a bool array.";
        return (false);
      }
    }
    else if (type == "AI")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_int_array())
      {
        pc.set_int_array(0,ps.get_int_array());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a int array.";
        return (false);
      }
    }
    else if (type == "AD")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_double_array())
      {
        pc.set_double_array(0,ps.get_double_array());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a double array.";
        return (false);
      }
    }
    else if (type == "M")
    {
      mprogram->find_sink(name,ps);
      if (ps.is_matrix())
      {
        pc.set_matrix(0,ps.get_matrix());
      }
      else
      {
        error = "INTERNAL ERROR - Variable is of Matrix type, but given source is not a matrix.";
        return (false);
      }
    }
    else
    {
      error = "INTERNAL ERROR - Encountered unknown type.";
      return (false);
    }


    size_t num_input_vars = fhandle->num_input_vars();
    for (size_t i=0; i < num_input_vars; i++)
    {
      ParserScriptVariableHandle ihandle = fhandle->get_input_var(i);
      name = ihandle->get_name();
      inum = ihandle->get_var_number();
      type = ihandle->get_type();
      flags = ihandle->get_flags();
      if (type == "S" || type == "V" || type == "T" )
      {
        if(flags & SCRIPT_SEQUENTIAL_VAR_E)
        {
          pc.set_variable(i+1,mprogram->get_sequential_variable(inum,0)->get_data());
        }
        else if(flags & SCRIPT_SINGLE_VAR_E)
        {
          pc.set_variable(i+1,mprogram->get_single_variable(inum)->get_data());
        }
        else if (flags & SCRIPT_CONST_VAR_E)
        {
          pc.set_variable(i+1,mprogram->get_const_variable(inum)->get_data());
        }
      }
      else if (type == "FD")
      {
        mprogram->find_source(name,ps);
        if (ps.is_vfield())
        {
          pc.set_vfield(i+1,ps.get_vfield());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Field type, but given source is not a field.";
          return (false);
        }
      }
      else if (type == "FM")
      {
        mprogram->find_source(name,ps);
        if (ps.is_vmesh())
        {
          pc.set_vmesh(i+1,ps.get_vmesh());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
          return (false);
        }
      }
      else if (type == "FN")
      {
        mprogram->find_source(name,ps);
        if (ps.is_vmesh())
        {
          pc.set_vmesh(i+1,ps.get_vmesh());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
          return (false);
        }
      }
      else if (type == "FE")
      {
        mprogram->find_source(name,ps);
        if (ps.is_vmesh())
        {
          pc.set_vmesh(i+1,ps.get_vmesh());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
          return (false);
        }
      }
      else if (type == "AB")
      {
        mprogram->find_source(name,ps);
        if (ps.is_bool_array())
        {
          pc.set_bool_array(i+1,ps.get_bool_array());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a bool array.";
          return (false);
        }
      }
      else if (type == "AI")
      {
        mprogram->find_source(name,ps);
        if (ps.is_int_array())
        {
          pc.set_int_array(i+1,ps.get_int_array());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a int array.";
          return (false);
        }
      }
      else if (type == "AD")
      {
        mprogram->find_source(name,ps);
        if (ps.is_double_array())
        {
          pc.set_double_array(i+1,ps.get_double_array());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a double array.";
          return (false);
        }
      }
      else if (type == "M")
      {
        mprogram->find_source(name,ps);
        if (ps.is_matrix())
        {
          pc.set_matrix(i+1,ps.get_matrix());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Matrix type, but given source is not a matrix.";
          return (false);
        }
      }
      else
      {
        error = "INTERNAL ERROR - Encountered unknown type.";
        return (false);
      }
    }

    mprogram->set_single_program_code(j,pcPtr);
  }

  // Process sequential list
  for (int np=0; np< num_proc; np++)
  {
    for (size_t j=0; j<num_sequential_functions; j++)
    {
      pprogram->get_sequential_function(j,fhandle);

      // Set the function pointer
      auto func = boost::dynamic_pointer_cast<ArrayMathFunction>(fhandle->get_function());
      ArrayMathProgramCodePtr pcPtr(new ArrayMathProgramCode(func->get_function()));
      ArrayMathProgramCode& pc = *pcPtr;

      ParserScriptVariableHandle ohandle = fhandle->get_output_var();
      onum = ohandle->get_var_number();
      type = ohandle->get_type();
      name = ohandle->get_name();

      if (type == "S" || type == "V" || type == "T" )
      {
        pc.set_variable(0,mprogram->get_sequential_variable(onum,np)->get_data());
      }
      else if (type == "FD")
      {
        mprogram->find_sink(name,ps);
        if (ps.is_vfield())
        {
          pc.set_vfield(0,ps.get_vfield());
        }
        else
        {
          error = "INTERNAL ERROR - Variable '"+name+"' is of Field type, but given source is not a field.";
          return (false);
        }
      }
      else if (type == "FM")
      {
        mprogram->find_sink(name,ps);
        if (ps.is_vmesh())
        {
          pc.set_vmesh(0,ps.get_vmesh());
        }
        else
        {
          error = "INTERNAL ERROR - Variable '"+name+"' is of Mesh type, but given source is not a mesh.";
          return (false);
        }
      }
      else if (type == "FN")
      {
        mprogram->find_sink(name,ps);
        if (ps.is_vmesh())
        {
          pc.set_vmesh(0,ps.get_vmesh());
        }
        else
        {
          error = "INTERNAL ERROR - Variable '"+name+"' is of Mesh type, but given source is not a mesh.";
          return (false);
        }
      }
      else if (type == "FE")
      {
        mprogram->find_sink(name,ps);
        if (ps.is_vmesh())
        {
          pc.set_vmesh(0,ps.get_vmesh());
        }
        else
        {
          error = "INTERNAL ERROR - Variable '"+name+"' is of Mesh type, but given source is not a mesh.";
          return (false);
        }
      }
      else if (type == "AB")
      {
        mprogram->find_sink(name,ps);
        if (ps.is_bool_array())
        {
          pc.set_bool_array(0,ps.get_bool_array());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a bool array.";
          return (false);
        }
      }
      else if (type == "AI")
      {
        mprogram->find_sink(name,ps);
        if (ps.is_int_array())
        {
          pc.set_int_array(0,ps.get_int_array());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a int array.";
          return (false);
        }
      }
      else if (type == "AD")
      {
        mprogram->find_sink(name,ps);
        if (ps.is_double_array())
        {
          pc.set_double_array(0,ps.get_double_array());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a double array.";
          return (false);
        }
      }
      else if (type == "M")
      {
        mprogram->find_sink(name,ps);
        if (ps.is_matrix())
        {
          pc.set_matrix(0,ps.get_matrix());
        }
        else
        {
          error = "INTERNAL ERROR - Variable '"+name+"' is of Matrix type, but given source is not a matrix.";
          return (false);
        }
      }
      else
      {
        error = "INTERNAL ERROR - Encountered unknown type.";
        return (false);
      }

      size_t num_input_vars = fhandle->num_input_vars();
      for (size_t i=0; i < num_input_vars; i++)
      {
        ParserScriptVariableHandle ihandle = fhandle->get_input_var(i);
        name = ihandle->get_name();
        inum = ihandle->get_var_number();
        type = ihandle->get_type();
        flags = ihandle->get_flags();

        if (type == "S" || type == "V" || type == "T" )
        {
          if(flags & SCRIPT_SEQUENTIAL_VAR_E)
          {
            if (flags & SCRIPT_CONST_VAR_E)
              pc.set_variable(i+1,mprogram->get_sequential_variable(inum,0)->get_data());
            else
              pc.set_variable(i+1,mprogram->get_sequential_variable(inum,np)->get_data());
          }

          else if(flags & SCRIPT_SINGLE_VAR_E)
          {
            pc.set_variable(i+1,mprogram->get_single_variable(inum)->get_data());
          }
          else if (flags & SCRIPT_CONST_VAR_E)
          {
            pc.set_variable(i+1,mprogram->get_const_variable(inum)->get_data());
          }
        }
        else if (type == "FD")
        {
          mprogram->find_source(name,ps);
          if (ps.is_vfield())
          {
            pc.set_vfield(i+1,ps.get_vfield());
          }
          else
          {
            error = "INTERNAL ERROR - Variable is of Field type, but given source is not a field.";
            return (false);
          }
        }
        else if (type == "FM")
        {
          mprogram->find_source(name,ps);
          if (ps.is_vmesh())
          {
            pc.set_vmesh(i+1,ps.get_vmesh());
          }
          else
          {
            error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
            return (false);
          }
        }
        else if (type == "FE")
        {
          mprogram->find_source(name,ps);
          if (ps.is_vmesh())
          {
            pc.set_vmesh(i+1,ps.get_vmesh());
          }
          else
          {
            error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
            return (false);
          }
        }
        else if (type == "FN")
        {
          mprogram->find_source(name,ps);
          if (ps.is_vmesh())
          {
            pc.set_vmesh(i+1,ps.get_vmesh());
          }
          else
          {
            error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a mesh.";
            return (false);
          }
        }
        else if (type == "AB")
        {
          mprogram->find_source(name,ps);
          if (ps.is_bool_array())
          {
            pc.set_bool_array(i+1,ps.get_bool_array());
          }
          else
          {
            error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a bool array.";
            return (false);
          }
        }
        else if (type == "AI")
        {
          mprogram->find_source(name,ps);
          if (ps.is_int_array())
          {
            pc.set_int_array(i+1,ps.get_int_array());
          }
          else
          {
            error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a int array.";
            return (false);
          }
        }
        else if (type == "AD")
        {
          mprogram->find_source(name,ps);
          if (ps.is_double_array())
          {
            pc.set_double_array(i+1,ps.get_double_array());
          }
          else
          {
            error = "INTERNAL ERROR - Variable is of Mesh type, but given source is not a double array.";
            return (false);
          }
        }
        else if (type == "M")
        {
          mprogram->find_source(name,ps);
          if (ps.is_matrix())
          {
            pc.set_matrix(i+1,ps.get_matrix());
          }
          else
          {
            error = "INTERNAL ERROR - Variable is of Matrix type, but given source is not a matrix.";
            return (false);
          }
        }
        else
        {
          error = "INTERNAL ERROR - Encountered unknown type.";
          return (false);
        }
      }
      mprogram->set_sequential_program_code(j,np,pcPtr);
    }
  }

  return (true);
}

bool
ArrayMathInterpreter::run(ArrayMathProgramHandle& mprogram,
                                std::string& error)
{
  // This does not optimally make use of the parser, in principal the
  // const part does not need to be generated after it is done once
  // Like wise the single part. For now we rerun everything every time
  ParserProgramHandle pprogram = mprogram->get_parser_program();
  ParserScriptFunctionHandle fhandle;

  // First: run the const ones
  size_t error_line;
  if(!mprogram->run_const(error_line))
  {
    pprogram->get_const_function(error_line,fhandle);
    error = " RUNTIME ERROR - Function '"+fhandle->get_name()+"' crashed. Try simplifying your expressions or removing duplicate assignments.";
    return (false);
  }

  // Second: run the single ones
  if(!mprogram->run_single(error_line))
  {
    pprogram->get_single_function(error_line,fhandle);
    error = " RUNTIME ERROR - Function '"+fhandle->get_name()+"' crashed. Try simplifying your expressions or removing duplicate assignments.";
    return (false);
  }

  // Third: run the sequential ones
  if(!mprogram->run_sequential(error_line))
  {
    pprogram->get_sequential_function(error_line,fhandle);
    error = " RUNTIME ERROR - Function '"+fhandle->get_name()+"' crashed. Try simplifying your expressions or removing duplicate assignments.";
    return (false);
  }

  return (true);
}


bool
ArrayMathInterpreter::add_fielddata_source(ArrayMathProgramHandle& pprogram,
                                           const std::string& name,
                                           FieldHandle field,
                                           std::string& error)
{
  if (!create_program(pprogram,error))
    return (false);
  return pprogram->add_source(name,field->vfield());
}

bool
ArrayMathInterpreter::add_fieldmesh_source(ArrayMathProgramHandle& pprogram,
                                           const std::string& name,
                                           FieldHandle field,
                                           std::string& error)
{
  if(!(create_program(pprogram,error))) return (false);
  return(pprogram->add_source(name,field->vmesh()));
}


bool
ArrayMathInterpreter::add_matrix_source(ArrayMathProgramHandle& pprogram,
                                        const std::string& name,
                                        MatrixHandle matrix,
                                        std::string& error)
{
  if(!(create_program(pprogram,error))) return (false);
  return(pprogram->add_source(name,matrix));
}

bool
ArrayMathInterpreter::add_bool_array_source(ArrayMathProgramHandle& pprogram,
                                        const std::string& name,
                                        std::vector<bool>* array,
                                        std::string& error)
{
  if(!(create_program(pprogram,error))) return (false);
  return(pprogram->add_source(name,array));
}

bool
ArrayMathInterpreter::add_int_array_source(ArrayMathProgramHandle& pprogram,
                                        const std::string& name,
                                        std::vector<int>* array,
                                        std::string& error)
{
  if(!(create_program(pprogram,error))) return (false);
  return(pprogram->add_source(name,array));
}

bool
ArrayMathInterpreter::add_double_array_source(ArrayMathProgramHandle& pprogram,
                                        const std::string& name,
                                        std::vector<double>* array,
                                        std::string& error)
{
  if(!(create_program(pprogram,error))) return (false);
  return(pprogram->add_source(name,array));
}


bool
ArrayMathInterpreter::add_fielddata_sink(ArrayMathProgramHandle& pprogram,
                                         const std::string& name,
                                         FieldHandle field,
                                         std::string& error)
{
  if(!(create_program(pprogram,error))) return (false);
  return(pprogram->add_sink(name,field->vfield()));
}

bool
ArrayMathInterpreter::add_fieldmesh_sink(ArrayMathProgramHandle& pprogram,
                                         const std::string& name,
                                         FieldHandle field,
                                         std::string& error)
{
  if(!(create_program(pprogram,error))) return (false);
  return(pprogram->add_sink(name,field->vmesh()));
}

bool
ArrayMathInterpreter::add_matrix_sink(ArrayMathProgramHandle& pprogram,
                                      const std::string& name,
                                      MatrixHandle matrix,
                                      std::string& error)
{
  if(!(create_program(pprogram,error))) return (false);
  return(pprogram->add_sink(name,matrix));
}

bool
ArrayMathInterpreter::add_bool_array_sink(ArrayMathProgramHandle& pprogram,
                                          const std::string& name,
                                          std::vector<bool>* array,
                                          std::string& error)
{
  if(!(create_program(pprogram,error))) return (false);
  return(pprogram->add_sink(name,array));
}

bool
ArrayMathInterpreter::add_int_array_sink(ArrayMathProgramHandle& pprogram,
                                         const std::string& name,
                                         std::vector<int>* array,
                                         std::string& error)
{
  if(!(create_program(pprogram,error))) return (false);
  return(pprogram->add_sink(name,array));
}

bool
ArrayMathInterpreter::add_double_array_sink(ArrayMathProgramHandle& pprogram,
                                            const std::string& name,
                                            std::vector<double>* array,
                                            std::string& error)
{
  if(!(create_program(pprogram,error))) return (false);
  return(pprogram->add_sink(name,array));
}


bool
ArrayMathInterpreter::set_array_size(ArrayMathProgramHandle& pprogram,
                                     size_type array_size)
{
  pprogram->set_array_size(array_size);
  return (true);
}



bool
ArrayMathProgram::add_source(const std::string& name, VField* vfield)
{
  ArrayMathProgramSource ps; ps.set_vfield(vfield);
  input_sources_[name] = ps;
  return (true);
}

bool
ArrayMathProgram::add_source(const std::string& name, VMesh* vmesh)
{
  ArrayMathProgramSource ps; ps.set_vmesh(vmesh);
  input_sources_[name] = ps;
  return (true);
}

bool
ArrayMathProgram::add_source(const std::string& name, MatrixHandle matrix)
{
  ArrayMathProgramSource ps; ps.set_matrix(matrix);
  input_sources_[name] = ps;
  return (true);
}

bool
ArrayMathProgram::add_source(const std::string& name, std::vector<bool>* array)
{
  ArrayMathProgramSource ps; ps.set_bool_array(array);
  input_sources_[name] = ps;
  return (true);
}

bool
ArrayMathProgram::add_source(const std::string& name, std::vector<int>* array)
{
  ArrayMathProgramSource ps; ps.set_int_array(array);
  input_sources_[name] = ps;
  return (true);
}

bool
ArrayMathProgram::add_source(const std::string& name, std::vector<double>* array)
{
  ArrayMathProgramSource ps; ps.set_double_array(array);
  input_sources_[name] = ps;
  return (true);
}


bool
ArrayMathProgram::add_sink(const std::string& name, VField* vfield)
{
  ArrayMathProgramSource ps; ps.set_vfield(vfield);
  output_sinks_[name] = ps;
  return (true);
}

bool
ArrayMathProgram::add_sink(const std::string& name, VMesh* vmesh)
{
  ArrayMathProgramSource ps; ps.set_vmesh(vmesh);
  output_sinks_[name] = ps;
  return (true);
}

bool
ArrayMathProgram::add_sink(const std::string& name, MatrixHandle matrix)
{
  ArrayMathProgramSource ps; ps.set_matrix(matrix);
  output_sinks_[name] = ps;
  return (true);
}

bool
ArrayMathProgram::add_sink(const std::string& name, std::vector<bool>* array)
{
  ArrayMathProgramSource ps; ps.set_bool_array(array);
  output_sinks_[name] = ps;
  return (true);
}

bool
ArrayMathProgram::add_sink(const std::string& name, std::vector<int>* array)
{
  ArrayMathProgramSource ps; ps.set_int_array(array);
  output_sinks_[name] = ps;
  return (true);
}

bool
ArrayMathProgram::add_sink(const std::string& name, std::vector<double>* array)
{
  ArrayMathProgramSource ps; ps.set_double_array(array);
  output_sinks_[name] = ps;
  return (true);
}

bool
ArrayMathProgram::find_source(const std::string& name,  ArrayMathProgramSource& ps)
{
 std::map<std::string,ArrayMathProgramSource>::iterator it = input_sources_.find(name);
 if (it == input_sources_.end()) return (false);
 ps = (*it).second; return (true);
}

bool
ArrayMathProgram::find_sink(const std::string& name,  ArrayMathProgramSource& ps)
{
 std::map<std::string,ArrayMathProgramSource>::iterator it = output_sinks_.find(name);
 if (it == output_sinks_.end()) return (false);
 ps = (*it).second; return (true);
}

bool
ArrayMathProgram::run_const(size_t& error_line)
{
  size_t size = const_functions_.size();
  for (size_t j=0; j<size; j++)
  {
    if(!(const_functions_[j]->run()))
    {
      error_line = j;
      return (false);
    }
  }

  return (true);
}

bool
ArrayMathProgram::run_single(size_t& error_line)
{
  size_t size = single_functions_.size();
  for (size_t j=0; j<size; j++)
  {
    if(!(single_functions_[j]->run()))
    {
      error_line = j;
      return (false);
    }
  }
  return (true);
}


bool
ArrayMathProgram::run_sequential(size_t& error_line)
{
  error_line_.resize(num_proc_,0);
  success_.resize(num_proc_,true);

  Parallel::RunTasks(boost::bind(&ArrayMathProgram::run_parallel, this, _1), num_proc_);

  for (int j=0; j<num_proc_; j++)
  {
    if (!success_[j])
    {
      error_line = error_line_[j];
      return (false);
    }
  }

  return (true);
}

void
ArrayMathProgram::run_parallel(int proc)
{
  index_type per_thread = array_size_/num_proc_;
  index_type start = proc*per_thread;
  index_type end   = (proc+1)*per_thread;
  index_type offset, sz;
  if (proc+1 == num_proc_) end = array_size_;

  offset = start;
  success_[proc] = true;

  while (offset < end)
  {
    sz = buffer_size_;
    if (offset+sz >= end) sz = end-offset;

    size_t size = sequential_functions_[proc].size();
    for (size_t j=0; j<size;j++)
    {
      sequential_functions_[proc][j]->set_index(offset);
      sequential_functions_[proc][j]->set_size(sz);
    }
    for (size_t j=0; j<size; j++)
    {
      if(!(sequential_functions_[proc][j]->run()))
      {
        error_line_[proc] = j;
        success_[proc] = false;
      }
    }
    offset += sz;
  }

  barrier_.wait();
}


void
ArrayMathProgramCode::print() const
{
  std::cout << "function_ = "<<function_<<"\n";
  for (size_t j=0;j<variables_.size(); j++)
  std::cout << "variable["<<j<<"]_ = "<<variables_[j]<<"\n";
}
