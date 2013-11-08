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

#include <boost/bimap.hpp>
#include <string>
#include <map>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

template<class T> 
class PortManager : boost::noncopyable
{
private:
  //boost::bimap<T, size_t> portIndexes_;
  std::multimap<PortId, T> ports_;
  ModuleInterface* module_;
  
public:
  PortManager();
  size_t size() const;
  size_t add(const T& item);
  void remove(size_t item);
  T operator[](size_t) const;
  bool hasPort(const PortId& id) const;
  void set_module(ModuleInterface* mod) { module_ = mod; }
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
size_t
PortManager<T>::add(const T &item)
{ 
  ports_[item->id()] = item;
  auto index = size() - 1;
  portIndexes_.insert(std::make_pair(item, index))
  return index;
}

template<class T>
void
PortManager<T>::remove(size_t item)
{
  auto it = portIndexes_.right.find(item);
  if (it == portIndexes_.end())
  {
    BOOST_THROW_EXCEPTION(PortOutOfBoundsException() << Core::ErrorMessage("PortManager tried to remove a port that does not exist"));
  }
  auto port = it->second;
  ports_.erase(port->id());
  portIndexes_.erase(it);
}

template<class T>
T
PortManager<T>::operator[](size_t item) const
{
  auto it = portIndexes_.find(item);
  if (it == ports_.end())
  {
    BOOST_THROW_EXCEPTION(PortOutOfBoundsException() << Core::ErrorMessage("PortManager tried to remove a port that does not exist"));
  }
  return it->second;
}

template <class T>
bool PortManager<T>::hasPort(const PortId& id) const
{
  return ports_.find(id) != ports_.end();
}

}}}

#endif
