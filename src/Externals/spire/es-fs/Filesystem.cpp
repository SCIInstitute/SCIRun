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


#include "Filesystem.hpp"
#include "fscomp/StaticFS.hpp"
#include <entity-system/GenericSystem.hpp>
#include <es-systems/SystemCore.hpp>

namespace es = spire;

namespace spire {

class FSSystem : public spire::GenericSystem<false, StaticFS>
{
public:
  static const char* getName() {return "es-fs:FSSystem";}

  void execute(spire::ESCoreBase& /* core */, uint64_t /* entityID */, const StaticFS* fs) override
  {
    // Update the static filesystem by casting off const.
    StaticFS* mutableFS = const_cast<StaticFS*>(fs);
    mutableFS->instance->update();
  }
};

const char* Filesystem::getFSSystemName()
{
  return FSSystem::getName();
}

void Filesystem::registerSystems(spire::Acorn& core)
{
  core.registerSystem<FSSystem>();
}

} // namespace spire
