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


///
///@file  CleanupManager.cc
///@brief Manage exitAll callbacks.
///
///@author
///       Michael Callahan
///       Department of Computer Science
///       University of Utah
///@date  November 2004
///

#include <Core/Thread/Legacy/CleanupManager.h>


#include <algorithm>

using namespace SCIRun::Core::Thread;

namespace SCIRun {

  std::shared_ptr<std::vector<CleanupManager::CMCPair>> CleanupManager::callbacks_;
  bool CleanupManager::initialized_ = false;
  std::shared_ptr<Mutex> CleanupManager::lock_;

  void CleanupManager::initialize()
  {
    if (initialized_)
      return;
    initialized_ = true;

    callbacks_.reset(new std::vector<CMCPair>);
    lock_.reset(new Mutex("CleanupManager lock"));
  }

  void CleanupManager::add_callback(CleanupManagerCallback cb, void* data)
  {
    if (!initialized_) initialize();

    Guard g(lock_->get());
    if (std::find(callbacks_->begin(), callbacks_->end(), CMCPair(cb, data)) == callbacks_->end())
    {
      callbacks_->push_back(CMCPair(cb, data));
    }
  }

  void CleanupManager::invoke_remove_callback(CleanupManagerCallback callback, void* data)
  {
    if (!initialized_) initialize();

    Guard g(lock_->get());
    callback(data);
    callbacks_->erase(std::remove(callbacks_->begin(), callbacks_->end(), CMCPair(callback, data)), callbacks_->end());
  }

  void CleanupManager::remove_callback(CleanupManagerCallback callback, void* data)
  {
    if (!initialized_) initialize();

    Guard g(lock_->get());
    callbacks_->erase(std::remove(callbacks_->begin(), callbacks_->end(), CMCPair(callback, data)), callbacks_->end());
  }

  void CleanupManager::call_callbacks()
  {
    if (!initialized_) initialize();

    Guard g(lock_->get());
    for (auto& callback : *callbacks_)
    {
      callback.first(callback.second);
    }
    callbacks_->clear();
  }
}
