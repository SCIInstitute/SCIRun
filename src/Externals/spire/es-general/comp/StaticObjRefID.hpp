#ifndef IAUNS_ES_GENERAL_COMP_STATICOBJREFID_HPP
#define IAUNS_ES_GENERAL_COMP_STATICOBJREFID_HPP

#include <cstdint>
#include <entity-system/GenericSystem.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <es-cereal/CerealCore.hpp>

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
  StaticObjRefID() {referenceID = CPM_ES_NS::BaseComponentContainer::StaticEntID + 1;}

  /// Retrieves a new object ID without having to know about 'this'.
  /// 'this' gets retrieved from the core and it is created if it does
  /// not already exist.
  static uint64_t getNewObjectID(CPM_ES_CEREAL_NS::CerealCore& core)
  {
    return getMe(core)->getNewID();
  }

  /// Allocates a block of ids and returns the ID of the first block entry.
  /// Useful for creating object pools. These pools can be used to logically
  /// sort entitys.
  static uint64_t getNewBlock(CPM_ES_CEREAL_NS::CerealCore& core, int32_t blockSize)
  {
    return getMe(core)->getBlock(blockSize);
  }

  // Required functions (CPM_ES_CEREAL_NS::CerealHeap)
  static const char* getName() {return "gp:StaticObjRefID";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& s, uint64_t /* entityID */)
  {
    s.serialize("refID", referenceID);
    return true;
  }

private:
  static StaticObjRefID* getMe(CPM_ES_CEREAL_NS::CerealCore& core)
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
