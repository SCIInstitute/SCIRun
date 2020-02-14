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


#ifndef CORE_PARSER_LINALGINTERPRETER_H
#define CORE_PARSER_LINALGINTERPRETER_H 1

#include <Core/Datatypes/Matrix.h>
#include <Core/Parser/Parser.h>

// Include files needed for Windows
#include <Core/Parser/share.h>

namespace SCIRun {

// Define class as they are not used in order

class LinAlgIntepreter;
class LinAlgFunction;

class LinAlgProgram;
class LinAlgProgramCode;
class LinAlgProgramVariable;

// Handles for a few of the classes
// As Program is stored in a large array we do not need a handle for that
// one. These are helper classes that are located elsewhere in memory

typedef boost::shared_ptr<LinAlgProgramVariable> LinAlgProgramVariableHandle;
typedef boost::shared_ptr<LinAlgProgram>         LinAlgProgramHandle;

//-----------------------------------------------------------------------------
// Functions for databasing the function calls that make up the program

// This class is used to describe each function in the system
// It describes the name + input arguments, the return type,
// additional flags and a pointer the the actual function that
// needs to be called


class SCISHARE LinAlgFunction : public ParserFunction {
  public:
    // Build a new function
    LinAlgFunction(
      bool (*function)(LinAlgProgramCode& pc, std::string& err),
      std::string function_id,
      std::string function_type,
      int function_flags
    );

    // Virtual destructor so I can do dynamic casts on this class
    virtual ~LinAlgFunction() {}

    // Get the pointer to the function
    bool (*get_function())(LinAlgProgramCode& pc, std::string& err)
      { return (function_); }

  private:
    // The function to call that needs to be called on the data
    bool (*function_)(LinAlgProgramCode& pc, std::string& err);

};


//-----------------------------------------------------------------------------
// Code segment class, all the function calls are based on this class
// providing the program with input and output variables all located in one
// piece of memory. Although this class points to auxilary memory block,
// an effort is made to store them all in the same array, to minimize
// page swapping

class SCISHARE LinAlgProgramCode {
  public:

    // Constructor
    LinAlgProgramCode(bool (*function)(LinAlgProgramCode& pc,std::string& err)) :
      function_(function) {}

    LinAlgProgramCode() :
      function_(0) {}

    // Set the function pointer
    inline void set_function(bool (*function)(LinAlgProgramCode& pc,std::string& err))
    {
      function_ = function;
    }

    // Get the function pointer
    inline bool (*get_function())(LinAlgProgramCode& pc,std::string& err)
    {
      return (function_);
    }

    // Tell the porgam where to temporary space has been allocated
    // for this part of the program
    inline void set_handle(size_t j, Core::Datatypes::MatrixHandle* variable)
    {
      if (j >= variables_.size()) variables_.resize(j+1);
      variables_[j] = variable;
    }

    // These functions are called by the actual code segments
    // For Matrix buffers
    inline Core::Datatypes::MatrixHandle* get_handle(size_t j)
      { return (variables_[j]); }

    inline Core::Datatypes::MatrixHandle& handle(size_t j)
      { return (*(variables_[j])); }

    // Run this code segment
    // Run time errors are reported by returning a false,
    // After which we can look in the parser script to see
    // which function failed
    inline bool run(std::string& err)
      { return (function_(*this, err)); }

    void print();

  private:
    // This is the minimal information needed to run the parsed program
    // In order improve performance, all the buffers and instructions are
    // grouped together so they fit in a few pages of the memory manager

    // Function call to evaluate this piece of the code
    bool (*function_)(LinAlgProgramCode& pc,std::string& err);

    // Location of where the data is stored, these are Matrix handle pointers
    // This way one can allocate new matrices and remove them when the handle
    // is no loner used. The handles are stored in a different array
    std::vector<Core::Datatypes::MatrixHandle*> variables_;
};



class SCISHARE LinAlgProgramVariable {

  public:

    // Constructor of the variable
    explicit LinAlgProgramVariable(const std::string& name) :
      name_(name), handle_(0)
      {
      }


    LinAlgProgramVariable(const std::string& name, Core::Datatypes::MatrixHandle handle) :
      name_(name), handle_(handle) {}

    // Retrieve the data pointer from the central temporal
    // storage
    Core::Datatypes::MatrixHandle* get_handle()  { return (&(handle_)); }
    Core::Datatypes::MatrixHandle handle() { return (handle_); }

  private:
    // Name of variable
    std::string name_;

    // Where the data needs to be store
    Core::Datatypes::MatrixHandle handle_;
};

class SCISHARE LinAlgProgramSource {
  public:
    LinAlgProgramSource() {}

    void          set_matrix(Core::Datatypes::MatrixHandle matrix) { matrix_ = matrix; }
    Core::Datatypes::MatrixHandle  get_matrix()                    { return (matrix_); }
    Core::Datatypes::MatrixHandle* get_handle()                    { return (&matrix_); }
    bool          is_matrix()                     { return (true); }

  private:
    Core::Datatypes::MatrixHandle  matrix_;
};


class SCISHARE LinAlgProgram {

  public:

    // Default constructor
    LinAlgProgram()
    {}

    // Get the optimization parameters, these can only be set when creating the
    // object as it depends on allocated buffer sizes and those are hard to change
    // when allocated

    bool add_source(const std::string& name, Core::Datatypes::MatrixHandle matrix);
    bool add_sink(const std::string& name);

    void resize_const_variables(size_t sz)
      { const_variables_.resize(sz); }
    void resize_single_variables(size_t sz)
      { single_variables_.resize(sz); }

    void resize_const_functions(size_t sz)
      { const_functions_.resize(sz); }
    void resize_single_functions(size_t sz)
      { single_functions_.resize(sz); }

    // Set variables which we use as temporal information structures
    /// @todo: need to remove them at some point
    void set_const_variable(size_t j, LinAlgProgramVariableHandle& handle)
      { const_variables_[j] = handle; }
    void set_single_variable(size_t j, LinAlgProgramVariableHandle& handle)
      { single_variables_[j] = handle; }

    LinAlgProgramVariableHandle get_const_variable(size_t j)
      { return (const_variables_[j]); }
    LinAlgProgramVariableHandle get_single_variable(size_t j)
      { return (single_variables_[j]); }

    // Set program code
    void set_const_program_code(size_t j, LinAlgProgramCode& pc)
      { const_functions_[j] = pc; }
    void set_single_program_code(size_t j, LinAlgProgramCode& pc)
      { single_functions_[j] = pc; }

    // Code to find the pointers that are given for sources and sinks
    bool find_source(const std::string& name,  LinAlgProgramSource& ps) const;
    bool find_sink(const std::string& name,  LinAlgProgramSource& ps) const;

    bool run_const(size_t& error_line, std::string& err);
    bool run_single(size_t& error_line, std::string& err);

    void set_parser_program(ParserProgramHandle handle) { pprogram_ = handle; }
    ParserProgramHandle get_parser_program() const { return (pprogram_); }

  private:

    // Source and Sink information
    std::map<std::string,LinAlgProgramSource> input_sources_;
    std::map<std::string,LinAlgProgramSource> output_sinks_;

    // Variable lists
    std::vector<LinAlgProgramVariableHandle> const_variables_;
    std::vector<LinAlgProgramVariableHandle> single_variables_;

    // Program code
    std::vector<LinAlgProgramCode> const_functions_;
    std::vector<LinAlgProgramCode> single_functions_;

    ParserProgramHandle pprogram_;

};

class SCISHARE LinAlgInterpreter {

  public:
    // The interpreter Creates executable code from the parsed code
    // The first step is setting the data sources and sinks


    //------------------------------------------------------------------------
    // Step 0 : create program variable
    bool create_program(LinAlgProgramHandle& mprogram,std::string& error);


    //------------------------------------------------------------------------
    // Step 1: add sources and sinks

    // Matrix sources
    bool add_matrix_source(LinAlgProgramHandle& pprogram,
                           const std::string& name,
                           Core::Datatypes::MatrixHandle matrix,
                           std::string& error);

    // Matrix sinks
    bool add_matrix_sink(LinAlgProgramHandle& pprogram,
                         const std::string& name,
                         std::string& error);

    //------------------------------------------------------------------------
    // Step 2: translate code and generate executable code

    // Main function for transscribing the parser output into a program that
    // can actually be executed
    bool translate(ParserProgramHandle& pprogram,
                   LinAlgProgramHandle& mprogram,
                   std::string& error);

    //------------------------------------------------------------------------
    // Step 3: Run the code

    bool run(LinAlgProgramHandle& mprogram,std::string& error);
};

}

#endif
