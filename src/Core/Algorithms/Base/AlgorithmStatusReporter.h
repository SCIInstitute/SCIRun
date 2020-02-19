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


#ifndef ALGORITHMS_BASE_ALGORITHMSTATUSREPORTER_H
#define ALGORITHMS_BASE_ALGORITHMSTATUSREPORTER_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <Core/Utils/ProgressReporter.h>
#include <Core/Algorithms/Base/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {

  /// @todo: integrate with logger type
  class SCISHARE AlgorithmStatusReporter : public Core::Utility::ProgressReporter
  {
  public:
    AlgorithmStatusReporter();
    ~AlgorithmStatusReporter() {}

    void report_start(const std::string&) const override {}
    void report_end() const override {}

    void update_progress(double percent) const override
    {
      if (updaterFunc_)
        updaterFunc_(percent);
    }

    using UpdaterFunc = boost::function<void(double)>;
    void setUpdaterFunc(UpdaterFunc func) { updaterFunc_ = func; }
    UpdaterFunc getUpdaterFunc() const { return updaterFunc_; }
  private:
    UpdaterFunc updaterFunc_;
    static UpdaterFunc defaultUpdaterFunc_;
  };

  class SCISHARE ScopedAlgorithmStatusReporter : boost::noncopyable
  {
  public:
    ScopedAlgorithmStatusReporter(const AlgorithmStatusReporter* asr, const std::string& tag);
    ~ScopedAlgorithmStatusReporter();
  private:
    const AlgorithmStatusReporter* asr_;
  };

  #define REPORT_STATUS(className) ScopedAlgorithmStatusReporter __asr(this, #className);

}}}

#endif
