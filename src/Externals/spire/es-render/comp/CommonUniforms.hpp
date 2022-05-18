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


#ifndef SPIRE_RENDER_COMPONENT_COMMON_UNIFORMS_HPP
#define SPIRE_RENDER_COMPONENT_COMMON_UNIFORMS_HPP

#include <es-log/trace-log.h>
#include <gl-platform/GLPlatform.hpp>
#include <es-cereal/ComponentSerialize.hpp>

#include <es-general/comp/StaticCamera.hpp>
#include <es-general/comp/CameraSelect.hpp>
#include <spire/scishare.h>

namespace ren {

struct SCISHARE CommonUniforms
{
  // -- Data --
  static const int MaxNumCommonUniforms = 5;

  enum COMMON_UNIFORMS
  {
    U_MODEL_VIEW_PROJECTION,                // uModelViewProjection  - Object -> World -> View -> Projection
    U_MODEL_VIEW,                           // uModelView    - Object -> World -> View
    U_MODEL,                                // uModel        - Object -> World transform
    U_VIEW_PROJECTION,                      // uViewProjection        - Inverse view projection matrix
    U_PROJECTION,                           // uProj          - Projection matrix
    U_INVERSE_VIEW,                         // uInverseView          - View to World
    U_CAM_VIEW_VEC,                         // uCamViewVec    - Viewing vector for the camera. Depends on projection matrix
    U_VIEW,                                 // uView   - Inverse view
    U_CAM_UP,                               // uCamUp         - 'Up' vector for the camera in world space
    U_CAM_POS,                              // uCamPos        - Camera position in world space.
    U_GLOBAL_TIME,                          // uGlobalTime    - Global time of the game. Used for animation.
    U_ASPECT_RATIO,                         // uAspectRatio   - The window aspect ratio
    U_WINDOW_WIDTH,                         // uWindowWidth   - The window's width
    UNIFORM_NONE,
  };

  int uniformSize; ///< How many elements in 'uniformType' and 'uniformLocation' are valid.
  COMMON_UNIFORMS uniformType[MaxNumCommonUniforms];
  GLint uniformLocation[MaxNumCommonUniforms];

  // -- Functions --

  CommonUniforms() {uniformSize = -1;}

  // Checks, and constructs if necessary, a uniform array covering
  // common uniforms for the given shaderID. You will have to const-cast
  // this CommonUniforms component in order to use this function, is it
  // modifies this component in-place.
  void checkUniformArray(GLuint shaderID);

  /// This function assumes that you have already bound the appropriate
  /// shader. It will apply all common uniforms with the given parameters.
  void applyCommonUniforms(const glm::mat4& objectToWorld,
                           const gen::StaticCameraData& cam,
                           double globalTime) const;

  void setAsUnitialized()
  {
    uniformSize = -1;
  }

  static const char* getName() {return "ren:CommonUniforms";}

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    // This component will be populated in real-time and doesn't need to be
    // serialized since the values will be dependent on this running
    // OpenGL instance.
    return true;
  }

};

} // namespace ren

#endif
