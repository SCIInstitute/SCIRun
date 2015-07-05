//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- Bounding Box library
//
// Author: Jonathan Bronson (bronson@sci.utah.edu)
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

#include "Rect3D.h"

using namespace Cleaver;


Rect3D Rect3D::boxContaining(const vec3 &p1, const vec3 &p2)
{
    vec3 min = vec3::min(p1,p2);
    vec3 max = vec3::max(p1,p2);

    return Rect3D(min,max);
}

Rect3D Rect3D::growToInclude(const vec3 &p) const
{
    vec3 new_min = vec3::min(this->min, p);
    vec3 new_max = vec3::max(this->max, p);

    return Rect3D(new_min, new_max);
}

bool Rect3D::contains(const vec3 &p) const
{
    if(p.x >= min.x && p.y >= min.y && p.z >= min.z &&
       p.x <= max.x && p.y <= max.y && p.z <= max.z)
        return true;
    else
        return false;
}

bool Rect3D::strictlyContains(const vec3 &p) const
{
    if(p.x > min.x && p.y > min.y && p.z > min.z &&
       p.x < max.x && p.y < max.y && p.z < max.z)
        return true;
    else
        return false;
}

bool Rect3D::intersects(const Rect3D &r) const
{
    if((this->min.x <= r.max.x && this->min.y <= r.max.y && this->min.z <= r.max.z) &&  // A.LO<=B.HI &&
       (this->max.x >= r.min.x && this->max.y >= r.min.y && this->max.z >= r.min.z))    // A.HI>=B.LO;
    {
        return true;
    }
    else
        return false;
}

bool Rect3D::intersect(const Rect3D &a, const Rect3D &b)
{
    if(((a.min.x <= b.max.x) && (a.min.y <= b.max.y) && (a.min.z <= b.max.z)) &&  // A.LO<=B.HI &&
       ((a.max.x >= b.min.x) && (a.max.y >= b.min.y) && (a.max.z >= b.min.z)))    // A.HI>=B.LO;
    {
        return true;
    }
    else
        return false;
}






















