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


// Billboard text: world-space anchor, glyph quad offset in pixels (#2148).

// Uniforms
uniform mat4     uModel        ;
uniform mat4     uView         ;
uniform mat4     uProjection   ;
uniform float    uAspectRatio  ;
uniform float    uWindowWidth  ;
uniform float    uAlwaysVisible;

// Attributes
attribute vec2  aPos;
attribute vec2  aTexCoord;
attribute vec3  aAnchor;
attribute vec4  aNormal;
attribute vec4  aColor;

//Outputs
varying vec2 fTexCoord;
varying vec4 fColor;

void main( void )
{
  mat4 modelView = uView * uModel;
  vec4 eyePos = modelView * vec4(aAnchor, 1.0);
  bool ortho = uProjection[3][3] != 0.0;

  bool culled = false;
  if (aNormal.w > 0.5)
  {
    vec3 n = (modelView * vec4(aNormal.xyz, 0.0)).xyz;
    vec3 toEye = ortho ? vec3(0.0, 0.0, 1.0) : -eyePos.xyz;
    culled = dot(n, toEye) <= 0.0;
  }

  // The quad sits at one depth while the surface under it does not, so a face
  // tilted toward the camera would cut the text off. Pull each vertex forward
  // by a multiple of its distance from the anchor. A slope of 3 clears surfaces
  // tilted up to about 70 degrees; much more lets labels just behind a surface
  // show through it.
  float worldPerPixel = 3.0 * 2.0 / (uWindowWidth * uProjection[0][0]);
  if (ortho)
  {
    eyePos.z += 0.01 * abs(eyePos.z) + length(aPos) * worldPerPixel;
  }
  else
  {
    float dist = length(eyePos.xyz);
    float nudge = 0.01 * dist + length(aPos) * worldPerPixel * -eyePos.z;
    eyePos.xyz *= max(0.0, 1.0 - nudge / dist);
  }

  vec4 clip = uProjection * eyePos;
  float x_scale = 2. / uWindowWidth;
  float y_scale = 2. / (uWindowWidth / uAspectRatio);
  clip.xy += vec2(aPos.x * x_scale, aPos.y * y_scale) * clip.w;
  if (uAlwaysVisible > 0.5)
    clip.z = -clip.w;

  // Outside the clip volume, so the quad is discarded.
  gl_Position = culled ? vec4(2.0, 2.0, 2.0, 1.0) : clip;
  fTexCoord = aTexCoord;
  fColor = aColor;
}
