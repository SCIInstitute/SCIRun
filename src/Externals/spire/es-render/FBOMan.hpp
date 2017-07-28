#ifndef IAUNS_RENDER_FBOMAN_HPP
#define IAUNS_RENDER_FBOMAN_HPP

#include <set>
#include <map>
#include <stack>
#include <gl-platform/GLPlatform.hpp>
#include <gl-shaders/GLShader.hpp>
#include <entity-system/BaseSystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <es-acorn/Acorn.hpp>

#include "comp/AssetPromise.hpp"

namespace ren {

  class FBOGarbageCollector;

  class FBOMan
  {
  public:
    FBOMan();
    virtual ~FBOMan();

    /// Adds a FBO whose data is already in memory. Returns the GLid of the
    /// generated FBO.
    GLuint createFBO(CPM_ES_CEREAL_NS::CerealCore& core, GLenum ttype,
      GLsizei npixelx, GLsizei npixely, GLsizei npixelz,
      const std::string& assetName);

    GLuint resizeFBO(CPM_ES_CEREAL_NS::CerealCore& core,
      const std::string& assetName,
      GLsizei npixelx, GLsizei npixely, GLsizei npixelz);

    GLuint getOrCreateFBO(CPM_ES_CEREAL_NS::CerealCore& core, GLenum ttype,
      GLsizei npixelx, GLsizei npixely, GLsizei npixelz,
      const std::string& assetName);

    void bindFBO(const std::string& assetName);
    void bindFBO(GLuint glid);
    void unbindFBO();

    bool readFBO(CPM_ES_CEREAL_NS::CerealCore& core,
      const std::string& assetName,
      GLint posx, GLint posy, GLsizei width, GLsizei height,
      GLvoid* value, GLvoid* depth);

    void removeInMemoryFBO(GLuint glid);

    /// Returns glid if \p assetName is in the VBO man. Returns 0 otherwise.
    /// Use sparingly.
    GLuint hasFBO(const std::string& assetName) const;

    // Use this structure if you are storing FBO data for later retrieval.
    struct MinFBOData
    {
      GLenum textureType;
      GLsizei numPixelsX;
      GLsizei numPixelsY;
      GLsizei numPixelsZ;
    };

    struct FBOData
    {
      FBOData(const std::string& name, GLenum ttype,
        GLsizei npixelx, GLsizei npixely, GLsizei npixelz) :
        assetName(name),
        textureType(ttype),
        numPixelsX(npixelx),
        numPixelsY(npixely),
        numPixelsZ(npixelz)
      {}

      std::string assetName;
      GLenum textureType;
      GLsizei numPixelsX;
      GLsizei numPixelsY;
      GLsizei numPixelsZ;
    };

    /// Retrieves FBO Data structure. An exception is thrown if the structure
    /// does not exist.
    FBOData getFBOData(const std::string& assetName) const;

    void modifyFBOData(const std::string& assetName, FBOData &fboData);

    /// Runs a single garbage collection cycle against the given ESCoreBase.
    /// This does not add an active system. It simply runs the garbage collection
    /// cycle and removes all shaders no longer in use. If this was a system
    /// that had promises, you would run promises before running a GC cycle
    /// since you don't want GC to remove useful shaders.
    void runGCCycle(CPM_ES_NS::ESCoreBase& core);

    /// Returns the GLID for the given assetName (texture name), if one has been
    /// generated. Returns 0 if the texture is not found in the system.
    GLuint getIDForAsset(const char* assetName) const;

    /// Retrieve the asset name from the given GLuint.
    std::string getAssetFromID(GLuint id) const;

    /// Retrieves the GC's name. You can use this in conjunction with
    /// SystemCore to setup an intermitent GC cycle.
    static const char* getGCName();

    /// Registers FBO managers systems. In this case, just the GC system.
    /// Other managers also have a promise system as well, which will fufill
    /// promises made to entities when assets are loaded from disk.
    static void registerSystems(CPM_ES_ACORN_NS::Acorn& core);

  private:
    friend class FBOGarbageCollector;

    uint64_t getEntityIDForName(const std::string& name);

    /// Runs garbage collection against a set of valid keys. All OpenGL ids not
    /// in validKeys will be removed from the system.
    void runGCAgainstVaidIDs(const std::set<GLuint>& validKeys);

    std::map<GLuint, FBOData>      mFBOData;
    /// Map from asset name to GL id.
    std::map<std::string, GLuint> mNameToGL;

    //stack of fbo ids
    std::stack<GLint> mFBOIds;
  };

} // namespace ren

#endif 
