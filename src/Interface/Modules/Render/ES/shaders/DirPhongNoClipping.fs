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

#ifdef OPENGL_ES
  #ifdef GL_FRAGMENT_PRECISION_HIGH
    precision highp float;
  #else
    precision mediump float;
  #endif
#endif

uniform vec3    uCamViewVec;
uniform vec4    uAmbientColor;
uniform vec4    uDiffuseColor;
uniform vec4    uSpecularColor;
uniform float   uSpecularPower;
uniform vec3    uLightDirectionView0;
uniform vec3    uLightDirectionView1;
uniform vec3    uLightDirectionView2;
uniform vec3    uLightDirectionView3;
uniform vec3    uLightColor0;
uniform vec3    uLightColor1;
uniform vec3    uLightColor2;
uniform vec3    uLightColor3;
uniform float   uTransparency;

// fog settings (intensity, start, end, 0.0)
uniform vec4    uFogSettings;
uniform vec4    uFogColor;

varying vec3    vNormal;
varying vec4    vPosWorld;
varying vec4    vPosView;

vec4 calculate_lighting(vec3 N, vec3 L, vec3 V, vec3 diffuseColor, vec3 specularColor, vec3 lightColor)
{
  vec3 H = normalize(V + L);
  float diffuse = max(0.0, dot(N, L));
  float specular = max(0.0, dot(N, H));
  specular = pow(specular, uSpecularPower);

  return vec4(lightColor * (diffuse * diffuseColor + specular * specularColor), 0.0);
}

void main()
{
  vec3 diffuseColor = uDiffuseColor.rgb;
  vec3 specularColor = uSpecularColor.rgb;
  vec3 ambientColor = uAmbientColor.rgb;
  float transparency = uTransparency;

  vec3 normal = normalize(vNormal);
  if (gl_FrontFacing) normal = -normal;
  vec3 cameraVector = -normalize(vPosView.xyz);

  gl_FragColor = vec4(ambientColor * diffuseColor, transparency);
  if (length(uLightDirectionView0) > 0.0)
    gl_FragColor += calculate_lighting(normal, uLightDirectionView0, cameraVector, diffuseColor, specularColor, uLightColor0);
  if (length(uLightDirectionView1) > 0.0)
    gl_FragColor += calculate_lighting(normal, uLightDirectionView1, cameraVector, diffuseColor, specularColor, uLightColor1);
  if (length(uLightDirectionView2) > 0.0)
    gl_FragColor += calculate_lighting(normal, uLightDirectionView2, cameraVector, diffuseColor, specularColor, uLightColor2);
  if (length(uLightDirectionView3) > 0.0)
    gl_FragColor += calculate_lighting(normal, uLightDirectionView3, cameraVector, diffuseColor, specularColor, uLightColor3);

  //calculate fog
  if (uFogSettings.x > 0.0)
  {
    vec4 fp;
    fp.x = uFogSettings.x;
    fp.y = uFogSettings.y;
    fp.z = uFogSettings.z;
    fp.w = abs(vPosView.z/vPosView.w);

    float fog_factor;
    fog_factor = (fp.z-fp.w)/(fp.z-fp.y);
    fog_factor = 1.0 - clamp(fog_factor, 0.0, 1.0);
    fog_factor = 1.0 - exp(-pow(fog_factor*2.5, 2.0));
    gl_FragColor.xyz = mix(clamp(gl_FragColor.xyz, 0.0, 1.0),
      clamp(uFogColor.xyz, 0.0, 1.0), fog_factor);
  }
}