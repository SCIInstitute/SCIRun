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


/// @todo Documentation Dataflow/Engine/Controller/ProvenanceItemImpl.h

#ifndef ENGINE_NETWORK_PROVENANCEITEMIMPL_H
#define ENGINE_NETWORK_PROVENANCEITEMIMPL_H

#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Engine/Controller/ProvenanceItem.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Dataflow/Engine/Controller/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {

  class SCISHARE ProvenanceItemBase : public ProvenanceItem<Networks::NetworkFileHandle>
  {
  public:
    explicit ProvenanceItemBase(Networks::NetworkFileHandle state);
    virtual Networks::NetworkFileHandle memento() const;
  protected:
    Networks::NetworkFileHandle state_;
  };

  class SCISHARE ModuleAddedProvenanceItem : public ProvenanceItemBase
  {
  public:
    ModuleAddedProvenanceItem(const std::string& moduleName, Networks::NetworkFileHandle state);
    virtual std::string name() const;
  private:
    std::string moduleName_;
  };

  class SCISHARE ModuleRemovedProvenanceItem : public ProvenanceItemBase
  {
  public:
    ModuleRemovedProvenanceItem(const SCIRun::Dataflow::Networks::ModuleId& moduleId, Networks::NetworkFileHandle state);
    virtual std::string name() const;
  private:
    SCIRun::Dataflow::Networks::ModuleId moduleId_;
  };

  class SCISHARE ConnectionAddedProvenanceItem : public ProvenanceItemBase
  {
  public:
    ConnectionAddedProvenanceItem(const SCIRun::Dataflow::Networks::ConnectionDescription& cd, Networks::NetworkFileHandle state);
    virtual std::string name() const;
  private:
    SCIRun::Dataflow::Networks::ConnectionDescription desc_;
  };

  class SCISHARE ConnectionRemovedProvenanceItem : public ProvenanceItemBase
  {
  public:
    ConnectionRemovedProvenanceItem(const SCIRun::Dataflow::Networks::ConnectionId& id, Networks::NetworkFileHandle state);
    virtual std::string name() const;
  private:
    SCIRun::Dataflow::Networks::ConnectionId id_;
  };

  class SCISHARE ModuleMovedProvenanceItem : public ProvenanceItemBase
  {
  public:
    ModuleMovedProvenanceItem(const SCIRun::Dataflow::Networks::ModuleId& moduleId, double newX, double newY, Networks::NetworkFileHandle state);
    virtual std::string name() const;
  private:
    SCIRun::Dataflow::Networks::ModuleId moduleId_;
    double newX_, newY_;
  };
}
}
}

#endif
