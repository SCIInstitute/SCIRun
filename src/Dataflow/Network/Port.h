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

  size_t nconnections() const override;
  Connection* connection(size_t) const override;
  bool hasConnectionCountIncreased() const override;

  virtual PortId id() const override { return id_; }
  virtual void setId(const PortId& id) override { id_ = id; }
  std::string get_typename() const override { return typeName_; }
  std::string get_colorname() const { return colorName_; }
  std::string get_portname() const override { return portName_; }

  virtual void attach(Connection* conn) override;
  virtual void detach(Connection* conn) override;

  virtual ModuleId getUnderlyingModuleId() const override;
  virtual size_t getIndex() const override;
  virtual void setIndex(size_t index) override;

  virtual boost::optional<ConnectionId> firstConnectionId() const override;
  virtual ModuleStateHandle moduleState() const override;

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
  mutable bool connectionCountIncreasedFlag_;
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
  virtual DatatypeSinkInterfaceHandle sink() const override;
  virtual Core::Datatypes::DatatypeHandleOption getData() const override;
  virtual bool isInput() const override { return true; } //boo
  virtual bool isDynamic() const override { return isDynamic_; }
  virtual InputPortInterface* clone() const override;
  virtual bool hasChanged() const override;
  virtual boost::signals2::connection connectDataOnPortHasChanged(const DataOnPortHasChangedSignalType::slot_type& subscriber) override;
  virtual void resendNewDataSignal() override;
  virtual boost::optional<std::string> connectedModuleId() const override;
  virtual ModuleStateHandle stateFromConnectedModule() const override;
private:
  bool shouldTriggerDataChange() const;
  DatatypeSinkInterfaceHandle sink_;
  bool isDynamic_;
};


class SCISHARE OutputPort : public Port, public OutputPortInterface
{
public:
  OutputPort(ModuleInterface* module, const ConstructionParams& params, DatatypeSourceInterfaceHandle source);
  virtual ~OutputPort();
  virtual DatatypeSourceInterfaceHandle source() const override { return source_; }
  virtual void sendData(Core::Datatypes::DatatypeHandle data) override;
  virtual bool isInput() const override { return false; } //boo
  virtual bool isDynamic() const override { return false; } /// @todo: design dynamic output ports
  virtual bool hasData() const override;
  virtual void attach(Connection* conn) override;
  virtual PortDataDescriber getPortDataDescriber() const override;
  virtual boost::signals2::connection connectConnectionFeedbackListener(const ConnectionFeedbackSignalType::slot_type& subscriber) override;
  virtual void sendConnectionFeedback(const Core::Datatypes::ModuleFeedback& info) override;
private:
  DatatypeSourceInterfaceHandle source_;
  ConnectionFeedbackSignalType cxnFeedback_;
};

#ifdef WIN32
#pragma warning (pop)
#endif

}}}

#endif
