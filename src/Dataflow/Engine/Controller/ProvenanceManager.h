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


/// @todo Documentation Dataflow/Engine/Controller/ProvenanceManager.h

#ifndef ENGINE_NETWORK_PROVENANCEMANAGER_H
#define ENGINE_NETWORK_PROVENANCEMANAGER_H

#include <stack>
#include <boost/noncopyable.hpp>
#include <Dataflow/Engine/Controller/ProvenanceItem.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Engine/Controller/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Engine {

  template <class Memento>
  class ProvenanceManager : boost::noncopyable
  {
  public:
    using Item = ProvenanceItem<Memento>;
    using ItemHandle = typename Item::Handle;
    using Stack = std::stack<ItemHandle>;
    using List = typename Stack::container_type;
    using IOType = Engine::NetworkIOInterface<Memento>;

    explicit ProvenanceManager(IOType* networkIO);
    void setInitialState(const Memento& initialState);
    void addItem(ItemHandle item);
    ItemHandle undo();
    ItemHandle redo();

    List undoAll();
    List redoAll();

    void clearAll();

    size_t undoSize() const;
    size_t redoSize() const;

    const IOType* networkIO() const;

  private:
    ItemHandle undo(bool restore);
    ItemHandle redo(bool restore);
    IOType* networkIO_;
    Stack undo_, redo_;
    boost::optional<Memento> initialState_;
  };



  template <class Memento>
  ProvenanceManager<Memento>::ProvenanceManager(IOType* networkIO) : networkIO_(networkIO) {}

  template <class Memento>
  void ProvenanceManager<Memento>::setInitialState(const Memento& initialState)
  {
    initialState_ = initialState;
  }

  template <class Memento>
  size_t ProvenanceManager<Memento>::undoSize() const
  {
    return undo_.size();
  }

  template <class Memento>
  size_t ProvenanceManager<Memento>::redoSize() const
  {
    return redo_.size();
  }

  template <class Memento>
  void ProvenanceManager<Memento>::addItem(typename ProvenanceManager<Memento>::ItemHandle item)
  {
    undo_.push(item);
    Stack().swap(redo_);
  }

  template <class Memento>
  void ProvenanceManager<Memento>::clearAll()
  {
    Stack().swap(undo_);
    Stack().swap(redo_);
  }

  template <class Memento>
  typename ProvenanceManager<Memento>::ItemHandle ProvenanceManager<Memento>::undo()
  {
    return undo(true);
  }

  template <class Memento>
  typename ProvenanceManager<Memento>::ItemHandle ProvenanceManager<Memento>::undo(bool restore)
  {
    if (!undo_.empty())
    {
      auto undone = undo_.top();
      undo_.pop();
      redo_.push(undone);

      //clear and load previous memento
      if (restore)
      {
        networkIO_->clear();
        if (!undo_.empty())
          networkIO_->loadNetwork(undo_.top()->memento());
        else if (initialState_)
          networkIO_->loadNetwork(initialState_.get());
      }

      return undone;
    }
    return ItemHandle();
  }

  template <class Memento>
  typename ProvenanceManager<Memento>::ItemHandle ProvenanceManager<Memento>::redo()
  {
    return redo(true);
  }

  template <class Memento>
  typename ProvenanceManager<Memento>::ItemHandle ProvenanceManager<Memento>::redo(bool restore)
  {
    if (!redo_.empty())
    {
      auto redone = redo_.top();
      redo_.pop();
      undo_.push(redone);

      //clear and load redone memento
      if (restore)
      {
        networkIO_->clear();
        networkIO_->loadNetwork(redone->memento());
      }

      return redone;
    }
    return ItemHandle();
  }

  template <class Memento>
  typename ProvenanceManager<Memento>::List ProvenanceManager<Memento>::undoAll()
  {
    List undone;
    while (0 != undoSize())
      undone.push_back(undo(false));
    networkIO_->clear();
    if (initialState_)
      networkIO_->loadNetwork(initialState_.get());
    return undone;
  }

  template <class Memento>
  typename ProvenanceManager<Memento>::List ProvenanceManager<Memento>::redoAll()
  {
    List redone;
    while (0 != redoSize())
      redone.push_back(redo(false));
    networkIO_->clear();
    networkIO_->loadNetwork(undo_.top()->memento());
    return redone;
  }

  template <class Memento>
  const typename ProvenanceManager<Memento>::IOType* ProvenanceManager<Memento>::networkIO() const
  {
    return networkIO_;
  }


}
}
}

#endif
