#include <gl-shaders/GLShader.hpp>

#include <es-general/util/Math.hpp>

#include "ClippingPlaneUniforms.h"
#include <sstream>
#include <glm/gtc/matrix_inverse.hpp>

namespace shaders = CPM_GL_SHADERS_NS;

namespace SCIRun {
  namespace Render {

    ClippingPlaneUniforms::ClippingPlaneUniforms()
    {
      std::ostringstream oss;
      for (int i = 0; i < 6; ++i)
      {
        hasClippingPlaneUniforms.push_back(false);
        locClippingPlaneUniforms.push_back(0);
        oss.str("");
        oss.clear();
        oss << "uClippingPlane" << i;
        strClippingPlaneCodes.push_back(oss.str());
        hasClippingPlaneCtrlUniforms.push_back(false);
        locClippingPlaneCtrlUniforms.push_back(0);
        oss.str("");
        oss.clear();
        oss << "uClippingPlaneCtrl" << i;
        strClippingPlaneCtrlCodes.push_back(oss.str());
      }
    }

    void ClippingPlaneUniforms::checkUniformArray(GLuint shaderID)
    {
      // Obtain uniforms from shader and decide which of the uniforms we can
      // provide automatically.
      std::vector<shaders::ShaderUniform> shaderUniforms = shaders::getProgramUniforms(shaderID);
      for (const shaders::ShaderUniform& uniform : shaderUniforms)
      {
        bool found = false;
        int c = 0;
        for (auto i : strClippingPlaneCodes)
        {
          if (uniform.nameInCode == i)
          {
            hasClippingPlaneUniforms[c] = true;
            locClippingPlaneUniforms[c] = uniform.uniformLoc;
            found = true;
            break;
          }
          c++;
        }
        if (found) continue;
        c = 0;
        for (auto i : strClippingPlaneCtrlCodes)
        {
          if (uniform.nameInCode == i)
          {
            hasClippingPlaneCtrlUniforms[c] = true;
            locClippingPlaneCtrlUniforms[c] = uniform.uniformLoc;
            found = true;
            break;
          }
          c++;
        }
      }
    }

    void ClippingPlaneUniforms::applyUniforms(const glm::mat4 &transform,
      const std::vector<glm::vec4> &clippingPlanes,
      const std::vector<glm::vec4> &clippingPlaneCtrls) const
    {
      int c = 0;
      for (auto i : hasClippingPlaneUniforms)
      {
        if (i && c < clippingPlanes.size())
        {
          /*glm::vec4 o = clippingPlanes[c] * (-clippingPlanes[c].w);
          o.w = 1;
          glm::vec4 n = clippingPlanes[c];
          n.w = 0;
          o = transform * o;
          n = glm::inverseTranspose(transform) * n;
          o.w = 0;
          n.w = 0;
          n.w = -glm::dot(o, n);
          GL(glUniform4f(locClippingPlaneUniforms[c], n.x, n.y, n.z, n.w));*/
          glm::vec3 n(clippingPlanes[c].x, clippingPlanes[c].y, clippingPlanes[c].z);
          n = glm::normalize(n);
          GL(glUniform4f(locClippingPlaneUniforms[c],
            n.x, n.y, n.z, clippingPlanes[c].w));
        }
        c++;
      }
      c = 0;
      for (auto i : hasClippingPlaneCtrlUniforms)
      {
        if (i && c < clippingPlaneCtrls.size())
          GL(glUniform4f(locClippingPlaneCtrlUniforms[c],
          clippingPlaneCtrls[c].x, clippingPlaneCtrls[c].y,
          clippingPlaneCtrls[c].z, clippingPlaneCtrls[c].w));
        c++;
      }
    }

  } // namespace Render
} // namespace SCIRun


