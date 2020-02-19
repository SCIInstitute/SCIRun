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


#ifndef ALGORITHMS_BASE_ALGORITHMLOGGER_H
#define ALGORITHMS_BASE_ALGORITHMLOGGER_H

#include <Core/Logging/LoggerInterface.h>
#include <Core/Algorithms/Base/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {

  class SCISHARE AlgorithmLogger : public Core::Logging::LegacyLoggerInterface
  {
  public:
    AlgorithmLogger();
    ~AlgorithmLogger();
    void setLogger(Core::Logging::LoggerHandle logger);
    Core::Logging::LoggerHandle getLogger() const { return logger_; }

    /// functions for the algorithm, so it can forward errors if needed
    virtual void error(const std::string& error) const;
    virtual bool errorReported() const;
    virtual void setErrorFlag(bool flag);
    virtual void warning(const std::string& warning) const;
    virtual void remark(const std::string& remark) const;
    virtual void status(const std::string& status) const;
  private:
    Core::Logging::LoggerHandle logger_;
    Core::Logging::LoggerHandle defaultLogger_;
  };

}}}

#endif
