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


#ifndef CORE_SERIALIZATION_NETWORK_MODULE_POSITION_GETTER_H
#define CORE_SERIALIZATION_NETWORK_MODULE_POSITION_GETTER_H

#include <map>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Serialization/Network/share.h>

namespace SCIRun {
namespace Dataflow {
namespace Networks {

  struct SCISHARE ModulePositions
  {
    typedef std::map<std::string, std::pair<double,double> > Data;
    Data modulePositions;
  };

  class SCISHARE NetworkEditorSerializationManager
  {
  public:
    virtual ~NetworkEditorSerializationManager() {}
    virtual ModulePositionsHandle dumpModulePositions(ModuleFilter filter) const = 0;
    virtual void updateModulePositions(const ModulePositions& modulePositions, bool selectAll) = 0;

    //TODO: refactor into dump/updateNotes, shouldn't need to distinguish parent.
    virtual ModuleNotesHandle dumpModuleNotes(ModuleFilter filter) const = 0;
    virtual void updateModuleNotes(const ModuleNotes& notes) = 0;
    virtual ConnectionNotesHandle dumpConnectionNotes(ConnectionFilter filter) const = 0;
    virtual void updateConnectionNotes(const ConnectionNotes& notes) = 0;

    virtual ModuleTagsHandle dumpModuleTags(ModuleFilter filter) const = 0;
    virtual void updateModuleTags(const ModuleTags& notes) = 0;

    virtual DisabledComponentsHandle dumpDisabledComponents(ModuleFilter modFilter, ConnectionFilter connFilter) const = 0;
    virtual void updateDisabledComponents(const DisabledComponents& disabled) = 0;

    virtual SubnetworksHandle dumpSubnetworks(ModuleFilter filter) const = 0;
    virtual void updateSubnetworks(const Subnetworks& subnets) = 0;

    virtual void copyNote(ModuleHandle from, ModuleHandle to) const = 0;
  };

}}}

#endif
