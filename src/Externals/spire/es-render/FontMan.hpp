#ifndef IAUNS_RENDER_FONTMAN_HPP
#define IAUNS_RENDER_FONTMAN_HPP

#include <map>
#include <set>
#include <es-cereal/CerealCore.hpp>
#include <entity-system/BaseSystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <gl-platform/GLPlatform.hpp>
#include <es-acorn/Acorn.hpp>

#include "BMFont.hpp"

namespace ren {

class FontGarbageCollector;
class FontPromiseVFFulfillment;

/// 
class FontMan
{
public:
  /// \param  numRetries  The number of retries we have to load the asset.
  ///                     Zombie promises will remain present in the system
  ///                     and a load will be re-attempted again when
  ///                     serialized and deserialized.
  FontMan(int numRetries = 2);
  virtual ~FontMan();

  /// Loads texture onto the given entityID.
  /// \param  core        Core base.
  /// \param  entityID    Entity ID which will receive the ren::Texture component.
  /// \param  assetName   Name of the texture to load.
  void loadFont(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
                const std::string& assetName);

  /// Runs a single garbage collection cycle on the current state of the core.
  void runGCCycle(CPM_ES_NS::ESCoreBase& core);

  /// Returns the entity ID for the given assetName, if one has been
  /// generated. Returns 0 if the asset is not found in the system.
  uint64_t getIDForAsset(const char* assetName) const;

  /// Retrieve the asset name from the given GLuint.
  std::string getAssetFromID(uint64_t id) const;

  /// Retrieve BMFont info from font id.
  const BMFont& getBMFontInfo(uint64_t id) const;
  
  /// Registers TextureMan's systems. Both the garbage collector and the promise
  /// fullfillment system are registered.
  static void registerSystems(CPM_ES_ACORN_NS::Acorn& core);

  /// Obtains the garbage collectors name so that you can setup intermitent
  /// garbage collection cycles.
  static const char* getGCName();

  /// Obtains the texture promise fullfilment system's name. This system should
  /// be installed in every core. You shouldn't need to run it every frame.
  /// Maybe about every 200 MS.
  static const char* getPromiseSystemName();

private:
  friend class FontGarbageCollector;
  friend class FontPromiseFulfillment;

  bool buildComponent(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
                      const std::string& assetName);

  void runGCAgainstVaidIDs(const std::set<uint64_t>& validKeys);

  /// Issues a request for a texture.
  void requestFont(CPM_ES_NS::ESCoreBase& core, const std::string& assetName,
                   int32_t numRetries);

  /// Callback issued when a texture has been read from disk and is ready
  /// to load.
  void loadFontCB(const std::string& textureName, bool error,
                  size_t bytesRead, uint8_t* buffer,
                  int32_t numRetries, CPM_ES_NS::ESCoreBase& core);

  struct FontInfo
  {
    FontInfo(uint64_t id, const std::string& name) :
        fontID(id),
        assetName(name)
    {}

    uint64_t      fontID;       ///< Unique font ID for this font.
    std::string   assetName;    ///< Name for asset.
    BMFont        fontInfo;     ///< Font information.
  };

  /// Retrieve font information from font ID.
  const FontInfo& getFontInfo(uint64_t id);

  /// Map from GL id to asset name.
  std::map<uint64_t, FontInfo> mIDToFont;

  /// Indicates whether new assets have been loaded but a promise fulfillment
  /// run has not been made. A GC cycle when this is true will lead to
  /// incorrect results. Hence, if a GC cycle is requested, a warning will
  /// be issued and the GC cycle will be aborted.
  bool mNewUnfulfilledAssets;

  /// Number of retries to attempt before resulting in promise failure.
  int32_t mNumRetries;

  /// Last font ID used.
  uint64_t mLastFontID;
};

} // namespace 

#endif 
