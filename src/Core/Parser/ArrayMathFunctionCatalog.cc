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


#include <Core/Parser/ArrayMathFunctionCatalog.h>
#include <Core/Thread/Mutex.h>

using namespace SCIRun;
using namespace SCIRun::Core::Thread;

//-----------------------------------------------------------------------------
// Setup main ParserFunctionCatalog

// Global function for retrieving the function catalog
ParserFunctionCatalogHandle
ArrayMathFunctionCatalog::get_catalog()
{
  static Mutex lock_("ArrayMathFunctionCatalog");
  static ArrayMathFunctionCatalogHandle catalog_;

  {
    Guard g(lock_.get());
    if (!catalog_)
    {
      catalog_.reset(new ArrayMathFunctionCatalog);
      InsertBasicArrayMathFunctionCatalog(catalog_);
      InsertSourceSinkArrayMathFunctionCatalog(catalog_);
      InsertScalarArrayMathFunctionCatalog(catalog_);
      InsertVectorArrayMathFunctionCatalog(catalog_);
      InsertTensorArrayMathFunctionCatalog(catalog_);
      InsertElementArrayMathFunctionCatalog(catalog_);
    }
  }

  return catalog_;
}

//-----------------------------------------------------------------------------
// Functions for ArrayMathFunctionCollection

// Adding a function to a collection class will cause it to be inserted into
// the main catalog. This way we can distribute the function definitions
// into different classes
void
ArrayMathFunctionCatalog::add_function(
            ArrayMathFunctionPtr function,
            const std::string& function_id,
            const std::string& return_type)
{
  ParserFunctionCatalog::add_function(boost::make_shared<ArrayMathFunction>(function, function_id, return_type, 0));
}


// A symmetric function can have its arguments entered in any combination
// that matches the types. Arguments can be swapped to match types, or
// arguments can be swapped to optimize code by removing duplicate function
// calls

void
ArrayMathFunctionCatalog::add_sym_function(
            ArrayMathFunctionPtr function,
            const std::string& function_id,
            const std::string& return_type)
{
  ParserFunctionCatalog::add_function(boost::make_shared<ArrayMathFunction>(function, function_id, return_type, PARSER_SYMMETRIC_FUNCTION_E));
}


void
ArrayMathFunctionCatalog::add_seq_function(
            ArrayMathFunctionPtr function,
            const std::string& function_id,
            const std::string& return_type)
{
  ParserFunctionCatalog::add_function(boost::make_shared<ArrayMathFunction>(function, function_id, return_type, PARSER_SEQUENTIAL_FUNCTION_E));
}

void
ArrayMathFunctionCatalog::add_sgl_function(
            ArrayMathFunctionPtr function,
            const std::string& function_id,
            const std::string& return_type)
{
  ParserFunctionCatalog::add_function(boost::make_shared<ArrayMathFunction>(function, function_id, return_type, PARSER_SINGLE_FUNCTION_E));
}

void
ArrayMathFunctionCatalog::add_cst_function(
            ArrayMathFunctionPtr function,
            const std::string& function_id,
            const std::string& return_type)
{
  ParserFunctionCatalog::add_function(boost::make_shared<ArrayMathFunction>(function, function_id, return_type, PARSER_CONST_FUNCTION_E));
}
