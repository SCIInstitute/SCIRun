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


#ifndef SPIRE_ES_GENERAL_COMP_STATICOBJREFID_HPP
#define SPIRE_ES_GENERAL_COMP_STATICOBJREFID_HPP

#include <es-log/trace-log.h>
#include <cstdint>
#include <entity-system/GenericSystem.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <es-cereal/CerealCore.hpp>
#include <spire/scishare.h>

namespace gen {

/// Reference ID class.
struct StaticObjRefID
{
  // -- Data --

  /// Current reference ID for objects. Will be incremented continuously
  /// for new objects.
  uint64_t referenceID;

  // -- Functions --

  // This is IMPORTANT! We must start spawning entities AFTER the static entity
  // ID. Otheriwes we can accidentally delete static entitys when deleting
  // regular components!
  StaticObjRefID() {referenceID = spire::BaseComponentContainer::StaticEntID + 1;}

  /// Retrieves a new object ID without having to know about 'this'.
  /// 'this' gets retrieved from the core and it is created if it does
  /// not already exist.
  static uint64_t getNewObjectID(spire::CerealCore& core)
  {
    return getMe(core)->getNewID();
  }

  /// Allocates a block of ids and returns the ID of the first block entry.
  /// Useful for creating object pools. These pools can be used to logically
  /// sort entitys.
  static uint64_t getNewBlock(spire::CerealCore& core, int32_t blockSize)
  {
    return getMe(core)->getBlock(blockSize);
  }

  // Required functions (spire::CerealHeap)
  static const char* getName() {return "gp:StaticObjRefID";}

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    s.serialize("refID", referenceID);
    return true;
  }

private:
  static StaticObjRefID* getMe(spire::CerealCore& core)
  {
    StaticObjRefID* me = core.getStaticComponent<StaticObjRefID>();
    if (me == nullptr)
    {
      // Note: We cannot just create a new StaticObjRefID here. A
      // renormalization would be required and we wouldn't be able to retrieve
      // a pointer to this class immediately.
      std::cerr << "No StaticObjRefID is not present in the core!" << std::endl;
      throw std::runtime_error("Unable to find StaticObjRefID");
    }

    return me;
  }

  uint64_t getNewID()
  {
    ++referenceID;
    return referenceID;
  }

  uint64_t getBlock(int32_t blockSize)
  {
    uint64_t blockBegin = referenceID;
    referenceID += static_cast<uint64_t>(blockSize);
    return blockBegin;
  }

};


} // namespace gen

#endif
