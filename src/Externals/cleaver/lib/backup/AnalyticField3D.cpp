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
// -- 3D Analytic Field
//
// Primary Author: Jonathan Bronson (bronson@sci.utah.edu)
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------

#include "AnalyticField3D.h"

using namespace Cleaver;

AnalyticField3D::AnalyticField3D()
{
    this->w = 32;
    this->h = 32;
    this->d = 32;
    this->m = 4;

    // add cyclinder 1
    x1.push_back(vec3(16, 23, 2));
    x2.push_back(vec3(16, 23, 30));
    r.push_back(7);

    // add cylinder 2
    x1.push_back(vec3(16, 9, 2));
    x2.push_back(vec3(16, 9, 30));
    r.push_back(7);
}


float AnalyticField3D::value(float x, float y, float z, int mat) const
{
    // if on exterior, always return padding value
    if(x < 2 || y < 2 || z < 2 || x >= 30 || y >= 30 || z >= 30){
        if(mat == m-1)
            return 10;
        else
            return -100;
    }

    // otherwise return appropriat values
    switch(mat){
    case 0:
        return 0;  // background always zero

    case 1:
    {
        // return analytic value for cylinder 1
        vec3 x0(x,y,z);

        double numer = L2((x2[0]-x1[0]).cross(x1[0]-x0));
        double denom = L2(x2[0]-x1[0]);
        double distance = numer / denom;
        double val = r[0] - distance;
        return (float)val;
    }
    case 2:
    {
        // return analytic value for cylinder 2
        vec3 x0(x,y,z);

        double numer = L2((x2[1]-x1[1]).cross(x1[1]-x0));
        double denom = L2(x2[1]-x1[1]);
        double distance = numer / denom;
        double val = r[1] - distance;
        return (float)val;
    }
    default:
        return -1000; // boundary small
    }
}


float AnalyticField3D::operator()(float x, float y, float z, int mat) const
{
    return value(x,y,z,mat);
}
