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

#include <cmath>
#include "GLState.hpp"

namespace spire {

//------------------------------------------------------------------------------
GLState::GLState() :
    mDepthTestEnable(true),
    mDepthFunc(GL_LESS),
    mCullFace(GL_BACK),
    mCullFaceEnable(false),
    mCullFrontFace(GL_CCW),
    mBlendEnable(true),
    mBlendEquation(GL_FUNC_ADD),
    mBlendFuncSrc(GL_SRC_ALPHA),
    mBlendFuncDst(GL_ONE_MINUS_SRC_ALPHA),
    mDepthMask(GL_TRUE),
    mColorMaskRed(GL_TRUE),
    mColorMaskGreen(GL_TRUE),
    mColorMaskBlue(GL_TRUE),
    mColorMaskAlpha(GL_TRUE),
    mLineWidth(2.0f),
    mTexActiveUnit(GL_TEXTURE0)
{
}

// Don't use this function in general, there are a lot reasons why it fails.
// For our purposes, it will be fine for checking line widths.
bool areFloatSame(float a, float b)
{
    return std::fabs(a - b) < 0.0001;
}

//------------------------------------------------------------------------------
bool GLState::operator==(const GLState &o) const
{
  return
      (   mDepthTestEnable    == o.mDepthTestEnable
       && mDepthFunc          == o.mDepthFunc
       && mCullFace           == o.mCullFace
       && mCullFaceEnable     == o.mCullFaceEnable
       && mCullFrontFace      == o.mCullFrontFace
       && mBlendEnable        == o.mBlendEnable
       && mBlendEquation      == o.mBlendEquation
       && mBlendFuncSrc       == o.mBlendFuncSrc
       && mBlendFuncDst       == o.mBlendFuncDst
       && mDepthMask          == o.mDepthMask
       && mColorMaskRed       == o.mColorMaskRed
       && mColorMaskGreen     == o.mColorMaskGreen
       && mColorMaskBlue      == o.mColorMaskBlue
       && mColorMaskAlpha     == o.mColorMaskAlpha
       && areFloatSame(mLineWidth, o.mLineWidth)
       && mTexActiveUnit      == o.mTexActiveUnit);

}

//------------------------------------------------------------------------------
size_t GLState::getMaxTextureUnits() const
{
  GLint tmp;
#ifdef GL_STATE_ES_2
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &tmp);
#else
  glGetIntegerv(GL_MAX_TEXTURE_UNITS, &tmp);
#endif

  return static_cast<size_t>(tmp);
}

//------------------------------------------------------------------------------
std::string GLState::getStateDescription()
{
  // Textual description of state.
  return "";
}

//------------------------------------------------------------------------------
void GLState::apply() const
{
  applyStateInternal(true, nullptr);
}

//------------------------------------------------------------------------------
void GLState::applyRelative(const GLState& state) const
{
  applyStateInternal(false, &state);
}

//------------------------------------------------------------------------------
void GLState::applyStateInternal(bool force, const GLState* state) const
{
  // Directly apply entire OpenGL state.
  applyDepthTestEnable(force, state);
  applyDepthFunc(force, state);
  applyCullFace(force, state);
  applyCullFaceEnable(force, state);
  applyFrontFace(force, state);
  applyBlendEnable(force, state);
  applyBlendEquation(force, state);
  applyBlendFunction(force, state);
  applyDepthMask(force, state);
  applyColorMask(force, state);
  applyLineWidth(force, state);
  applyActiveTexture(force, state);
}

//------------------------------------------------------------------------------
void GLState::readStateFromOpenGL()
{
  GL_CHECK();

  GLint e;
  glGetIntegerv(GL_DEPTH_FUNC, &e);
  mDepthTestEnable    = glIsEnabled(GL_DEPTH_TEST) != 0;
  mDepthFunc          = static_cast<GLenum>(e);

  glGetIntegerv(GL_CULL_FACE_MODE, &e);
  mCullFace           = static_cast<GLenum>(e);
  mCullFaceEnable     = glIsEnabled(GL_CULL_FACE) != 0;

  glGetIntegerv(GL_FRONT_FACE, &e);
  mCullFrontFace      = static_cast<GLenum>(e);

  mBlendEnable = glIsEnabled(GL_BLEND) != 0;

  GL(glGetIntegerv(GL_BLEND_EQUATION_RGB, &e));
  mBlendEquation = static_cast<GLenum>(e);

  GLint src, dest;
#ifdef GL_STATE_ES_2
  glGetIntegerv(GL_BLEND_SRC_RGB, &src);
  glGetIntegerv(GL_BLEND_DST_RGB, &dest);
#else
  glGetIntegerv(GL_BLEND_SRC, &src);
  glGetIntegerv(GL_BLEND_DST, &dest);
#endif
  mBlendFuncSrc = static_cast<GLenum>(src);
  mBlendFuncDst = static_cast<GLenum>(dest);

  GLboolean	b;
  glGetBooleanv(GL_DEPTH_WRITEMASK, &b);
  mDepthMask = (b != 0);

  GLboolean	 col[4];
  glGetBooleanv(GL_COLOR_WRITEMASK, col);
  mColorMaskRed   = col[0];
  mColorMaskGreen = col[1];
  mColorMaskBlue  = col[2];
  mColorMaskAlpha = col[3];

  // Line width
  GLfloat f;
  glGetFloatv(GL_LINE_WIDTH, &f);
  mLineWidth = f;

  // Active texture unit
  glGetIntegerv(GL_ACTIVE_TEXTURE, &e);
  mTexActiveUnit = e;
}


//------------------------------------------------------------------------------
void GLState::applyDepthTestEnable(bool force, const GLState* cur) const
{
  if (force || (cur && cur->mDepthTestEnable != mDepthTestEnable))
  {
    if (mDepthTestEnable)
      GL(glEnable(GL_DEPTH_TEST));
    else
      GL(glDisable(GL_DEPTH_TEST));
  }
}

//------------------------------------------------------------------------------
void GLState::applyDepthFunc(bool force, const GLState* cur) const
{
  if (force || (cur && cur->mDepthFunc != mDepthFunc))
  {
    GL(glDepthFunc(mDepthFunc));
  }
}

//------------------------------------------------------------------------------
void GLState::applyCullFace(bool force, const GLState* cur) const
{
  if (force || (cur && cur->mCullFace != mCullFace))
  {
    GL(glCullFace(mCullFace));
  }
}

//------------------------------------------------------------------------------
void GLState::applyCullFaceEnable(bool force, const GLState* cur) const
{
  if (force || (cur && cur->mCullFaceEnable != mCullFaceEnable))
  {
    if (mCullFaceEnable)
      GL(glEnable(GL_CULL_FACE));
    else
      GL(glDisable(GL_CULL_FACE));
  }
}

//------------------------------------------------------------------------------
void GLState::applyFrontFace(bool force, const GLState* cur) const
{
  if (force || (cur && cur->mCullFrontFace != mCullFrontFace))
  {
    GL(glFrontFace(mCullFrontFace));
  }
}

//------------------------------------------------------------------------------
void GLState::applyBlendEnable(bool force, const GLState* cur) const
{
  if (force || (cur && cur->mBlendEnable != mBlendEnable))
  {
    if (mBlendEnable)
      GL(glEnable(GL_BLEND));
    else
      GL(glDisable(GL_BLEND));
  }
}

//------------------------------------------------------------------------------
void GLState::applyBlendEquation(bool force, const GLState* cur) const
{
  if (force || (cur && cur->mBlendEquation != mBlendEquation))
  {
    GL(glBlendEquation(mBlendEquation));
  }
}

//------------------------------------------------------------------------------
void GLState::applyBlendFunction(bool force, const GLState* cur) const
{
  if (   force
      || (cur && cur->mBlendFuncSrc != mBlendFuncSrc)
      || (cur && cur->mBlendFuncDst != mBlendFuncDst))
  {
    GL(glBlendFunc(mBlendFuncSrc, mBlendFuncDst));
  }
}

//------------------------------------------------------------------------------
void GLState::applyDepthMask(bool force, const GLState* cur) const
{
  if (force || (cur && cur->mDepthMask != mDepthMask))
  {
    GL(glDepthMask(mDepthMask));
  }
}

//------------------------------------------------------------------------------
void GLState::applyLineWidth(bool force, const GLState* cur) const
{
  if (force || (cur && cur->mLineWidth != mLineWidth))
  {
    GL(glLineWidth(mLineWidth));
  }
}


//------------------------------------------------------------------------------
void GLState::applyColorMask(bool force, const GLState* cur) const
{
  if (   force
      || (cur && cur->mColorMaskRed != mColorMaskRed)
      || (cur && cur->mColorMaskGreen != mColorMaskGreen)
      || (cur && cur->mColorMaskBlue != mColorMaskBlue)
      || (cur && cur->mColorMaskAlpha != mColorMaskAlpha))
  {
    GL(glColorMask(mColorMaskRed, mColorMaskGreen, mColorMaskBlue, mColorMaskAlpha));
  }
}

//------------------------------------------------------------------------------
void GLState::setColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
  mColorMaskRed   = red;
  mColorMaskGreen = green;
  mColorMaskBlue  = blue;
  mColorMaskAlpha = alpha;
}

//------------------------------------------------------------------------------
std::tuple<GLboolean, GLboolean, GLboolean, GLboolean> GLState::getColorMask() const
{
  return std::make_tuple(mColorMaskRed, mColorMaskGreen, mColorMaskBlue, mColorMaskAlpha);
}

//------------------------------------------------------------------------------
void GLState::applyActiveTexture(bool force, const GLState* cur) const
{
  if (force || (cur && cur->mTexActiveUnit!= mTexActiveUnit))
  {
    glActiveTexture(mTexActiveUnit);
  }
}


} // namespace spire
