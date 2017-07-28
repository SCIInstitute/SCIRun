#ifndef IAUNS_RENDER_ASSETPROMISE_HPP
#define IAUNS_RENDER_ASSETPROMISE_HPP

#include <es-cereal/ComponentSerialize.hpp>

namespace ren {

struct AssetPromise
{
  // -- Data --
  static const int MaxAssetName = 32;

  // Name of the asset that this entity has been promised. This asset may
  // very well decompose into several items, such as a VBO / IBO pair and
  // a set of shaders. All of the necessary components will be added to
  // the object when the asset is finally loaded.
  char name[MaxAssetName];

  // This is an indicator variable. If it is false (because we serialized)
  // then a new request needs to be issued by the system to
  bool requestInitiated;

  // The number of errors this asset has encountered.
  uint32_t numErrors;

  // -- Functions --

  static const char* getName() {return "ren:AssetPromise";}

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */,
                 uint64_t /* entityID */)
  {
    return true;
  }
};

} // namespace ren

#endif 
