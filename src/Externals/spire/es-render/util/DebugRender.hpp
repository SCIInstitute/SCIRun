#ifndef SPIRE_ES_RENDER_UTIL_DEBUG_RENDER_HPP
#define SPIRE_ES_RENDER_UTIL_DEBUG_RENDER_HPP

#include <es-log/trace-log.h>
#include <var-buffer/VarBuffer.hpp>
#include <es-cereal/CerealCore.hpp>

#include "../comp/VecUniform.hpp"
#include "../comp/CommonUniforms.hpp"
#include <spire/scishare.h>

// Simple class to help render lines and triangle fans. Not made for speed,
// just for debugging purposes.
namespace ren {

// Class used for debug rendering of geometry. Primarily used for the physics
// engine. The VBO has aPos (3 floats) and aLineCircum (1 float).
class DebugRender
{
public:

  /// If you use this constructor, be sure to call reaquire shader in order
  /// to setup the correct shader.
  DebugRender();

  // The constructor looks up the shader that will be used for rendering lines.
  explicit DebugRender(spire::CerealCore& core);

  // Clear
  void clear();

  // Reacquires the shader. You can do this instead of recreating the class
  // every frame and save some dynamic memory allocation. You would generally
  // reaquire the shader every frame because we have no way of knowning whether
  // a GC cycle was run and GCed our shader.
  void reacquireShader(spire::CerealCore& core);

  // Adds a line to the debug renderer.
  void addLine(const glm::vec3& a, const glm::vec3& b);

  // Finalizes a closed object by inserting a line between the first and
  // the last point added.
  void finalizeClosedObject();

  // Renders the added set of lines as a series of lines.
  void render(const glm::mat4& trafo,
              const gen::StaticCameraData& data,
              float globalTime, const glm::vec4& color,
              bool triangleFan);

private:

  int   mLines;     ///< Number of lines.
  float mCurCircum; ///< Current cicumfrence.

  glm::vec3 mFirstPoint;
  glm::vec3 mLastPoint;

  GLuint mShaderID;

  spire::VarBuffer  mVBO;
  VecUniform      mColorUniform;
  CommonUniforms  mCommonUniforms;
};

}

#endif
