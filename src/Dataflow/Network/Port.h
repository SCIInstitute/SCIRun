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

/// @todo Documentation Dataflow/Network/Port.h


#ifndef DATAFLOW_NETWORK_PORT_H
#define DATAFLOW_NETWORK_PORT_H 

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/signals2/signal.hpp>
#include <Dataflow/Network/PortInterface.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {
  
class SCISHARE Port : virtual public PortInterface, boost::noncopyable
{
public:
  struct ConstructionParams
  {
    PortId id_;
    std::string type_name, port_name;
    bool isDynamic_;
    ConstructionParams(const PortId& id, const std::string& type, bool isDynamic)
      : id_(id), type_name(type), port_name(id.name), isDynamic_(isDynamic) {}
  };
  Port(ModuleInterface* module, const ConstructionParams& params);
  virtual ~Port();

  size_t nconnections() const;
  const Connection* connection(size_t) const;

  virtual PortId id() const { return id_; }
  std::string get_typename() const { return typeName_; }
  std::string get_colorname() const { return colorName_; }
  std::string get_portname() const { return portName_; }

  virtual void attach(Connection* conn);
  virtual void detach(Connection* conn);

  virtual ModuleId getUnderlyingModuleId() const;
  virtual size_t getIndex() const;
  virtual void setIndex(size_t index);

  /// @todo:
  // light interface

protected:
  ModuleInterface* module_;
  std::vector<Connection*> connections_;
  size_t index_;
  PortId id_;

  const std::string typeName_;
  const std::string portName_;
  const std::string colorName_;
};

#ifdef WIN32
#pragma warning (push)
#pragma warning (disable : 4250)
#endif

/// @todo: discuss this interface design
class SCISHARE InputPort : public Port, public InputPortInterface
{
public:
  InputPort(ModuleInterface* module, const ConstructionParams& params, DatatypeSinkInterfaceHandle sink);
  virtual ~InputPort();
  virtual void attach(Connection* conn) override;
  virtual void detach(Connection* conn) override;
  virtual DatatypeSinkInterfaceHandle sink() const;
  virtual Core::Datatypes::DatatypeHandleOption getData() const;
  virtual bool isInput() const { return true; } //boo
  virtual bool isDynamic() const { return isDynamic_; }
  virtual InputPortInterface* clone() const;
  virtual bool hasChanged() const;
  virtual boost::signals2::connection connectDataOnPortHasChanged(const DataOnPortHasChangedSignalType::slot_type& subscriber);
private:
  DatatypeSinkInterfaceHandle sink_;
  bool isDynamic_;
};


class SCISHARE OutputPort : public Port, public OutputPortInterface
{
public:
  OutputPort(ModuleInterface* module, const ConstructionParams& params, DatatypeSourceInterfaceHandle source);
  virtual ~OutputPort();
  virtual void sendData(Core::Datatypes::DatatypeHandle data);
  virtual bool isInput() const { return false; } //boo
  virtual bool isDynamic() const { return false; } /// @todo: design dynamic output ports
  virtual bool hasData() const override;
  virtual void attach(Connection* conn) override;
private:
  DatatypeSourceInterfaceHandle source_;
};

#ifdef WIN32
#pragma warning (pop)
#endif

}}}

#endif
