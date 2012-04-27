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

#ifndef MOCK_PORTS_H
#define MOCK_PORTS_H

#include <Core/Dataflow/Network/PortInterface.h>
#include <Core/Dataflow/Network/DataflowInterfaces.h>
#include <gmock/gmock.h>

namespace SCIRun {
  namespace Domain {
    namespace Networks {
      namespace Mocks
      {
        class MockInputPort : public InputPortInterface  
        {
        public:
          MOCK_METHOD1(attach, void(Connection*));
          MOCK_METHOD1(detach, void(Connection*));
          MOCK_CONST_METHOD0(nconnections, size_t());
          MOCK_CONST_METHOD1(connection, const Connection*(size_t));
          MOCK_CONST_METHOD0(get_colorname, std::string());
          MOCK_CONST_METHOD0(get_portname, std::string());
          MOCK_METHOD0(reset, void());
          MOCK_METHOD0(finish, void());
          MOCK_METHOD0(getData, Datatypes::DatatypeHandleOption());
        };

        typedef boost::shared_ptr<MockInputPort> MockInputPortPtr;

        class MockOutputPort : public OutputPortInterface  
        {
        public:
          MOCK_METHOD1(attach, void(Connection*));
          MOCK_METHOD1(detach, void(Connection*));
          MOCK_CONST_METHOD0(nconnections, size_t());
          MOCK_CONST_METHOD1(connection, const Connection*(size_t));
          MOCK_CONST_METHOD0(get_colorname, std::string());
          MOCK_CONST_METHOD0(get_portname, std::string());
          MOCK_METHOD0(reset, void());
          MOCK_METHOD0(finish, void());
          MOCK_METHOD1(sendData, void(Datatypes::DatatypeHandle));
        };

        typedef boost::shared_ptr<MockOutputPort> MockOutputPortPtr;

        class MockDatatypeSink : public DatatypeSinkInterface
        {
        public:
          MOCK_METHOD0(waitForData, void());
          MOCK_METHOD0(receive, Datatypes::DatatypeHandleOption());
        };

        typedef boost::shared_ptr<MockDatatypeSink> MockDatatypeSinkPtr;

        class MockDatatypeSource : public DatatypeSourceInterface
        {
        public:
          MOCK_METHOD1(send, void(Datatypes::DatatypeHandle));
        };

        typedef boost::shared_ptr<MockDatatypeSource> MockDatatypeSourcePtr;
      }
    }
  }
}

#endif
