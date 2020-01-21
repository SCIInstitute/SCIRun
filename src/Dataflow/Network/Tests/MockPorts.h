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


#ifndef MOCK_PORTS_H
#define MOCK_PORTS_H

#include <Dataflow/Network/PortInterface.h>
#include <Dataflow/Network/DataflowInterfaces.h>
#include <Dataflow/Network/ConnectionId.h>
#include <boost/optional/optional_io.hpp>
#include <gmock/gmock.h>

namespace SCIRun {
  namespace Dataflow {
    namespace Networks {
      namespace Mocks {

        class MockPortDescription : public PortDescriptionInterface
        {
        public:
          MOCK_CONST_METHOD0(nconnections, size_t());
          MOCK_CONST_METHOD0(get_typename, std::string());
          MOCK_CONST_METHOD0(get_portname, std::string());
          MOCK_CONST_METHOD0(isInput, bool());
          MOCK_CONST_METHOD0(isDynamic, bool());
          MOCK_CONST_METHOD0(getUnderlyingModuleId, ModuleId());
          MOCK_CONST_METHOD0(getIndex, size_t());
          MOCK_CONST_METHOD0(id, PortId());
          MOCK_CONST_METHOD0(firstConnectionId, boost::optional<ConnectionId>());
        };

        typedef boost::shared_ptr<MockPortDescription> MockPortDescriptionPtr;

        class MockInputPort : public InputPortInterface
        {
        public:
          MOCK_METHOD1(attach, void(Connection*));
          MOCK_METHOD1(detach, void(Connection*));
          MOCK_CONST_METHOD0(nconnections, size_t());
          MOCK_CONST_METHOD1(connection, Connection*(size_t));
          MOCK_CONST_METHOD0(get_typename, std::string());
          MOCK_CONST_METHOD0(get_portname, std::string());
          MOCK_CONST_METHOD0(getData, Core::Datatypes::DatatypeHandleOption());
          MOCK_CONST_METHOD0(sink, DatatypeSinkInterfaceHandle());
          MOCK_CONST_METHOD0(isInput, bool());
          MOCK_CONST_METHOD0(isDynamic, bool());
          MOCK_CONST_METHOD0(getUnderlyingModuleId, ModuleId());
          MOCK_CONST_METHOD0(getIndex, size_t());
          MOCK_CONST_METHOD0(clone, InputPortInterface*());
          MOCK_CONST_METHOD0(id, PortId());
          MOCK_METHOD1(setId, void(const PortId&));
          MOCK_CONST_METHOD0(hasChanged, bool());
          MOCK_METHOD1(setIndex, void(size_t));
          MOCK_METHOD1(connectDataOnPortHasChanged, boost::signals2::connection(const DataOnPortHasChangedSignalType::slot_type&));
          MOCK_CONST_METHOD0(firstConnectionId, boost::optional<ConnectionId>());
          MOCK_METHOD0(resendNewDataSignal, void());
          MOCK_CONST_METHOD0(moduleState, ModuleStateHandle());
          MOCK_CONST_METHOD0(connectedModuleId, boost::optional<std::string>());
          MOCK_CONST_METHOD0(stateFromConnectedModule, ModuleStateHandle());
          MOCK_CONST_METHOD0(hasConnectionCountIncreased, bool());
        };

        typedef boost::shared_ptr<MockInputPort> MockInputPortPtr;

        class MockOutputPort : public OutputPortInterface
        {
        public:
          MOCK_METHOD1(attach, void(Connection*));
          MOCK_METHOD1(detach, void(Connection*));
          MOCK_CONST_METHOD0(nconnections, size_t());
          MOCK_CONST_METHOD1(connection, Connection*(size_t));
          MOCK_CONST_METHOD0(get_typename, std::string());
          MOCK_CONST_METHOD0(get_portname, std::string());
          MOCK_METHOD1(sendData, void(Core::Datatypes::DatatypeHandle));
          MOCK_CONST_METHOD0(isInput, bool());
          MOCK_CONST_METHOD0(isDynamic, bool());
          MOCK_CONST_METHOD0(getUnderlyingModuleId, ModuleId());
          MOCK_CONST_METHOD0(getIndex, size_t());
          MOCK_CONST_METHOD0(id, PortId());
          MOCK_METHOD1(setId, void(const PortId&));
          MOCK_METHOD1(setIndex, void(size_t));
          MOCK_CONST_METHOD0(hasData, bool());
          MOCK_CONST_METHOD0(source, DatatypeSourceInterfaceHandle());
          MOCK_CONST_METHOD0(getPortDataDescriber, PortDataDescriber());
          MOCK_METHOD1(connectConnectionFeedbackListener, boost::signals2::connection(const ConnectionFeedbackSignalType::slot_type&));
          MOCK_METHOD1(sendConnectionFeedback, void(const Core::Datatypes::ModuleFeedback&));
          MOCK_CONST_METHOD0(firstConnectionId, boost::optional<ConnectionId>());
          MOCK_CONST_METHOD0(moduleState, ModuleStateHandle());
          MOCK_CONST_METHOD0(hasConnectionCountIncreased, bool());
        };

        typedef boost::shared_ptr<MockOutputPort> MockOutputPortPtr;

        class MockDatatypeSink : public DatatypeSinkInterface
        {
        public:
          MOCK_CONST_METHOD0(clone, DatatypeSinkInterface*());
          MOCK_METHOD0(waitForData, void());
          MOCK_METHOD0(invalidateProvider, void());
          MOCK_METHOD0(receive, Core::Datatypes::DatatypeHandleOption());
          MOCK_CONST_METHOD0(hasChanged, bool());
          MOCK_METHOD1(connectDataHasChanged, boost::signals2::connection(const DataHasChangedSignalType::slot_type&));
          MOCK_METHOD0(forceFireDataHasChanged, void());
        };

        typedef boost::shared_ptr<MockDatatypeSink> MockDatatypeSinkPtr;

        class MockDatatypeSource : public DatatypeSourceInterface
        {
        public:
          MOCK_METHOD1(cacheData, void(Core::Datatypes::DatatypeHandle));
          MOCK_CONST_METHOD1(send, void(DatatypeSinkInterfaceHandle));
          MOCK_CONST_METHOD0(hasData, bool());
          MOCK_CONST_METHOD0(describeData, std::string());
        };

        typedef boost::shared_ptr<MockDatatypeSource> MockDatatypeSourcePtr;
      }
    }
  }
}

#endif
