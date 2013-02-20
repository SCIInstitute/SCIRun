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

#ifndef ENGINE_NETWORK_HISTORYITEM_H
#define ENGINE_NETWORK_HISTORYITEM_H

#include <deque>
#include <boost/noncopyable.hpp>
#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Command/Command.h>
#include <Dataflow/Engine/Controller/Share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {
  
  class SCISHARE HistoryItem : boost::noncopyable
  {
  public:
    ~HistoryItem();
    //virtual Core::Commands::CommandHandle command() const = 0;
    //virtual Networks::NetworkXMLHandle undoMemento() const = 0;
    virtual Networks::NetworkXMLHandle memento() const = 0;
    virtual std::string name() const = 0;
  };

  typedef boost::shared_ptr<HistoryItem> HistoryItemHandle;

  class SCISHARE HistoryManager : boost::noncopyable
  {
  public:
    typedef std::deque<HistoryItemHandle> List;
    explicit HistoryManager(Networks::NetworkHandle network);
    void addItem(HistoryItemHandle item);
    HistoryItemHandle undo();
    HistoryItemHandle redo();
    List undoMultiple(size_t count);
    List redoMultiple(size_t count);

    size_t size() const;
    HistoryItemHandle at(size_t index) const;
    List::const_iterator begin() const;
    List::const_iterator end() const;

  private:
    List list_;
  };
}
}
}

#endif