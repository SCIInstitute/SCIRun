/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

// Uniforms
uniform mat4    uProjIVObject;      // Projection transform * Inverse View
uniform mat4    uObject;            // Object -> World
uniform vec3    uCamViewVec;        // Camera 'at' vector in world space
uniform vec4    uAmbientColor;      // Ambient color
uniform vec4    uDiffuseColor;      // Diffuse color
uniform vec4    uSpecularColor;     // Specular color     
uniform float   uSpecularPower;     // Specular power
uniform vec3    uLightDirWorld;     // Directional light (world space).

// Attributes
attribute vec3  aPos;
attribute vec3  aNormal;

// Outputs to the fragment shader.
varying vec4    fColor;

void main( void )
{
  vec3  invLightDir     = -uLightDirWorld;
  vec3  worldSpaceNorm  = vec3(uObject * vec4(aNormal, 0.0));
  float diffuse         = max(0.0, dot(worldSpaceNorm, invLightDir));
  vec3  reflection      = reflect(invLightDir, worldSpaceNorm);
  float spec            = max(0.0, dot(reflection, uCamViewVec));

  // Convert color into gamma space before rasterization.
  spec        = pow(spec, uSpecularPower);
  fColor      = pow(diffuse * spec * uSpecularColor + diffuse * uDiffuseColor + uAmbientColor, vec4(1.0/2.2));

  gl_Position = uProjIVObject * vec4(aPos, 1.0);
}
