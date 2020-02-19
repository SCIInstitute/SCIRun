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

#include <gl-shaders/GLShader.hpp>

#include <es-general/util/Math.hpp>

#include "ClippingPlaneUniforms.h"
#include <sstream>
#include <glm/gtc/matrix_inverse.hpp>

namespace shaders = spire;

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
          glm::vec3 n(clippingPlanes[c].x, clippingPlanes[c].y, clippingPlanes[c].z);
          if (n != glm::vec3{0, 0, 0})
          {
            n = glm::normalize(n);
            GL(glUniform4f(locClippingPlaneUniforms[c],
              n.x, n.y, n.z, clippingPlanes[c].w));
          }
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
