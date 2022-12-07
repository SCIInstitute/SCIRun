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


#ifndef ALGORITHMS_BASE_ALGORITHMPRECONDITIONS_H
#define ALGORITHMS_BASE_ALGORITHMPRECONDITIONS_H

#include <Core/Utils/Exception.h>
#include <Core/Algorithms/Base/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {

  struct SCISHARE AlgorithmInputException : virtual ExceptionBase
  {
  };

  struct SCISHARE AlgorithmProcessingException : virtual ExceptionBase
  {
  };

  struct SCISHARE AlgorithmParameterNotFound : virtual AlgorithmProcessingException /// @todo??
  {
  };

#define THROW_ALGORITHM_INPUT_ERROR_WITH_TYPE_AND_MESSAGE(type, messageToPrint, messageForErrorObject) { error(messageToPrint); BOOST_THROW_EXCEPTION(SCIRun::Core::Algorithms::AlgorithmInputException() << type((messageForErrorObject))); }
#define THROW_ALGORITHM_INPUT_ERROR_WITH_TYPE(type, message) THROW_ALGORITHM_INPUT_ERROR_WITH_TYPE_AND_MESSAGE(type, message, message)
#define THROW_ALGORITHM_INPUT_ERROR(message) THROW_ALGORITHM_INPUT_ERROR_WITH_TYPE(SCIRun::Core::ErrorMessage, message)
#define THROW_ALGORITHM_INPUT_ERROR_SIMPLE(message) BOOST_THROW_EXCEPTION(SCIRun::Core::Algorithms::AlgorithmInputException() << SCIRun::Core::ErrorMessage((message)));
#define IF_CHECK_FAILED_THROW_ALGORITHM_INPUT_ERROR(flagMessage) if (!std::get<0>(flagMessage)) THROW_ALGORITHM_INPUT_ERROR(std::get<1>(flagMessage));

#define THROW_ALGORITHM_INPUT_ERROR_WITH_TYPE_AND_MESSAGE_WITH(algo, type, messageToPrint, messageForErrorObject) { algo->error(messageToPrint); BOOST_THROW_EXCEPTION(SCIRun::Core::Algorithms::AlgorithmInputException() << type((messageForErrorObject))); }
#define THROW_ALGORITHM_INPUT_ERROR_WITH_TYPE_WITH(algo, type, message) THROW_ALGORITHM_INPUT_ERROR_WITH_TYPE_AND_MESSAGE_WITH(algo, type, message, message)
#define THROW_ALGORITHM_INPUT_ERROR_WITH(algo, message) THROW_ALGORITHM_INPUT_ERROR_WITH_TYPE_WITH(algo, SCIRun::Core::ErrorMessage, message)

#define THROW_ALGORITHM_PROCESSING_ERROR_WITH_TYPE(type, message) { error(message); BOOST_THROW_EXCEPTION(SCIRun::Core::Algorithms::AlgorithmProcessingException() << type((message))); }
#define THROW_ALGORITHM_PROCESSING_ERROR(message) THROW_ALGORITHM_PROCESSING_ERROR_WITH_TYPE(SCIRun::Core::ErrorMessage, message)

#define ENSURE_ALGORITHM_INPUT_NOT_NULL(var, message)  if (!(var)) THROW_ALGORITHM_INPUT_ERROR_WITH_TYPE(SCIRun::Core::NullObjectInfo, message) else (void)0

#define ENSURE_POSITIVE_DOUBLE(var, message)  if ((var) <= 0) \
  THROW_ALGORITHM_INPUT_ERROR_WITH_TYPE_AND_MESSAGE(DoubleOutOfRangeInfo, message, \
    DoubleOutOfRangeInfo::value_type( \
    std::string(message), \
    var, \
    boost::numeric::interval<double>(0, std::numeric_limits<double>::infinity()))) \
    else (void)0

#define ENSURE_POSITIVE_INT(var, message) if ((var) <= 0) \
    THROW_ALGORITHM_INPUT_ERROR_WITH_TYPE_AND_MESSAGE(IntOutOfRangeInfo, message, \
    IntOutOfRangeInfo::value_type( \
    std::string(message), \
    var, \
    boost::numeric::interval<int>(0, std::numeric_limits<int>::infinity()))) \
    else (void)0

#define ENSURE_FILE_EXISTS(filename) if (!boost::filesystem::exists(filename))\
  THROW_ALGORITHM_INPUT_ERROR_WITH_TYPE(FileNotFound, std::string("File not found: (") + (filename.size() != 0 ? filename : "<no file set>") + ")") \
  else (void)0

}}}

#endif
