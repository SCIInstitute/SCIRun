//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Conforming Tetrahedral Meshing Library
//
// -- vector library
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


#ifndef VEC3_H
#define VEC3_H

#ifndef PI
#define PI 3.14159265
#endif

#include <iostream>


namespace Cleaver
{

class vec3
{
public:
    vec3(double x=0, double y=0, double z=0);

public:
    double x;
    double y;
    double z;

    bool operator!=(const vec3 &a);
    bool operator==(const vec3 &a);
    bool operator<(const vec3 &a) const;
    vec3& operator=(const vec3 &a);
    vec3& operator+=(const vec3 &a);
    vec3& operator*=(double c);
    vec3& operator/=(double c);

    double& operator[](const size_t);
    double  operator[](const size_t) const;

    double dot(const vec3 &b) const;
    vec3 cross(const vec3 &b);

    static vec3 zero;
    static vec3 unitX;
    static vec3 unitY;
    static vec3 unitZ;
    static vec3 min(const vec3 &a, const vec3 &b);
    static vec3 max(const vec3 &a, const vec3 &b);

    std::string toString() const;
    friend std::ostream& operator<<(std::ostream &stream, const vec3 &v);
};

vec3 cross(const vec3 &a, const vec3 &b);
double dot(const vec3 &a, const vec3 &b);
double length(const vec3 &a);
double L1(const vec3 &a);
double L2(const vec3 &a);
double clamp(double value, double min, double max);
vec3 normalize(const vec3 &v1);

double vec2polar(const vec3 &a);

vec3 operator+(const vec3 &a, const vec3 &b);
vec3 operator-(const vec3 &a, const vec3 &b);
vec3 operator*(const vec3 &a, double b);
vec3 operator*(double a, const vec3 &b);
vec3 operator/(const vec3 &a, double b);

}

#endif // VEC3_H
