/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <string>
#include <Dataflow/Engine/Controller/HistoryItemImpl.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

HistoryItemBase::HistoryItemBase(NetworkFileHandle state) : state_(state)
{

}

NetworkFileHandle HistoryItemBase::memento() const
{
  return state_;
}

ModuleAddedHistoryItem::ModuleAddedHistoryItem(const std::string& moduleName, NetworkFileHandle state)
  : HistoryItemBase(state), moduleName_(moduleName)
{

}

std::string ModuleAddedHistoryItem::name() const
{
  return "Module Added: " + moduleName_;
}

ModuleRemovedHistoryItem::ModuleRemovedHistoryItem(const std::string& moduleId, NetworkFileHandle state)
  : HistoryItemBase(state), moduleId_(moduleId)
{

}

std::string ModuleRemovedHistoryItem::name() const
{
  return "Module Removed: " + moduleId_;
}

ConnectionAddedHistoryItem::ConnectionAddedHistoryItem(const SCIRun::Dataflow::Networks::ConnectionDescription& cd, NetworkFileHandle state)
  : HistoryItemBase(state), desc_(cd)
{

}

std::string ConnectionAddedHistoryItem::name() const
{
  return "Connection added: " + ConnectionId::create(desc_).id_;
}

ConnectionRemovedHistoryItem::ConnectionRemovedHistoryItem(const SCIRun::Dataflow::Networks::ConnectionId& id, NetworkFileHandle state)
  : HistoryItemBase(state), id_(id)
{

}

std::string ConnectionRemovedHistoryItem::name() const
{
  return "Connection Removed: " + id_.id_;
}