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
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <vector>
#include <map>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

template<class T> 
class PortManager : boost::noncopyable
{
private:
  std::vector<T> ports_;
  ModuleInterface* module_;
  std::string lastportname_;
  
public:
  PortManager();
  size_t size() const;
  void add(const T& item);
  void remove(int item);
  T operator[](size_t) const;
  void set_module(ModuleInterface* mod) { module_ = mod; }
  void set_lastportname(const std::string& name) { lastportname_ = name; }
  void apply(boost::function<void(T&)> func);
  void resetAll();
};

struct PortOutOfBoundsException : virtual Core::ExceptionBase {};

template<class T>
PortManager<T>::PortManager() :
  module_(0)
{
}

template<class T>
size_t
PortManager<T>::size() const
{ 
  return ports_.size();
}

template<class T>
void
PortManager<T>::add(const T &item)
{ 
  ports_.push_back(item);
}

template<class T>
void
PortManager<T>::remove(int item)
{
  if (static_cast<int>(ports_.size()) <= item)
  {
    BOOST_THROW_EXCEPTION(PortOutOfBoundsException() << Core::ErrorMessage("PortManager tried to remove a port that does not exist"));
  }
  ports_.erase(ports_.begin() + item);
}

template<class T>
T
PortManager<T>::operator[](size_t item) const
{
  return ports_[item];
}

template<class T>
void 
PortManager<T>::apply(boost::function<void(T&)> func)
{
  BOOST_FOREACH(T& port, ports_)
    func(port);
}

template<class T>
void 
PortManager<T>::resetAll()
{
  //apply(boost::bind(&T::reset, boost::lambda::_1));
}

}}}

#endif
