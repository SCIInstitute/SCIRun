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


#include <iostream>
#include <Modules/Basic/LoggingTester.h>
#include <Core/Logging/Log.h>
#include <Core/Logging/ScopedTimeRemarker.h>
#include <Core/Logging/ScopedFunctionLogger.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <boost/thread.hpp>

using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;

MODULE_INFO_DEF(LoggingTester, Testing, SCIRun)

LoggingTester::LoggingTester() : Module(staticInfo_, false)
{
  INITIALIZE_PORT(Input);
}

void LoggingTester::execute()
{
  // simplest method
  std::cout << "std::cout message" << std::endl;

  // legacy logging methods
  status("LoggingTester status message");
  remark("LoggingTester remark message");
  warning("LoggingTester warning message");
  error("LoggingTester error message");

  // logging macros
  LOG_TRACE("LoggingTester LOG_TRACE call at line {}", __LINE__);
  DEBUG_LOG_LINE_INFO
  LOG_DEBUG("LoggingTester LOG_DEBUG call at line {}", __LINE__);
  logInfo("LoggingTester logInfo call at line {}", __LINE__);
  logWarning("LoggingTester logWarning call at line {}", __LINE__);
  logError("LoggingTester logError call at line {}", __LINE__);
  logCritical("LoggingTester logCritical call at line {}", __LINE__);

  {
    ScopedTimeLogger s("Example ScopedTimeLogger, sleeping for 0.1 seconds");
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  }

  {
    ScopedTimeRemarker s(this, "Example ScopedTimeRemarker, sleeping for 0.1 seconds");
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
  }

  LOG_FUNCTION_SCOPE;

  // special macros for module/algo classes--should go last
  THROW_ALGORITHM_INPUT_ERROR("Example algo input error log+throw.");
}

void LoggingTester::setStateDefaults()
{

}
