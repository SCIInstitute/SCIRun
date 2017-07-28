#include <gl-shaders/GLShader.hpp>

#include <es-general/util/Math.hpp>

#include "CommonUniforms.hpp"

namespace shaders = CPM_GL_SHADERS_NS;

namespace ren {

void CommonUniforms::checkUniformArray(GLuint shaderID)
{
  // Obtain uniforms from shader and decide which of the uniforms we can
  // provide automatically.
  uniformSize = 0;
  std::vector<shaders::ShaderUniform> shaderUniforms = shaders::getProgramUniforms(shaderID);
  for (const shaders::ShaderUniform& uniform : shaderUniforms)
  {
    COMMON_UNIFORMS foundUniform = UNIFORM_NONE;
    if (uniform.nameInCode == "uProjIVObject")      foundUniform = OBJ_PROJECTION_INVERSE_VIEW_OBJECT;
    else if (uniform.nameInCode == "uViewObject")   foundUniform = OBJ_VIEW_OBJECT;
    else if (uniform.nameInCode == "uObject")       foundUniform = OBJ_OBJECT;
    else if (uniform.nameInCode == "uProjIV")       foundUniform = CAM_PROJECTION_INVERSE_VIEW;
    else if (uniform.nameInCode == "uView")         foundUniform = CAM_VIEW;
    else if (uniform.nameInCode == "uInverseView")  foundUniform = CAM_INVERSE_VIEW;
    else if (uniform.nameInCode == "uProjection")   foundUniform = CAM_PROJECTION;
    else if (uniform.nameInCode == "uCamViewVec")   foundUniform = CAM_VIEW_VEC;
    else if (uniform.nameInCode == "uCamUp")        foundUniform = CAM_UP;
    else if (uniform.nameInCode == "uCamPos")       foundUniform = CAM_POS;
    else if (uniform.nameInCode == "uGlobalTime")   foundUniform = GLOBAL_TIME;
    else if (uniform.nameInCode == "uAspectRatio")   foundUniform = ASPECT_RATIO;
    else if (uniform.nameInCode == "uWindowWidth")   foundUniform = WINDOW_WIDTH;

    if (foundUniform != UNIFORM_NONE)
    {
      if (uniformSize == MaxNumCommonUniforms)
      {
        std::cerr << "components::CommonUniforms - Overflowed maximum number of common uniforms!" << std::endl;
        throw std::runtime_error("Common uniform overflow!");
        return;
      }

      uniformType[uniformSize]      = foundUniform;
      uniformLocation[uniformSize]  = uniform.uniformLoc;
      ++uniformSize;
    }
  }
}

void CommonUniforms::applyCommonUniforms(const glm::mat4& objectToWorld,
                                         const gen::StaticCameraData& cam,
                                         double globalTime) const
{
  // Now render the static geom, with the given shader (in reference to the
  // currentGLState), at the given position and transform.
  glm::mat4 mat;
  glm::vec3 vec;
  const GLfloat* ptr;
  for (int i = 0; i < uniformSize; ++i)
  {
    switch (uniformType[i])
    {
      case OBJ_PROJECTION_INVERSE_VIEW_OBJECT:
        mat = cam.projIV * objectToWorld;
        ptr = glm::value_ptr(mat);
        GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
        break;

      case OBJ_VIEW_OBJECT:
        mat = cam.worldToView * objectToWorld;
        ptr = glm::value_ptr(mat);
        GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
        break;

      case OBJ_OBJECT:
        ptr = glm::value_ptr(objectToWorld);
        GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
        break;

      case CAM_PROJECTION_INVERSE_VIEW:
        ptr = glm::value_ptr(cam.projIV);
        GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
        break;

      case CAM_PROJECTION:
        ptr = glm::value_ptr(cam.projection);
        GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
        break;

      case CAM_VIEW:
        {
          glm::mat4 view = cam.getView();
          ptr = glm::value_ptr(view);
          GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
          break;
        }
        
      case CAM_VIEW_VEC:
        {
          glm::mat4 view = cam.getView();
          glm::vec3 viewVec = view[2].xyz();
          vec = -viewVec; // Our projection matrix looks down negative Z.
          GL(glUniform3f(uniformLocation[i], vec.x, vec.y, vec.z));
          break;
        }

      case CAM_INVERSE_VIEW:
        {
          ptr = glm::value_ptr(cam.worldToView);
          GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
        }

      case CAM_UP:
        {
          glm::mat4 view = cam.getView();
          vec = view[1].xyz();
          GL(glUniform3f(uniformLocation[i], vec.x, vec.y, vec.z));
          break;
        }

      case GLOBAL_TIME:
        {
          GL(glUniform1f(uniformLocation[i], static_cast<float>(globalTime)));
          break;
        }

      case CAM_POS:
        {
          glm::mat4 view = cam.getView();
          glm::vec3 pos(view[3].x, view[3].y, view[3].z);
          GL(glUniform3f(uniformLocation[i], pos.x, pos.y, pos.z));
        }
        break;

      case ASPECT_RATIO:
        {
          float aspect = cam.aspect;
          GL(glUniform1f(uniformLocation[i], aspect));
        }
        break;
        
      case WINDOW_WIDTH:
        {
          float width = cam.winWidth;
          GL(glUniform1f(uniformLocation[i], width));
        }
        break;

      case UNIFORM_NONE:
        std::cerr << "Attempting to apply common uniform: UNIFORM_NONE" << std::endl;
        break;
    }
  }
}

} // namespace ren


