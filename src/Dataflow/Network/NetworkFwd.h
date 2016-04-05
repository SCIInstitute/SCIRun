/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

/// @todo Documentation Dataflow/Network/NetworkFwd.h


#ifndef DATAFLOW_NETWORK_NETWORK_FWD_H
#define DATAFLOW_NETWORK_NETWORK_FWD_H

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/any.hpp>
#include <map>


namespace SCIRun {
namespace Dataflow {
namespace Networks {

class NetworkInterface;
class ModuleInterface;
class ModuleDisplayInterface;
class ModuleStateInterface;
class ModuleStateInterfaceFactory;
class PortInterface;
struct PortId;
class PortDescriptionInterface;
class Connection;
class InputPortInterface;
class OutputPortInterface;
struct ConnectionId;
struct ModuleId;
struct ModuleDescription;
struct ModuleLookupInfo;
class ModuleFactory;
class ModuleInfoProvider;
struct ConnectionDescription;
class DatatypeSourceInterface;
class DatatypeSinkInterface;
class ExecutableLookup;
class ExecutableObject;
class NetworkXML;
struct ModulePositions;
struct ModuleNotes;
struct ConnectionNotes;
struct ModuleTags;
/// @todo: rename this
struct NetworkFile;
class NetworkGlobalSettings;
class NetworkEditorSerializationManager;
class ConnectionMakerService;
class NetworkEditorControllerInterface;
class ReexecuteStrategyFactory;
class MetadataMap;

typedef boost::shared_ptr<NetworkInterface> NetworkHandle;
typedef boost::shared_ptr<ModuleInterface> ModuleHandle;
typedef boost::shared_ptr<ModuleStateInterface> ModuleStateHandle;
typedef boost::shared_ptr<ModuleStateInterfaceFactory> ModuleStateFactoryHandle;
typedef boost::shared_ptr<ReexecuteStrategyFactory> ReexecuteStrategyFactoryHandle;
typedef boost::shared_ptr<PortInterface> PortHandle;
typedef boost::shared_ptr<Connection> ConnectionHandle;
typedef boost::shared_ptr<InputPortInterface> InputPortHandle;
typedef boost::shared_ptr<OutputPortInterface> OutputPortHandle;
typedef boost::shared_ptr<ModuleFactory> ModuleFactoryHandle;
typedef boost::shared_ptr<DatatypeSinkInterface> DatatypeSinkInterfaceHandle;
typedef boost::shared_ptr<DatatypeSourceInterface> DatatypeSourceInterfaceHandle;
typedef boost::shared_ptr<NetworkXML> NetworkXMLHandle;
typedef boost::shared_ptr<ModulePositions> ModulePositionsHandle;
typedef boost::shared_ptr<ModuleNotes> ModuleNotesHandle;
typedef boost::shared_ptr<ConnectionNotes> ConnectionNotesHandle;
typedef boost::shared_ptr<ModuleTags> ModuleTagsHandle;
typedef boost::shared_ptr<NetworkFile> NetworkFileHandle;

typedef std::map<std::string, std::map<std::string, std::map<std::string, ModuleDescription>>> ModuleDescriptionMap;
typedef boost::function<bool(SCIRun::Dataflow::Networks::ModuleHandle)> ModuleFilter;
typedef boost::function<bool(const SCIRun::Dataflow::Networks::ConnectionDescription&)> ConnectionFilter;

}}}


#endif
