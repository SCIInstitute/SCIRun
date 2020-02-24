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


// For memory management

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <Core/Parser/LinAlgInterpreter.h>
#include <Core/Parser/LinAlgFunctionCatalog.h>

using namespace SCIRun::Core::Datatypes;

namespace SCIRun {

  // Function constructor

  LinAlgFunction::LinAlgFunction(
    bool(*function)(LinAlgProgramCode& pc, std::string& err),
    std::string function_id,
    std::string function_type,
    int function_flags
    ) : ParserFunction(function_id, function_type, function_flags)
  {
    function_ = function;
  }


  // -------------------------------------------------------------------------
  // Create program if it has not yet been created

  bool LinAlgInterpreter::create_program(LinAlgProgramHandle& mprogram, std::string& error)
  {
    if (!mprogram)
    {
      mprogram.reset(new LinAlgProgram());

      if (!mprogram)
      {
        error = "INTERNAL ERROR - Could not allocate LinAlgProgram.";
        return (false);
      }
    }
    return (true);
  }

  // -------------------------------------------------------------------------
  // Translate a parser script

  bool LinAlgInterpreter::translate(ParserProgramHandle& pprogram, LinAlgProgramHandle& mprogram, std::string& error)
  {
    // Create program is needed
    if (!(create_program(mprogram, error))) return (false);
    mprogram->set_parser_program(pprogram);

    // -------------------------------------------------------------------------
    // Process const part of the program

    // Determine the buffer sizes that need to be allocated

    // Get the number of variables/function calls involved
    size_t num_const_variables = pprogram->num_const_variables();
    size_t num_const_functions = pprogram->num_const_functions();
    size_t num_single_variables = pprogram->num_single_variables();
    size_t num_single_functions = pprogram->num_single_functions();

    // Reserve space for const part of the program
    mprogram->resize_const_variables(num_const_variables);
    mprogram->resize_const_functions(num_const_functions);
    mprogram->resize_single_variables(num_single_variables);
    mprogram->resize_single_functions(num_single_functions);

    // Variable part
    ParserScriptVariableHandle chandle;
    ParserScriptVariableHandle vhandle;
    ParserScriptVariableHandle phandle;
    ParserScriptFunctionHandle fhandle;

    int onum, inum, kind;
    std::string type, name;
    int flags;

    // Now assign buffers to variables

    for (size_t j = 0; j < num_const_variables; j++)
    {
      // Get the next constant variable
      pprogram->get_const_variable(j, vhandle);

      // Determine the name of the variable
      name = vhandle->get_name();

      // Determine the type to see what we need to do
      // Rhis helps with determining the types of input and
      // output variables
      type = vhandle->get_type();

      // Determine the kind of the variable
      kind = vhandle->get_kind();

      LinAlgProgramVariableHandle pvhandle;

      // S = a Scalar variable. Basically a number, we translate this into
      // 1x1 matrix so all functions can be run on matrices
      if (type == "S")
      {
        // Insert constant variables directly into the buffer
        // This variable should be read only, hence we should be
        // able to store it right away

        MatrixHandle Scalar;
        if (kind == SCRIPT_CONSTANT_SCALAR_E)
        {
          double val = vhandle->get_scalar_value();
          MatrixHandle scalar(new DenseMatrix(1, 1, val));
          // Generate a new program variable with a preset value
          pvhandle.reset(new LinAlgProgramVariable(name, scalar));
        }
        else
        {
          // Generate a new program variable where the value is stored
          // In this interpreter all variables are computed on the fly
          // and do not need memory, one a handle to tell where it is
          // allocated.
          pvhandle.reset(new LinAlgProgramVariable(name));
        }
      }
      else
      {
        // Generate a new program variable where the value is stored
        // In this interpreter all variables are computed on the fly
        // and do not need memory, one a handle to tell where it is
        // allocated.
        pvhandle.reset(new LinAlgProgramVariable(name));
      }

      // Add this variable to the code
      mprogram->set_const_variable(j, pvhandle);
    }

    // This includes all constants and all the variables derived from it
    // It should not contain anything depending on input
    for (size_t j = 0; j < num_single_variables; j++)
    {
      // Get the next constant variable
      pprogram->get_single_variable(j, vhandle);

      // Determine the name of the variable
      name = vhandle->get_name();

      // Determine the type to see what we need to do
      type = vhandle->get_type();

      // Determine the kind of the variable
      kind = vhandle->get_kind();

      LinAlgProgramVariableHandle pvhandle;
      if (type == "S")
      {
        // Insert constant variables directly into the buffer
        // This variable should be read only, hence we should be
        // able to store it right away

        MatrixHandle Scalar;
        if (kind == SCRIPT_CONSTANT_SCALAR_E)
        {
          double val = vhandle->get_scalar_value();
          MatrixHandle scalar(new DenseMatrix(1, 1, val));
          // Generate a new program variable with a preset value
          pvhandle.reset(new LinAlgProgramVariable(name, scalar));
        }
        else
        {
          // Generate a new program variable where the value is stored
          // In this interpreter all variables are computed on the fly
          // and do not need memory, one a handle to tell where it is
          // allocated.
          pvhandle.reset(new LinAlgProgramVariable(name));
        }
      }
      else
      {
        // Generate a new program variable where the value is stored
        // In this interpreter all variables are computed on the fly
        // and do not need memory, one a handle to tell where it is
        // allocated.
        pvhandle.reset(new LinAlgProgramVariable(name));
      }

      // Add this variable to the code
      mprogram->set_single_variable(j, pvhandle);
    }

    // Function part
    LinAlgProgramSource ps;

    for (size_t j = 0; j < num_const_functions; j++)
    {
      // Get the function of that was defined in the parser
      pprogram->get_const_function(j, fhandle);

      // Set the function pointer, we up cast it to the real type as the parser
      // does not know anything about this interpreter and the argument the
      // function will take in.
      auto func = boost::dynamic_pointer_cast<LinAlgFunction>(fhandle->get_function());

      //Each function is a line in the parsed/interpreted code
      LinAlgProgramCode pc(func->get_function());

      // Get the pointer to the output variable
      ParserScriptVariableHandle ohandle = fhandle->get_output_var();

      // Extract the information from the node
      onum = ohandle->get_var_number();   // number in the list of variables
      type = ohandle->get_type();         // Type of variable
      name = ohandle->get_name();         // name of the variable
      flags = ohandle->get_flags();

      if (type == "S")
      {
        if (flags & SCRIPT_SINGLE_VAR_E)
        {
          pc.set_handle(0, mprogram->get_single_variable(onum)->get_handle());
        }
        else if (flags & SCRIPT_CONST_VAR_E)
        {
          pc.set_handle(0, mprogram->get_const_variable(onum)->get_handle());
        }
      }
      else if (type == "MO")
      {
        mprogram->find_sink(name, ps);
        if (ps.is_matrix())
        {
          pc.set_handle(0, ps.get_handle());
        }
        else
        {
          error = "INTERNAL ERROR - Variable is of Matrix type, but given sink is not a matrix.";
          return (false);
        }
      }
      else
      {
        error = "INTERNAL ERROR - Encountered unknown type.";
        return (false);
      }

      size_t num_input_vars = fhandle->num_input_vars();

      for (size_t i = 0; i < num_input_vars; i++)
      {
        // Extract information from the input nodes
        ParserScriptVariableHandle ihandle = fhandle->get_input_var(i);

        name = ihandle->get_name();
        inum = ihandle->get_var_number();
        type = ihandle->get_type();
        flags = ihandle->get_flags();

        if (type == "S")
        {
          if (flags & SCRIPT_SINGLE_VAR_E)
          {
            pc.set_handle(i + 1, mprogram->get_single_variable(inum)->get_handle());
          }
          else if (flags & SCRIPT_CONST_VAR_E)
          {
            pc.set_handle(i + 1, mprogram->get_const_variable(inum)->get_handle());
          }
        }
        else if (type == "MI")
        {
          mprogram->find_source(name, ps);
          if (ps.is_matrix())
          {
            pc.set_handle(i + 1, ps.get_handle());
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

      mprogram->set_const_program_code(j, pc);
    }

    for (size_t j = 0; j < num_single_functions; j++)
    {
      pprogram->get_single_function(j, fhandle);

      // Set the function pointer
      auto func = boost::dynamic_pointer_cast<LinAlgFunction>(fhandle->get_function());
      LinAlgProgramCode pc(func->get_function());

      ParserScriptVariableHandle ohandle = fhandle->get_output_var();
      onum = ohandle->get_var_number();
      type = ohandle->get_type();
      name = ohandle->get_name();
      flags = ohandle->get_flags();

      if ((type == "S"))
      {
        if (flags & SCRIPT_SINGLE_VAR_E)
        {
          pc.set_handle(0, mprogram->get_single_variable(onum)->get_handle());
        }
        else if (flags & SCRIPT_CONST_VAR_E)
        {
          pc.set_handle(0, mprogram->get_const_variable(onum)->get_handle());
        }
      }
      else if (type == "MO")
      {
        mprogram->find_sink(name, ps);
        if (ps.is_matrix())
        {
          pc.set_handle(0, ps.get_handle());
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
      for (size_t i = 0; i < num_input_vars; i++)
      {
        ParserScriptVariableHandle ihandle = fhandle->get_input_var(i);

        name = ihandle->get_name();
        inum = ihandle->get_var_number();
        type = ihandle->get_type();
        flags = ihandle->get_flags();

        if ((type == "S"))
        {
          if (flags & SCRIPT_SINGLE_VAR_E)
          {
            pc.set_handle(i + 1, mprogram->get_single_variable(inum)->get_handle());
          }
          else if (flags & SCRIPT_CONST_VAR_E)
          {
            pc.set_handle(i + 1, mprogram->get_const_variable(inum)->get_handle());
          }
        }
        else if (type == "MI")
        {
          mprogram->find_source(name, ps);
          if (ps.is_matrix())
          {
            pc.set_handle(i + 1, ps.get_handle());
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

      mprogram->set_single_program_code(j, pc);
    }

    return (true);
  }

  bool LinAlgInterpreter::run(LinAlgProgramHandle& mprogram, std::string& error)
  {
    // This does not optimally make use of the parser, in principal the
    // const part does not need to be generated after it is done once
    // Like wise the single part. For now we rerun everything every time
    ParserProgramHandle pprogram = mprogram->get_parser_program();
    ParserScriptFunctionHandle fhandle;

    // First: run the const ones
    size_t error_line;
    std::string err;

    if (!(mprogram->run_const(error_line, err)))
    {
      if (err == "")
      {
        pprogram->get_const_function(error_line, fhandle);
        error = " RUNTIME ERROR - const Function '" + fhandle->get_name() + "' crashed for unknown reason.";
        return (false);
      }
      else
      {
        pprogram->get_const_function(error_line, fhandle);
        error = " RUNTIME ERROR - const Function '" + fhandle->get_name() + "' failed: " + err;
        return (false);
      }
    }

    // Second: run the single ones
    if (!(mprogram->run_single(error_line, err)))
    {
      if (err == "")
      {
        pprogram->get_single_function(error_line, fhandle);
        error = " RUNTIME ERROR - single Function '" + fhandle->get_name() + "' crashed for unknown reason.";
        return (false);
      }
      else
      {
        pprogram->get_single_function(error_line, fhandle);
        error = " RUNTIME ERROR - single Function '" + fhandle->get_name() + "' failed: " + err;
        return (false);
      }
    }

    return (true);
  }


  bool LinAlgInterpreter::add_matrix_source(LinAlgProgramHandle& pprogram, const std::string& name, MatrixHandle matrix, std::string& error)
  {
    if (!(create_program(pprogram, error))) return (false);
    return(pprogram->add_source(name, matrix));
  }

  bool LinAlgInterpreter::add_matrix_sink(LinAlgProgramHandle& pprogram, const std::string& name,
    std::string& error)
  {
    if (!(create_program(pprogram, error))) return (false);
    return(pprogram->add_sink(name));
  }


  bool LinAlgProgram::add_source(const std::string& name, MatrixHandle matrix)
  {
    LinAlgProgramSource ps; ps.set_matrix(matrix);
    input_sources_[name] = ps;
    return (true);
  }


  bool LinAlgProgram::add_sink(const std::string& name)
  {
    LinAlgProgramSource ps; ps.set_matrix(0);
    output_sinks_[name] = ps;
    return (true);
  }


  bool LinAlgProgram::find_source(const std::string& name, LinAlgProgramSource& ps) const
  {
    auto it = input_sources_.find(name);
    if (it == input_sources_.end()) return (false);
    ps = (*it).second; return (true);
  }


  bool LinAlgProgram::find_sink(const std::string& name, LinAlgProgramSource& ps) const
  {
    auto it = output_sinks_.find(name);
    if (it == output_sinks_.end()) return (false);
    ps = (*it).second; return (true);
  }


  bool LinAlgProgram::run_const(size_t& error_line, std::string& err)
  {
    size_t size = const_functions_.size();
    for (size_t j = 0; j < size; j++)
    {
      if (!(const_functions_[j].run(err)))
      {
        error_line = j;
        return (false);
      }
    }
    return (true);
  }


  bool LinAlgProgram::run_single(size_t& error_line, std::string& err)
  {
    size_t size = single_functions_.size();
    for (size_t j = 0; j < size; j++)
    {
      if (!(single_functions_[j].run(err)))
      {
        error_line = j;
        return (false);
      }
    }
    return (true);
  }


  // Code for debugging

  void LinAlgProgramCode::print()
  {
    std::cout << "function_ = " << function_ << "\n";
    for (size_t j = 0; j < variables_.size(); j++)
    {
      std::cout << "variable[" << j << "]_ = " << variables_[j] << "\n";
    }
  }

} // end namespace
