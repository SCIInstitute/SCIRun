#ifndef IAUNS_ES_RENDER_UTIL_SHADER_HPP
#define IAUNS_ES_RENDER_UTIL_SHADER_HPP

#include <es-cereal/CerealCore.hpp>
#include <gl-shaders/GLShader.hpp>

#include "../comp/StaticVBOMan.hpp"
#include "../VBOMan.hpp"

namespace ren {

void addShaderVSFS(CPM_ES_CEREAL_NS::CerealCore& core, uint64_t entityID,
                   const std::string& shader);

// Class to help construct attributes given a VBO and a shader.
// NOTE: The given VBOID *must* be present in the static VBO man.
// Otherwise this class will not know how to compare the shader attributes
// and the VBO attributes.
template <int MaxNumAttributes = 5>
class ShaderVBOAttribs
{
public:
  ShaderVBOAttribs() : mAttribSize(-1), mStride(0) {}
  ShaderVBOAttribs(GLuint vboID, GLuint shaderID, const StaticVBOMan& vboMan)
  {
    setup(vboID, shaderID, vboMan);
  }
  ShaderVBOAttribs(GLuint vboID, GLuint shaderID, const VBOMan& vboMan)
  {
    setup(vboID, shaderID, vboMan);
  }

  /// Bind pre-applied attributes setup with the 'setup' function.
  void bind() const
  {
    if (isSetup()) {
      CPM_GL_SHADERS_NS::bindPreappliedAttrib(mAppliedAttribs,
                                    static_cast<size_t>(mAttribSize), mStride);
    } else {
      std::cerr << "Attempted to bind uninitialized attributes!" << std::endl;
    }
  }

  /// Unbind pre-applied atttributes.
  void unbind() const
  {
    if (isSetup()) {
      CPM_GL_SHADERS_NS::unbindPreappliedAttrib(mAppliedAttribs, 
                                      static_cast<size_t>(mAttribSize));
    } else {
      std::cerr << "Attempted to unbind unitialized attributes!" << std::endl;
    }

  }

  /// Sets up this class 'ShaderVBOAttribs' such that it attributes can be
  /// applied before rendering.
  void setup(GLuint vboID, GLuint shaderID, const StaticVBOMan& vboMan)
  {
    setup(vboID, shaderID, *(vboMan.instance_));
  }

  /// Sets up this class 'ShaderVBOAttribs' such that it attributes can be
  /// applied before rendering.
  void setup(GLuint vboID, GLuint shaderID, const VBOMan& vboMan)
  {
    /// NOTE: If this statement proves to be a performance problem (because
    ///       we are looking up the shader's attributes using OpenGL), then
    ///       we can cache the attributes in the shader manager and import
    ///       them using the StaticShaderMan component.
    std::vector<CPM_GL_SHADERS_NS::ShaderAttribute> attribs = 
        CPM_GL_SHADERS_NS::getProgramAttributes(shaderID);
    CPM_GL_SHADERS_NS::sortAttributesAlphabetically(attribs);

    // Lookup the VBO and its attributes by GL id.
    std::vector<CPM_GL_SHADERS_NS::ShaderAttribute> vboAttribs =
        vboMan.getVBOAttributes(vboID);

    if (vboAttribs.size() < attribs.size())
    {
      std::cerr << "ren::RenderSimpleGeom: Unable to satisfy shader! Not enough attributes." << std::endl;
    }

    // Use the shader and VBO data to construct an applied state.
    std::tuple<size_t, size_t> sizes = buildPreappliedAttrib(
        &vboAttribs[0], vboAttribs.size(),
        &attribs[0], attribs.size(),
        mAppliedAttribs, MaxNumAttributes
        );

    mAttribSize = static_cast<int>(std::get<0>(sizes));
    mStride = std::get<1>(sizes);
  }

  /// Returns true if the shader and VBO attributes have been merged together
  /// into our appliedAttributes array.
  bool isSetup() const
  {
    return (mAttribSize != -1);
  }

  void setAsUninitialized()
  {
    mAttribSize = -1;
  }

private:
  int     mAttribSize;   ///< How many elements in 'appliedAttribs' are valid.
  size_t  mStride;       ///< Stride between elements in the vertex buffer.

  CPM_GL_SHADERS_NS::ShaderAttributeApplied mAppliedAttribs[MaxNumAttributes];
};

} // namespace ren

#endif 
