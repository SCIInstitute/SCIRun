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

