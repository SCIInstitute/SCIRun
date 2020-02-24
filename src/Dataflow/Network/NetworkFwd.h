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


/// @todo Documentation Dataflow/Network/NetworkFwd.h


#ifndef DATAFLOW_NETWORK_NETWORK_FWD_H
#define DATAFLOW_NETWORK_NETWORK_FWD_H

#include <Core/Utils/SmartPointers.h>
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
struct DisabledComponents;
struct Subnetworks;
/// @todo: rename this
struct NetworkFile;
struct ToolkitFile;
class NetworkGlobalSettings;
class NetworkEditorSerializationManager;
class ConnectionMakerService;
class NetworkEditorControllerInterface;
class ReexecuteStrategyFactory;
class MetadataMap;
class ModuleBuilder;

typedef SharedPointer<NetworkInterface> NetworkHandle;
typedef SharedPointer<ModuleInterface> ModuleHandle;
typedef SharedPointer<ModuleStateInterface> ModuleStateHandle;
typedef SharedPointer<ModuleStateInterfaceFactory> ModuleStateFactoryHandle;
typedef SharedPointer<ReexecuteStrategyFactory> ReexecuteStrategyFactoryHandle;
typedef SharedPointer<PortInterface> PortHandle;
typedef SharedPointer<Connection> ConnectionHandle;
typedef SharedPointer<InputPortInterface> InputPortHandle;
typedef SharedPointer<OutputPortInterface> OutputPortHandle;
typedef SharedPointer<ModuleFactory> ModuleFactoryHandle;
typedef SharedPointer<DatatypeSinkInterface> DatatypeSinkInterfaceHandle;
typedef SharedPointer<DatatypeSourceInterface> DatatypeSourceInterfaceHandle;
typedef SharedPointer<NetworkXML> NetworkXMLHandle;
typedef SharedPointer<ModulePositions> ModulePositionsHandle;
typedef SharedPointer<ModuleNotes> ModuleNotesHandle;
typedef SharedPointer<ConnectionNotes> ConnectionNotesHandle;
typedef SharedPointer<ModuleTags> ModuleTagsHandle;
typedef SharedPointer<DisabledComponents> DisabledComponentsHandle;
typedef SharedPointer<NetworkFile> NetworkFileHandle;
typedef SharedPointer<Subnetworks> SubnetworksHandle;

typedef std::map<std::string, std::map<std::string, std::map<std::string, ModuleDescription>>> ModuleDescriptionMap;
typedef boost::function<bool(ModuleHandle)> ModuleFilter;
using ConnectionFilter = boost::function<bool(const ConnectionDescription&)>;

}}}


#endif
