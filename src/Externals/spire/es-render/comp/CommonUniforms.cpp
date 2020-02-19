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


#include <gl-shaders/GLShader.hpp>

#include <es-general/util/Math.hpp>

#include "CommonUniforms.hpp"

namespace shaders = spire;

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
         if (uniform.nameInCode == "uModel")                foundUniform = U_MODEL;
    else if (uniform.nameInCode == "uView")                 foundUniform = U_VIEW;
    else if (uniform.nameInCode == "uProjection")           foundUniform = U_PROJECTION;
    else if (uniform.nameInCode == "uModelView")            foundUniform = U_MODEL_VIEW;
    else if (uniform.nameInCode == "uViewProjection")       foundUniform = U_VIEW_PROJECTION;
    else if (uniform.nameInCode == "uModelViewProjection")  foundUniform = U_MODEL_VIEW_PROJECTION;
    else if (uniform.nameInCode == "uInverseView")          foundUniform = U_INVERSE_VIEW;
    else if (uniform.nameInCode == "uCamViewVec")           foundUniform = U_CAM_VIEW_VEC;
    else if (uniform.nameInCode == "uCamUp")                foundUniform = U_CAM_UP;
    else if (uniform.nameInCode == "uCamPos")               foundUniform = U_CAM_POS;
    else if (uniform.nameInCode == "uGlobalTime")           foundUniform = U_GLOBAL_TIME;
    else if (uniform.nameInCode == "uAspectRatio")          foundUniform = U_ASPECT_RATIO;
    else if (uniform.nameInCode == "uWindowWidth")          foundUniform = U_WINDOW_WIDTH;

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
      case U_MODEL_VIEW_PROJECTION:
        mat = cam.viewProjection * objectToWorld;
        ptr = glm::value_ptr(mat);
        GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
        break;

      case U_MODEL_VIEW:
        mat = cam.view * objectToWorld;
        ptr = glm::value_ptr(mat);
        GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
        break;

      case U_MODEL:
        ptr = glm::value_ptr(objectToWorld);
        GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
        break;

      case U_VIEW_PROJECTION:
        ptr = glm::value_ptr(cam.viewProjection);
        GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
        break;

      case U_PROJECTION:
        ptr = glm::value_ptr(cam.projection);
        GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
        break;

      case U_INVERSE_VIEW:
        {
          glm::mat4 view = cam.getInverseView();
          ptr = glm::value_ptr(view);
          GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
          break;
        }

      case U_CAM_VIEW_VEC:
        {
          glm::mat4 view = cam.getInverseView();
          glm::vec3 viewVec = view[2].xyz();
          vec = -viewVec; // Our projection matrix looks down negative Z.
          GL(glUniform3f(uniformLocation[i], vec.x, vec.y, vec.z));
          break;
        }

      case U_VIEW:
        {
          ptr = glm::value_ptr(cam.view);
          GL(glUniformMatrix4fv(uniformLocation[i], 1, false, ptr));
        }

      case U_CAM_UP:
        {
          glm::mat4 view = cam.getInverseView();
          vec = view[1].xyz();
          GL(glUniform3f(uniformLocation[i], vec.x, vec.y, vec.z));
          break;
        }

      case U_GLOBAL_TIME:
        {
          GL(glUniform1f(uniformLocation[i], static_cast<float>(globalTime)));
          break;
        }

      case U_CAM_POS:
        {
          glm::mat4 view = cam.getInverseView();
          glm::vec3 pos(view[3].x, view[3].y, view[3].z);
          GL(glUniform3f(uniformLocation[i], pos.x, pos.y, pos.z));
        }
        break;

      case U_ASPECT_RATIO:
        {
          float aspect = cam.aspect;
          GL(glUniform1f(uniformLocation[i], aspect));
        }
        break;

      case U_WINDOW_WIDTH:
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
