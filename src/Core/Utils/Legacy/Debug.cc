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


/// @todo Documentation Core/Utils/Legacy/Debug.cc

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

#include <Core/Utils/Legacy/Debug.h>
#include <Core/Thread/Legacy/Mutex.h>
#include <map>
#include <iostream>

#if DEBUG

namespace SCIRun {

Mutex* scirun_debug_object_mutex = 0;
std::map<std::string,int>* scirun_debug_object_count_ = 0;

/// @todo
/// Convert this utility to a Debug class which will eliminate the need
/// for this type of initialization.

void
debug_ensure_mutex()
{
  if (scirun_debug_object_mutex == 0) scirun_debug_object_mutex = new Mutex("Mutex for object counting");
  if (scirun_debug_object_count_ == 0) scirun_debug_object_count_ = new std::map<std::string,int>();
}

// Count locking objects
void
debug_incr_object_count(const std::string& type)
{
  debug_ensure_mutex();
  scirun_debug_object_mutex->lock();

  std::map<std::string,int>::iterator it = scirun_debug_object_count_->find(type);
  if (it == scirun_debug_object_count_->end()) (*scirun_debug_object_count_)[type] = 0;
  (*scirun_debug_object_count_)[type]++;

  scirun_debug_object_mutex->unlock();
}

void
debug_decr_object_count(const std::string& type)
{
  debug_ensure_mutex();
  scirun_debug_object_mutex->lock();
  (*scirun_debug_object_count_)[type]--;
  if ((*scirun_debug_object_count_)[type] == 0) {
    scirun_debug_object_count_->erase(type);
  }
  scirun_debug_object_mutex->unlock();
}

void
debug_print_objects()
{
  debug_ensure_mutex();
  scirun_debug_object_mutex->lock();
  std::map<std::string,int>::iterator it = scirun_debug_object_count_->begin();
  std::map<std::string,int>::iterator it_end = scirun_debug_object_count_->end();

  std::cerr << "======================================"<<std::endl;
  while(it != it_end)
  {
    if ((*it).second)
    {
      std::cerr << "["<< (*it).first << "]="<< (*it).second << std::endl;
    }
    ++it;
  }
  std::cerr << "======================================"<<std::endl;

  scirun_debug_object_mutex->unlock();
}

size_t debug_number_of_objects()
{
  debug_ensure_mutex();
  scirun_debug_object_mutex->lock();
  size_t cnt = scirun_debug_object_count_->size();
  scirun_debug_object_mutex->unlock();

  return (cnt);
}

void debug_tag_default_number_of_objects()
{
  debug_ensure_mutex();
  scirun_debug_object_mutex->lock();
  scirun_debug_object_count_->clear();
  scirun_debug_object_mutex->unlock();
}


} // End namespace SCIRun
#endif

#endif
