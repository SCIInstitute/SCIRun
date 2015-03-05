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
uniform float    uAspectRatio  ;      
uniform float    uWindowWidth  ;
uniform float    uExtraSpace   ;
uniform float    uDisplaySide  ;
uniform float    uDisplayLength;

// Attributes
attribute vec3  aPos;
attribute vec2  aTexCoord;

//Outputs
varying vec2 fTexCoord;

void main( void )
{
  bool ex = uExtraSpace == 1.;
  bool ds = uDisplaySide == 0.;
  bool full = uDisplayLength == 1.;
  bool half1 = uDisplayLength == 0.;
  vec3 newPos = aPos;
  float x_scale = .5 / uWindowWidth;
  float y_scale = 1. / (uWindowWidth / uAspectRatio);
  float x_trans = ds?(-1.+(ex?.05:0.)):(-1.+(full?(aPos.z*1.8+.1):(aPos.z*.9+(half1?0.+(ex?.05:0.):
                                       1.1+(ex?-.05:0.)))) - 15. / uWindowWidth);
  float y_trans = ds?(-1.+(full?(aPos.z*1.8+.1):(aPos.z*.9+(half1?0.+(ex?.05:0.):
                                       1.1+(ex?-.05:0.)))) - 2. / uWindowWidth):(-1.+(ex?.05:0.));
  gl_Position = vec4(newPos.x * x_scale + x_trans, 
                     newPos.y * y_scale + y_trans, 0., 1.0);
  fTexCoord = aTexCoord;
}
