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
///@file  ThreadGroup.cc
///@brief A set of threads
///
///@author
///       Steve Parker
///       Department of Computer Science
///       University of Utah
///@date  June 1997
///

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

#include <Core/Thread/Legacy/ThreadGroup.h>
#include <Core/Thread/Legacy/Thread.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define for if(0);else for
#endif

namespace SCIRun {

ThreadGroup* ThreadGroup::s_default_group;

ThreadGroup::ThreadGroup(const char* name, ThreadGroup* parentGroup)
    : lock_("ThreadGroup lock"), name_(name), parent_(parentGroup)
{
    if(parentGroup==0){
        parent_=s_default_group;
        if(parent_) // It could still be null if we are making the first one
	    parent_->addme(this);
    }
}

ThreadGroup::~ThreadGroup()
{
}

int
ThreadGroup::numActive(bool countDaemon)
{
    lock_.lock();
    int total=0;
    if(countDaemon){
        total=(int)threads_.size();
    } else {
	for(std::vector<Thread*>::iterator iter=threads_.begin();
	    iter != threads_.end();iter++)
	    if((*iter)->isDaemon())
		total++;
    }
    for(std::vector<ThreadGroup*>::iterator iter=groups_.begin();
	iter != groups_.end();iter++)
        total+=(*iter)->numActive(countDaemon);
    lock_.unlock();
    return total;
}

void
ThreadGroup::stop()
{
    lock_.lock();
    for(std::vector<ThreadGroup*>::iterator iter=groups_.begin();
	iter != groups_.end();iter++)
        (*iter)->stop();
    for(std::vector<Thread*>::iterator iter=threads_.begin();
	iter != threads_.end();iter++)
        (*iter)->stop();
    lock_.unlock();
}

void
ThreadGroup::resume()
{
    lock_.lock();
    for(std::vector<ThreadGroup*>::iterator iter=groups_.begin();
	iter != groups_.end();iter++)
        (*iter)->resume();
    for(std::vector<Thread*>::iterator iter=threads_.begin();
	iter != threads_.end();iter++)
        (*iter)->resume();
    lock_.unlock();
}

void
ThreadGroup::join()
{
    lock_.lock();
    for(std::vector<ThreadGroup*>::iterator iter=groups_.begin();
	iter != groups_.end();iter++)
        (*iter)->join();
    for(std::vector<Thread*>::iterator iter=threads_.begin();
	iter != threads_.end();iter++)
        (*iter)->join();
    lock_.unlock();
}

void
ThreadGroup::detach()
{
    lock_.lock();
    for(std::vector<ThreadGroup*>::iterator iter=groups_.begin();
	iter != groups_.end();iter++)
        (*iter)->detach();
    for(std::vector<Thread*>::iterator iter=threads_.begin();
	iter != threads_.end();iter++)
        (*iter)->detach();
    lock_.unlock();
}

ThreadGroup*
ThreadGroup::parentGroup()
{
    return parent_;
}

void
ThreadGroup::addme(ThreadGroup* t)
{
    lock_.lock();
    groups_.push_back(t);
    lock_.unlock();
}

void
ThreadGroup::addme(Thread* t)
{
    lock_.lock();
    threads_.push_back(t);
    lock_.unlock();
}


} // End namespace SCIRun

#endif
