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


#ifndef DATAFLOW_NETWORK_PORTMANAGER_H
#define DATAFLOW_NETWORK_PORTMANAGER_H 

#include <Dataflow/Network/Port.h>
#include <Core/Utils/Exception.h>

#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <deque>
#include <map>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

template<class T> 
class PortManager : boost::noncopyable
{
private:
  std::map<size_t, T> ports_;
  ModuleInterface* module_;
  size_t counter_;
  std::string lastportname_;
  
public:
  PortManager();
  size_t size() const;
  size_t add(const T& item);
  void remove(size_t item);
  T operator[](size_t) const;
  bool containsKey(size_t id) const;
  void set_module(ModuleInterface* mod) { module_ = mod; }
  void set_lastportname(const std::string& name) { lastportname_ = name; }
  //void apply(boost::function<void(T&)> func);
  void resetAll();
};

struct PortOutOfBoundsException : virtual Core::ExceptionBase {};

template<class T>
PortManager<T>::PortManager() :
  module_(0),
  counter_(0)
{
}

template<class T>
size_t
PortManager<T>::size() const
{ 
  return ports_.size();
}

template<class T>
size_t
PortManager<T>::add(const T &item)
{ 
  size_t index = counter_++;
  ports_[index] = item;
  return index;
}

template<class T>
void
PortManager<T>::remove(size_t item)
{
  auto it = ports_.find(item);
  if (it == ports_.end())
  {
    BOOST_THROW_EXCEPTION(PortOutOfBoundsException() << Core::ErrorMessage("PortManager tried to remove a port that does not exist"));
  }
  ports_.erase(it);
//  for (size_t i = 0; i < ports_.size(); ++i)
    //ports_[i]->setIndex(i);
}

template<class T>
T
PortManager<T>::operator[](size_t item) const
{
  auto it = ports_.find(item);
  if (it == ports_.end())
  {
    BOOST_THROW_EXCEPTION(PortOutOfBoundsException() << Core::ErrorMessage("PortManager tried to remove a port that does not exist"));
  }
  return it->second;
}

//template<class T>
//void 
//PortManager<T>::apply(boost::function<void(T&)> func)
//{
//  BOOST_FOREACH(T& port, ports_)
//    func(port);
//}

template <class T>
bool PortManager<T>::containsKey(size_t id)
{
  return ports_.find(id) != ports_.end();
}

template<class T>
void 
PortManager<T>::resetAll()
{
  //apply(boost::bind(&T::reset, boost::lambda::_1));
}

}}}

#endif
