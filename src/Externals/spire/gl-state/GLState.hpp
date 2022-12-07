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


#ifndef SPIRE_GL_STATE_H
#define SPIRE_GL_STATE_H

/// \todo Handle state relative to some default state?

#include <es-log/trace-log.h>
#include <cstddef>
#include <tuple>
#include <gl-platform/GLPlatform.hpp>
#include <spire/scishare.h>

namespace spire {

// Texture state is not managed. This GLState class is not for the fixed
// function pipeline. See:
// https://www.opengl.org/discussion_boards/showthread.php/163092-Passing-Multiple-Textures-from-OpenGL-to-GLSL-shader
// You will

class SCISHARE GLState
{
public:

  GLState();

  bool operator==(const GLState &other) const;

  /// String representation of entire GLState. String includes '\n' chars.
  std::string getStateDescription();

  /// Applies OpenGL state immediately. It does not check the current OpenGL
  /// state, or any other GLState. It applies the entire GLState blindly.
  /// Use this when you want a clean OpenGL state.
  void apply() const;

  /// Applies this state relative to another GLState (state), which should
  /// represent the current OpenGL state.
  void applyRelative(const GLState& state) const;

  /// Attempts to detect errors in the OpenGL state (invalid state settings).
  /// Returns true if the state was verified, otherwise false is returned.
  /// and \p errorString , if given, is populated with a specific error.
  bool verifyState(std::string& errorString) const;

  /// Reads OpenGL state from OpenGL and updates all appropriate class
  /// variables. This modifies the entire state.
  /// This call will internally change the active texture unit with
  /// glActiveTexture, but will switch back to the last active state
  /// before returning.
  void readStateFromOpenGL();

  /// This reads state from OpenGL, only call when a context is active.
  size_t getMaxTextureUnits() const;

  /// Functions for getting/setting specific OpenGL states.
  /// The apply... family of functions all have the same parameters:
  /// (force, currentState).
  /// When calling apply, curState will be modified to the value of the
  /// current state.
  /// force     - If true, then the current state is applied regardless
  ///             of the relative GLState.
  /// curState  - Current OpenGL state. This will be modified if it differs
  ///             from the set GL state.
  /// @{

  /// Enable depth test.
  /// OpenGL: glEnable(GL_DEPTH_TEST) or glDisable(GL_DEPTH_TEST)
  void    setDepthTestEnable(bool value)  {mDepthTestEnable = value;}
  bool    getDepthTestEnable() const      {return mDepthTestEnable;}
  void    applyDepthTestEnable(bool force, const GLState* curState = nullptr) const;

  /// Enable depth function.
  /// OpenGL: glDepthFunc(value)
  /// Example values: GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER,
  ///                 GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS.
  void    setDepthFunc(GLenum value)      {mDepthFunc = value;}
  GLenum  getDepthFunc() const            {return mDepthFunc;}
  void    applyDepthFunc(bool force, const GLState* curState = nullptr) const;

  /// Set culling state.
  /// OpenGL: glCullFace(value)
  /// Example values: GL_FRONT, GL_BACK.
  void    setCullFace(GLenum value) {mCullFace = value;}
  GLenum  getCullFace() const       {return mCullFace;}
  void    applyCullFace(bool force, const GLState* curState = nullptr) const;

  /// Enable face culling.
  /// OpenGL: glEnable(GL_CULL_FACE) or glDisable(GL_CULL_FACE)
  void    setCullFaceEnable(bool value) {mCullFaceEnable = value;}
  bool    getCullFaceEnable() const     {return mCullFaceEnable;}
  void    applyCullFaceEnable(bool force, const GLState* curState = nullptr) const;

  /// Set culling front face order.
  /// OpenGL: glFrontFace(value)
  /// Example values: GL_CCW, GL_CW.
  void    setFrontFace(GLenum value)  {mCullFrontFace = value;}
  GLenum  getFrontFace() const        {return mCullFrontFace;}
  void    applyFrontFace(bool force, const GLState* curState = nullptr) const;

  /// Enable / disable blending.
  /// OpenGL: glEnable(GL_BLEND) or glDisable(GL_BLEND)
  void    setBlendEnable(bool value)   {mBlendEnable = value;}
  bool    getBlendEnable() const       {return mBlendEnable;}
  void    applyBlendEnable(bool force, const GLState* curState = nullptr) const;

  /// Set the blending equation
  /// OpenGL: glBlendEquation(value)
  /// Example values: GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT,
  ///                 GL_MIN (no ES 2.0), GL_MAX (no ES 2.0).
  void    setBlendEquation(GLenum value)  {mBlendEquation = value;}
  GLenum  getBlendEquation() const        {return mBlendEquation;}
  void    applyBlendEquation(bool force, const GLState* curState = nullptr) const;

  /// Set blending function
  /// OpenGL: glBlendFunc(src, dst)
  /// Example values: GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR,
  ///                 GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA,
  ///                 GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
  ///                 GL_SRC_ALPHA_SATURATE.
  void    setBlendFunction(GLenum src, GLenum dest)  {mBlendFuncSrc = src; mBlendFuncDst = dest;}
  std::pair<GLenum, GLenum> getBlendFunction() const {return std::make_pair(mBlendFuncSrc, mBlendFuncDst);}
  void    applyBlendFunction(bool force, const GLState* curState = nullptr) const;

  /// Set depth mask
  /// OpenGL: glDepthMask(value)
  void    setDepthMask(GLboolean value)   {mDepthMask = value;}
  GLboolean getDepthMask() const          {return mDepthMask;}
  void    applyDepthMask(bool force, const GLState* curState = nullptr) const;

  /// Set color mask
  /// OpenGL: glColorMask(red, green, blue, alpha)
  void    setColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
  std::tuple<GLboolean, GLboolean, GLboolean, GLboolean> getColorMask() const;
  void    applyColorMask(bool force, const GLState* curState = nullptr) const;

  /// Set line width.
  /// OpenGL: glLineWidth()
  void    setLineWidth(float width) {mLineWidth = width;}
  float   getLineWidth() const      {return mLineWidth;}
  void    applyLineWidth(bool force, const GLState* curState = nullptr) const;

  /// Set active texture unit.
  /// OpenGL: glActiveTexture(value)
  /// Example values: GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, ...
  void    setActiveTexture(GLenum value)      {mTexActiveUnit = value;}
  GLenum  getActiveTexture()                  {return mTexActiveUnit;}
  void    applyActiveTexture(bool force, const GLState* curState = nullptr) const;
  /// @}

  bool        mDepthTestEnable;
  GLenum      mDepthFunc;

  GLenum      mCullFace;
  bool        mCullFaceEnable;
  GLenum      mCullFrontFace;

  bool        mBlendEnable;
  GLenum      mBlendEquation;
  GLenum      mBlendFuncSrc;
  GLenum      mBlendFuncDst;

  GLboolean   mDepthMask;

  GLboolean   mColorMaskRed;
  GLboolean   mColorMaskGreen;
  GLboolean   mColorMaskBlue;
  GLboolean   mColorMaskAlpha;

  float       mLineWidth;       ///< glLineWidth(...)

  GLenum      mTexActiveUnit;   ///< Active texture unit.

private:

  void applyStateInternal(bool force, const GLState* state) const;
};

} // namespace spire

#endif
