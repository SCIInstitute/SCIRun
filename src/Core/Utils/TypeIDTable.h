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


/// @todo Documentation Core/Utils/TypeIDTable.h

#ifndef CORE_UTILS_TYPEIDTABLE_H
#define CORE_UTILS_TYPEIDTABLE_H

#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <Core/Utils/share.h>

namespace SCIRun
{
namespace Core
{
namespace Utility
{
  /// A thread-safe map used for constructor lookup in Mesh/Field factories.

  template <class CtorInfo>
  class TypeIDTable : boost::noncopyable
  {
  public:
    typedef boost::optional<const CtorInfo&> CtorInfoOption;

    //do locking internally
    CtorInfoOption findConstructorInfo(const std::string& key) const
    {
      boost::mutex::scoped_lock s(lock_);
      auto iter = lookup_.find(key);
      if (iter == lookup_.end())
        return CtorInfoOption();
      return CtorInfoOption(iter->second);
    }

    bool registerConstructorInfo(const std::string& key, const CtorInfo& info)
    {
      boost::mutex::scoped_lock s(lock_);
      auto iter = lookup_.find(key);
      if (iter != lookup_.end())
      {
        if (iter->second != info)
        {
          /// @todo: improve for testing
          /// @todo: use real logger here
          //TODO inject new logger std::cerr << "WARNING: duplicate type exists: " << key << "\n";
          return false;
        }
      }
      lookup_[key] = info;
      return true;
    }

    size_t size() const { return lookup_.size(); }

  private:
    mutable boost::mutex lock_;
    std::map<std::string, CtorInfo> lookup_;
  };

}}}

#endif
