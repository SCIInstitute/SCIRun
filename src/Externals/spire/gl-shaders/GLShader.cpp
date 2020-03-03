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


#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

/// author James Hughes
/// date   January 2014

#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <functional>
#include "GLShader.hpp"

namespace spire {

GLsizei calculateStride(const ShaderAttribute* array, size_t size);

/// Retrieves the number of components associated with a particular GL type.
size_t getSizeOfGLType(GLenum type);
size_t getNumComponentsOfGLType(GLenum type);
GLenum getBaseTypeOfGLType(GLenum type);
size_t getSizeOfBaseGLType(GLenum type);

GLuint loadShaderProgram(const std::list<ShaderSource>& shaders)
{
  GLuint program = glCreateProgram();
  GL_CHECK();
  if (0 == program)
  {
    // This usually indicates an invalid context.
    throw std::runtime_error("Unable to create GL program using glCreateProgram.");
    return 0;
  }

  // Vector of compiled shaders alongside a function to delete the program and
  // all shaders.
  std::vector<GLuint> compiledShaders;
  auto deleteShaders = [&]()
  {
    for (auto compShader = compiledShaders.begin(); compShader != compiledShaders.end(); ++compShader)
    {
      glDeleteShader(*compShader);
    }
  };
  auto deleteProgramAndShaders = [&]()
  {
    deleteShaders();
    glDeleteProgram(program);
  };

  // Compile all shaders.
  int idx = 0;
  for (auto it = shaders.begin(); it != shaders.end(); ++it)
  {
    GLuint shader = glCreateShader(it->mShaderType);
    GL_CHECK();
    if (0 == shader)
    {
      deleteProgramAndShaders();
      throw std::runtime_error("Failed to create shader using glCreateShader");
    }

    // Add shader to list now, so it will be removed via any call to
    // deleteProgramAndShaders.
    compiledShaders.push_back(shader);

    // Set the source and compile.
    std::string fullSource;
    for (const char* source : it->mSources)
    {
      fullSource += source;
    }
    const char* contents = fullSource.c_str();
    GL(glShaderSource(shader, 1, &contents, NULL));
    GL(glCompileShader(shader));

    // Check the compile status.
    GLint compiled;
    GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled));
    if (!compiled)
    {
      GLint infoLen = 0;

      GL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen));
      if (infoLen > 1)
      {
        char* infoLog = new char[infoLen];

        GL(glGetShaderInfoLog(shader, infoLen, NULL, infoLog));
        std::cerr << "Error compiling shader program with index " << idx << ":"
                  << std::endl << infoLog << std::endl;

        delete[] infoLog;
      }

      deleteProgramAndShaders();
      throw std::runtime_error("Failed to compile shader.");
    }

    // Attach the shader to the program
    GL(glAttachShader(program, shader));

    ++idx;
  }

  // Link program.
  GL(glLinkProgram(program));

	// Check the link status
	GLint linked;
	GL(glGetProgramiv(program, GL_LINK_STATUS, &linked));
	if (!linked)
	{
		GLint infoLen = 0;
		GL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen));

		if (infoLen > 1)
		{
      char* infoLog = new char[infoLen];

			GL(glGetProgramInfoLog(program, infoLen, NULL, infoLog));
      std::cerr << "Error linking program:" << std::endl;
      std::cerr << infoLog << std::endl;

      delete[] infoLog;
		}

    deleteProgramAndShaders();
    throw std::runtime_error("Failed to link shader.");
	}

  // Remove unnecessary compiled shaders.
  deleteShaders();

  return program;
}

std::vector<ShaderAttribute> getProgramAttributes(GLuint program)
{
  // Check the active attributes.
  GLint activeAttributes;
  GL(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &activeAttributes));

  std::vector<ShaderAttribute> attributes;
  const int maxAttribNameSize = GL_ACTIVE_ATTRIBUTE_MAX_LENGTH;
  char attributeName[maxAttribNameSize];
  for (int i = 0; i < activeAttributes; i++)
  {
    GLsizei charsWritten = 0;
    GLint attribSize;
    GLenum type;

    GL(glGetActiveAttrib(program, static_cast<GLuint>(i), maxAttribNameSize,
                         &charsWritten, &attribSize, &type, attributeName));

    GLint loc = glGetAttribLocation(program, attributeName);

    attributes.push_back(ShaderAttribute(attributeName, attribSize, type, loc));
  }

  return attributes;
}

void sortAttributesAlphabetically(std::vector<ShaderAttribute>& attribs)
{
  auto comparison = [](const ShaderAttribute& lhs, const ShaderAttribute& rhs)
  {
    return lhs.nameInCode < rhs.nameInCode;
  };
  std::sort(attribs.begin(), attribs.end(), comparison);
}

void bindAllAttributes(const ShaderAttribute* array, size_t size)
{
  GLsizei stride = calculateStride(array, size);
  size_t offset = 0;
  for (size_t i = 0; i < size; ++i)
  {
    GL(glEnableVertexAttribArray(static_cast<GLuint>(array[i].attribLoc)));
    GL(glVertexAttribPointer(static_cast<GLuint>(array[i].attribLoc),
                             array[i].numComps, array[i].baseType, array[i].normalize,
                             stride, reinterpret_cast<const void*>(offset)));
    offset += array[i].sizeBytes;
  }
}

void unbindAllAttributes(const ShaderAttribute* array, size_t size)
{
  for (size_t i = 0; i < size; ++i)
  {
    GL(glDisableVertexAttribArray(static_cast<GLuint>(array[i].attribLoc)));
  }
}

void bindSubsetAttributes(const ShaderAttribute* superset, size_t supersetSize,
                          const ShaderAttribute* subset, size_t subsetSize)
{
  if (supersetSize == subsetSize)
  {
    std::cerr << "bindSubsetAttributes: Warning - supersetSize == subsetSize\n";
    std::cerr << "When this equality holds, you should directly call bindAllAttributes\n";
    std::cerr << "instead of bindSubsetAttributes." << std::endl;
  }

  GLsizei stride = calculateStride(superset, supersetSize);
  size_t offset = 0;
  for (size_t i = 0; i < supersetSize; ++i)
  {
    int attribIndex = hasAttribute(subset, subsetSize, superset[i].nameInCode);
    if (attribIndex != -1)
    {
      GL(glEnableVertexAttribArray(static_cast<GLuint>(subset[attribIndex].attribLoc)));
      GL(glVertexAttribPointer(static_cast<GLuint>(subset[attribIndex].attribLoc),
                               subset[attribIndex].numComps, subset[attribIndex].baseType,
                               superset[i].normalize, stride,
                               reinterpret_cast<const void*>(offset)));
    }
    offset += superset[i].sizeBytes;
  }
}

void unbindSubsetAttributes(const ShaderAttribute* superset, size_t supersetSize,
                            const ShaderAttribute* subset, size_t subsetSize)
{
  for (size_t i = 0; i < supersetSize; ++i)
  {
    int attribIndex = hasAttribute(subset, subsetSize, superset[i].nameInCode);
    if (attribIndex != -1)
    {
      GL(glDisableVertexAttribArray(static_cast<GLuint>(subset[attribIndex].attribLoc)));
    }
  }
}

std::tuple<size_t, size_t> buildPreappliedAttrib(
    const ShaderAttribute* superset, size_t supersetSize,
    const ShaderAttribute* subset, size_t subsetSize,
    ShaderAttributeApplied* out, size_t outMaxSize)
{
  GLsizei stride = calculateStride(superset, supersetSize);
  size_t offset = 0;
  size_t appliedSize = 0;

  for (size_t i = 0; i < supersetSize; ++i)
  {
    int attribIndex = hasAttribute(subset, subsetSize, superset[i].nameInCode);
    if (attribIndex != -1)
    {
      if (appliedSize == outMaxSize)
      {
        std::cerr << "gl-shaders - buildPreAppliedAttrib: outMaxSize too small" << std::endl;
        throw std::runtime_error("outMaxSize too small.");
        return std::make_tuple(0,0);
      }

      out[appliedSize].attribLoc = subset[attribIndex].attribLoc;
      out[appliedSize].baseType  = subset[attribIndex].baseType;
      out[appliedSize].numComps  = subset[attribIndex].numComps;
      out[appliedSize].normalize = superset[i].normalize;
      out[appliedSize].offset    = offset;

      ++appliedSize;
    }
    offset += superset[i].sizeBytes;
  }

  return std::make_tuple(appliedSize, stride);
}



void bindPreappliedAttrib(const ShaderAttributeApplied* array, size_t size, size_t stride)
{
  for (size_t i = 0; i < size; ++i)
  {
    GL(glEnableVertexAttribArray(static_cast<GLuint>(array[i].attribLoc)));
    GL(glVertexAttribPointer(static_cast<GLuint>(array[i].attribLoc),
                             array[i].numComps, array[i].baseType, array[i].normalize,
                             stride, reinterpret_cast<const void*>(array[i].offset)));
  }
}

void unbindPreappliedAttrib(const ShaderAttributeApplied* array, size_t size)
{
  for (size_t i = 0; i < size; ++i)
  {
    GL(glDisableVertexAttribArray(static_cast<GLuint>(array[i].attribLoc)));
  }
}

GLsizei calculateStride(const ShaderAttribute* array, size_t size)
{
  // Calculate the stride if it is not already given to us.
  GLsizei stride = 0;
  for (size_t i = 0; i < size; ++i)
  {
    stride += array[i].sizeBytes;
  }

  return stride;
}

std::vector<ShaderUniform> getProgramUniforms(GLuint program)
{
  GLint activeUniforms;
  GL(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms));

  std::vector<ShaderUniform> uniforms;
  const int maxUniformNameSize = GL_ACTIVE_UNIFORM_MAX_LENGTH;
  char uniformName[maxUniformNameSize];
  for (int i = 0; i < activeUniforms; i++)
  {
    GLsizei charsWritten = 0;
    GLint uniformSize;
    GLenum type;

    GL(glGetActiveUniform(program, static_cast<GLuint>(i), maxUniformNameSize,
                          &charsWritten, &uniformSize, &type, uniformName));

    GLint loc = glGetUniformLocation(program, uniformName);

    uniforms.push_back(ShaderUniform(uniformName, uniformSize, type, loc));
  }

  return uniforms;
}

int hasAttribute(const ShaderAttribute* array, size_t size, const std::string& name)
{
  for (size_t i = 0; i < size; ++i)
  {
    if (array[i].nameInCode == name)
    {
      return i;
    }
  }

  return -1;
}

ShaderAttribute::ShaderAttribute() :
    size(0),
    sizeBytes(0),
    type(GL_FLOAT),
    attribLoc(0),
    normalize(0),
    nameInCode("")
{}

ShaderAttribute::ShaderAttribute(const std::string& name, GLint s, GLenum t,
                                 GLint loc, GLboolean norm) :
    size(s),
    sizeBytes(0),
    type(t),
    attribLoc(loc),
    normalize(norm),
    nameInCode(name)
{
  sizeBytes = getSizeOfGLType(t) * s;
  baseType = getBaseTypeOfGLType(t);
  numComps = getNumComponentsOfGLType(t) * s;
}

bool operator==(const ShaderAttribute& a, const ShaderAttribute& b)
{
  // We do not compare size or type because these will vary based on
  // compiler. We normalize to a baseType, size in bytes, and number of
  // components. We only want to know if the attribute is the same in the
  // sense that we can substitute the underlying data.
  return //(a.size == b.size)
      (a.sizeBytes == b.sizeBytes)
      //&& (a.type == b.type)
      //&& (a.attribLoc == b.attribLoc)
      //&& (a.normalize == b.normalize)
      && (a.baseType == b.baseType)
      && (a.numComps == b.numComps)
      && (a.nameInCode == b.nameInCode);
}

bool operator!=(const ShaderAttribute& a, const ShaderAttribute& b)
{
  return !(a == b);
}

ShaderUniform::ShaderUniform(const std::string& name, GLint s, GLenum t, GLint loc) :
    size(s),
    type(t),
    uniformLoc(loc),
    nameInCode(name)
{}

bool operator==(const ShaderUniform& a, const ShaderUniform& b)
{
  return (a.size == b.size)
      && (a.type == b.type)
      //&& (a.uniformLoc == b.uniformLoc)
      && (a.nameInCode == b.nameInCode);
}

bool operator!=(const ShaderUniform& a, const ShaderUniform& b)
{
  return !(a == b);
}

size_t getSizeOfGLType(GLenum type)
{
  size_t numComponents = getNumComponentsOfGLType(type);
  GLenum baseType      = getBaseTypeOfGLType(type);
  size_t baseTypeSize  = getSizeOfBaseGLType(baseType);
  return numComponents * baseTypeSize;
}

size_t getNumComponentsOfGLType(GLenum type)
{
  switch (type)
  {
    case GL_FLOAT: return 1;
    case GL_FLOAT_VEC2: return 2;
    case GL_FLOAT_VEC3: return 3;
    case GL_FLOAT_VEC4: return 4;
    case GL_FLOAT_MAT2: return 2 * 2;
    case GL_FLOAT_MAT3: return 3 * 3;
    case GL_FLOAT_MAT4: return 4 * 4;

#ifdef GL_FLOAT_MAT2X3
    case GL_FLOAT_MAT2X3: return 2 * 3;
    case GL_FLOAT_MAT2X4: return 2 * 4;
    case GL_FLOAT_MAT3X2: return 3 * 2;
    case GL_FLOAT_MAT3X4: return 3 * 4;
    case GL_FLOAT_MAT4X2: return 4 * 2;
    case GL_FLOAT_MAT4X3: return 4 * 3;
#endif

    case GL_INT: return 1;
#ifdef GL_INT_VEC2
    case GL_INT_VEC2: return 2;
    case GL_INT_VEC3: return 3;
    case GL_INT_VEC4: return 4;
#endif
    case GL_UNSIGNED_INT: return 1;

#ifdef GL_UNSIGNED_INT_VEC2
    case GL_UNSIGNED_INT_VEC2: return 2;
    case GL_UNSIGNED_INT_VEC3: return 3;
    case GL_UNSIGNED_INT_VEC4: return 4;
#endif

    case GL_DOUBLE: return 1;
#ifdef GL_DOUBLE_VEC2
    case GL_DOUBLE_VEC2: return 2;
    case GL_DOUBLE_VEC3: return 3;
    case GL_DOUBLE_VEC4: return 4;
    case GL_DOUBLE_MAT2: return 2 * 2;
    case GL_DOUBLE_MAT3: return 3 * 3;
    case GL_DOUBLE_MAT4: return 4 * 4;
#endif

#ifdef GL_DOUBLE_MAT2X3
    case GL_DOUBLE_MAT2X3: return 2 * 3;
    case GL_DOUBLE_MAT2X4: return 2 * 4;
    case GL_DOUBLE_MAT3X2: return 3 * 2;
    case GL_DOUBLE_MAT3X4: return 3 * 4;
    case GL_DOUBLE_MAT4X2: return 4 * 2;
    case GL_DOUBLE_MAT4x3: return 4 * 3;
#endif

    default:
      // Most of the other cases not covered are going to be one element long.
      return 1;
  }
}

GLenum getBaseTypeOfGLType(GLenum type)
{
  switch (type)
  {
    case GL_FLOAT:
    case GL_FLOAT_VEC2:
    case GL_FLOAT_VEC3:
    case GL_FLOAT_VEC4:
    case GL_FLOAT_MAT2:
    case GL_FLOAT_MAT3:
    case GL_FLOAT_MAT4:
#ifdef GL_FLOAT_MAT2X3
    case GL_FLOAT_MAT2X3:
    case GL_FLOAT_MAT2X4:
    case GL_FLOAT_MAT3X2:
    case GL_FLOAT_MAT3X4:
    case GL_FLOAT_MAT4X2:
    case GL_FLOAT_MAT4X3:
#endif
      return GL_FLOAT;

    case GL_BYTE:           return GL_BYTE;
    case GL_UNSIGNED_BYTE:  return GL_UNSIGNED_BYTE;
    case GL_SHORT:          return GL_SHORT;
    case GL_UNSIGNED_SHORT: return GL_UNSIGNED_SHORT;

    case GL_INT:
#ifdef GL_INT_VEC2
    case GL_INT_VEC2:
    case GL_INT_VEC3:
    case GL_INT_VEC4:
#endif
      return GL_INT;

    case GL_UNSIGNED_INT:
#ifdef GL_UNSIGNED_INT_VEC2
    case GL_UNSIGNED_INT_VEC2:
    case GL_UNSIGNED_INT_VEC3:
    case GL_UNSIGNED_INT_VEC4:
#endif
      return GL_UNSIGNED_INT;

    case GL_DOUBLE:
#ifdef GL_DOUBLE_VEC2
    case GL_DOUBLE_VEC2:
    case GL_DOUBLE_VEC3:
    case GL_DOUBLE_VEC4:
    case GL_DOUBLE_MAT2:
    case GL_DOUBLE_MAT3:
    case GL_DOUBLE_MAT4:
#endif

#ifdef GL_DOUBLE_MAT2X3
    case GL_DOUBLE_MAT2X3:
    case GL_DOUBLE_MAT2X4:
    case GL_DOUBLE_MAT3X2:
    case GL_DOUBLE_MAT3X4:
    case GL_DOUBLE_MAT4X2:
    case GL_DOUBLE_MAT4x3:
      return GL_DOUBLE;
#endif

    default:
      // Always default to a GL_FLOAT
      std::cerr << "Warning - getBaseTypeOfGLType: unrecognized GL type: " << type << std::endl;
      return GL_FLOAT;
  }
}

size_t getSizeOfBaseGLType(GLenum type)
{
  switch (type)
  {
    case GL_FLOAT:          return sizeof(GLfloat);
    case GL_BYTE:           return 1;
    case GL_UNSIGNED_BYTE:  return 1;
    case GL_SHORT:          return 2;
    case GL_UNSIGNED_SHORT: return 2;
    case GL_DOUBLE:         return sizeof(GLdouble);
    case GL_INT:            return sizeof(GLint);
    case GL_UNSIGNED_INT:   return sizeof(GLuint);
    default:                return sizeof(GLfloat);
  }
}

} // namespace spire
