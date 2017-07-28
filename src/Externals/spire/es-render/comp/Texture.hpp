#ifndef IAUNS_COMPONENT_RENDER_TEXTURE_HPP
#define IAUNS_COMPONENT_RENDER_TEXTURE_HPP

#include <es-cereal/ComponentSerialize.hpp>

#include <gl-platform/GLPlatform.hpp>
#include <es-cereal/CerealCore.hpp>
#include "TexturePromise.hpp"
#include "StaticTextureMan.hpp"

namespace ren {

struct Texture
{
  // -- Data --
  static const int MaxUniformName = 24;
  GLuint          glid;         ///< glid associated with the loaded texture.
  int32_t         textureUnit;  ///< The texture unit this texture is bound to.
  GLint           uniformLocation;
  char            uniformName[MaxUniformName];
  GLenum          textureType;
  GLsizei textureWidth;
  GLsizei textureHeight;
  GLsizei textureDepth;
  GLint internalFormat;
  GLenum format;
  GLenum type;
  GLint filter;

  // -- Functions --
  Texture()
  {
    textureUnit = 0;
    glid = 0;
    uniformLocation = -1;
    uniformName[0] = '\0';
    textureType = GL_TEXTURE_2D;
  }

  static const char* getName() {return "ren:Texture";}

  void checkUniform(GLuint shaderID);
  void applyUniform() const;
  bool isSetUp() const;

  void setUniformName(const char* name)
  {
    size_t nameLen = std::strlen(name);
    if (nameLen < MaxUniformName - 1)
    {
      std::strncpy(uniformName, name, MaxUniformName);
    }
    else
    {
      std::cerr << "Texture: Unable to set uniform name: " << name << " Name must be " 
                << (MaxUniformName - 1) << "characters or shorter." << std::endl;
    }
  }

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& s, uint64_t entityID)
  {
    // The logic below ensures we deserialize with promises, not with actual
    // texture assets (which wouldn't make sense with OpenGL assets).
    if (s.isDeserializing())
    {
      std::string assetName;
      int32_t tu;
      s.serialize("name", assetName);
      s.serialize("unit", tu);

      // Build texture promise.
      TexturePromise newPromise;
      newPromise.requestInitiated = false;
      newPromise.textureUnit = tu;
      newPromise.setAssetName(assetName.c_str());

      CPM_ES_CEREAL_NS::CerealCore& core 
          = dynamic_cast<CPM_ES_CEREAL_NS::CerealCore&>(s.getCore());
      core.addComponent(entityID, newPromise);

      return false; // We do not want to add this texture component back into the components.
                    // Instead we rely on the texture promise we created above.
    }
    else
    {
      CPM_ES_CEREAL_NS::CerealCore& core 
          = dynamic_cast<CPM_ES_CEREAL_NS::CerealCore&>(s.getCore());
      StaticTextureMan* staticTX = core.getStaticComponent<StaticTextureMan>();
      std::weak_ptr<TextureMan> texMan = staticTX->instance_;

      // Find the asset name associated with our glid and serialize it out.
      if (std::shared_ptr<TextureMan> tm = texMan.lock()) {
        std::string assetName = tm->getAssetFromID(glid);
        s.serialize("name", assetName);
        s.serialize("unit", textureUnit);
        return true;
      }
      return false;
    }
  }
};

} // namespace ren

#endif 
