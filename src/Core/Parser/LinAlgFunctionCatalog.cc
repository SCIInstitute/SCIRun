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


#include <Core/Parser/LinAlgFunctionCatalog.h>
#include <Core/Thread/Mutex.h>

namespace SCIRun {

  using namespace SCIRun::Core::Thread;
  using namespace SCIRun::Core::Datatypes;

//-----------------------------------------------------------------------------
// Setup main ParserFunctionCatalog

// Global function for retrieving the function catalog
ParserFunctionCatalogHandle
LinAlgFunctionCatalog::get_catalog()
{
  static Mutex lock_("LinAlgFunctionCatalog");
  static LinAlgFunctionCatalogHandle catalog_;

  lock_.lock();
  if (!catalog_)
  {
    catalog_.reset(new LinAlgFunctionCatalog);
    InsertBasicLinAlgFunctionCatalog(catalog_);
    InsertSourceSinkLinAlgFunctionCatalog(catalog_);
    InsertScalarLinAlgFunctionCatalog(catalog_);
  }
  lock_.unlock();

  return catalog_;
}

//-----------------------------------------------------------------------------
// Functions for LinAlgFunctionCollection

// Adding a function to a collection class will cause it to be inserted into
// the main catalog. This way we can distribute the function definitions
// into different classes
void
LinAlgFunctionCatalog::add_function(
            bool (*function)(LinAlgProgramCode& pc, std::string& str),
            std::string function_id,
            std::string return_type)
{
  // Adding function to catalog
  ParserFunctionCatalog::add_function(boost::make_shared<LinAlgFunction>(function,function_id,return_type,0));
}


// A symmetric function can have its arguments entered in any combination
// that matches the types. Argmuments can be swapped to match types, or
// arguments can be swapped to optimize code by removing duplicate functio
// calls

void
LinAlgFunctionCatalog::add_sym_function(
            bool (*function)(LinAlgProgramCode& pc, std::string& str),
            std::string function_id,
            std::string return_type)
{
  // Adding function to catalog
  ParserFunctionCatalog::add_function(boost::make_shared<LinAlgFunction>(function, function_id, return_type,
                            PARSER_SYMMETRIC_FUNCTION_E));
}

}
