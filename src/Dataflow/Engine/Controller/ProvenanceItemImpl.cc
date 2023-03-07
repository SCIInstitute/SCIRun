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
#include <Dataflow/Engine/Python/NetworkEditorPythonInterface.h>
#include <Core/Logging/Log.h>
#include <spdlog/fmt/fmt.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

ProvenanceItemBase::ProvenanceItemBase(NetworkFileHandle state, SharedPointer<NetworkEditorPythonInterface> nedPy) : state_(state), nedPy_(nedPy)
{
}

NetworkFileHandle ProvenanceItemBase::memento() const
{
  return state_;
}

ModuleAddedProvenanceItem::ModuleAddedProvenanceItem(const std::string& moduleName, const std::string& modId, NetworkFileHandle state, SharedPointer<NetworkEditorPythonInterface> nedPy)
  : ProvenanceItemBase(state, nedPy), moduleName_(moduleName), moduleId_(modId)
{
}

std::string ModuleAddedProvenanceItem::name() const
{
  return "Module Added: " + moduleName_;
}

std::string ModuleAddedProvenanceItem::undoCode() const
{
  if (redone_)
  {
    logCritical("here is where i need to pull the most recently added id");
    moduleId_ = nedPy_->mostRecentAddModuleId();
  }
  return fmt::format("scirun_remove_module(\"{}\")", moduleId_);
}

std::string ModuleAddedProvenanceItem::redoCode() const
{
  redone_ = true;
  return fmt::format("scirun_add_module(\"{}\")", moduleName_);
}

ModuleRemovedProvenanceItem::ModuleRemovedProvenanceItem(const ModuleId& moduleId, NetworkFileHandle state, SharedPointer<NetworkEditorPythonInterface> nedPy)
  : ProvenanceItemBase(state, nedPy), moduleId_(moduleId)
{
}

std::string ModuleRemovedProvenanceItem::name() const
{
  return "Module Removed: " + moduleId_.id_;
}

std::string ModuleRemovedProvenanceItem::undoCode() const
{
  redone_ = true;
  return fmt::format("scirun_add_module(\"{}\")", moduleId_.name_);
}

std::string ModuleRemovedProvenanceItem::redoCode() const
{
  if (redone_)
  {
    //logCritical("here is where i need to pull the most recently added id");
    //moduleId_ = nedPy_->mostRecentAddModuleId();
  }
  return fmt::format("scirun_remove_module(\"{}\")", moduleId_.id_);
}

ConnectionAddedProvenanceItem::ConnectionAddedProvenanceItem(const SCIRun::Dataflow::Networks::ConnectionDescription& cd, NetworkFileHandle state, SharedPointer<NetworkEditorPythonInterface> nedPy)
  : ProvenanceItemBase(state, nedPy), desc_(cd)
{
}

std::string ConnectionAddedProvenanceItem::name() const
{
  return "Connection added: " + ConnectionId::create(desc_).id_;
}

ConnectionRemovedProvenanceItem::ConnectionRemovedProvenanceItem(const SCIRun::Dataflow::Networks::ConnectionId& id, NetworkFileHandle state, SharedPointer<NetworkEditorPythonInterface> nedPy)
  : ProvenanceItemBase(state, nedPy), id_(id)
{
}

std::string ConnectionRemovedProvenanceItem::name() const
{
  return "Connection Removed: " + id_.id_;
}

ModuleMovedProvenanceItem::ModuleMovedProvenanceItem(const SCIRun::Dataflow::Networks::ModuleId& moduleId, double newX, double newY, double oldX, double oldY, 
  NetworkFileHandle state, SharedPointer<NetworkEditorPythonInterface> nedPy)
  : ProvenanceItemBase(state, nedPy), moduleId_(moduleId), newX_(newX), newY_(newY), oldX_(oldX), oldY_(oldY)
{
}

std::string ModuleMovedProvenanceItem::name() const
{
  std::ostringstream ostr;
  ostr << "Module " << moduleId_.id_ << " moved to (" << newX_ << "," << newY_ << ")";
  return ostr.str();
}

std::string ModuleMovedProvenanceItem::undoCode() const
{
  return fmt::format("scirun_move_module(\"{}\", {}, {})", moduleId_.id_, oldX_, oldY_);
}

std::string ModuleMovedProvenanceItem::redoCode() const
{
  return fmt::format("scirun_move_module(\"{}\", {}, {})", moduleId_.id_, newX_, newY_);
}
