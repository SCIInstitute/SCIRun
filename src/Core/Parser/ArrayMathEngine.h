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


#ifndef CORE_PARSER_ARRAYMATHENGINE_H
#define CORE_PARSER_ARRAYMATHENGINE_H 1

#include <Core/Logging/ConsoleLogger.h>
#include <Core/Parser/ArrayMathInterpreter.h>
#include <Core/Parser/Parser.h>

// Include files needed for Windows
#include <Core/Parser/share.h>

namespace SCIRun {

class SCISHARE NewArrayMathEngine : public Parser, public ArrayMathInterpreter
{
  public:
    // Classes for storing data on output variables
    class OutputFieldData {
      public:
        std::string array_name_;
        std::string field_name_;
        int         output_basis_order_;
        std::string output_datatype_;
        FieldHandle field_;
    };

    class OutputFieldMesh {
      public:
        std::string array_name_;
        std::string field_name_;
        FieldHandle field_;
    };

    class OutputMatrix {
      public:
        std::string  array_name_;
        std::string  matrix_name_;
        Core::Datatypes::MatrixHandle matrix_;
        size_type    size_;
    };

    class OutputBoolArray {
      public:
        std::string        array_name_;
        std::string        bool_array_name_;
        std::vector<bool>* bool_array_;
    };

    class OutputIntArray {
      public:
        std::string       array_name_;
        std::string       int_array_name_;
        std::vector<int>* int_array_;
    };

    class OutputDoubleArray {
      public:
        std::string          array_name_;
        std::string          double_array_name_;
        std::vector<double>* double_array_;
    };

  public:
    // CALLS TO THIS CLASS SHOULD BE MADE IN THE ORDER
    // THAT THE FUNCTIONS ARE GIVEN HERE

    // Make sure it starts with a clean definition file
    NewArrayMathEngine() { clear(); pr_ = &def_pr_; }

    void setLogger(Core::Logging::LegacyLoggerInterface* logger) { pr_ = logger; }

    // Generate inputs for field data and field data properties
    bool add_input_fielddata(const std::string& name,
                             FieldHandle field);
    bool add_input_fielddata_location(const std::string& name,
                                      FieldHandle field);
    bool add_input_fielddata_coordinates(const std::string& xname,
                                         const std::string& yname,
                                         const std::string& zname,
                                         FieldHandle field);
    bool add_input_fielddata_location(const std::string& name,
                                      FieldHandle field,
                                      int basis_order);
    bool add_input_fielddata_coordinates(const std::string& xname,
                                         const std::string& yname,
                                         const std::string& zname,
                                         FieldHandle field,
                                         int basis_order);
    bool add_input_fielddata_element(const std::string& name,
                                     FieldHandle field);
    bool add_input_fielddata_element(const std::string& name,
                                     FieldHandle field,
                                     int basis_order);

    bool add_input_fieldnodes(const std::string& name,
                              FieldHandle field);
    bool add_input_fieldnodes_coordinates(const std::string& xname,
                              const std::string& yname,
                              const std::string& zname,
                              FieldHandle field);

    // Generate input matrices
    bool add_input_matrix(const std::string& name,
                          Core::Datatypes::MatrixHandle matrix);

    bool add_input_fullmatrix(const std::string& name,
                          Core::Datatypes::MatrixHandle matrix);

    // Generate input arrays
    bool add_input_bool_array(const std::string& name,   std::vector<bool>* array);
    bool add_input_int_array(const std::string& name,    std::vector<int>* array);
    bool add_input_double_array(const std::string& name, std::vector<double>* array);

    bool add_index(const std::string& name);
    bool add_size(const std::string& name);


    // Setup an output Field that contains the altered field data
    // One can add change the basis_order and datatype if needed
    bool add_output_fielddata(const std::string& name,
                              FieldHandle field,
                              int output_basis_order,
                              const std::string& output_datatype);

    // Setup an output Field that contains the altered field data
    // This one takes the input field datatype and basis order
    bool add_output_fielddata(const std::string& name,
                              FieldHandle field);


    // Setup a field whose nodes need to be changed, the data is copied
    // and the field nodes are changed
    bool add_output_fieldnodes(const std::string& name,
                               FieldHandle field);

    // Setup a matrix for output
    bool add_output_matrix(const std::string& name);

    // Setup a matrix for output, with a predefined length
    bool add_output_matrix(const std::string& name,
                           size_type size);

    bool add_output_fullmatrix(const std::string& name,
                               Core::Datatypes::MatrixHandle matrix);

    bool add_output_bool_array(const std::string& name,
                               std::vector<bool>* array);
    bool add_output_int_array(const std::string& name,
                              std::vector<int>* array);
    bool add_output_double_array(const std::string& name,
                                 std::vector<double>* array);

    // Setup the expression
    bool add_expressions(const std::string& expressions);

    // Run the expressions in parallel
    bool run();

    // Extract handles to the results
    bool get_field(const std::string& name, FieldHandle& field);
    bool get_matrix(const std::string& name, Core::Datatypes::MatrixHandle& matrix);

    // Clean up the engine
    void clear();

  private:
    Core::Logging::ConsoleLogger def_pr_;
    // Progress reporter for reporting error
    Core::Logging::LegacyLoggerInterface* pr_;

    // Parser program : the structure of the expressions and simple reduction
    // of the expressions to simple function calls
    ParserProgramHandle    pprogram_;
    // Wrapper around the function calls, this piece actually executes the code
    ArrayMathProgramHandle mprogram_;

    // Expression to evaluate before the main expression
    // This one is to extract the variables from the data sources
    // This reduces the amount of actual functions we need to implement
    std::string pre_expression_;
    // The user defined expression
    std::string expression_;
    // Expression to get the data back into the data sinks
    std::string post_expression_;

    // The size of the array engine, the first call that add an array that is
    // bigger than 1, will set this variable
    // Any subsequent array that does not match the size will cause an error
    size_type array_size_;

    // Data that needs to be stored as it is needed before and after the parser is
    // done. An output needs to be set before the parser, otherwise it is optimized
    // away, but the type is only know when the parser has validated and optimized
    // the expression tree

    std::vector<OutputFieldData> fielddata_;
    std::vector<OutputFieldMesh> fieldmesh_;
    std::vector<OutputMatrix>    matrixdata_;
    std::vector<OutputBoolArray>   boolarraydata_;
    std::vector<OutputIntArray>    intarraydata_;
    std::vector<OutputDoubleArray>   doublearraydata_;

};

}

#endif
