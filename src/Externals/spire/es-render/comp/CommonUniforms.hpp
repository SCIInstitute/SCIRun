#ifndef IAUNS_RENDER_COMPONENT_COMMON_UNIFORMS_HPP
#define IAUNS_RENDER_COMPONENT_COMMON_UNIFORMS_HPP

#include <gl-platform/GLPlatform.hpp>
#include <es-cereal/ComponentSerialize.hpp>

#include <es-general/comp/StaticCamera.hpp>
#include <es-general/comp/CameraSelect.hpp>

namespace ren {

struct CommonUniforms
{
  // -- Data --
  static const int MaxNumCommonUniforms = 5;
  
  enum COMMON_UNIFORMS
  {
    OBJ_PROJECTION_INVERSE_VIEW_OBJECT,   // uProjIVObject  - Object -> World -> View -> Projection
    OBJ_VIEW_OBJECT,                      // uViewObject    - Object -> World -> View
    OBJ_OBJECT,                           // uObject        - Object -> World transform
    CAM_PROJECTION_INVERSE_VIEW,          // uProjIV        - Inverse view projection matrix
    CAM_PROJECTION,                       // uProj          - Projection matrix
    CAM_VIEW,                             // uView          - View to World
    CAM_VIEW_VEC,                         // uCamViewVec    - Viewing vector for the camera. Depends on projection matrix
    CAM_INVERSE_VIEW,                     // uInverseView   - Inverse view
    CAM_UP,                               // uCamUp         - 'Up' vector for the camera in world space
    CAM_POS,                              // uCamPos        - Camera position in world space.
    GLOBAL_TIME,                          // uGlobalTime    - Global time of the game. Used for animation.
    ASPECT_RATIO,                         // uAspectRatio   - The window aspect ratio
    WINDOW_WIDTH,                         // uWindowWidth   - The window's width
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

  bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    // This component will be populated in real-time and doesn't need to be
    // serialized since the values will be dependent on this running
    // OpenGL instance.
    return true;
  }

};

} // namespace ren

#endif 
