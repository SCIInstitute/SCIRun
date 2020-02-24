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


//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- 3D Image Data Field
//
// Primary Author: Jonathan Bronson (bronson@sci.utah.edu)
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------

#include "ImageField3D.h"
#include <cmath>

using namespace Cleaver;

float ImageField3D::value(float x, float y, float z, int mat) const
{
    // return array value if on the grid
    if(fmod(x,1.0f) == 0 && fmod(y,1.0f) == 0 && fmod(z,1.0f) == 0)
        return data[(int)(m*(x + y*w + z*w*h) + mat)];

    // otherwise interpolate
    else
    {
        int i = (int)x;
        int j = (int)y;
        int k = (int)z;

        float t = fmod(x,1.0f);
        float u = fmod(y,1.0f);
        float v = fmod(z,1.0f);

        double C000 = data[m*(i + j*w + k*w*h) + mat];
        double C001 = data[m*(i + j*w + (k+1)*w*h) + mat];
        double C010 = data[m*(i + (j+1)*w + k*w*h) + mat];
        double C011 = data[m*(i + (j+1)*w + (k+1)*w*h) + mat];
        double C100 = data[m*(i+1 + j*w + k*w*h) + mat];
        double C101 = data[m*(i+1 + j*w + (k+1)*w*h) + mat];
        double C110 = data[m*(i+1 + (j+1)*w + k*w*h) + mat];
        double C111 = data[m*(i+1 + (j+1)*w + (k+1)*w*h) + mat];

        return float((1-t)*(1-u)*(1-v)*C000 + (1-t)*(1-u)*(v)*C001 +
                     (1-t)*  (u)*(1-v)*C010 + (1-t)*  (u)*(v)*C011 +
                       (t)*(1-u)*(1-v)*C100 +   (t)*(1-u)*(v)*C101 +
                       (t)*  (u)*(1-v)*C110 +   (t)*  (u)*(v)*C111);

    }
}

float ImageField3D::operator()(float x, float y, float z, int mat) const
{
    return value(x,y,z,mat);
}
