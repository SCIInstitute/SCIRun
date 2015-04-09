#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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



namespace SCIRun {

typedef std::pair<CleanupManagerCallback, void *> CMCPair;

std::vector<CMCPair>* CleanupManager::callbacks_ = NULL;
bool    CleanupManager::initialized_ = false;
Mutex * CleanupManager::lock_ = NULL;

void
CleanupManager::initialize()
{
  if( initialized_ )
    return;
  initialized_ = true;

  callbacks_ = new std::vector<CMCPair>;
  lock_ = new Mutex("CleanupManager lock");
}

void
CleanupManager::add_callback(CleanupManagerCallback cb, void *data)
{
  if( !initialized_ ) initialize();

  lock_->lock();
  if (std::find(callbacks_->begin(), callbacks_->end(), CMCPair(cb, data))
      == callbacks_->end())
    {
      callbacks_->push_back(CMCPair(cb, data));
    }
  lock_->unlock();
}

void
CleanupManager::invoke_remove_callback(CleanupManagerCallback callback,
				       void *data)
{
  if( !initialized_ ) initialize();

  lock_->lock();
  callback(data);
  callbacks_->erase(std::remove(callbacks_->begin(), callbacks_->end(),
                                CMCPair(callback, data)),
                    callbacks_->end());
  lock_->unlock();
}

void
CleanupManager::remove_callback(CleanupManagerCallback callback, void *data)
{
  if( !initialized_ ) initialize();

  lock_->lock();
  callbacks_->erase(std::remove(callbacks_->begin(), callbacks_->end(),
                                CMCPair(callback, data)),
                    callbacks_->end());
  lock_->unlock();
}

void
CleanupManager::call_callbacks()
{
  if( !initialized_ ) initialize();

  lock_->lock();
  for (unsigned int i = 0; i < callbacks_->size(); i++)
    {
      (*callbacks_)[i].first((*callbacks_)[i].second);
    }
  callbacks_->clear();
  lock_->unlock();

  // Should memory be cleaned up here?
//   initialized_ = false;
//   delete lock_;
//   delete callbacks_;
}


} // End namespace SCIRun

#endif
