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


#ifndef SPIRE_ES_ACORN_HPP
#define SPIRE_ES_ACORN_HPP

#include <es-log/trace-log.h>
#include <entity-system/ESCoreBase.hpp>
#include <es-cereal/CerealCore.hpp>
#include <es-systems/SystemCore.hpp>
#include <set>
#include <spire/scishare.h>

namespace spire {

class SCISHARE Acorn : public spire::CerealCore
{
public:
  Acorn();
  virtual ~Acorn();

  /// Execute core with current time in milliseconds.
  virtual void execute(double constantFrameTime) = 0;

  /// Runs the system once over all applicable components.
  void runSystemOnce(const std::string& systemName);

  /// Adds a kernel space system. No system added to the kernel space can
  /// be removed. A warning is issued if user space attempts to add a system
  /// that is already in the kernel. Same goes for removal of systems.
  /// See es-systems/SystemCore for a description of the parameters.
  void addKernelSystem(const std::string& name, uint64_t ms = 0,
                       uint64_t referenceTime = 0, uint64_t stagger = 0);

  /// Adds a user space system. These are the only systems that can be removed.
  /// Check to ensure the system is not already present before loading.
  void addUserSystem(const std::string& name, uint64_t ms = 0,
                     uint64_t referenceTime = 0, uint64_t stagger = 0);

  /// This function does not check to see if the system already exists before
  /// adding. It adds it anyways and es_system will sort out any conflicts
  /// during renormalization. This is useful if you know that the system may
  /// be removed during the course of the current frame and you want it to
  /// remain after its duplicate is removed from the system. Used in the
  /// loading system.
  void addUserSystemForced(const std::string& name, uint64_t ms = 0,
                           uint64_t referenceTime = 0, uint64_t stagger = 0);

  /// Remove user space system.
  void removeUserSystem(const std::string& name);

  /// Remove all user space systems.
  void removeAllUserSystems();

  /// Returns true if the system is active.
  bool isSystemActive(const std::string& name);

  /// This component will not be cleared with the rest of the components when
  /// clearAllNonExemptComponents is issued.
  template <typename T>
  void addExemptComponent()
  {
    mExemptComponents.insert(spire::TemplateID<T>::getID());
  }

  /// Adds a garbage collector system. Can be used in conjunction with
  /// garbageCollect() to run a one time garbage collection cycle, usually
  /// inbetween loading stages in the program. Alternatively, you can have
  /// these garbage collectors running on scheduled.
  void addGarbageCollectorSystem(const std::string& sys);

  /// Runs one garbage collection cycle across all garbage collectable systems.
  /// This is very expensive function as you can imagine. It involves much
  /// dynamic memory deallocation *and* allocation (to create the garbage
  /// collectors). If you cannot incur this cost, take the list of garbage
  /// collectable systems and have them run at infrequent intervals which
  /// will ammoritize the cost over time.
  void runCompleteGC();

  /// Clears out all components *except* for the components matching the
  /// given list of template IDs. You will have to manually clean the components
  /// in the exception list if any entities in those components need to be
  /// removed.
  void clearAllNonExemptComponents(bool debug = false);

  /// Display entity vs system info. Whether the entity satisfies the system
  /// and if it doesn't, what components it needs to satisfy the system.
  /// Also displays number of components associated with the entity for
  /// each system component. Expensive function -- involves dynamic memory
  /// allocation.
  bool displayEntityVersusSystemInfo(uint64_t entityID, const std::string& name);

  /// Registers a system with the SystemCore.
  template <typename T>
  void registerSystem()
  {
    mSystems->registerSystem<T>();
  }

protected:

  /// Get the system core.
  std::shared_ptr<spire::SystemCore> getSystemCore() { return mSystems; }

  /// Returns true if the system is present.
  bool warnIfSystemPresent(const std::string& name);

  /// Systems associated with kernel space. Systems will never get removed
  /// during the execution of the program.
  std::set<std::string> mKernelSystems;

  /// Systems associated with user space. These systems are frequently added
  /// and removed depending on the currently needs of the system.
  std::set<std::string> mUserSystems;

  /// Contains all systems that correspond to garbage collectors that can
  /// be run across the entire entity component system.
  std::vector<std::string> mGarbageCollectorSystems;

  /// The set of exempt components. Exempt components are similar to kernel
  /// space systems. They can still be removed, but they will not be removed
  /// by the clearAllNonExemptComponents function.
  std::set<uint64_t> mExemptComponents;

  /// All registered systems.
  std::shared_ptr<spire::SystemCore>  mSystems;
};

} // namespace spire

#endif
