#ifndef IAUNS_RENDER_COMPONENT_FBO_HPP
#define IAUNS_RENDER_COMPONENT_FBO_HPP

#include <glm/glm.hpp>
#include <gl-platform/GLPlatform.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <es-render/comp/Texture.hpp>

namespace ren {

  struct FBO
  {
    // -- Data --
    GLuint glid;
    GLenum textureType;
    bool initialized;

    // -- Functions --
    FBO()
    {
      glid = 0;
      textureType = GL_TEXTURE_2D;
      initialized = false;
    }

    static const char* getName() { return "ren:FBO"; }

    struct TextureData
    {
      GLenum att;
      std::string texName;
    };

    std::vector<TextureData> textures;

    bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
    {
      /// Nothing needs to be serialized. This is context specific.
      return true;
    }
  };

} // namespace ren

#endif 
