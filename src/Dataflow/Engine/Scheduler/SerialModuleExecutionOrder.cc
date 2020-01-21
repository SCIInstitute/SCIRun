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


#include <Dataflow/Engine/Scheduler/SerialModuleExecutionOrder.h>

using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

ModuleExecutionOrder::ModuleExecutionOrder()
{
}

ModuleExecutionOrder::ModuleExecutionOrder(const ModuleExecutionOrder::ModuleIdList& list) : list_(list)
{
}

ModuleExecutionOrder::const_iterator ModuleExecutionOrder::begin() const
{
  return list_.begin();
}

ModuleExecutionOrder::const_iterator ModuleExecutionOrder::end() const
{
  return list_.end();
}

bool SCIRun::Dataflow::Engine::operator==(const ModuleExecutionOrder& lhs, const ModuleExecutionOrder& rhs)
{
  return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

bool SCIRun::Dataflow::Engine::operator!=(const ModuleExecutionOrder& lhs, const ModuleExecutionOrder& rhs)
{
  return !(lhs == rhs);
}

std::ostream& SCIRun::Dataflow::Engine::operator<<(std::ostream& out, const ModuleExecutionOrder& order)
{
  std::copy(order.begin(), order.end(), std::ostream_iterator<ModuleExecutionOrder::value_type>(out, "\n"));
  return out;
}
