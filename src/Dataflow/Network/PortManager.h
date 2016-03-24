/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

/// @todo Documentation Dataflow/Network/PortManager.h


#ifndef DATAFLOW_NETWORK_PORTMANAGER_H
#define DATAFLOW_NETWORK_PORTMANAGER_H

#include <Dataflow/Network/Port.h>
#include <Core/Utils/Exception.h>

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <string>
#include <map>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

template<class T>
class PortManager : boost::noncopyable
{
private:
  typedef std::map<PortId, T> PortMap;
  typedef std::map<std::string, bool> DynamicMap;
  PortMap ports_;
  DynamicMap isDynamic_;
  ModuleInterface* module_;
  void checkDynamicPortInvariant(const std::string& name);
  void throwForPortNotFound(const PortId& id) const;
  std::vector<T> findAllByName(const std::string& name) const;
  int lastIndexByName(const std::string& name) const;
  std::vector<T> findAllByNameImpl(const std::string& name) const;

public:
  PortManager();
  size_t size() const;
  size_t add(const T& item);
  void remove(const PortId& id);
  T operator[](const PortId& id);
  T operator[](const PortId& id) const;
  std::vector<T> operator[](const std::string& name) const;
  bool hasPort(const PortId& id) const;
  void set_module(ModuleInterface* mod) { module_ = mod; }
  std::vector<T> view() const;
};

struct SCISHARE PortOutOfBoundsException : virtual Core::ExceptionBase {};

template<class T>
PortManager<T>::PortManager() :
  module_(nullptr)
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
PortManager<T>::add(const T& item)
{
  auto lastIndexWithSameName = lastIndexByName(item->id().name);

  ports_[item->id()] = item;
  isDynamic_[item->id().name] = item->isDynamic();

  if (item->isDynamic())
  {
    checkDynamicPortInvariant(item->id().name);

    if (lastIndexWithSameName >= 0)
    {
      const auto newPortIndex = lastIndexWithSameName + 1;
      //std::cout << "cloned port: " << item->id().toString() << " newIndex: " << newPortIndex << std::endl;

      for (auto& portPair : ports_)
      {
        //std::cout << "\t id " << portPair.second->id().toString() << " index before setting " << portPair.second->getIndex() << std::endl;
        if (portPair.second->getIndex() >= newPortIndex)
          portPair.second->incrementIndex();
      }

      //for (const auto& portPair : ports_)
      //{
      //  std::cout << "\t id " << portPair.second->id().toString() << " index after setting " << portPair.second->getIndex() << std::endl;
      //}

      return newPortIndex;
    }
  }
  //if (item->isDynamic())
  //  std::cout << "original port: " << item->id().toString() << " newIndex: " << size() - 1 << std::endl;
  return size() - 1;
}

template<class T>
int
PortManager<T>::lastIndexByName(const std::string& name) const
{
  auto matches = findAllByNameImpl(name);

  if (matches.empty())
    return -1;

  //std::cout << name << "  Input port object indexes:\n";
  //for (const auto& input : matches)
  //{
  //  std::cout << input->id() << " " << input->id().name << " " << input->getIndex() << std::endl;
  //}

  return static_cast<int>((*std::max_element(matches.begin(), matches.end(), [](const T& port1, const T& port2) { return port1->getIndex() < port2->getIndex(); }))->getIndex());
}

template<class T>
void
PortManager<T>::checkDynamicPortInvariant(const std::string& name)
{
  auto byName = findAllByName(name);
  const size_t lastIndex = byName.size() - 1;
  std::vector<PortId> toRemove;
  for (int i = 0; i < byName.size(); ++i)
  {
    auto port = byName[i];
    if (0 == port->nconnections() && i != lastIndex)
      toRemove.push_back(port->id());
  }
  for (const auto& id : toRemove)
    remove(id);
}

template<class T>
void
PortManager<T>::remove(const PortId& id)
{
  auto it = ports_.find(id);
  if (it == ports_.end())
  {
    std::ostringstream ostr;
    ostr << "PortManager tried to remove a port that does not exist: " << id;
    BOOST_THROW_EXCEPTION(PortOutOfBoundsException() << Core::ErrorMessage(ostr.str()));
  }
  auto removedIndex = it->second->getIndex();
  //std::cout << "~~~removing port " << id.toString() << " index " << removedIndex << std::endl;
  ports_.erase(it);
  for (auto& portPair : ports_)
  {
    if (portPair.second->getIndex() > removedIndex)
    {
      //std::cout << "\t resetting index " << portPair.second->id().toString() << " " << portPair.second->getIndex()-1 << std::endl;
      portPair.second->decrementIndex();
    }
  }
}

template<class T>
T
PortManager<T>::operator[](const PortId& id)
{
  auto it = ports_.find(id);
  if (it == ports_.end())
  {
    if (isDynamic_[id.name])
    {
      auto byName = findAllByName(id.name);
      if (byName.empty())
      {
        throwForPortNotFound(id);
      }
      auto newPort = boost::shared_ptr<typename T::element_type>(byName[0]->clone());
      newPort->setId(id);
      newPort->setIndex(add(newPort));
      return newPort;
    }
    else
      throwForPortNotFound(id);
  }
  return it->second;
}

template<class T>
void
PortManager<T>::throwForPortNotFound(const PortId& id) const
{
  std::ostringstream ostr;
  ostr << "PortManager tried to access a port that does not exist: " << id;
  BOOST_THROW_EXCEPTION(PortOutOfBoundsException() << Core::ErrorMessage(ostr.str()));
}

template<class T>
T
PortManager<T>::operator[](const PortId& id) const
{
  auto it = ports_.find(id);
  if (it == ports_.end())
  {
    throwForPortNotFound(id);
  }
  return it->second;
}

template<class T>
std::vector<T> PortManager<T>::operator[](const std::string& name) const
{
  return findAllByName(name);
}

template<class T>
std::vector<T> PortManager<T>::findAllByName(const std::string& name) const
{
  auto portsWithName = findAllByNameImpl(name);

  if (portsWithName.empty())
  {
    BOOST_THROW_EXCEPTION(PortOutOfBoundsException() << Core::ErrorMessage("PortManager does not contain a port by name: " + name));
  }

  return portsWithName;
}

template<class T>
std::vector<T> PortManager<T>::findAllByNameImpl(const std::string& name) const
{
  std::vector<T> portsWithName;

  boost::copy(
    ports_ | boost::adaptors::map_values
    | boost::adaptors::filtered([&](const T& port) { return port->get_portname() == name; }), std::back_inserter(portsWithName));

  return portsWithName;
}

template <class T>
std::vector<T> PortManager<T>::view() const
{
  std::vector<T> portVector;
  boost::copy(ports_ | boost::adaptors::map_values, std::back_inserter(portVector));
  std::sort(portVector.begin(), portVector.end(), [](const T& lhs, const T& rhs) { return lhs->getIndex() < rhs->getIndex(); });
  return portVector;
}

template <class T>
bool PortManager<T>::hasPort(const PortId& id) const
{
  return ports_.find(id) != ports_.end();
}

}}}

#endif
