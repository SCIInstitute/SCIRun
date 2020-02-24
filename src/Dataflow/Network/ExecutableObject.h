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


/// @todo Documentation Dataflow/Network/ExecutableObject.h

#ifndef DATAFLOW_NETWORK_EXECUTABLE_OBJECT_H
#define DATAFLOW_NETWORK_EXECUTABLE_OBJECT_H

#include <boost/signals2.hpp>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  using ExecuteBeginsSignalType = boost::signals2::signal<void (const ModuleId&)>;
  using ExecuteEndsSignalType = boost::signals2::signal<void (double, const ModuleId&)>; // 1st parameter is execution time
  using ErrorSignalType = boost::signals2::signal<void (const ModuleId&)>;

  class SCISHARE ExecutableObject
  {
  public:
    virtual ~ExecutableObject() {}
    virtual bool executeWithSignals() = 0;

    virtual boost::signals2::connection connectExecuteBegins(const ExecuteBeginsSignalType::slot_type& subscriber) = 0;
    virtual boost::signals2::connection connectExecuteEnds(const ExecuteEndsSignalType::slot_type& subscriber) = 0;
    virtual boost::signals2::connection connectErrorListener(const ErrorSignalType::slot_type& subscriber) = 0;
  };

}}}

#endif
