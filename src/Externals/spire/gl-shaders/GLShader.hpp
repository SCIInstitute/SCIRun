/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


/// author James Hughes
/// date   January 2014

#ifndef SPIRE_GLSHADER_HPP
#define SPIRE_GLSHADER_HPP

#include <es-log/trace-log.h>
// All functions below assume there is a valid OpenGL context active.
#include <vector>
#include <list>
#include <tuple>
#include <cstdint>
#include <gl-platform/GLPlatform.hpp>
#include <spire/scishare.h>

namespace spire {

struct SCISHARE ShaderSource
{
  /// \p source     Sources for the shader. Input directly into 'glShaderSource'.
  /// \p shaderType One of: GL_COMPUTE_SHADER, GL_VERTEX_SHADER,
  ///               GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER,
  ///               GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER
  ShaderSource(const std::vector<const char*>& sources, GLenum shaderType) :
      mSources(sources),
      mShaderType(shaderType)
  {}

  std::vector<const char*>  mSources;
  GLenum                    mShaderType;
};

/// Mimics glCreateProgram. Loads, compiles, and links shaders then returns the
/// OpenGL ID. Returns 0 if an error occured. Uses std::cerr to output any
/// important information regarding errors.
GLuint loadShaderProgram(const std::list<ShaderSource>& shaders);

struct SCISHARE ShaderAttribute
{
  ShaderAttribute();

  /// \param name       Name of the attribute in code.
  /// \param s          Size of the attributes in units of type (t).
  /// \param t          OpenGL type of the attribute.
  /// \param loc        Location of the attribute in the shader (set to 0 if this is
  ///                   VBO attribute list)..
  /// \param normalize  If 1, then this attribute will be normalized between 0-1.
  ///                   Only used if this is a VBO attribute list.
  ShaderAttribute(const std::string& name, GLint s, GLenum t, GLint loc = 0,
                  GLboolean normalize = 0);

  GLint     size;       ///< Size of attribute, in units of 'type'.
  size_t    sizeBytes;  ///< Size of the attribute, in bytes. Calculated in constructor.
  GLenum    type;       ///< GL type.
  GLint     attribLoc;  ///< Location as returned by glGetAttribLocation.
  GLboolean normalize;  ///< Always set to false. This is not data coming from
                        ///< OpenGL, it is only useful in the context of
                        ///< sending attributes to OpenGL. With this, we have
                        ///< all we need to call glVertexAttribPointer.

  // The following variables are calculated for you in the constructor.
  GLenum    baseType;   ///< Base GL type.
  int       numComps;   ///< Number of components in the GL base type.

  std::string nameInCode; ///< name of the attribute in-code.
};

bool operator==(const ShaderAttribute& a, const ShaderAttribute& b);
bool operator!=(const ShaderAttribute& a, const ShaderAttribute& b);

/// Determines if the given attribute array has the attribute with 'name'.
/// Will perform a strcmp on every string (nameInCode).
/// \return -1 if no attribute exists, otherwise this returns the index to
///         the attribute.
int hasAttribute(const ShaderAttribute* array, size_t size, const std::string& name);

/// Collects all shader attributes into a vector of ShaderAttribute.
SCISHARE std::vector<ShaderAttribute> getProgramAttributes(GLuint program);

/// Sorts a vector of shader attributes alphabetically by 'nameInCode'.
SCISHARE void sortAttributesAlphabetically(std::vector<ShaderAttribute>& attribs);

/// Binds all attributes in given ShaderAttribute array.
/// Note: Be sure to set the normalize ShaderAttribute variable appropriately.
void bindAllAttributes(const ShaderAttribute* array, size_t size);

/// Unbinds all attributes as bound by bindAllAttributes.
void unbindAllAttributes(const ShaderAttribute* array, size_t size);

/// Binds all of \p subset's attributes, using superset as the guide that
/// determines the stride between attributes. Use this when a VBO has more
/// attributes than the shader accepts. This function is less efficient than
/// bindAllAttributes above. If supersetSize == subsetSize, this function
/// will print a warning, then proceed. If it finds unsatsified attributes,
/// an exception will be thrown.
/// Note: Be sure to set the normalize ShaderAttribute variable appropriately.
void bindSubsetAttributes(const ShaderAttribute* superset, size_t supersetSize,
                          const ShaderAttribute* subset, size_t subsetSize);

/// Unbind all attributes bound by bindSubsetAttributes.
void unbindSubsetAttributes(const ShaderAttribute* superset, size_t supersetSize,
                            const ShaderAttribute* subset, size_t subsetSize);

/// Minimal structure based on the intersection between shader and VBO.
struct SCISHARE ShaderAttributeApplied
{
  GLint       attribLoc;    ///< Attribute location from the shader.
  GLenum      baseType;     ///< Base OpenGL type of the attribute.
  GLint       numComps;     ///< Number of components of type \p baseType.
  GLboolean   normalize;    ///< Taken from the VBO's attribute list.
  uint32_t    offset;       ///< Calculated offset into VBO's memory.
};

/// Builds a sequence of applied attributes. Use this to set set up a VBO for
/// rendering with a particular shader. If an error occurs, a runtime exception
/// will be thrown, but the tuple <0,0> will be returned on systems that do not
/// use exceptions.
/// \param superset     The set that specifies the stride of the buffer.
///                     Commonly corresponds to the VBO's attribute list.
/// \param supersetSize The size of \p superset.
/// \param subset       The subset that determines the attribute location.
///                     Commonly corresponds to the shader's attribute list.
/// \param subsetSize   The size of \p subset.
/// \param out          Array that will be modified with the appropriate binding
///                     values.
/// \param outMaxSize   Maximum size of \p out. An exception is thrown if this
///                     is not latge enough to hold all attributes.
/// \return The first parameter of the tuple is the resultant size of the
///         modified ShaderAtributeApplied array. The second is the stride of
///         all components combined together.
/// \note  *ONLY* the following attributes are used inside of super set (the
///         rest are ignored: nameInCode, sizeBytes, and normalize.
SCISHARE std::tuple<size_t, size_t> buildPreappliedAttrib(
    const ShaderAttribute* superset, size_t supersetSize,
    const ShaderAttribute* subset, size_t subsetSize,
    ShaderAttributeApplied* out, size_t outMaxSize);

/// Binds shader attributes based off of the intersection of a superset and
/// subset as calculated prior by buildPreAppliedAttrib. This function is more
/// efficient and cache friendly than bindAllAttributes or bindSubsetAttributes.
/// \param array  \p out from buildPreAppliedAttrib.
/// \param size   First tuple parameter from buildPreAppliedAttrib.
/// \param stride Second tuple parameter from buildPreAppliedAttrib.
SCISHARE void bindPreappliedAttrib(const ShaderAttributeApplied* array, size_t size,
                          size_t stride);

/// Unbind all attributes bound in bindPreappliedAttrib.
SCISHARE void unbindPreappliedAttrib(const ShaderAttributeApplied* array, size_t size);

/// Generic structure for holding a shader uniform.
struct SCISHARE ShaderUniform
{
  ShaderUniform(const std::string& name, GLint s, GLenum t, GLint loc);

  GLint       size;         ///< Size of uniform, in units of 'type'.
  GLenum      type;         ///< GL type.
  GLint       uniformLoc;   ///< Location as returned by glGetUniformLocation.
  std::string nameInCode;   ///< Name of the uniform in-code.
};

bool operator==(const ShaderUniform& a, const ShaderUniform& b);
bool operator!=(const ShaderUniform& a, const ShaderUniform& b);

/// Collects all shader uniforms into a vector of ShaderUniform.
SCISHARE std::vector<ShaderUniform> getProgramUniforms(GLuint program);

} // namespace spire

#endif
