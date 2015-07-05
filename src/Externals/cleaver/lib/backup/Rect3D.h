//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- Bounding Box library
//
//  Author: Jonathan Bronson (bronson@sci.utah.edu)
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

#ifndef RECT3D_H
#define RECT3D_H

#include "vec3.h"

namespace Cleaver
{

class Rect3D
{
public:
    Rect3D() : min(vec3::zero), max(vec3::zero){ }
    Rect3D(const vec3 &lower, const vec3 &upper) : min(lower), max(upper){ }
    Rect3D(float lx, float ly, float lz, float ux, float uy, float uz) : min(lx,ly,lz), max(ux,uy,uz){ }

    static bool intersect(const Rect3D &a, const Rect3D &b);
    static Rect3D boxContaining(const vec3 &p1, const vec3 &p2);
    Rect3D growToInclude(const vec3 &p) const;
    bool contains(const vec3 &p) const;
    bool strictlyContains(const vec3 &p) const;
    bool intersects(const Rect3D &r) const;

    vec3 min;
    vec3 max;
};

}

#endif // RECT3D_H
