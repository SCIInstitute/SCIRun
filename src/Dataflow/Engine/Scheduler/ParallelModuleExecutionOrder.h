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


#ifndef ENGINE_SCHEDULER_PARALLEL_MODULE_EXECUTION_ORDER_H
#define ENGINE_SCHEDULER_PARALLEL_MODULE_EXECUTION_ORDER_H

#include <map>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Engine/Scheduler/SchedulerInterfaces.h>
#include <Dataflow/Engine/Scheduler/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {

  class SCISHARE ParallelModuleExecutionOrder
  {
  public:
    typedef std::multimap<int, Networks::ModuleId> ModulesByGroup;
    typedef ModulesByGroup::value_type value_type;
    typedef ModulesByGroup::iterator iterator;
    typedef ModulesByGroup::const_iterator const_iterator;

    ParallelModuleExecutionOrder();
    ParallelModuleExecutionOrder(const ParallelModuleExecutionOrder& other);
    explicit ParallelModuleExecutionOrder(const ModulesByGroup& map);
    size_t size() const;
    const_iterator begin() const;
    const_iterator end() const;
    int minGroup() const;
    int maxGroup() const;
    std::pair<const_iterator,const_iterator> getGroup(int order) const;
    int groupOf(const Networks::ModuleId& id) const;
  private:
    ModulesByGroup map_;
  };

  SCISHARE std::ostream& operator<<(std::ostream& out, const ParallelModuleExecutionOrder& order);

}
}}

#endif
