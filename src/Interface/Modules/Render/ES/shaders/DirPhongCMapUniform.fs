/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

uniform vec3    uCamViewVec;        // Camera 'at' vector in world space
uniform vec4    uAmbientColor;      // Ambient color
uniform vec4    uSpecularColor;     // Specular color     
uniform float   uSpecularPower;     // Specular power
uniform vec3    uLightDirWorld;     // Directional light (world space).
uniform float   uTransparency;

// Lighting in world space. Generally, it's better to light in eye space if you
// are dealing with point lights. Since we are only dealing with directional
// lights we light in world space.
varying vec3  vNormal;
varying vec4  vColor;

void main()
{
  // Remember to always negate the light direction for these lighting
  // calculations. The dot product takes on its greatest values when the angle
  // between the two vectors diminishes.
  vec3  invLightDir = -uLightDirWorld;
  vec3  normal      = normalize(vNormal);
  float diffuse     = max(0.0, dot(normal, invLightDir));

  // Note, the following is a hack due to legacy meshes still being supported.
  // We light the object as if it was double sided. We choose the normal based
  // on the normal that yields the largest diffuse component.
  float diffuseInv  = max(0.0, dot(-normal, invLightDir));

  if (diffuse < diffuseInv)
  {
    diffuse = diffuseInv;
    normal = -normal;
  }

  vec3  reflection  = reflect(invLightDir, normal);
  float spec        = max(0.0, dot(reflection, uCamViewVec));

  vec4 diffuseColor = vColor;

  spec              = pow(spec, uSpecularPower);
  gl_FragColor      = vec4(diffuse * spec * uSpecularColor + diffuse * diffuseColor + uAmbientColor).rgb, uTransparency);
}

