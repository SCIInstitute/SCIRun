//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- Bounding Box
//
//  Author: Jonathan Bronson (bronson@sci.utah.edu)
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
//  Copyright (C) 2011, 2012, Jonathan Bronson
//  Scientific Computing & Imaging Institute
//  University of Utah
//
//  Permission is  hereby  granted, free  of charge, to any person
//  obtaining a copy of this software and associated documentation
//  files  ( the "Software" ),  to  deal in  the  Software without
//  restriction, including  without limitation the rights to  use,
//  copy, modify,  merge, publish, distribute, sublicense,  and/or
//  sell copies of the Software, and to permit persons to whom the
//  Software is  furnished  to do  so,  subject  to  the following
//  conditions:
//
//  The above  copyright notice  and  this permission notice shall
//  be included  in  all copies  or  substantial  portions  of the
//  Software.
//
//  THE SOFTWARE IS  PROVIDED  "AS IS",  WITHOUT  WARRANTY  OF ANY
//  KIND,  EXPRESS OR IMPLIED, INCLUDING  BUT NOT  LIMITED  TO THE
//  WARRANTIES   OF  MERCHANTABILITY,  FITNESS  FOR  A  PARTICULAR
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT  SHALL THE AUTHORS OR
//  COPYRIGHT HOLDERS  BE  LIABLE FOR  ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
//  USE OR OTHER DEALINGS IN THE SOFTWARE.
//-------------------------------------------------------------------
//-------------------------------------------------------------------
#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "vec3.h"

namespace Cleaver
{

class BoundingBox
{
public:
    BoundingBox() : origin(vec3::zero), size(vec3::zero){}
    BoundingBox(const vec3 &origin, const vec3 &size) : origin(origin), size(size){ }
    BoundingBox(double x, double y, double z, const vec3 &size) : origin(vec3(x,y,z)), size(size){ }
    BoundingBox(const vec3 &origin, double w, double h, double d) : origin(origin), size(vec3(w,h,d)){ }
    BoundingBox(double x, double y, double z, double w, double h, double d) : origin(vec3(x,y,z)),size(vec3(w,h,d)){ }

    bool contains(const vec3 &x) const;

    vec3 center() const;
    vec3 minCorner() const;
    vec3 maxCorner() const;

    vec3 origin;
    vec3 size;    
};

}

#endif // BOUNDINGBOX_H
