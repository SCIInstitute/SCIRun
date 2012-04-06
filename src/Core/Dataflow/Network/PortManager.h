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


#ifndef CORE_DATAFLOW_NETWORK_PORTMANAGER_H
#define CORE_DATAFLOW_NETWORK_PORTMANAGER_H 

#include <Core/Dataflow/Network/Port.h>

#include <string>
#include <vector>
#include <map>

namespace SCIRun {
namespace Domain {
namespace Networks {

template<class T> 
class PortManager : boost::noncopyable
{
private:
  std::vector<T>   ports_;
  ModuleInterface*          module_;
  std::string      lastportname_;
  
public:
  PortManager();
  int size();
  void add(const T &item);
  void remove(int item);
  T operator[](int);
  T get_port(int);
  void set_module(ModuleInterface* mod) { module_ = mod; }
  void set_lastportname(const std::string& name) { lastportname_ = name; }
};

template<class T>
PortManager<T>::PortManager() :
  module_(0)
{
}

template<class T>
int
PortManager<T>::size()
{ 
  size_t s = ports_.size();
  return (static_cast<int>(s)); 
}

template<class T>
T
PortManager<T>::get_port(int item)
{
  T handle(0);
  if (item < static_cast<int>(ports_.size())) 
  {
    handle = ports_[item];
  }
  return(handle);
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
    throw "PortManager tried to remove a port that does not exist";
  }
  ports_.erase(ports_.begin() + item);
}

template<class T>
T
PortManager<T>::operator[](int item)
{
  T t = ports_[item];
  return (t);
}

}}}

#endif
