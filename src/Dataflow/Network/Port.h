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

  PortId internalId() const override { return internalId_; }
  void setId_DynamicCase(const PortId& id) override { internalId_ = externalId_ = id; }
  void setInternalId(const PortId& id) override { internalId_ = id; }
  PortId externalId() const override { return externalId_; }
  std::string get_typename() const override { return typeName_; }
  std::string get_colorname() const { return colorName_; }
  std::string get_portname() const override { return portName_; }

  void attach(Connection* conn) override;
  void detach(Connection* conn) override;

  ModuleId getUnderlyingModuleId() const override;
  size_t getIndex() const override;
  void setIndex(size_t index) override;

  std::optional<ConnectionId> firstConnectionId() const override;
  ModuleStateHandle moduleState() const override;
  ModuleInterface* underlyingModule() const override { return module_; }

  /// @todo:
  // light interface

protected:
  ModuleInterface* module_;
  std::vector<Connection*> connections_;
  size_t index_;
  PortId internalId_;
  PortId externalId_;

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
  void attach(Connection* conn) override;
  void detach(Connection* conn) override;
  DatatypeSinkInterfaceHandle sink() const override;
  Core::Datatypes::DatatypeHandleOption getData() const override;
  bool isInput() const override { return true; } //boo
  bool isDynamic() const override { return isDynamic_; }
  InputPortInterface* clone() const override;
  bool hasChanged() const override;
  boost::signals2::connection connectDataOnPortHasChanged(const DataOnPortHasChangedSignalType::slot_type& subscriber) override;
  void resendNewDataSignal() override;
  std::optional<std::string> connectedModuleId() const override;
  ModuleStateHandle stateFromConnectedModule() const override;
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
  DatatypeSourceInterfaceHandle source() const override { return source_; }
  void sendData(Core::Datatypes::DatatypeHandle data) override;
  bool isInput() const override { return false; } //boo
  bool isDynamic() const override { return false; } /// @todo: design dynamic output ports
  bool hasData() const override;
  Core::Datatypes::DatatypeHandle peekData() const override;
  void attach(Connection* conn) override;
  PortDataDescriber getPortDataDescriber() const override;
  boost::signals2::connection connectConnectionFeedbackListener(const ConnectionFeedbackSignalType::slot_type& subscriber) override;
  void sendConnectionFeedback(const Core::Datatypes::ModuleFeedback& info) override;
private:
  DatatypeSourceInterfaceHandle source_;
  ConnectionFeedbackSignalType cxnFeedback_;
};

#ifdef WIN32
#pragma warning (pop)
#endif

}}}

#endif
