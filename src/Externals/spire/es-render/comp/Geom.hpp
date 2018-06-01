#ifndef SPIRE_RENDER_COMPONENT_GEOM_HPP
#define SPIRE_RENDER_COMPONENT_GEOM_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <spire/scishare.h>

namespace ren {

struct Geom
{
  // -- Data --
  static const int MaxAssetName = 32;

  /// Name of the geometry asset that was loaded.
  char assetName[MaxAssetName];

  // -- Functions --
  Geom()
  {
    assetName[0] = 0;
  }

  void setAssetName(const char* name)
  {
    size_t nameLen = std::strlen(name);
    if (nameLen < MaxAssetName - 1)
    {
      std::strncpy(assetName, name, MaxAssetName);
    }
    else
    {
      std::cerr << "ShaderPromiseVF: Unable to set name: " << name << " Name must be "
                << (MaxAssetName - 1) << "characters or shorter." << std::endl;
    }
  }

  static const char* getName() {return "ren:Geom";}

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    std::string asset = assetName;
    s.serialize("name", asset);
    return true;
  }
};

} // namespace ren

#endif
