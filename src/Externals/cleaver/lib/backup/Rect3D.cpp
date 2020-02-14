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
// -- Bounding Box library
//
// Author: Jonathan Bronson (bronson@sci.utah.edu)
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
