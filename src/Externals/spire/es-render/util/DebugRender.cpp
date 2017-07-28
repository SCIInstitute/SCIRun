

#include <glm/glm.hpp>
#include <gl-shaders/GLShader.hpp>

#include <glm/gtc/type_ptr.hpp>           // glm::value_ptr

#include <es-general/comp/StaticCamera.hpp>

#include "es-render/util/Lines.hpp"
#include "DebugRender.hpp"

namespace ren {

DebugRender::DebugRender() :
    mLines(0),
    mCurCircum(0),
    mColorUniform("uColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), VecUniform::VEC4),
    mShaderID(0)
{}

DebugRender::DebugRender(CPM_ES_CEREAL_NS::CerealCore& core) :
    mLines(0),
    mCurCircum(0),
    mColorUniform("uColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), VecUniform::VEC4)
{
  reaquireShader(core);
}
  
void DebugRender::clear()
{
  mLines = 0;
  mCurCircum = 0;
  mVBO.clear();
}

void DebugRender::finalizeClosedObject()
{
  addLine(mFirstPoint, mLastPoint);
}

void DebugRender::reaquireShader(CPM_ES_CEREAL_NS::CerealCore& core)
{
  mShaderID = getColorLineShader(core);

  mColorUniform.checkUniform(mShaderID);
  mCommonUniforms.checkUniformArray(mShaderID);
}

void DebugRender::addLine(const glm::vec3& a, const glm::vec3& b)
{
  if (mVBO.getBufferSize() == 0)
    mFirstPoint = a;

  mVBO.write<float>(a.x);
  mVBO.write<float>(a.y);
  mVBO.write<float>(a.z);
  mVBO.write<float>(mCurCircum);

  float length = glm::length(a - b);
  mCurCircum += length;

  mVBO.write<float>(b.x);
  mVBO.write<float>(b.y);
  mVBO.write<float>(b.z);
  mVBO.write<float>(mCurCircum);

  ++mLines;
  mLastPoint = b;
}

void DebugRender::render(const glm::mat4& trafo,
                         const gen::StaticCameraData& data,
                         float globalTime, const glm::vec4& color,
                         bool triangleFan)
{
  GL(glUseProgram(mShaderID));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  mColorUniform.uniform = color;
  mColorUniform.applyUniform();
  mCommonUniforms.applyCommonUniforms(trafo, data, globalTime);

  char* lineMemory = mVBO.getBuffer();

  /// \todo Make the following glVertexAttribPointer more robust! It isn't
  ///       necessarily pointing to the correct attribute!

	int stride = sizeof(float) * 3 + sizeof(float);

#ifdef EMSCRIPTEN
  // WebGL specific functionality!
  // Emscripten can use glDrawArrays, but only in OpenGL ES emulation mode.
  // And even then, it goes ahead and creates the buffer for you anyways.
  // Since this is just a debugging aid, we do the ultra slow thing of
  // generating a new buffer for every object render call.
  GLuint arrayBuffer, elementBuffer;
  glGenBuffers(1, &arrayBuffer);
  glGenBuffers(1, &elementBuffer);

  glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
  glBufferData(GL_ARRAY_BUFFER, mVBO.getBufferSize(), lineMemory, GL_STATIC_DRAW);

  const int elementBufferSize = 256;
  if (mLines < elementBufferSize / 2)
  {
    // Generate element data manually...
    uint16_t elementArray[elementBufferSize];

    if (triangleFan == false)
    {
      for (int i = 0; i < mLines; i++)
      {
        elementArray[i*2]   = i*2;
        elementArray[i*2+1] = i*2+1;
      }

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint64_t) * mLines * 2, elementArray, GL_STATIC_DRAW);
    }
    else
    {
      for (int i = 0; i < mLines * 2; i++)
      {
        elementArray[i] = i;
      }

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint64_t) * mLines * 2, elementArray, GL_STATIC_DRAW);
    }

    glBindAttribLocation(mShaderID, 0, "aPos");
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    if (triangleFan == false)
      glDrawElements(GL_LINES, mLines * 2, GL_UNSIGNED_SHORT, 0);
    else
      glDrawElements(GL_TRIANGLE_FAN, mLines * 2, GL_UNSIGNED_SHORT, 0);
  }
  else
  {
    std::cerr << "DebugRender: Element buffer not large enough." << std::endl;
  }

  glDeleteBuffers(1, &arrayBuffer);
  glDeleteBuffers(1, &elementBuffer);
#else
  glBindAttribLocation(mShaderID, 0, "aPos");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, lineMemory);
	glEnableVertexAttribArray(0);

  // We don't specify this attribute because it is not used in the shader yet.
	// lineMemory += sizeof(float) * 3;
	// glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, lineMemory);
	// glEnableVertexAttribArray(1);

  if (triangleFan == false)
		glDrawArrays(GL_LINES, 0, mLines * 2);
	else
		glDrawArrays(GL_TRIANGLE_FAN, 0, mLines * 2);
#endif

	glDisableVertexAttribArray(0);
}


}
