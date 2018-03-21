#ifndef SPIRE_RENDER_GEOMPROMISE_HPP
#define SPIRE_RENDER_GEOMPROMISE_HPP

#include <es-log/trace-log.h>
#include <cstring>
#include <es-cereal/ComponentSerialize.hpp>
#include <spire/scishare.h>

namespace ren {

struct GeomPromise
{
  // -- Data --
  static const int MaxAssetName = 64;

  /// Name of the shader that was promised.
  char assetName[MaxAssetName];

  // Indicator variable. Never serialized out and is used to indicate when
  // we need to re-issue a request to the filesystem.
  bool requestInitiated;

  // -- Functions --
  GeomPromise()
  {
    requestInitiated = false;
    assetName[0] = 0;
  }

  static const char* getName()    {return "ren:GeomPromise";}

  void setAssetName(const char* name)
  {
    size_t nameLen = std::strlen(name);
    if (nameLen < MaxAssetName - 1)
    {
      std::strncpy(assetName, name, MaxAssetName);
    }
    else
    {
      logRendererError("GeomPromise: Unable to set name: {}. Name must be {} characters or shorter.",
        name, MaxAssetName - 1);
    }
  }

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    std::string asset = assetName;
    s.serialize("name", asset);

    // If we are deserializing, then we need to absolutely sure that
    // requestInitiated is false (although it should be from the constructor
    // and the fact that we did not serialize it).
    if (s.isDeserializing()) requestInitiated = false;
    return true;
  }

};

} // namespace ren

#endif
