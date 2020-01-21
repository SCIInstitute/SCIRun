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


/// @todo Documentation Dataflow/Engine/Controller/ProvenanceItemImpl.cc

#include <string>
#include <sstream>
#include <Dataflow/Engine/Controller/ProvenanceItemImpl.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

ProvenanceItemBase::ProvenanceItemBase(NetworkFileHandle state) : state_(state)
{
}

NetworkFileHandle ProvenanceItemBase::memento() const
{
  return state_;
}

ModuleAddedProvenanceItem::ModuleAddedProvenanceItem(const std::string& moduleName, NetworkFileHandle state)
  : ProvenanceItemBase(state), moduleName_(moduleName)
{
}

std::string ModuleAddedProvenanceItem::name() const
{
  return "Module Added: " + moduleName_;
}

ModuleRemovedProvenanceItem::ModuleRemovedProvenanceItem(const ModuleId& moduleId, NetworkFileHandle state)
  : ProvenanceItemBase(state), moduleId_(moduleId)
{
}

std::string ModuleRemovedProvenanceItem::name() const
{
  return "Module Removed: " + moduleId_.id_;
}

ConnectionAddedProvenanceItem::ConnectionAddedProvenanceItem(const SCIRun::Dataflow::Networks::ConnectionDescription& cd, NetworkFileHandle state)
  : ProvenanceItemBase(state), desc_(cd)
{
}

std::string ConnectionAddedProvenanceItem::name() const
{
  return "Connection added: " + ConnectionId::create(desc_).id_;
}

ConnectionRemovedProvenanceItem::ConnectionRemovedProvenanceItem(const SCIRun::Dataflow::Networks::ConnectionId& id, NetworkFileHandle state)
  : ProvenanceItemBase(state), id_(id)
{
}

std::string ConnectionRemovedProvenanceItem::name() const
{
  return "Connection Removed: " + id_.id_;
}

ModuleMovedProvenanceItem::ModuleMovedProvenanceItem(const SCIRun::Dataflow::Networks::ModuleId& moduleId, double newX, double newY, NetworkFileHandle state)
  : ProvenanceItemBase(state), moduleId_(moduleId), newX_(newX), newY_(newY)
{
}

std::string ModuleMovedProvenanceItem::name() const
{
  std::ostringstream ostr;
  ostr << "Module " << moduleId_.id_ << " moved to (" << newX_ << "," << newY_ << ")";
  return ostr.str();
}
