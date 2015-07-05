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
//
// Copyright (c) 2011, Scientific Computing & Imaging Institute and
// the University of Utah. All Rights Reserved.
//
// Redistribution and use in source and binary forms is strictly
// prohibited without the direct consent of the author.
//
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

