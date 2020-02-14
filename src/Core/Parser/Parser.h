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


#ifndef CORE_PARSER_PARSER_H
#define CORE_PARSER_PARSER_H 1

#include <Core/Utils/StringUtil.h>
#include <Core/Thread/Mutex.h>
#include <map>
#include <list>

// Include files needed for Windows
#include <Core/Parser/share.h>

namespace SCIRun {

// The parser builds a tree based on three different classes
// ParserNode : This class is a piece of an expression. The ParserNode
//              class marks: functions, variables, constants, and string
//              constants. This are currently the pieces that are supported
// ParserTree : This class is the toplevel class of an expression. It binds
//              the output variable with the tree of nodes that describe
//              how a variable is computed.
// ParserProgram: This class simply binds a series of expressions together.
// ParserVariable: A variable for the the parser program. This class contains
//              the names of the variables that are defined at the start of the
//              program and which need to be defined at the end of the program
// ParserFunctionCatalog: This class contains a list of functions that are
//              available, and hence is used for validating the program

class ParserNode;
class ParserTree;
class ParserProgram;
class ParserVariable;
class ParserFunction;
class ParserFunctionCatalog;
class ParserScriptFunction;
class ParserScriptVariable;

// We use handles (Note these are not thread safe hence constrain them
// to one thread) for memory management

typedef boost::shared_ptr<ParserNode>                   ParserNodeHandle;
typedef boost::shared_ptr<ParserTree>                   ParserTreeHandle;
typedef boost::shared_ptr<ParserProgram>                ParserProgramHandle;
typedef boost::shared_ptr<ParserVariable>               ParserVariableHandle;
typedef boost::shared_ptr<ParserFunctionCatalog> ParserFunctionCatalogHandle;
typedef boost::shared_ptr<ParserScriptFunction>         ParserScriptFunctionHandle;
typedef boost::shared_ptr<ParserScriptVariable>         ParserScriptVariableHandle;

// Define the four classes of parser components

enum {
  PARSER_CONSTANT_SCALAR_E = 1,
  PARSER_CONSTANT_STRING_E = 2,
  PARSER_VARIABLE_E = 3,
  PARSER_FUNCTION_E = 4
};

// This piece of the code defines the types the parser recognizes
// Define variable types that are used to validate and to predict
// output types.


// This part defines the attributes of the variables

enum {
  // A sequence denotes a vectorized variable, hence when this flag is set
  // the function will apply the function to an array, instead of a single variable
  SCRIPT_SEQUENTIAL_VAR_E = 1,
  // Simple variable this is the default
  SCRIPT_SINGLE_VAR_E     = 2,
  // Reserved for numerical constants
  SCRIPT_CONST_VAR_E      = 4,
  // Internal marker to mark which variables are actually used to compute the output
  SCRIPT_USED_VAR_E       = 8,
  // Internal marker telling an output is optional
  SCRIPT_OPTIONAL_VAR_E   = 16
};

class SCISHARE ParserVariable {
  public:
    // In order to use handles, we need to add reference counting
    int ref_cnt;

  public:
    ParserVariable(std::string name, std::string type) :
      ref_cnt(0),
      name_(name),
      type_(type),
      flags_(SCRIPT_SINGLE_VAR_E)
      {}

    ParserVariable(std::string name, std::string type, int flags) :
      ref_cnt(0),
      name_(name),
      type_(type),
      flags_(flags)
      {
        // Default is a single variable
        // Currently one can choose between a sequence or a single
        if (flags == 0) flags_ = SCRIPT_SINGLE_VAR_E;
      }


    // Retrieve the type of the variable
    std::string  get_type()  { return (type_); }
    //Set the type of the variable
    void set_type(std::string type) { type_ = type; }

    // Retrieve the flags of the variable
    int get_flags() { return (flags_); }

    // Get the name of the variable
    std::string get_name() { return (name_); }

  private:
    // The name of the variable
    std::string name_;

    // The type of the variable
    std::string type_;

    // Flags that guide the parser
    int flags_;
};

typedef boost::shared_ptr<class ParserFunction> ParserFunctionHandle;

class SCISHARE ParserNode
{
  public:
    ParserNode(int kind, const std::string& value) :
      kind_(kind),
      type_("U"),
      value_(value)
    {}

    ParserNode(int kind, const std::string& value, const std::string& type) :
      kind_(kind),
      type_(type),
      value_(value)
    {}

    // Retrieve the kind of the node (this is function, constant, string, etc)
    int get_kind()    { return (kind_); }

    // Retrieve the type of the node (this is the return type of the variable)
    std::string get_type()    { return (type_); }

    // Retrieve the value of the value
    std::string get_value()   { return (value_); }

    // Set the value of the node
    void set_value(const std::string& value) { value_ = value; }

    // Get a pointer to one of the arguments of a function
    ParserNodeHandle get_arg(size_t j)  { return args_[j]; }

    // Set the argument to a sub tree
    void set_arg(size_t j, ParserNodeHandle handle)
    {
      if (j >= args_.size()) args_.resize(j+1);
      args_[j] = handle;
    }

    // Set the argument to a sub tree (using pointers)
    void set_arg(size_t j, ParserNode* ptr)
    {
      if (j >= args_.size()) args_.resize(j+1);
      args_[j].reset(ptr);
    }

    // Set a copy to the function pointer, this is used later by the interpreter
    void set_function(ParserFunctionHandle func) { function_ = func; }
    // Retrieve the function pointer
    ParserFunctionHandle get_function()          { return (function_); }

    // Set the type of the node (this is used by the validator)
    void set_type(const std::string& type) { type_ = type; }

    // Retrieve the number of arguments a function has
    size_t num_args() { return (args_.size()); }

    // For debugging
    void print(int level) const;

  private:
    // Define the kind of the node, this can be a constant, a variable, or
    // a function.
    int kind_;

    // The return type of this node, this is used in the second pass
    // where expressions are validated
    std::string type_;

    // The name of the variable or the function, the constant scalar value,
    // or the contents of a parsed string
    std::string value_;

    // Pieces for functions
    // Argument trees of the function arguments
    std::vector<ParserNodeHandle> args_;
    // Pointer to where the function is
    ParserFunctionHandle function_;
};


class SCISHARE ParserTree {
  public:
    // In order to use handles, we need to add reference counting
    int ref_cnt;

  public:
    // Constructor
    ParserTree(const std::string& varname, ParserNodeHandle expression) :
      ref_cnt(0),
      varname_(varname),
      expression_(expression),
      type_("U")
    {}

    // Retrieve the name of the variable that needs to be assigned
    std::string get_varname()    { return (varname_); }

    // Retrieve the tree for computing the expression
    ParserNodeHandle get_expression_tree() { return expression_; }
    // Set expression tree
    void set_expression_tree(ParserNodeHandle handle) { expression_ = handle; }

    // Set the final output type of the expression
    void set_type(const std::string& type) { type_ = type; }

    // Retrieve final output type
    const std::string& get_type() { return (type_); }

    // For debugging
    void print() const;

  private:
    // The name of the variable that needs to be assigned
    std::string varname_;

    // The tree of functions that need to be called to compute this variable
    ParserNodeHandle expression_;

    // Return type of the expression
    std::string type_;
};


typedef std::map<std::string,ParserVariableHandle> ParserVariableList;

class SCISHARE ParserProgram {
  public:
    // In order to use handles, we need to add reference counting
    int ref_cnt;

  public:
    ParserProgram() : ref_cnt(0) {}

    // Add an expression to a program: this is a combination of the raw
    // unparsed program code and the expression tree
    void add_expression(const std::string& expression_string,
                        ParserTreeHandle expression_tree)
    {
      std::pair<std::string,ParserTreeHandle> expression(expression_string,expression_tree);
      expressions_.push_back(expression);
    }

    // Retrieve an expression from a program
    // This gets one expression in unparsed and parsed form from the program
    void get_expression(int expression_num,
                        std::string& expression_string,
                        ParserTreeHandle& expression_handle)
    {
      expression_string = expressions_[expression_num].first;
      expression_handle = expressions_[expression_num].second;
    }

    void get_expression(int expression_num,
                        ParserTreeHandle& expression_handle)
    {
      expression_handle = expressions_[expression_num].second;
    }

    // Retrieve the number of expressions in the program
    size_t num_expressions()  { return (expressions_.size()); }


    // Add an input variable to the program
    void add_input_variable(const std::string& name, const std::string& type = "U", int flags = 0)
    {
      input_variables_[name].reset(new ParserVariable(name,type,flags));
    }

    // Add an output variable to the program
    void add_output_variable(const std::string& name, const std::string& type = "U", int flags = 0)
    {
      output_variables_[name].reset(new ParserVariable(name,type,flags));
    }

    void get_input_variables(ParserVariableList& var_list)
    {
      var_list = input_variables_;
    }

    void get_output_variables(ParserVariableList& var_list)
    {
      var_list = output_variables_;
    }

    ParserFunctionCatalogHandle get_catalog() { return catalog_; }
    void set_catalog(ParserFunctionCatalogHandle catalog) { catalog_ = catalog;}


    // Insert the variables
    void add_const_var(ParserScriptVariableHandle& handle);
    void add_single_var(ParserScriptVariableHandle& handle);
    void add_sequential_var(ParserScriptVariableHandle& handle);

    void add_const_function(ParserScriptFunctionHandle& handle);
    void add_single_function(ParserScriptFunctionHandle& handle);
    void add_sequential_function(ParserScriptFunctionHandle& handle);


    // Get the variables that need to be assigned
    size_t num_const_variables();
    size_t num_single_variables();
    size_t num_sequential_variables();

    bool get_const_variable(size_t j,ParserScriptVariableHandle& handle);
    bool get_single_variable(size_t j,ParserScriptVariableHandle& handle);
    bool get_sequential_variable(size_t j,ParserScriptVariableHandle& handle);

    // Get the functions that need to be assigned
    size_t num_const_functions();
    size_t num_single_functions();
    size_t num_sequential_functions();

    bool get_const_function(size_t j,ParserScriptFunctionHandle& handle);
    bool get_single_function(size_t j,ParserScriptFunctionHandle& handle);
    bool get_sequential_function(size_t j,ParserScriptFunctionHandle& handle);


    // For debugging
    void print() const;

  private:
    // Short cut to the parser function catalog
    ParserFunctionCatalogHandle catalog_;

    // The list of expressions, we store both the raw expression as well as the
    // parsed version in order to effectively report back to user which expression
    // is faulty. As the parsed version is harder to read, the raw version is kept.
    std::vector<std::pair<std::string,ParserTreeHandle> > expressions_;

    // List of variables that exist when the program starts. This contains the
    // list of variables the program can use, without them being defined in one
    // of the expressions
    ParserVariableList input_variables_;

    // List of output variables that need to be generated as they are required
    // by the rest of the program. These variables need to be defined at the
    // end of the program
    ParserVariableList output_variables_;

    // The next series of variables represent the next stage of the parser
    // In this stage everything is a variable or a function, and we have two
    // lists of constants one for double constants and one for string constants

    std::vector<ParserScriptVariableHandle> const_variables_;
    std::vector<ParserScriptVariableHandle> single_variables_;
    std::vector<ParserScriptVariableHandle> sequential_variables_;

    std::vector<ParserScriptFunctionHandle> const_functions_;
    std::vector<ParserScriptFunctionHandle> single_functions_;
    std::vector<ParserScriptFunctionHandle> sequential_functions_;
};

enum {
  // Define a function as sequential it will be evaluated for each instance in
  // the sequence. This one is used for functions without parameters like rand
  // so they trigger for each case
  PARSER_SEQUENTIAL_FUNCTION_E = 1,
  PARSER_SINGLE_FUNCTION_E = 2,
  PARSER_CONST_FUNCTION_E = 4,
  // Variables are symmetric, this will tell the optimizer that the order of
  // variables is not of interest
  PARSER_SYMMETRIC_FUNCTION_E = 16
};


class SCISHARE ParserFunction
{
  public:
    ParserFunction(const std::string& function_id, const std::string& type) :
      function_id_(function_id),
      function_type_(type) {}

    ParserFunction(const std::string& function_id,const std::string& type, int fflags) :
      function_id_(function_id),
      function_type_(type),
      function_flags_(fflags) {}

    // Virtual destructor so we can do dynamic casts on this class
    virtual ~ParserFunction() {}

    // Retrieve the function ID string
    const std::string& get_function_id() const { return (function_id_); }
    // Retrieve the function return type

    const std::string& get_return_type() const { return (function_type_); }
    // Retrieve the function flags
    int get_flags() const { return (function_flags_); }

  private:
    // The ID of the function, which is the name plus arguments in one string
    std::string function_id_;

    // The return type of the function
    std::string function_type_;

    // Flags define properties of this function
    int function_flags_;
};

// The list of functions
typedef std::map<std::string,ParserFunctionHandle> ParserFunctionList;

// The ParserFunctionCatalog servers two purposes:
//  - Tell the parser which functions are valid
//  - Tell the parser what the return type is of each function call

class SCISHARE ParserFunctionCatalog
{
  public:
    ParserFunctionCatalog();

    // Add definitions of functions to the list
    // Various calls with different amounts of arguments
    void add_function(ParserFunctionHandle function);

    // Retrieve a function from the data base
    bool find_function(const std::string& function_id,
                       ParserFunctionHandle& function);

    // For debugging
    void print() const;
  private:

    // List of functions and their return type
    ParserFunctionList functions_;
    Core::Thread::Mutex lock_;
};


// In the end we build script of how to compute the
// results

enum {
  SCRIPT_INPUT_E = 1,
  SCRIPT_OUTPUT_E = 2,
  SCRIPT_CONSTANT_SCALAR_E = 4,
  SCRIPT_CONSTANT_STRING_E = 8,
  SCRIPT_VARIABLE_E = 16
};

class ParserScriptFunction
{
  public:
    ParserScriptFunction(const std::string& name, ParserFunctionHandle function) :
      name_(name), flags_(0), function_(function) {}

    // Get the name of the function
    const std::string& get_name() const { return (name_); }

    // Get the number of input variables this function depends on
    size_t num_input_vars() const { return (input_variables_.size()); }

    ParserScriptVariableHandle get_input_var(size_t j) const
    {
      if (j < input_variables_.size()) return (input_variables_[j]);
      return (ParserScriptVariableHandle(0));
    }

    void set_input_var(size_t j, ParserScriptVariableHandle& handle)
      {
        if (j >= input_variables_.size()) input_variables_.resize(j+1);
        input_variables_[j] = handle;
      }

    ParserScriptVariableHandle get_output_var() const { return (output_variable_); }
    void set_output_var(ParserScriptVariableHandle& handle) { output_variable_ = handle; }

    ParserFunctionHandle get_function() const { return (function_); }

    int get_flags() const             { return (flags_); }
    void set_flags(int flags)   { flags_ |= flags; }
    void clear_flags()          { flags_ = 0; }

    // For debugging
    void print() const;

  private:
    // The name of the function
    std::string name_;

    // Flags that describe whether this is a single or sequential call
    int flags_;

    // Pointer to the function information block
    ParserFunctionHandle function_;

    // Input variables in the function depends on
    std::vector<ParserScriptVariableHandle> input_variables_;

    // Output variable
    ParserScriptVariableHandle output_variable_;
};


class ParserScriptVariable {

  public:
    // Create input variable
    ParserScriptVariable(const std::string& name, const std::string& uname, const std::string& type, int flags) :
      kind_(SCRIPT_INPUT_E),
      type_(type),
      flags_(flags),
      name_(name),
      uname_(uname),
      scalar_value_(0.0),
      var_number_(0)
    {}

    // Create output variable
    ParserScriptVariable(const std::string& uname, const std::string& type, int flags) :
      kind_(SCRIPT_VARIABLE_E),
      type_(type),
      flags_(flags),
      uname_(uname),
      scalar_value_(0.0),
      var_number_(0)
    {}

    // Create scalar const variable
    ParserScriptVariable(const std::string& uname,
                         double value) :
      kind_(SCRIPT_CONSTANT_SCALAR_E),
      type_("S"),
      flags_(SCRIPT_CONST_VAR_E),
      uname_(uname),
      scalar_value_(value),
      var_number_(0)
    {}

    // Create string const variable
    ParserScriptVariable(const std::string& uname,
                         const std::string& value) :
      kind_(SCRIPT_CONSTANT_STRING_E),
      type_("A"),
      flags_(SCRIPT_CONST_VAR_E),
      uname_(uname),
      scalar_value_(0.0),
      string_value_(value),
      var_number_(0)
    {}


    // Set/get parent
    void set_parent(ParserScriptFunctionHandle& handle) { parent_ = handle; }
    ParserScriptFunctionHandle get_parent() const { return (parent_); }

    // Get kind of the variable
    int get_kind() const { return (kind_); }
    void set_kind(int kind) { kind_ = kind; }

    const std::string& get_type() const { return (type_); }
    void set_type(const std::string& type) { type_ = type; }

    // Get/set flags of the variable
    int get_flags() const { return (flags_); }
    void set_flags(int flags) { flags_ |= flags; }
    void clear_flags() { flags_ = 0; }

    void set_const_var() { flags_ |= SCRIPT_CONST_VAR_E; }
    void set_single_var() { flags_ |= SCRIPT_SINGLE_VAR_E; }
    void set_sequential_var() { flags_ |= SCRIPT_SEQUENTIAL_VAR_E; }

    bool is_const_var() const { return (flags_ & SCRIPT_CONST_VAR_E) != 0; }
    bool is_single_var() const { return (flags_ & SCRIPT_SINGLE_VAR_E) != 0; }
    bool is_sequential_var() const { return (flags_ & SCRIPT_SEQUENTIAL_VAR_E) != 0; }

    // Get the name and the unique name
    const std::string& get_name() const  { return (name_); }
    void set_name(const std::string& name) { name_ = name; }

    const std::string& get_uname() const { return (uname_); }
    void set_uname(const std::string& uname) { uname_ = uname; }


    // Get the constant values
    double      get_scalar_value() const { return (scalar_value_); }
    const std::string& get_string_value() const { return (string_value_); }

    // Dependence of this variable
    void compute_dependence();

    const std::string& get_dependence() const { return (dependence_); }
    void clear_dependence() { dependence_.clear(); }

    int get_var_number() const { return (var_number_); }
    void set_var_number(int var_number) { var_number_ = var_number; }

    // For debugging
    void print() const;

  private:
    // The function that created this variable
    ParserScriptFunctionHandle parent_;

    // The kind of variable
    int kind_;

    // The type of the variable
    std::string type_;

    // The flags for sequential/single/constant
    int flags_;

    // Name of the variable for input and output variables
    std::string name_;

    // Unique name of the variable
    std::string uname_;

    std::string dependence_;

    // For const scalar/string variables
    double        scalar_value_;
    std::string   string_value_;

    int var_number_;
};


//-----------------------------------------------------------------------------
// Below the code for the parser starts
// The parser takes a piece of raw code and analyzes its contents and creates
// a program tree that splits the pieces of the expression in the right
// hierarchical order.


class SCISHARE Parser {

  // Classes for storing the operator information
  class BinaryOperator {
    public:
      std::string operator_;
      std::string funname_;
      int         priority_;
  };

  class UnaryOperator {
    public:
      std::string operator_;
      std::string funname_;
  };

  public:
    Parser();

    bool parse(ParserProgramHandle& program,
               std::string& expressions,
               std::string& error);

    bool add_input_variable(ParserProgramHandle& program,
                            const std::string& name,
                            const std::string& type,
                            int flags = 0);

    bool add_output_variable(ParserProgramHandle& program,
                             const std::string& name,
                             const std::string& type = "U",
                             int flags = 0);

    bool add_output_variable(ParserProgramHandle program,
                             const std::string& name);

    bool get_input_variable_type(ParserProgramHandle program,
                                 const std::string& name, std::string& type);

    bool get_output_variable_type(ParserProgramHandle program,
                                 const std::string& name, std::string& type);


    bool get_input_variable_type(ParserProgramHandle program,
                                 const std::string& name,
                                 std::string& type,
                                 int& flags);

    bool get_output_variable_type(ParserProgramHandle program,
                                 const std::string& name,
                                 std::string& type,
                                 int& flags);


    bool validate(ParserProgramHandle program,
                  ParserFunctionCatalogHandle catalog,
                  std::string& error);

    bool optimize(ParserProgramHandle program,
                  std::string& error);

    //--------------------------------------------------------------------------
    // Setup of parser

    // These functions add or overload the function names of default operators
    // and even add additional operators if a new one is specified.

    // These functions are intended for customizing the parser
    // NOTE OPERATOR NAMES CURRENTLY CANNOT BE LONGER THAN TWO CHARACTERS
    // IF LONGER OPERATOR NAMES ARE NEEDED THE CODE SHOULD BE UPGRADED
    void add_binary_operator(const std::string& op, const std::string& funname, int priority);
    void add_unary_pre_operator(const std::string& op, const std::string& funname);
    void add_unary_post_operator(const std::string& op, const std::string& funname);

    // Mark special variable names as constants
    void add_numerical_constant(const std::string& name,double val);

  private:
    // Get an expression from the program code. Expressions are read sequentially
    // and this function will modify the input and remove the first expression.
    bool scan_expression(std::string& expressions,
                         std::string& expression);


    // This function breaks down an expression into two components
    // The part to the left of the equal sign
    // The part to the right of the equal sign
    // The variable name is checked to see whether it is valid
    // The expression part is not parsed by this function that is done separately
    bool split_expression(std::string& expression,
                          std::string& varname,
                          std::string& vartree);

    // Parse the parts on the right hand side of the equal sign of the expression
    bool parse_expression_tree(std::string& expression,
                               ParserNodeHandle& ehandle,
                               std::string& error);


    // Scan whether the string starts with a sub expression, i.e. '( ..... )'
    bool scan_sub_expression(std::string& expression, std::string& subexpression);

    // Scan whether the string starts with a subs assignment expression, i.e. '[ ..... ]'
    bool scan_subs_expression(std::string& expression, std::string& subexpression);


    // Scan for a numeric constant at the start of the string
    // A numeric constant must start with a number
    // Numbers like 0x12 077 1.23E-8 etc should all be detected by this
    // function. It strips out the number and returns that piece of the string
    bool scan_constant_value(std::string& expression, std::string& value);

    // Scan for a variable name at the start of the string
    // A variable name should start with a-zA-Z or _ and can have
    // numbers for the second character and further.
    // It should not contain any spaces.
    // This function checks as well if the name is followed by spaces and '('
    // If so it is not a variable name, but a function name
    // This function strips out the variable name out of expression and
    // returns in var_name
    bool scan_variable_name(std::string& expression, std::string& var_name);

    // Scan for a string. A srting is a piece of code between quotes e.g. "foo"
    // This function should recognize most escape characters in the string and
    // will translate them as well
    bool scan_constant_string(std::string& expression, std::string& str);

    // Scan for a function. A function starts with a variable name, but is
    // followed by optional spaces and then (...,....,...), which is the
    // argument list. This function should scan for those argument lists
    bool scan_function(std::string& expression, std::string& function);

    // Scan for the equal sign, this is used to parse expressions
    // Each expression should contain one
    bool scan_equal_sign(std::string& expression);

    // Split a string with code for a function into the function name and into
    // its arguments
    void split_function(std::string& expression,
                        std::string& fun_name,
                        std::vector<std::string>& fun_args);

    // Split a string with code for a subs function into its arguments
    void split_subs(std::string& expression,
                        std::vector<std::string>& start_args,
                        std::vector<std::string>& step_args,
                        std::vector<std::string>& end_args,
                        std::string& varname);


    // Check for syntax errors in the code
    bool check_syntax(std::string& expression, std::string& error);


    // Scan whether the expression starts with a binary operator
    bool scan_binary_operator(std::string& expression,
                              std::string& binary_operator);

    // Scan whether the expression starts with an unary operator
    bool scan_pre_unary_operator(std::string& expression,
                                 std::string& unary_operator);

    // Scan whether the expression starts with an unary operator
    bool scan_post_unary_operator(std::string& expression,
                                  std::string& unary_operator);

    // Get the functionname of the unary operator
    // The name that should be in the FunctionCatalog
    bool get_unary_function_name(std::string& unary_operator,
                                 std::string& fun_name);

    // Get the functionname of the binary operator
    // The name that should be in the FunctionCatalog
    bool get_binary_function_name(std::string& binary_operator,
                                  std::string& fun_name);

    // Binary operators need a priority in which they are evaluated
    // This one returns the priority of each operator
    bool get_binary_priority(std::string& binary_operator, int& priority);


    //------------------------------------------------------------
    // General functions for parsing text
    // Remove a parentices pair at the start and end of the string
    bool remove_global_parentices(std::string& expression);

    // Remove // and /* comments from the expression
    void remove_comments(std::string& expression);


    //------------------------------------------------------------
    // Sub functions for validation
    bool recursive_validate(ParserNodeHandle handle,
                            ParserFunctionCatalogHandle fhandle,
                            ParserVariableList& var_list,
                            std::string& error,
                            std::string& expression);


    //------------------------------------------------------------
    // Sub functions for optimization
    void optimize_mark_used(ParserScriptFunctionHandle& fhandle);

    bool optimize_process_node(ParserNodeHandle& nhandle,
          std::list<ParserScriptVariableHandle>& variables,
          std::map<std::string,ParserScriptVariableHandle>& named_variables,
          std::list<ParserScriptFunctionHandle>& functions,
          ParserScriptVariableHandle& ohandle,
          int& cnt,
          std::string& error);

  private:
    // List of supported binary operators e.g. + - * /
    std::map<std::string,BinaryOperator> binary_operators_;
    // List of pre unary opertors e.g. + - ! ~
    std::map<std::string,UnaryOperator> unary_pre_operators_;
    // List of post unary opertors e.g. '
    std::map<std::string,UnaryOperator> unary_post_operators_;

    // List of numerical constants, e.g. nan, inf, false, true etc.
    std::map<std::string,double> numerical_constants_;

};

}

#endif
