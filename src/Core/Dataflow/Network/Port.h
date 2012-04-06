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


#ifndef CORE_DATAFLOW_NETWORK_PORT_H
#define CORE_DATAFLOW_NETWORK_PORT_H 

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <Core/Dataflow/Network/NetworkFwd.h>

namespace SCIRun {
namespace Domain {
namespace Networks {

  class PortInterface
  {
  public:
    virtual ~PortInterface() {}
    virtual void attach(Connection* conn) = 0;
    virtual void detach(Connection* conn/*, bool blocked*/) = 0;
  };
  
  class InputPortInterface : public PortInterface
  {
  };
  
  class OutputPortInterface : public PortInterface
  {
  };

class Port : public PortInterface, boost::noncopyable
{
public:
  Port(ModuleInterface* module, const std::string& type_name, const std::string& port_name, const std::string& color_name);
  virtual ~Port();

  int nconnections();
  ConnectionHandle connection(int);

  std::string get_typename() const;
  std::string get_colorname() const;
  std::string get_portname() const;

  virtual void attach(Connection* conn);
  virtual void detach(Connection* conn/*, bool blocked*/);

protected:
  ModuleInterface* module;
  std::vector<ConnectionHandle> connections;

private:
  std::string type_name;
  std::string port_name;
  std::string color_name;
};


class InputPort : public Port, public InputPortInterface
{
public:
  InputPort(ModuleInterface* module, const std::string& type_name, const std::string& port_name, const std::string& color_name);
  virtual ~InputPort();
  
private:
  InputPort(const InputPort&);
  InputPort& operator=(const InputPort&);

  virtual void update_light();
};


class OutputPort : public Port, public OutputPortInterface
{
public:
  OutputPort(ModuleInterface* module, const std::string& type_name, const std::string& port_name, const std::string& color_name);
  virtual ~OutputPort();
};

}}}

#endif
