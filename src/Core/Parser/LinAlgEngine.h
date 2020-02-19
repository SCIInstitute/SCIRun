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


#ifndef CORE_PARSER_LINALGENGINE_H
#define CORE_PARSER_LINALGENGINE_H 1

#include <Core/Logging/LoggerFwd.h>
#include <Core/Parser/LinAlgInterpreter.h>
#include <Core/Parser/LinAlgFunctionCatalog.h>
#include <Core/Parser/Parser.h>

// Include files needed for Windows
#include <Core/Parser/share.h>

namespace SCIRun {

class SCISHARE NewLinAlgEngine : public Parser, public LinAlgInterpreter
{
  public:
    // Classes for storing data on output variables

    class OutputMatrix {
      public:
        std::string  array_name_;
        std::string  matrix_name_;
        Core::Datatypes::MatrixHandle matrix_;
    };

  public:
    // CALLS TO THIS CLASS SHOULD BE MADE IN THE ORDER
    // THAT THE FUNCTIONS ARE GIVEN HERE

    // Make sure it starts with a clean definition file
    NewLinAlgEngine();// { pr_ = &(def_pr_); clear(); }

    // Setup a progress reporter
    void setLogger(Core::Logging::LoggerHandle pr) { pr_ = pr; }

    // Generate input matrices
    bool add_input_matrix(const std::string& name, Core::Datatypes::MatrixHandle matrix);

    // Setup a matrix for output
    bool add_output_matrix(const std::string& name);

    // Setup the expression
    bool add_expressions(const std::string& expressions);

    // Run the expressions in parallel
    bool run();

    // Extract handles to the results
    bool get_matrix(const std::string& name, Core::Datatypes::MatrixHandle& matrix);

    // Clean up the engine
    void clear();

  private:
    Core::Logging::LoggerHandle  def_pr_;
    // Progress reporter for reporting error
    Core::Logging::LoggerHandle pr_;

    // Parser program : the structure of the expressions and simple reduction
    // of the expressions to simple function calls
    ParserProgramHandle    pprogram_;
    // Wrapper around the function calls, this piece actually executes the code
    LinAlgProgramHandle    mprogram_;

    // Expression to evaluate before the main expression
    // This one is to extract the variables from the data sources
    // This reduces the amount of actual functions we need to implement
    std::string pre_expression_;
    // The user defined expression
    std::string expression_;
    // Expression to get the data back into the data sinks
    std::string post_expression_;

    // Data that needs to be stored as it is needed before and after the parser is
    // done. An output needs to be set before the parser, otherwise it is optimized
    // away, but the type is only know when the parser has validated and optimized
    // the expression tree

    std::vector<OutputMatrix>    matrixdata_;
};

}

#endif
