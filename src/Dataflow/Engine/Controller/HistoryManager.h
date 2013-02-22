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

#ifndef ENGINE_NETWORK_HISTORYMANAGER_H
#define ENGINE_NETWORK_HISTORYMANAGER_H

#include <stack>
#include <boost/noncopyable.hpp>
#include <Dataflow/Engine/Controller/HistoryItem.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Engine/Controller/Share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {
  
  template <class Memento>
  class HistoryManager : boost::noncopyable
  {
  public:
    typedef HistoryItem<Memento> Item;
    typedef typename Item::Handle ItemHandle;
    typedef std::stack<ItemHandle> Stack;
    typedef typename Stack::container_type List;

    explicit HistoryManager(Engine::NetworkIOInterface<Memento>* networkIO);
    void addItem(ItemHandle item);
    ItemHandle undo();
    ItemHandle redo();
    
    List undoAll();
    List redoAll();

    void clearAll();

    size_t undoSize() const;
    size_t redoSize() const;

  private:
    Engine::NetworkIOInterface<Memento>* networkIO_;
    Stack undo_, redo_;
  };



  template <class Memento>
  HistoryManager<Memento>::HistoryManager(NetworkIOInterface<Memento>* networkIO) : networkIO_(networkIO) {}

  template <class Memento>
  size_t HistoryManager<Memento>::undoSize() const
  {
    return undo_.size();
  }

  template <class Memento>
  size_t HistoryManager<Memento>::redoSize() const
  {
    return redo_.size();
  }

  template <class Memento>
  void HistoryManager<Memento>::addItem(typename HistoryManager<Memento>::ItemHandle item)
  {
    undo_.push(item);
    Stack().swap(redo_);
  }

  template <class Memento>
  void HistoryManager<Memento>::clearAll()
  {
    Stack().swap(undo_);
    Stack().swap(redo_);
  }

  template <class Memento>
  typename HistoryManager<Memento>::ItemHandle HistoryManager<Memento>::undo()
  {
    if (!undo_.empty())
    {
      auto undone = undo_.top();
      undo_.pop();
      redo_.push(undone);

      //clear and load previous memento
      networkIO_->clear();
      if (!undo_.empty())
        networkIO_->loadNetwork(undo_.top()->memento());
      
      return undone;
    }
    return ItemHandle();
  }

  template <class Memento>
  typename HistoryManager<Memento>::ItemHandle HistoryManager<Memento>::redo()
  {
    if (!redo_.empty())
    {
      auto redone = redo_.top();
      redo_.pop();
      undo_.push(redone);

      //clear and load redone memento
      networkIO_->clear();
      networkIO_->loadNetwork(redone->memento());
      
      return redone;
    }
    return ItemHandle();
  }

  template <class Memento>
  typename HistoryManager<Memento>::List HistoryManager<Memento>::undoAll()
  {
    List undone;
    while (0 != undoSize())
      undone.push_back(undo());
    return undone;
  }

  template <class Memento>
  typename HistoryManager<Memento>::List HistoryManager<Memento>::redoAll()
  {
    List redone;
    while (0 != redoSize())
      redone.push_back(redo());
    return redone;
  }





}
}
}

#endif