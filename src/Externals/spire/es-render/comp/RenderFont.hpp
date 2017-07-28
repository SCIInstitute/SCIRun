#ifndef IAUNS_RENDER_COMPONENT_RENDER_FONT_HPP
#define IAUNS_RENDER_COMPONENT_RENDER_FONT_HPP

#include <glm/glm.hpp>
#include <gl-shaders/GLShader.hpp>
#include <es-cereal/ComponentSerialize.hpp>

#include "StaticVBOMan.hpp"
#include "StaticFontMan.hpp"

namespace ren {

struct RenderFont
{
  // Note, the rendered text's origin is to the bottom right of the origin.
  // Based on xoffset, some of the text may be rendered slightly left of origin.
  enum TextRenderingType
  {
    RENDER_LINE,
    RENDER_MULTILINE, // NOT IMPLEMENTED
    RENDER_BOX        // NOT IMPLEMENTED
  };

  // -- Data --
  static const size_t MaxStringSize = 128;
  static const int MaxFontAttributes = 4;

  // We own this vbo and ibo since they were only constructed for our string.
  // It will be destroyed on component destruct. We don't use the default
  // quads for this because 1) we construct the entire string in the vbo, 
  // 2) we don't use 0 and 1 texcoords, and 3) we have extra information being
  // sent to the shader regarding the center of the tex quads.
  GLuint vbo;
  GLuint ibo;

  // Information related to rendering our custom ibo.
  GLenum  primMode;   ///< GL_TRIANGLE_STRIP...
  GLenum  primType;   ///< GL_UNSIGNED_SHORT...
  GLsizei numPrims;

  // The text that will be rendered to the screen.
  char text[MaxStringSize];

  // Text rendering type (line, multiline [includes \n], and box)
  TextRenderingType renderType;

  // Shader attributes computed by determining the common attributes between
  // the shader and the attributes that we supply.
  int attribSize;
  size_t stride;
  CPM_GL_SHADERS_NS::ShaderAttributeApplied appliedAttribs[MaxFontAttributes]; 

  // Uniform location for text scaling.
  GLint numTextCharsUniformLoc;

  // -- Functions --

  RenderFont()
  {
    vbo = 0;
    ibo = 0;
    attribSize = 0;
    stride = 0;
    renderType = RENDER_LINE;
    text[0] = '\0';
    numTextCharsUniformLoc = -1;
  }

  RenderFont(const char* inputText)
  {
    vbo = 0;
    ibo = 0;
    attribSize = 0;
    stride = 0;
    renderType = RENDER_LINE;
    setText(inputText);
    numTextCharsUniformLoc = -1;
  }

  void componentDestruct(uint64_t /*id*/)
  {
    // Destroy vbo and ibo.
    if (vbo != 0) GL(glDeleteBuffers(1, &vbo));
    if (ibo != 0) GL(glDeleteBuffers(1, &ibo));

    ibo = 0;
    vbo = 0;
  }

  void setText(const char* str)
  {
    size_t nameLen = std::strlen(str);
    if (nameLen < MaxStringSize - 1)
    {
      std::strncpy(text, str, MaxStringSize);

      // Destroy existing VBO's and IBO's. They will be recreated.
      if (isSetUp())
      {
        componentDestruct(0);
      }
    }
    else
    {
      std::cerr << "RenderFont: Unable to set string: " << str << " Name must be " 
                << (MaxStringSize - 1) << "characters or shorter." << std::endl;
    }
  }

  static const char* getName() {return "ren:RenderFont";}

  // Returns true if this simple geom instance has been appropriately setup
  // to handle rendering.
  bool isSetUp() const;

  // Constructs a new vbo, and ibo with the given font ID.
  void constructFontGeometry(const StaticFontMan& fontMan, uint64_t fontID);

  // Setup attributes for this font based off of the shader ID.
  void setupAttributesAndUniforms(GLuint shaderID);

  // Applies uniforms
  void applyUniforms() const;

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    // Shouldn't need to serialize these values. They are context specific.
    return true;
  }
};

} // namespace ren

#endif 
