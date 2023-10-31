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

    ProvenanceManager(IOType* networkIO, Core::PythonCommandInterpreterInterface* py);
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
    IOType* networkIO_;
    Core::PythonCommandInterpreterInterface* py_;
    Stack undo_, redo_;
    std::optional<Memento> initialState_;
  };



  template <class Memento>
  ProvenanceManager<Memento>::ProvenanceManager(IOType* networkIO, Core::PythonCommandInterpreterInterface* py)
    : networkIO_(networkIO), py_(py) {}

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
    if (!undo_.empty())
    {
      auto undone = undo_.top();
      if (py_)
        py_->run_string(undone->undoCode());
      else
        logCritical("Undo/redo not available without Python enabled.");
      undo_.pop();
      redo_.push(undone);

      {
        //logCritical("TODO: memento-based undo is disabled while a python-based implementation is developed.");

        //networkIO_->clear();
        //if (!undo_.empty())
        //  networkIO_->loadNetwork(undo_.top()->memento());
        //else if (initialState_)
        //  networkIO_->loadNetwork(*initialState_);
      }

      return undone;
    }
    return nullptr;
  }

  template <class Memento>
  typename ProvenanceManager<Memento>::ItemHandle ProvenanceManager<Memento>::redo()
  {
    if (!redo_.empty())
    {
      auto redone = redo_.top();
      if (py_)
        py_->run_string(redone->redoCode());
      else
        logCritical("Undo/redo not available without Python enabled.");
      redo_.pop();
      undo_.push(redone);

      //clear and load redone memento
      // if (true)
      // {
      //   logCritical("TODO: memento-based redo is disabled while a python-based implementation is developed.");
      //   //networkIO_->clear();
      //   //networkIO_->loadNetwork(redone->memento());
      // }

      return redone;
    }
    return nullptr;
  }

  template <class Memento>
  typename ProvenanceManager<Memento>::List ProvenanceManager<Memento>::undoAll()
  {
    logCritical("TODO: memento-based undo is disabled while a python-based implementation is developed.");
    List undone;
    while (0 != undoSize())
      undone.push_back(undo());
    // networkIO_->clear();
    // if (initialState_)
    //   networkIO_->loadNetwork(*initialState_);
    return undone;
  }

  template <class Memento>
  typename ProvenanceManager<Memento>::List ProvenanceManager<Memento>::redoAll()
  {
    logCritical("TODO: memento-based redo is disabled while a python-based implementation is developed.");
    List redone;
    while (0 != redoSize())
      redone.push_back(redo());
    // networkIO_->clear();
    // networkIO_->loadNetwork(undo_.top()->memento());
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
