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


#include <Core/Parser/LinAlgEngine.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Logging/ConsoleLogger.h>

namespace SCIRun {

  using namespace SCIRun::Core::Datatypes;

  NewLinAlgEngine::NewLinAlgEngine() : def_pr_(new Core::Logging::ConsoleLogger), pr_(def_pr_)
  {
    clear();
  }

bool
NewLinAlgEngine::add_input_matrix(const std::string& name, MatrixHandle matrix)
{
  std::string error_str;
  if (!matrix)
  {
    pr_->error("No input matrix '"+name+"'.");
    return (false);
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname =  "__"+name;
  pre_expression_ += name+"=get_input_matrix("+tname+");";

  // Add the variable to the interpreter
  if(!(add_matrix_source(mprogram_,tname,matrix,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }
  // Add the variable to the parser
  if(!(add_input_variable(pprogram_,tname,"MI",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }

  return (true);
}


bool
NewLinAlgEngine::add_output_matrix(const std::string& name)
{
  std::string error_str;
  std::string tname =  "__"+name;

  post_expression_ += tname+"=set_output_matrix("+name+");";

  if(!(add_matrix_sink(mprogram_,tname,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }

  // Add the variable to the parser
  if(!(add_output_variable(pprogram_,tname,"MO",0)))
  {
    pr_->error("Could not add variable to the parser program.");
    return (false);
  }

  // Store information for processing after parsing has succeeded
  OutputMatrix m;
  m.array_name_ = name;
  m.matrix_name_ = tname;
  m.matrix_ = 0;
  matrixdata_.push_back(m);

  return (true);
}


bool
NewLinAlgEngine::add_expressions(const std::string& expressions)
{
  expression_ += expressions;
  return (true);
}


bool
NewLinAlgEngine::get_matrix(const std::string& name, MatrixHandle& matrix)
{
  std::string tname =  "__"+name;

  LinAlgProgramSource ps;
  if( mprogram_->find_sink(tname,ps))
  {
    matrix = ps.get_matrix();
    return (true);
  }

  return (false);
}

bool
NewLinAlgEngine::run()
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
  ParserFunctionCatalogHandle catalog = LinAlgFunctionCatalog::get_catalog();

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

  // Translate the code
  if (!(translate(pprogram_,mprogram_,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }

  // Run the program
  if (!(LinAlgInterpreter::run(mprogram_,error_str)))
  {
    pr_->error(error_str);
    return (false);
  }

  return (true);
}



void
NewLinAlgEngine::clear()
{
  // Reset all values
  pprogram_.reset();
  mprogram_.reset();

  pre_expression_.clear();
  expression_.clear();
  post_expression_.clear();

  matrixdata_.clear();
}

} // end namespace
