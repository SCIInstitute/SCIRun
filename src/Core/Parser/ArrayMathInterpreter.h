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


#ifndef CORE_PARSER_ARRAYMATHPROGRAM_H
#define CORE_PARSER_ARRAYMATHPROGRAM_H 1

#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Datatypes/Legacy/Base/Types.h>
#include <Core/Datatypes/Legacy/Field/FieldFwd.h>

#include <Core/Thread/Parallel.h>
#include <Core/Thread/Barrier.h>

#include <Core/Containers/StackBasedVector.h>

#include <Core/Parser/Parser.h>

#include <boost/function.hpp>
#include <boost/variant.hpp>
#include <boost/noncopyable.hpp>
// Include files needed for Windows
#include <Core/Parser/share.h>

namespace SCIRun {

// Define class as they are not used in order

class ArrayMathIntepreter;
class ArrayMathFunction;

class ArrayMathProgram;
class ArrayMathProgramCode;
class ArrayMathProgramVariable;

// Handles for a few of the classes
// As Program is stored in a large array we do not need a handle for that
// one. These are helper classes that are located elsewhere in memory

typedef boost::shared_ptr<ArrayMathProgramVariable> ArrayMathProgramVariableHandle;
typedef boost::shared_ptr<ArrayMathProgram>         ArrayMathProgramHandle;

//-----------------------------------------------------------------------------
// Functions for databasing the function calls that make up the program

// This class is used to describe each function in the system
// It describes the name + input arguments, the return type,
// additional flags and a pointer the the actual function that
// needs to be called

typedef boost::function<bool(ArrayMathProgramCode&)> ArrayMathFunctionPtr;

class SCISHARE ArrayMathFunction : public ParserFunction {
  public:
    // Build a new function
    ArrayMathFunction(
      ArrayMathFunctionPtr function,
      const std::string& function_id,
      const std::string& function_type,
      int function_flags
    );

    virtual ~ArrayMathFunction() {}

    ArrayMathFunctionPtr get_function() const
      { return (function_); }

  private:
    // The function to call that needs to be called on the data
    ArrayMathFunctionPtr function_;

};


//-----------------------------------------------------------------------------
// Code segment class, all the function calls are based on this class
// providing the program with input and output variables all located in one
// piece of memory. Although this class points to auxilary memory block,
// an effort is made to store them all in the same array, to minimize
// page swapping

  class SCISHARE ArrayMathProgramCode : boost::noncopyable {
  public:

    // Constructor
    ArrayMathProgramCode(ArrayMathFunctionPtr function) :
      function_(function), index_(0), size_(1) {}

    ArrayMathProgramCode() :
      function_(0), index_(0), size_(1) {}

    inline void set_function(ArrayMathFunctionPtr function)
    {
      function_ = function;
    }

    inline ArrayMathFunctionPtr get_function() const
    {
      return (function_);
    }

    // Tell the program where the temporary space has been allocated
    // for this part of the program
    inline void set_variable(size_t j, double* variable)
    {
      if (j >= variables_.size()) variables_.resize(j+1);
      variables_[j] = variable;
    }

    // In case of a mesh, a pointer to the mesh can be stored in the
    // program code as well
    inline void set_vmesh(size_t j, VMesh* vmesh)
    {
      if (j >= variables_.size()) variables_.resize(j+1);
      variables_[j] = vmesh;
    }

    // In case of a field, a pointer to the field can be stored in the
    // program code as well
    inline void set_vfield(size_t j, VField* vfield)
    {
      if (j >= variables_.size()) variables_.resize(j+1);
      variables_[j] = vfield;
    }

    // In case of a field, a pointer to a dense/column matrix can be stored in the
    // program code as well
    inline void set_matrix(size_t j, Core::Datatypes::MatrixHandle matrix)
    {
      if (j >= variables_.size()) variables_.resize(j+1);
      variables_[j] = matrix;
    }

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    // In general case, set a pointer
    inline void set_pointer(size_t j, void* pointer)
    {
      if (j >= variables_.size()) variables_.resize(j+1);
      variables_[j] = pointer;
    }
#endif

    inline void set_bool_array(size_t j, std::vector<bool>* array)
    {
      if (j >= variables_.size()) variables_.resize(j+1);
      variables_[j] = array;
    }

   inline void set_int_array(size_t j, std::vector<int>* array)
    {
      if (j >= variables_.size()) variables_.resize(j+1);
      variables_[j] = array;
    }

   inline void set_double_array(size_t j, std::vector<double>* array)
    {
      if (j >= variables_.size()) variables_.resize(j+1);
      variables_[j] = array;
    }

    // Set the index, we keep this in the list so the program knows which
    // element we need to process.
    inline void set_index(index_type index) { index_ = index; }

    // Set the size of the array that needs to be processed
    inline void set_size(size_type size) { size_ = size; }
    inline size_type get_size() const { return(size_); }

    // These functions are called by the actual code segments
    // For Scalar, Vector and Tensor buffers
    inline double* get_variable(size_t j)
      { return boost::get<double*>(variables_[j]); }
    // For Mesh buffers
    inline VMesh*  get_vmesh(size_t j)
      { return boost::get<VMesh*>(variables_[j]); }
    // For Field buffers
    inline VField* get_vfield(size_t j)
      { return boost::get<VField*>(variables_[j]); }
    // For Matrix buffers
    inline Core::Datatypes::MatrixHandle get_matrix(size_t j)
      { return boost::get<Core::Datatypes::MatrixHandle>(variables_[j]); }

    inline std::vector<bool>* get_bool_array(size_t j)
      { return boost::get<std::vector<bool>*>(variables_[j]); }
    inline std::vector<int>* get_int_array(size_t j)
      { return boost::get<std::vector<int>*>(variables_[j]); }
    inline std::vector<double>* get_double_array(size_t j)
      { return boost::get<std::vector<double>*>(variables_[j]); }

    // Get the current index
    inline index_type get_index() const
      { return (index_); }

    // Run this code segment
    // Run time errors are reported by returning a false,
    // After which we can look in the parser script to see
    // which function failed
    inline bool run()
      { return (function_(*this)); }

    void print() const;

  private:
    // This is the minimal information needed to run the parsed program
    // In order improve performance, all the buffers and instructions are
    // grouped together so they fit in a few pages of the memory manager

    // Function call to evaluate this piece of the code
    ArrayMathFunctionPtr function_;

    // Location of where the data is stored
    typedef boost::variant<
      double*,
      Core::Datatypes::MatrixHandle,
      VField*,
      VMesh*,
      std::vector<bool>*,
      std::vector<int>*,
      std::vector<double>*
    > variable_type;
    StackBasedVector<variable_type,3> variables_;

    // Index in where we are in the selection
    index_type    index_;

    // Sequence size
    size_type     size_;
};

  typedef boost::shared_ptr<ArrayMathProgramCode> ArrayMathProgramCodePtr;



class SCISHARE ArrayMathProgramVariable
{
  public:
    ArrayMathProgramVariable(const std::string& name, double* data) :
      name_(name), data_(data) {}
    double* get_data() const  { return (data_); }

  private:
    std::string name_;
    double*     data_;
};

 /// @todo replace with boost::variant
class SCISHARE ArrayMathProgramSource {

  public:
    ArrayMathProgramSource() :
      vmesh_(0), vfield_(0), matrix_(0), bool_array_(0), int_array_(0), double_array_(0) {}

      void    set_vmesh(VMesh* vmesh)    { vmesh_ = vmesh; }
      VMesh*  get_vmesh()   const             { return (vmesh_); }
      bool    is_vmesh() const                { return (vmesh_ != 0); }

      void    set_vfield(VField* vfield) { vfield_ = vfield; }
      VField* get_vfield() const               { return (vfield_); }
      bool    is_vfield() const                { return (vfield_ != 0); }

      void    set_matrix(Core::Datatypes::MatrixHandle matrix) { matrix_ = matrix; }
      Core::Datatypes::MatrixHandle get_matrix()  const             { return (matrix_); }
      bool    is_matrix()  const              { return (matrix_ != 0); }

      void    set_bool_array(std::vector<bool>* array) { bool_array_ = array; }
      std::vector<bool>* get_bool_array() const   { return (bool_array_); }
      bool    is_bool_array()  const             { return (bool_array_ != 0); }

      void    set_int_array(std::vector<int>* array)  { int_array_ = array; }
      std::vector<int>* get_int_array()  const   { return (int_array_); }
      bool    is_int_array()   const             { return (int_array_ != 0); }

      void    set_double_array(std::vector<double>* array)  { double_array_ = array; }
      std::vector<double>* get_double_array() const { return (double_array_); }
      bool    is_double_array()   const            { return (double_array_ != 0); }

  private:
    VMesh*    vmesh_;
    VField*   vfield_;
    Core::Datatypes::MatrixHandle   matrix_;

    std::vector<bool>*    bool_array_;
    std::vector<int>*     int_array_;
    std::vector<double>*  double_array_;
};


  class SCISHARE ArrayMathProgram : boost::noncopyable {

  public:
    ArrayMathProgram() : num_proc_(Core::Thread::Parallel::NumCores()), barrier_("ArrayMathProgram", num_proc_)
    {
      // Buffer size describes how many values of a sequential variable are
      // grouped together for vectorized execution
      buffer_size_ = 128;
      array_size_ = 1;
    }

    // Constructor that allows overloading the default optimization parameters
    ArrayMathProgram(size_type array_size,
      size_type buffer_size,int num_proc = -1) :
      num_proc_(num_proc < 1 ? Core::Thread::Parallel::NumCores() : num_proc),
      barrier_("ArrayMathProgram", num_proc_)
    {
      // Buffer size describes how many values of a sequential variable are
      // grouped together for vectorized execution
      buffer_size_ = buffer_size;
      array_size_ = array_size;
    }

    // Get the optimization parameters, these can only be set when creating the
    // object as it depends on allocated buffer sizes and those are hard to change
    // when allocated
    // Get the number of entries that are processed at once
    size_type get_buffer_size() const { return (buffer_size_); }
    // Get the number of processors
    int get_num_proc() const { return (num_proc_); }

    // Set the size of the array to process
    size_type get_array_size() const { return (array_size_); }
    void set_array_size(size_type array_size) { array_size_ = array_size; }

    bool add_source(const std::string& name, VField* vfield);
    bool add_source(const std::string& name, VMesh*  vmesh);
    bool add_source(const std::string& name, Core::Datatypes::MatrixHandle matrix);
    bool add_source(const std::string& name, std::vector<bool>* array);
    bool add_source(const std::string& name, std::vector<int>* array);
    bool add_source(const std::string& name, std::vector<double>* array);

    bool add_sink(const std::string& name, VField* vfield);
    bool add_sink(const std::string& name, VMesh*  vmesh);
    bool add_sink(const std::string& name, Core::Datatypes::MatrixHandle matrix);
    bool add_sink(const std::string& name, std::vector<bool>* array);
    bool add_sink(const std::string& name, std::vector<int>* array);
    bool add_sink(const std::string& name, std::vector<double>* array);

    void resize_const_variables(size_t sz)
      { const_variables_.resize(sz); }
    void resize_single_variables(size_t sz)
      { single_variables_.resize(sz); }
    void resize_sequential_variables(size_t sz)
      {
        sequential_variables_.resize(num_proc_);
        for (int np=0; np < num_proc_; np++)
          sequential_variables_[np].resize(sz);
      }

    void resize_const_functions(size_t sz)
      { const_functions_.resize(sz); }
    void resize_single_functions(size_t sz)
      { single_functions_.resize(sz); }
    void resize_sequential_functions(size_t sz)
      {
        sequential_functions_.resize(num_proc_);
        for (int np=0; np < num_proc_; np++)
          sequential_functions_[np].resize(sz);
      }

    // Central buffer for all parameters
    double* create_buffer(size_t size)
    { buffer_.resize(size); return buffer_.empty() ? 0 : (&(buffer_[0])); }

    // Set variables which we use as temporal information structures
    /// @todo: need to remove them at some point
    void set_const_variable(size_t j, ArrayMathProgramVariableHandle handle)
      { const_variables_[j] = handle; }
    void set_single_variable(size_t j, ArrayMathProgramVariableHandle handle)
      { single_variables_[j] = handle; }
    void set_sequential_variable(size_t j, size_t np, ArrayMathProgramVariableHandle handle)
      { sequential_variables_[np][j] = handle; }

    ArrayMathProgramVariableHandle get_const_variable(size_t j) const
      { return (const_variables_[j]); }
    ArrayMathProgramVariableHandle get_single_variable(size_t j) const
      { return (single_variables_[j]); }
    ArrayMathProgramVariableHandle get_sequential_variable(size_t j, size_t np) const
      { return (sequential_variables_[np][j]); }

    // Set program code
    void set_const_program_code(size_t j, ArrayMathProgramCodePtr pc)
      { const_functions_[j] = pc; }
    void set_single_program_code(size_t j, ArrayMathProgramCodePtr pc)
      { single_functions_[j] = pc; }
    void set_sequential_program_code(size_t j, size_t np, ArrayMathProgramCodePtr pc)
      { sequential_functions_[np][j] = pc; }

    // Code to find the pointers that are given for sources and sinks
    bool find_source(const std::string& name,  ArrayMathProgramSource& ps);
    bool find_sink(const std::string& name,  ArrayMathProgramSource& ps);

    bool run_const(size_t& error_line);
    bool run_single(size_t& error_line);
    bool run_sequential(size_t& error_line);

    void set_parser_program(ParserProgramHandle handle) { pprogram_ = handle; }
    ParserProgramHandle get_parser_program() { return (pprogram_); }

  private:

    // General parameters that determine how many values are computed at
    // the same time and how many processors to use
    size_type buffer_size_;
    int num_proc_;

    // The size of the array we are using
    size_type array_size_;

    // Memory buffer
    std::vector<double> buffer_;

    // Source and Sink information
    std::map<std::string,ArrayMathProgramSource> input_sources_;
    std::map<std::string,ArrayMathProgramSource> output_sinks_;

    // Variable lists
    std::vector<ArrayMathProgramVariableHandle> const_variables_;
    std::vector<ArrayMathProgramVariableHandle> single_variables_;
    std::vector<std::vector<ArrayMathProgramVariableHandle> > sequential_variables_;

    // Program code
    std::vector<ArrayMathProgramCodePtr> const_functions_;
    std::vector<ArrayMathProgramCodePtr> single_functions_;
    std::vector<std::vector<ArrayMathProgramCodePtr> > sequential_functions_;

    ParserProgramHandle pprogram_;

    // For parallel code
  private:
    void run_parallel(int proc);

    // Error reporting parallel code
    std::vector<size_type>  error_line_;
    std::vector<bool>       success_;

    Core::Thread::Barrier barrier_;
};

class SCISHARE ArrayMathInterpreter {

  public:
    // The interpreter Creates executable code from the parsed code
    // The first step is setting the data sources and sinks


    //------------------------------------------------------------------------
    // Step 0 : create program variable
    bool create_program(ArrayMathProgramHandle& mprogram,std::string& error);


    //------------------------------------------------------------------------
    // Step 1: add sources and sinks

    // Field data/node/element sources
    bool add_fielddata_source(ArrayMathProgramHandle& pprogram,
                              const std::string& name,
                              FieldHandle field,
                              std::string& error);
    bool add_fieldmesh_source(ArrayMathProgramHandle& pprogram,
                              const std::string& name,
                              FieldHandle field,
                              std::string& error);

    // Matrix sources
    bool add_matrix_source(ArrayMathProgramHandle& pprogram,
                           const std::string& name,
                           Core::Datatypes::MatrixHandle matrix,
                           std::string& error);

    bool add_bool_array_source(ArrayMathProgramHandle& pprogram,
                           const std::string& name,
                           std::vector<bool>* array,
                           std::string& error);

    bool add_int_array_source(ArrayMathProgramHandle& pprogram,
                           const std::string& name,
                           std::vector<int>* array,
                           std::string& error);

    bool add_double_array_source(ArrayMathProgramHandle& pprogram,
                           const std::string& name,
                           std::vector<double>* array,
                           std::string& error);


    // Field data/node/element sinks
    bool add_fielddata_sink(ArrayMathProgramHandle& pprogram,
                            const std::string& name,
                            FieldHandle field,
                            std::string& error);
    bool add_fieldmesh_sink(ArrayMathProgramHandle& pprogram,
                            const std::string& name,
                            FieldHandle field,
                            std::string& error);

    // Matrix sinks
    bool add_matrix_sink(ArrayMathProgramHandle& pprogram,
                         const std::string& name,
                         Core::Datatypes::MatrixHandle matrix,
                         std::string& error);

    bool add_bool_array_sink(ArrayMathProgramHandle& pprogram,
                         const std::string& name,
                         std::vector<bool>* array,
                         std::string& error);

    bool add_int_array_sink(ArrayMathProgramHandle& pprogram,
                         const std::string& name,
                         std::vector<int>* array,
                         std::string& error);

    bool add_double_array_sink(ArrayMathProgramHandle& pprogram,
                         const std::string& name,
                         std::vector<double>* array,
                         std::string& error);


    //------------------------------------------------------------------------
    // Step 2: translate code and generate executable code

    // Main function for transcribing the parser output into a program that
    // can actually be executed
    bool translate(ParserProgramHandle& pprogram,
                   ArrayMathProgramHandle& mprogram,
                   std::string& error);


    //------------------------------------------------------------------------
    // Step 3: Set the array size

    bool set_array_size(ArrayMathProgramHandle& mprogram,size_type array_size);

    //------------------------------------------------------------------------
    // Step 4: Run the code

    bool run(ArrayMathProgramHandle& mprogram,std::string& error);

};

}

#endif
