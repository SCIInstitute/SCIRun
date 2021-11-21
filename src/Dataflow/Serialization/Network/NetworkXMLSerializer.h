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


/// @todo Documentation Dataflow/Serialization/Network/NetworkXMLSerializer.h

#ifndef CORE_SERIALIZATION_NETWORK_NETWORK_XML_SERIALIZER_H
#define CORE_SERIALIZATION_NETWORK_NETWORK_XML_SERIALIZER_H

#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Algorithms/Base/AlgorithmFwd.h>
#include <iosfwd>
#include <boost/noncopyable.hpp>
#include <Dataflow/Serialization/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  class SCISHARE NetworkXMLConverter : boost::noncopyable
  {
  public:
    NetworkXMLConverter(ModuleFactoryHandle moduleFactory, ModuleStateFactoryHandle stateFactory, Core::Algorithms::AlgorithmFactoryHandle algoFactory,
      ReexecuteStrategyFactoryHandle reexFactory,
      NetworkEditorControllerInterface* nec, NetworkEditorSerializationManager* nesm = nullptr);
    NetworkHandle from_xml_data(const NetworkXML& data);
    NetworkFileHandle to_xml_data(const NetworkHandle& network);

    struct NetworkAppendInfo
    {
      size_t newModuleStartIndex;
      std::map<std::string, std::string> moduleIdMapping;
    };

    NetworkAppendInfo appendXmlData(const NetworkXML& data);
  private:
    ModuleFactoryHandle moduleFactory_;
    ModuleStateFactoryHandle stateFactory_;
    Core::Algorithms::AlgorithmFactoryHandle algoFactory_;
    ReexecuteStrategyFactoryHandle reexFactory_;
    NetworkEditorControllerInterface* controller_;
    NetworkEditorSerializationManager* nesm_;
  };

  class SCISHARE NetworkToXML : boost::noncopyable
  {
  public:
    explicit NetworkToXML(NetworkEditorSerializationManager* nesm = nullptr);
    NetworkFileHandle to_xml_data(const NetworkHandle& network);
    NetworkFileHandle to_xml_data(const NetworkHandle& network, ModuleFilter modFilter, ConnectionFilter connFilter);
  private:
    NetworkEditorSerializationManager* nesm_;
  };

  class SCISHARE NetworkXMLSerializer : boost::noncopyable
  {
  public:
    void save_xml(const NetworkXML& data, const std::string& filename);
    void save_xml(const NetworkXML& data, std::ostream& ostr);
    NetworkXMLHandle load_xml(const std::string& filename);
    NetworkXMLHandle load_xml(std::istream& istr);
  };
}}}

#endif
