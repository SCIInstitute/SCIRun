/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#ifndef CORE_UTILS_EXCEPTION_H
#define CORE_UTILS_EXCEPTION_H

#include <stdexcept>
#include <boost/exception/all.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/numeric/interval.hpp>
#include <Core/Utils/Share.h>

namespace SCIRun 
{
namespace Core
{
  //See:
  //http://www.boost.org/doc/libs/1_51_0/libs/exception/doc/exception_types_as_simple_semantic_tags.html

  struct SCISHARE ExceptionBase : virtual std::exception, virtual boost::exception
  {
    virtual const char* what() const throw();
    std::string typeName() const;
  };

  typedef boost::error_info<struct tag_error_message, std::string> ErrorMessage;
  typedef boost::error_info<struct tag_null_object, std::string> NullObjectInfo;
  typedef boost::error_info<struct tag_file_not_found, std::string> FileNotFound;
  //TODO: discuss location/type
  typedef boost::error_info<struct tag_linear_algebra_error, std::string> LinearAlgebraErrorMessage;
  //TODO: make macro for various types
  typedef boost::error_info<struct tag_double_out_of_range, boost::tuple<std::string, double, boost::numeric::interval<double> > > DoubleOutOfRangeInfo;
  typedef boost::error_info<struct tag_int_out_of_range, boost::tuple<std::string, int, boost::numeric::interval<int> > > IntOutOfRangeInfo;

  typedef boost::error_info<struct tag_dimension_mismatch, std::string> DimensionMismatchInfo;
  typedef boost::error_info<struct tag_invalid_argument_value, std::string> InvalidArgumentValueInfo;
  typedef boost::error_info<struct tag_not_implemented, std::string> NotImplementedInfo;
  
  // TODO: move these exceptions to new exception header file once it exists
  struct SCISHARE DimensionMismatch : virtual ExceptionBase
  {
  };

  struct SCISHARE InvalidArgumentValue : virtual ExceptionBase
  {
  };
  
  // TODO: should not need this in production code.
  //
  // Any prototype code using this exception should be reviewed and improved!!!
  struct SCISHARE NotImplemented : virtual ExceptionBase
  {
  };

#define ENSURE_DIMENSIONS_MATCH(var1, var2, message)  if (var1 != var2) \
  BOOST_THROW_EXCEPTION(DimensionMismatch() << DimensionMismatchInfo( \
    DimensionMismatchInfo::value_type( \
      std::string(message) )))

#define REPORT_INVALID_ARGUMENT_VALUE(message) \
  BOOST_THROW_EXCEPTION(InvalidArgumentValue() << InvalidArgumentValueInfo( \
    InvalidArgumentValueInfo::value_type( \
      std::string(message) )))
  
// TODO: should not need this in production code.
//
// Any prototype code using this exception should be reviewed and improved!!!
#define REPORT_NOT_IMPLEMENTED(message) \
  BOOST_THROW_EXCEPTION(NotImplemented() << NotImplementedInfo( \
    NotImplementedInfo::value_type( \
      std::string(message) )))

}
}

#endif
