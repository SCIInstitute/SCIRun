#ifndef IAUNS_ES_RENDER_GEOMMAN_HPP
#define IAUNS_ES_RENDER_GEOMMAN_HPP

#include <map>
#include <set>
#include <es-cereal/CerealCore.hpp>
#include <gl-shaders/GLShader.hpp>
#include <entity-system/BaseSystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <es-acorn/Acorn.hpp>

namespace ren {

/// Geometry manager. Will load geometry from a single file. Any shaders
/// associated with the geometry will also be loaded alongside VBOs and IBOs. 
class GeomMan
{
public:
  /// \param  numRetries  The number of retries we have to load the asset.
  ///                     Zombie promises will remain present in the system
  ///                     and a load will be re-attempted again when
  ///                     serialized and deserialized.
  GeomMan(int numRetries = 2);
  virtual ~GeomMan();

  /// Loads geometry from disk and associates any loaded VBOs, IBOs, and
  /// shaders with the given entityID.
  /// \param  core      Core base.
  /// \param  entityID  Entity ID which will receive any ren::IBO, ren::VBO,
  ///                   and ren::Shaders loaded alongside the geometry.
  /// \param  assetName Name of the asset to load. Without extension.
  void loadGeometry(CPM_ES_CEREAL_NS::CerealCore& core,
                    uint64_t entityID, const std::string& assetName);

  /// Runs a single GC cycle over Geom components. This does not affect
  /// components other than the Geom component (VBOs, IBOs, Shaders, etc...).
  /// You will have to run GC cycles over the other components in order
  /// to fully clean up the system.
  void runGCCycle(CPM_ES_NS::ESCoreBase& core);

  /// Registers just the promise fullfilment system and the garbage collector.
  static void registerSystems(CPM_ES_ACORN_NS::Acorn& core);

  /// Retrieves GeomMan's garbage collector name.
  static const char* getGCName();

  /// Retrieves the promise fullfilment system name.
  static const char* getPromiseSystemName();

private:
  friend class GeomPromiseFulfillment;
  friend class GeomGarbageCollector;

  bool buildComponent(CPM_ES_CEREAL_NS::CerealCore& core,
                      uint64_t entityID, const std::string& assetName);

  /// Runs garbage collection against a collection of valid asset names.
  void runGCAgainstVaidNames(const std::set<std::string>& validAssets);

  /// Initiates a request for a geometry object.
  void requestAsset(CPM_ES_NS::ESCoreBase& core, const std::string& assetName,
                    int32_t numRetries);

  /// Callback issued when an asset has been loaded, or it was unable to
  /// be loaded.
  void loadAssetCB(const std::string& assetName, bool error,
                   size_t bytesRead, uint8_t* buffer, int32_t numRetries,
                   CPM_ES_NS::ESCoreBase& core);

  struct GeomItem
  {
    /// Name of the shader asset.
    std::string shaderName;

    struct TextureItem
    {
      TextureItem(const std::string& n, const std::string& sampler, uint32_t unit) :
          name(n),
          samplerName(sampler),
          textureUnit(unit)
      {}

      std::string name;
      std::string samplerName;
      uint32_t textureUnit;
    };

    /// List of all texture names loaded in with this geometry item.
    std::list<TextureItem> textures;
  };

  /// Returns true if there is a GeomItem for the given asset name.
  bool hasGeomItemForAsset(const char* assetName);

  /// Mapping of asset name onto the appropriate shader asset.
  /// Appropriate VBOs, IBOs, and other assets can be found via the asset name.
  std::map<std::string, GeomItem> mNameMap;

  /// Indicates whether new assets have been loaded but a promise fulfillment
  /// run has not been made. A GC cycle when this is true will lead to
  /// incorrect results. Hence, if a GC cycle is requested, a warning will
  /// be issued and the GC cycle will be aborted.
  bool mNewUnfulfilledAssets;

  /// Number of retries to attempt before promise failure.
  int32_t mNumRetries;
};

} // namespace ren 

#endif 
