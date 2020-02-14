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


#include <Dataflow/Engine/Scheduler/ParallelModuleExecutionOrder.h>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

ParallelModuleExecutionOrder::ParallelModuleExecutionOrder()
{
}

ParallelModuleExecutionOrder::ParallelModuleExecutionOrder(const ParallelModuleExecutionOrder& other) : map_(other.map_)
{
}

ParallelModuleExecutionOrder::ParallelModuleExecutionOrder(const ParallelModuleExecutionOrder::ModulesByGroup& map) : map_(map)
{
}

size_t ParallelModuleExecutionOrder::size() const
{
  return map_.size();
}

ParallelModuleExecutionOrder::const_iterator ParallelModuleExecutionOrder::begin() const
{
  return map_.begin();
}

ParallelModuleExecutionOrder::const_iterator ParallelModuleExecutionOrder::end() const
{
  return map_.end();
}

int ParallelModuleExecutionOrder::minGroup() const
{
  return map_.empty() ? -1 : map_.begin()->first;
}

int ParallelModuleExecutionOrder::maxGroup() const
{
  return map_.empty() ? -1 : map_.rbegin()->first;
}

std::pair<ParallelModuleExecutionOrder::const_iterator, ParallelModuleExecutionOrder::const_iterator> ParallelModuleExecutionOrder::getGroup(int order) const
{
  return map_.equal_range(order);
}

int ParallelModuleExecutionOrder::groupOf(const ModuleId& id) const
{
  for (const auto& p : map_)
  {
    if (p.second == id)
      return p.first;
  }
  return -1;
}

std::ostream& SCIRun::Dataflow::Engine::operator<<(std::ostream& out, const ParallelModuleExecutionOrder& order)
{
  // platform-independent sorting for verification purposes.
  typedef std::pair<int,ModuleId> GroupModPair;
  std::vector<GroupModPair> vec(order.begin(), order.end());
  std::sort(vec.begin(), vec.end(), [](const GroupModPair& v1, const GroupModPair& v2) -> bool { if (v1.first < v2.first) return true; if (v1.first > v2.first) return false; return v1.second < v2.second; });
  for (const GroupModPair& v : vec)
  {
    out << v.first << " " << v.second << std::endl;
  }
  return out;
}
