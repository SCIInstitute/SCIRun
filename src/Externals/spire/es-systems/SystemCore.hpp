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


#ifndef SPIRE_ES_SYSTEMS_SYSTEMCORE_HPP
#define SPIRE_ES_SYSTEMS_SYSTEMCORE_HPP

#include <es-log/trace-log.h>
#include <list>
#include <entity-system/ESCoreBase.hpp>
#include <tny/tny.hpp>
#include "SystemFactory.hpp"
#include <spire/scishare.h>

namespace spire {

namespace core_detail {

template <typename T>
class has_getname_fun
{
    typedef char one;
    typedef long two;

    template <typename C> static one test( decltype(&C::getName) ) ;
    template <typename C> static two test(...);

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

} // namespace core_detail

class SCISHARE SystemCore
{
public:
  std::string toString(std::string prefix) const;

  /// Perform requested additions and removals of systems that occured
  /// during the frame.
  void renormalize();

  /// Runs all systems.
  void runSystems(spire::ESCoreBase& core, uint64_t referenceTime);

  /// Registers the system with the serialization system so that a system can
  /// be created on-demand during deserialization.
  template <typename T>
  void registerSystem()
  {
    // Perform a static assert on the system to make sure it has a static
    // function that grabs the name.
    static_assert( core_detail::has_getname_fun<T>::value, "System does not expose a getName function." );

    // Ensure there is no duplicate system.
    if (mSystemFactory.hasSystem(T::getName()))
    {
      std::cerr << "es-systems: System with duplicate name." << " Name: " << T::getName() << std::endl;
      throw std::runtime_error("es-systems: System with duplicate name.");
      return;
    }

    mSystemFactory.registerSystem<T>(T::getName());
  }

  /// This is only used when testing the system. It has little practical
  /// use outside of registration tests.
  void clearRegisteredSystems()
  {
    mSystemFactory.clearSystems();
  }

  /// Add active system via name.
  /// If ms is positive, then it represents the number of milliseconds that
  /// should elapse before the system walks its components. If referenceTime
  /// is 0, then the system will be executed at the next valid time.
  /// @param stagger  If non-zero, then
  void addActiveSystem(const std::string& name, uint64_t ms = 0,
                       uint64_t referenceTime = 0, uint64_t stagger = 0);

  /// Remove active system via name.
  void removeActiveSystem(const std::string& name);

  /// Remove all active systems. Does not remove the systems immediately.
  /// Waits for a renormalize.
  void removeAllActiveSystems();

  /// Add active system via type.
  template <typename T>
  void addActiveSystemViaType(int64_t ms = 0, uint64_t referenceTime = 0,
                              uint64_t stagger = 0)
  {
    static_assert( core_detail::has_getname_fun<T>::value, "System does not expose a getName function." );
    addActiveSystem(T::getName(), ms, referenceTime, stagger);
  }

  /// Remove active system via type.
  template <typename T>
  void removeActiveSystemViaType()
  {
    static_assert( core_detail::has_getname_fun<T>::value, "System does not expose a getName function." );
    removeActiveSystem(T::getName());
  }

  /// Create a new system and return its shared_ptr.
  std::shared_ptr<spire::BaseSystem> createNewSystem(const std::string& name)
  {
    return mSystemFactory.newSystemFromName(name.c_str());
  }

  /// Serializes active systems.
  /// The caller is responsible for calling Tny_free on the returned Tny*.
  /// The function does not presume that you use shared_ptr's, nor does this
  /// class use the pointers passed in after the function returns.
  Tny* serializeActiveSystems();

  /// Deserializes active systems serialized within \p data.
  /// Deserialize systems. This function will create new instances of the
  /// systems serialized out with serializeAllSystems, if the systems have
  /// been appropriately registered. This allows you to recreate the
  /// programmatic state of the application.
  /// The caller is responsible for deleting all of the pointers returned.
  /// It is advisable to migrate the pointers into std::shared_ptr's instead
  /// of leaving them as raw pointers.
  /// Any systems that are not currently active will be placed on the active
  /// list.
  void deserializeActiveSystems(Tny* data, uint64_t referenceTime);

  /// Returns true if the system is currently active.
  bool isSystemActive(const std::string& name) const;

private:

  struct SystemItem
  {
    explicit SystemItem(const std::string& n) :
        systemName(n),
        interval(0),
        stagger(0),
        nextExecutionTime(0)
    {}

    SystemItem(const std::string& n, std::shared_ptr<spire::BaseSystem> sys,
               uint64_t updateInterval, uint64_t referenceTime, uint64_t stag) :
        system(sys),
        systemName(n),
        interval(updateInterval),
        stagger(stag)
    {
      nextExecutionTime = calcNextExecutionTime(referenceTime);
    }

    SystemItem(const SystemItem& other) :
        system(other.system),
        systemName(other.systemName),
        interval(other.interval),
        stagger(other.stagger),
        nextExecutionTime(other.nextExecutionTime)
    {}

    // This function will modify the next execution time. Only use it when you
    // are checking whether or not the system should execute.
    bool shouldExecute(uint64_t referenceTime);

    /// Calculate next reference time taking into account current system stagger.
    uint64_t calcNextExecutionTime(uint64_t referenceTime);

    /// Pointer to our system.
    std::shared_ptr<spire::BaseSystem>  system;

    /// We store the name alongside our system pointer because BaseSystem
    /// does not expose a getName function. We expose it at compile time.
    std::string systemName;

    uint64_t    interval;           ///< Update interval in MS.
    uint64_t    stagger;            ///< Offset into interval, relative to reference time,
                                    ///< at which this system should execute.
                                    ///< Used to stagger system execution in a
                                    ///< predictable way.
    uint64_t    nextExecutionTime;  ///< Next execution time in MS from reference.
  };

  static bool systemCompare(const SystemItem& a, const SystemItem& b);

  /// Alphabetically sorted system list. Executed in alphabetical order.
  std::vector<SystemItem>   mSystems;

  /// Systems to add during renormalization.
  std::vector<SystemItem>   mSystemsToAdd;

  /// Systems to remove during renormalize.
  std::vector<std::string>  mSystemsToRemove;

  /// Factory that stores all registered systems.
  SystemFactory      mSystemFactory;
};

} // namespace spire

#endif
