#ifndef IAUNS_RENDER_FONTPROMISE_HPP
#define IAUNS_RENDER_FONTPROMISE_HPP

#include <cstring>
#include <es-cereal/ComponentSerialize.hpp>

namespace ren {

struct FontPromise
{
  // -- Data --
  static const int MaxAssetName = 64;

  /// Name of the shader that was promised.
  char assetName[MaxAssetName];

  // Indicator variable. Never serialized out and is used to indicate when
  // we need to re-issue a request to the filesystem.
  bool requestInitiated;

  // -- Functions --
  FontPromise()
  {
    requestInitiated = false;
    assetName[0] = 0;
  }

  static const char* getName()    {return "ren:FontPromise";}

  void setAssetName(const char* name)
  {
    size_t nameLen = std::strlen(name);
    if (nameLen < MaxAssetName - 1)
    {
      std::strncpy(assetName, name, MaxAssetName);
    }
    else
    {
      std::cerr << "FontPromise: Unable to set name: " << name << " Name must be " 
                << (MaxAssetName - 1) << "characters or shorter." << std::endl;
    }
  }

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& s, uint64_t /* entityID */)
  {
    std::string asset = assetName;
    s.serialize("name", asset);

    // If we are deserializing, then we need to absolutely sure that
    // requestInitiated is false (although it should be from the constructor
    // and the fact that we did not serialize it).
    if (s.isDeserializing())
    {
      requestInitiated = false;
      setAssetName(asset.c_str());
    }
    return true;
  }

};

} // namespace ren 

#endif 
