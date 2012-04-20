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
#include <Core/Dataflow/Network/PortInterface.h>

namespace SCIRun {
namespace Domain {
namespace Networks {
  
class Port : virtual public PortInterface, boost::noncopyable
{
public:
  struct ConstructionParams
  {
    std::string type_name, port_name, color_name;
    ConstructionParams(const std::string& p, const std::string& t, const std::string& c)
      : type_name(t), port_name(p), color_name(c) {}
  };
  Port(ModuleInterface* module, const ConstructionParams& params);
  virtual ~Port();

  size_t nconnections() const;
  const Connection* connection(size_t) const;

  std::string get_typename() const { return typeName_; }
  std::string get_colorname() const { return colorName_; }
  std::string get_portname() const {return portName_; }

  virtual void attach(Connection* conn);
  virtual void detach(Connection* conn);

  virtual void reset() {}
  virtual void finish() {}

protected:
  ModuleInterface* module_;
  std::vector<Connection*> connections_;

private:
  std::string typeName_;
  std::string portName_;
  std::string colorName_;
};

//TODO: discuss this interface design
#pragma warning (push)
#pragma warning (disable : 4250)

class InputPort : public Port, public InputPortInterface
{
public:
  InputPort(ModuleInterface* module, const ConstructionParams& params);
  virtual ~InputPort();
};


class OutputPort : public Port, public OutputPortInterface
{
public:
  OutputPort(ModuleInterface* module, const ConstructionParams& params);
  virtual ~OutputPort();
};

#pragma warning (pop)

}}}

#endif
