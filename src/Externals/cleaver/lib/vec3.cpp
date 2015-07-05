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


#include "vec3.h"
#include "math.h"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;
namespace Cleaver
{

// static variables
vec3 vec3::zero(0,0,0);
vec3 vec3::unitX(1,0,0);
vec3 vec3::unitY(0,1,0);
vec3 vec3::unitZ(0,0,1);

// default constructor
vec3::vec3(double x, double  y, double z) : x(x), y(y), z(z)
{
}

bool vec3::operator!=(const vec3 &a)
{
    if(this->x != a.x ||
       this->y != a.y ||
       this->z != a.z)
        return true;
    else
        return false;
}

bool vec3::operator==(const vec3 &a)
{
    if(this->x == a.x &&
       this->y == a.y &&
       this->z == a.z)
        return true;
    else
        return false;
}

bool vec3::operator<(const vec3 &a) const
{
    return ((this->x < a.x) ||
            (this->x == a.x && this->y < a.y) ||
            (this->x == a.x && this->y == a.y && this->z < a.z));
}

vec3& vec3::operator=(const vec3 &a)
{
    this->x = a.x;
    this->y = a.y;
    this->z = a.z;

    return *this;
}

vec3& vec3::operator+=(const vec3 &a)
{
    this->x += a.x;
    this->y += a.y;
    this->z += a.z;

    return *this;
}

vec3& vec3::operator*=(double c)
{
    this->x *= c;
    this->y *= c;
    this->z *= c;

    return *this;
}

vec3& vec3::operator/=(double c)
{
    this->x /= c;
    this->y /= c;
    this->z /= c;
    return *this;
}

double& vec3::operator[](const size_t idx)
{
    switch(idx)
    {
        case 0: return this->x;
        case 1: return this->y;
        case 2: return this->z;
        default: throw -1;  // bad index
    }
}

// TODO: Restructure this class so this lookup is FASTER
double vec3::operator[](const size_t idx) const
{
    switch(idx)
    {
        case 0: return this->x;
        case 1: return this->y;
        case 2: return this->z;
        default: throw -1;  // bad index
    }
}

double vec3::dot(const vec3 &b) const
{
    return this->x*b.x + this->y*b.y + this->z*b.z;
}

vec3 vec3::cross(const vec3 &b)
{
    return vec3(this->y*b.z - this->z*b.y, this->z*b.x - this->x*b.z, this->x*b.y - this->y*b.x);
}

vec3 cross(const vec3 &a, const vec3 &b)
{
    return vec3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

double dot(const vec3 &a, const vec3 &b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

double L1(const vec3 &a)
{
     return a.x + a.y + a.z;
}

double L2(const vec3 &a)
{
     return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

double length(const vec3 &a)
{
    return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}



vec3 normalize(const vec3 &v1)
{
    return v1 / length(v1);
}

double vec2polar(const vec3 &a)
{
    if(a.x > 0){
        if(a.y >= 0)
            return atan(a.y / a.x);
        else
            return atan(a.y / a.x) + 2*PI;
    }
    else if(a.x < 0){
        return atan(a.y/a.x) + PI;
    }
    else{
        if(a.y > 0)
            return PI/2;
        else if (a.y < 0)
            return 3*PI/2;
        else
            return 0;
    }
}

vec3 operator+(const vec3 &a, const vec3 &b)
{
    return vec3(a.x+b.x, a.y+b.y, a.z+b.z);
}

vec3 operator-(const vec3 &a, const vec3 &b)
{
    return vec3(a.x-b.x, a.y-b.y, a.z-b.z);
}

vec3 operator*(const vec3 &a, double s)
{
    return vec3(s*a.x, s*a.y, s*a.z);
}

vec3 operator*(double s, const vec3 &a)
{
    return vec3(s*a.x, s*a.y, s*a.z);
}

vec3 operator/(const vec3 &a, double s)
{
    return vec3(a.x/s, a.y/s, a.z/s);
}

std::ostream &operator<<(std::ostream &stream, const vec3 &v)
{
    stream << std::fixed;
    //return stream << "[" << std::setprecision(3) << v.x << ", " << v.y << ", " << v.z << "]";
    return stream << std::setprecision(3) << v.x << " " << v.y << " " << v.z;
}

double clamp(double value, double min, double max)
{
    if(value < min)
        return min;
    else if(value > max)
        return max;
    else
        return value;
}

std::string vec3::toString() const
{
    std::stringstream ss;
    ss << "[" << std::setprecision(5) << this->x << ", " << this->y << ", " << this->z << "]";
    return ss.str();
}


vec3 vec3::min(const vec3 &a, const vec3 &b)
{
    return vec3((a.x < b.x) ? a.x : b.x,
                (a.y < b.y) ? a.y : b.y,
                (a.z < b.z) ? a.z : b.z);
}

vec3 vec3::max(const vec3 &a, const vec3 &b)
{
    return vec3((a.x > b.x) ? a.x : b.x,
                (a.y > b.y) ? a.y : b.y,
                (a.z > b.z) ? a.z : b.z);
}

}
