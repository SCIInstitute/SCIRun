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


#ifndef SPIRE_SYSTEM_SYSTEMFACTORY_HPP
#define SPIRE_SYSTEM_SYSTEMFACTORY_HPP

#include <es-log/trace-log.h>
#include <memory>
#include <stdexcept>
#include <entity-system/ESCoreBase.hpp>
#include <spire/scishare.h>

namespace spire {

class SCISHARE SystemFactory
{
  typedef std::shared_ptr<spire::BaseSystem> (*ClassFactoryFunPtr)();
public:

  template <typename T>
  void registerSystem(const char* name)
  {
    ClassFactoryFunPtr fun = &createSystem<T>;
    auto ret = mMap.insert(std::make_pair(name, fun));
    if (!std::get<1>(ret))
    {
      std::cerr << "es-systems: Duplicate system name: " << name << std::endl;
      throw std::runtime_error("Duplicate system name.");
    }
  }

  /// new's a new system based purely on name.
  /// Warns and returns a nullptr if \p name is not found.
  std::shared_ptr<spire::BaseSystem> newSystemFromName(const char* name);

  /// True if the system with the given name exists in our map.
  bool hasSystem(const char* name);

  /// Clear registered systems.
  void clearSystems() {mMap.clear();}

private:

  template<typename T>
  static std::shared_ptr<spire::BaseSystem> createSystem()
  {
    return std::shared_ptr<spire::BaseSystem>(new T);
  }

  std::map<std::string, ClassFactoryFunPtr> mMap;
};

} // namespace spire

#endif
