#ifndef IAUNS_RENDER_TEXTUREMAN_HPP
#define IAUNS_RENDER_TEXTUREMAN_HPP

#include <map>
#include <set>
#include <es-cereal/CerealCore.hpp>
#include <entity-system/BaseSystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <gl-platform/GLPlatform.hpp>
#include <es-acorn/Acorn.hpp>

namespace ren {

  class Texture;
  /// Basic texture manager. Very similar to the shader manager.
  class TextureMan
  {
  public:
    /// \param  numRetries  The number of retries we have to load the asset.
    ///                     Zombie promises will remain present in the system
    ///                     and a load will be re-attempted again when
    ///                     serialized and deserialized.
    TextureMan(int numRetries = 2);
    virtual ~TextureMan();

    /// Loads texture onto the given entityID.
    /// \param  core        Core base.
    /// \param  entityID    Entity ID which will receive the ren::Texture component.
    /// \param  assetName   Name of the texture to load.
    void loadTexture(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
      const std::string& assetName, int32_t textureUnit,
      const std::string& uniformName);

    //create an empty texture
    ren::Texture createTexture(
      const std::string& assetName,
      GLsizei textureWidth, GLsizei textureHeight,
      GLint internalFormat, GLenum format,
      GLenum type, GLint filter);

    //create a texture from font face
    ren::Texture createTexture(
      const std::string& assetName,
      GLsizei textureWidth, GLsizei textureHeight,
      const std::vector<uint8_t>& bitmap);

    //resize texture
    bool resizeTexture(
      ren::Texture &tex, GLsizei textureWidth,
      GLsizei textureHeight);

    /// Runs a single garbage collection cycle on the current state of the core.
    void runGCCycle(CPM_ES_NS::ESCoreBase& core);

    /// Returns the GLID for the given assetName (texture name), if one has been
    /// generated. Returns 0 if the texture is not found in the system.
    GLuint getIDForAsset(const char* assetName) const;

    /// Retrieve the asset name from the given GLuint.
    std::string getAssetFromID(GLuint id) const;

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
    friend class TextureGarbageCollector;
    friend class TexturePromiseFulfillment;

    /// Returns false if we failed to generate the component because the asset
    /// has not been loaded yet.
    bool buildComponent(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
      const std::string& assetName, int32_t textureUnit,
      const std::string& uniformName);

    /// Run garbage collection against validKeys.
    void runGCAgainstVaidIDs(const std::set<GLuint>& validKeys);

    /// Issues a request for a texture.
    void requestTexture(CPM_ES_NS::ESCoreBase& core, const std::string& assetName,
      int32_t numRetries);

    /// Callback issued when a texture has been read from disk and is ready
    /// to load.
    void loadTextureCB(const std::string& textureName, bool error,
      size_t bytesRead, uint8_t* buffer,
      int32_t numRetries, CPM_ES_NS::ESCoreBase& core);


    /// Called from loadTextureCB.
    void loadRawPNG(const std::string& assetName, uint8_t* buffer,
      size_t bytesRead, int numRetries,
      CPM_ES_NS::ESCoreBase& core);

    void loadRawITX(const std::string& assetName, uint8_t* buffer,
      size_t bytesRead, int numRetries,
      CPM_ES_NS::ESCoreBase& core);

    /// Map from GL id to asset name.
    std::map<GLuint, std::string> mGLToName;

    /// Map from asset name to GL id.
    std::map<std::string, GLuint> mNameToGL;

    /// Indicates whether new assets have been loaded but a promise fulfillment
    /// run has not been made. A GC cycle when this is true will lead to
    /// incorrect results. Hence, if a GC cycle is requested, a warning will
    /// be issued and the GC cycle will be aborted.
    bool mNewUnfulfilledAssets;

    /// Number of retries to attempt before resulting in promise failure.
    int32_t mNumRetries;
  };

} // namespace ren

#endif 
