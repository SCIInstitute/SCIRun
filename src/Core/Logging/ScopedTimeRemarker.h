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


/// @todo Documentation Core/Logging/ScopedTimeRemarker.h

#ifndef CORE_LOGGING_SCOPEDTIMEREMARKER_H
#define CORE_LOGGING_SCOPEDTIMEREMARKER_H

#include <string>
#include <chrono>
#include <Core/Logging/LoggerFwd.h>
#include <Core/Logging/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Logging
    {
      class SCISHARE SimpleScopedTimer
      {
      public:
        SimpleScopedTimer();
        SimpleScopedTimer(const SimpleScopedTimer&) = delete;
        SimpleScopedTimer& operator=(const SimpleScopedTimer&) = delete;
        double elapsedSeconds() const;
      private:
        const std::chrono::time_point<std::chrono::steady_clock> start_;
      };

      class SCISHARE ScopedTimeRemarker
      {
      public:
        ScopedTimeRemarker(LegacyLoggerInterface* log, const std::string& label);
        ~ScopedTimeRemarker();
      private:
        LegacyLoggerInterface* log_;
        std::string label_;
        SimpleScopedTimer timer_;
      };

      class SCISHARE ScopedTimeLogger
      {
      public:
        explicit ScopedTimeLogger(const std::string& label, bool shouldLog = true);
        ~ScopedTimeLogger();
      private:
        std::string label_;
        bool shouldLog_;
        SimpleScopedTimer timer_;
      };
    }
  }
}


#endif
