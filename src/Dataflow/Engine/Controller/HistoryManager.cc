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

#include <Dataflow/Engine/Controller/HistoryManager.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;

HistoryManager::HistoryManager(NetworkIOHandle networkIO) : networkIO_(networkIO) {}

size_t HistoryManager::undoSize() const
{
  return undo_.size();
}

size_t HistoryManager::redoSize() const
{
  return redo_.size();
}

void HistoryManager::addItem(HistoryItemHandle item)
{
  undo_.push(item);
}

void HistoryManager::clearAll()
{
  Stack().swap(undo_);
  Stack().swap(redo_);
}

HistoryItemHandle HistoryManager::undo()
{
  if (!undo_.empty())
  {
    auto undone = undo_.top();
    undo_.pop();
    networkIO_->loadNetwork(undone->memento());
    redo_.push(undone);
    return undone;
  }
  return HistoryItemHandle();
}

HistoryItemHandle HistoryManager::redo()
{
  if (!redo_.empty())
  {
    auto redone = redo_.top();
    redo_.pop();
    networkIO_->loadNetwork(redone->memento());
    undo_.push(redone);
    return redone;
  }
  return HistoryItemHandle();
}

HistoryManager::List HistoryManager::undoAll()
{
  List undone;
  while (0 != undoSize())
    undone.push_back(undo());
  return undone;
}

HistoryManager::List HistoryManager::redoAll()
{
  List redone;
  while (0 != redoSize())
    redone.push_back(redo());
  return redone;
}