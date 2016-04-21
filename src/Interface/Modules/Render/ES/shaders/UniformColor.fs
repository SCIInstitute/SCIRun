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
    // Default precision
    precision highp float;
  #else
    precision mediump float;
  #endif
#endif

uniform float uTransparency;

//clipping planes
uniform vec4    uClippingPlane0;    // clipping plane 0
uniform vec4    uClippingPlane1;    // clipping plane 1
uniform vec4    uClippingPlane2;    // clipping plane 2
uniform vec4    uClippingPlane3;    // clipping plane 3
uniform vec4    uClippingPlane4;    // clipping plane 4
uniform vec4    uClippingPlane5;    // clipping plane 5
//clipping plane controls
uniform vec4    uClippingPlaneCtrl0;// clipping plane 0 control (visible, showFrame, reverseNormal, 0)
uniform vec4    uClippingPlaneCtrl1;// clipping plane 1 control (visible, showFrame, reverseNormal, 0)
uniform vec4    uClippingPlaneCtrl2;// clipping plane 2 control (visible, showFrame, reverseNormal, 0)
uniform vec4    uClippingPlaneCtrl3;// clipping plane 3 control (visible, showFrame, reverseNormal, 0)
uniform vec4    uClippingPlaneCtrl4;// clipping plane 4 control (visible, showFrame, reverseNormal, 0)
uniform vec4    uClippingPlaneCtrl5;// clipping plane 5 control (visible, showFrame, reverseNormal, 0)

//fog
uniform vec4    uFogSettings;       // fog settings (intensity, start, end, 0.0)
uniform vec4    uFogColor;          // fog color

varying vec4	fColor;
varying vec4    vPos;//for clipping plane calc
varying vec4    vFogCoord;// for fog calculation

void main()
{
  float fPlaneValue;
  if (uClippingPlaneCtrl0.x > 0.5)
  {
    fPlaneValue = dot(vPos, uClippingPlane0);
    fPlaneValue = uClippingPlaneCtrl0.z > 0.5 ? -fPlaneValue : fPlaneValue;
    if (fPlaneValue < 0.0)
      discard;
  }
  if (uClippingPlaneCtrl1.x > 0.5)
  {
    fPlaneValue = dot(vPos, uClippingPlane1);
    fPlaneValue = uClippingPlaneCtrl1.z > 0.5 ? -fPlaneValue : fPlaneValue;
    if (fPlaneValue < 0.0)
      discard;
  }
  if (uClippingPlaneCtrl2.x > 0.5)
  {
    fPlaneValue = dot(vPos, uClippingPlane2);
    fPlaneValue = uClippingPlaneCtrl2.z > 0.5 ? -fPlaneValue : fPlaneValue;
    if (fPlaneValue < 0.0)
      discard;
  }
  if (uClippingPlaneCtrl3.x > 0.5)
  {
    fPlaneValue = dot(vPos, uClippingPlane3);
    fPlaneValue = uClippingPlaneCtrl3.z > 0.5 ? -fPlaneValue : fPlaneValue;
    if (fPlaneValue < 0.0)
      discard;
  }
  if (uClippingPlaneCtrl4.x > 0.5)
  {
    fPlaneValue = dot(vPos, uClippingPlane4);
    fPlaneValue = uClippingPlaneCtrl4.z > 0.5 ? -fPlaneValue : fPlaneValue;
    if (fPlaneValue < 0.0)
      discard;
  }
  if (uClippingPlaneCtrl5.x > 0.5)
  {
    fPlaneValue = dot(vPos, uClippingPlane5);
    fPlaneValue = uClippingPlaneCtrl5.z > 0.5 ? -fPlaneValue : fPlaneValue;
    if (fPlaneValue < 0.0)
      discard;
  }

	gl_FragColor 		= vec4(fColor.xyz,uTransparency);

  //calculate fog
  if (uFogSettings.x > 0.0)
  {
    vec4 fp;
    fp.x = uFogSettings.x;
    fp.y = uFogSettings.y;
    fp.z = uFogSettings.z;
    fp.w = abs(vFogCoord.z/vFogCoord.w);
    
    float fog_factor;
    fog_factor = (fp.z-fp.w)/(fp.z-fp.y);
    fog_factor = 1.0 - clamp(fog_factor, 0.0, 1.0);
    fog_factor = 1.0 - exp(-pow(fog_factor*2.5, 2.0));
    gl_FragColor.xyz = mix(clamp(gl_FragColor.xyz, 0.0, 1.0),
      clamp(uFogColor.xyz, 0.0, 1.0), fog_factor);
  }
}
