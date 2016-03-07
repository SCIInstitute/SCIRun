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
uniform float   uFDToggle;          // Field data toggle

// Attributes
attribute vec3  aPos;
attribute vec3  aNormal;
attribute vec4  aColor;
attribute vec4  aColorSecondary;

// Outputs to the fragment shader.
varying vec3    vNormal;
varying vec4    vColor;
varying vec4    vPos;//for clipping plane calc

void main( void )
{
  gl_Position = uProjIVObject * vec4(aPos, 1.0);

  vNormal  = vec3(uObject * vec4(aNormal, 0.0));

  vec4 colorData = uFDToggle * aColor + (1.0 - uFDToggle) * aColorSecondary;
  vColor = colorData;
  vPos = vec4(aPos, 1.0);
}
