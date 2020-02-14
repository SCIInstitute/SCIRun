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


///
///@file  ProgressReporter.h
///
///@author
///       Yarden Livnat
///       Department of Computer Science
///       University of Utah
///@date  July 2003
///


#ifndef SCIRun_Core_Util_ProgressReporter_h
#define SCIRun_Core_Util_ProgressReporter_h


#include <string>

#include <Core/Utils/share.h>

namespace SCIRun {
  namespace Core {
    namespace Utility {

      class SCISHARE ProgressReporter
      {
        public:
          virtual ~ProgressReporter();

          virtual void report_start(const std::string& tag) const = 0;
          virtual void report_end() const = 0;

          // Execution time progress.
          // Percent is number between 0.0-1.0
          virtual void update_progress(double percent) const = 0;
          template <typename T1, typename T2>
          inline void update_progress_max(T1 current, T2 max) const { update_progress(((double)current) / max); }
      };

}}}

#endif
