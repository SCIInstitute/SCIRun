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

#ifndef ENGINE_NETWORK_HISTORYITEMIMPL_H
#define ENGINE_NETWORK_HISTORYITEMIMPL_H

#include <Dataflow/Engine/Controller/HistoryItem.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Dataflow/Engine/Controller/Share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {
  
  class SCISHARE HistoryItemBase : public HistoryItem<Networks::NetworkFileHandle>
  {
  public:
    explicit HistoryItemBase(Networks::NetworkFileHandle state);
    virtual Networks::NetworkFileHandle memento() const;
  protected:
    Networks::NetworkFileHandle state_;
  };

  class SCISHARE ModuleAddedHistoryItem : public HistoryItemBase
  {
  public:
    ModuleAddedHistoryItem(const std::string& moduleName, Networks::NetworkFileHandle state);
    virtual std::string name() const;
  private:
    std::string moduleName_;
  };

  class SCISHARE ModuleRemovedHistoryItem : public HistoryItemBase
  {
  public:
    ModuleRemovedHistoryItem(const std::string& moduleId, Networks::NetworkFileHandle state);
    virtual std::string name() const;
  private:
    std::string moduleId_;
  };

  class SCISHARE ConnectionAddedHistoryItem : public HistoryItemBase
  {
  public:
    ConnectionAddedHistoryItem(const SCIRun::Dataflow::Networks::ConnectionDescription& cd, Networks::NetworkFileHandle state);
    virtual std::string name() const;
  private:
    SCIRun::Dataflow::Networks::ConnectionDescription desc_;
  };

  class SCISHARE ConnectionRemovedHistoryItem : public HistoryItemBase
  {
  public:
    ConnectionRemovedHistoryItem(const SCIRun::Dataflow::Networks::ConnectionId& id, Networks::NetworkFileHandle state);
    virtual std::string name() const;
  private:
    SCIRun::Dataflow::Networks::ConnectionId id_;
  };

  class SCISHARE ModuleMovedHistoryItem : public HistoryItemBase
  {
  public:
    ModuleMovedHistoryItem(const std::string& moduleId, double newX, double newY, Networks::NetworkFileHandle state);
    virtual std::string name() const;
  private:
    std::string moduleId_;
    double newX_, newY_;
  };
}
}
}

#endif