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


/// @todo Documentation Core/Utils/StringUtil.h

#ifndef CORE_UTILS_STRINGUTIL_H
#define CORE_UTILS_STRINGUTIL_H 1

#include <sstream>
#include <istream>
#include <fstream>
#include <vector>
#include <iterator>
#include <type_traits>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>
#ifndef Q_MOC_RUN
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>
#endif
#include <Core/Utils/share.h>

namespace SCIRun
{
namespace Core
{

template <typename T>
std::vector<T*> toVectorOfRawPointers(const std::vector<boost::shared_ptr<T>>& vec)
{
  std::vector<T*> raws;
  raws.reserve(vec.size());
  std::transform(vec.begin(), vec.end(), std::back_inserter(raws), [](boost::shared_ptr<T> ptr) { return ptr.get(); });
  return raws;
}

template <typename T>
std::string to_string(const T& t)
{
  std::ostringstream o;
  o << t;
  return o.str();
}

template <typename T>
std::vector<T> parseLineOfNumbers(const std::string& line)
{
  std::istringstream stream(line);
  std::vector<T> numbers((std::istream_iterator<T>(stream)), (std::istream_iterator<T>()));

  return numbers;
}

template <class T, class Iter>
std::vector<boost::shared_ptr<T>> downcast_range(Iter begin, Iter end)
{
  std::vector<boost::shared_ptr<T>> output;
  std::transform(begin, end, std::back_inserter(output), [](const typename Iter::value_type& p) { return boost::dynamic_pointer_cast<T>(p); });
  return output;
}

template <class T, class Cont>
std::vector<boost::shared_ptr<T>> downcast_range(const Cont& container)
{
  return downcast_range<T>(container.begin(), container.end());
}

template <class T, class Iter>
std::vector<boost::shared_ptr<T>> upcast_range(Iter begin, Iter end)
{
  //BOOST_STATIC_ASSERT(boost::is_base_of<T, typename Iter::value_type>::value);
  std::vector<boost::shared_ptr<T>> output;
  std::transform(begin, end, std::back_inserter(output), [](const typename Iter::value_type& p) { return boost::static_pointer_cast<T>(p); });
  return output;
}

template <class T, class Cont>
std::vector<boost::shared_ptr<T>> upcast_range(const Cont& container)
{
  return upcast_range<T>(container.begin(), container.end());
}

struct SCISHARE AtomicCounter
{
  int operator()() const;
private:
  static boost::atomic<int> counter_;
};

SCISHARE bool replaceSubstring(std::string& str, const std::string& from, const std::string& to);

template <typename... T>
auto zip(const T&... containers) -> boost::iterator_range<boost::zip_iterator<decltype(boost::make_tuple(std::begin(containers)...))>>
{
  auto zip_begin = boost::make_zip_iterator(boost::make_tuple(std::begin(containers)...));
  auto zip_end = boost::make_zip_iterator(boost::make_tuple(std::end(containers)...));
  return boost::make_iterator_range(zip_begin, zip_end);
}

//template<typename T, typename = void>
//struct has_id : std::false_type { };
//
//template<typename T>
//struct has_id<T, decltype(std::declval<T>().id, void())> : std::true_type{};

#define DEFINE_MEMBER_CHECKER(member) \
    template<typename T, typename V = bool> \
    struct has_ ## member : std::false_type { }; \
    template<typename T> \
    struct has_ ## member<T, \
        typename std::enable_if< \
            !std::is_same<decltype(std::declval<T>().member), void>::value, \
            bool \
                    >::type \
                > : std::true_type { };

#define HAS_MEMBER(C, member) \
    has_ ## member<C>::value

}}


template <typename T>
class TypeDeterminer;

namespace std
{
  template <class T>
  std::ostream& operator<<(std::ostream& o, const std::vector<T>& vec)
  {
    o << "[";
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(o, " "));
    o << "]";
    return o;
  }
}

#endif
