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

// Uniforms
uniform mat4    uProjIVObject;      // Projection transform * Inverse View
uniform mat4    uObject;            // Object -> World
uniform mat4    uInverseView;       // world -> view

// Attributes
attribute vec3  aPos;
attribute vec3  aNormal;

// Outputs to the fragment shader.
varying vec3    vNormal;
varying vec4    vPos;//for clipping plane calc
varying vec4    vFogCoord;// for fog calculation

void main( void )
{
  // Todo: Add gamma correction factor of 2.2. For textures, we assume that it
  // was generated in gamma space, and we need to convert it to linear space.
  vNormal  = normalize(vec3(uObject * vec4(aNormal, 0.0)));
  vPos = vec4(aPos, 1.0);
  vFogCoord = uInverseView * vPos;
  gl_Position = uProjIVObject * vec4(aPos, 1.0);
}
