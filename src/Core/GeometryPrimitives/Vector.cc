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


///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Persistent/Persistent.h>

#include <iostream>
#include <sstream>

using namespace SCIRun::Core::Geometry;
using namespace SCIRun;

bool SCIRun::Core::Geometry::operator==(const Vector& v1, const Vector& v2)
{
  return v1[0] == v2[0] && v1[1] == v2[1] && v1[2] == v2[2];
}

bool SCIRun::Core::Geometry::operator!=(const Vector& v1, const Vector& v2)
{
  return !(v1 == v2);
}

std::ostream& SCIRun::Core::Geometry::operator<<( std::ostream& os, const Vector& v )
{
  os << '[' << v.x() << ' ' << v.y() << ' ' << v.z() << ']';
  return os;
}

void
  Vector::find_orthogonal(Vector& v1, Vector& v2) const
{
  Vector v0(Cross(*this, Vector(1,0,0)));
  if(v0.length2() == 0)
  {
    v0=Cross(*this, Vector(0,1,0));
  }
  v1=Cross(*this, v0);
  v1.safe_normalize();
  v2=Cross(*this, v1);
  v2.safe_normalize();
}

std::string
Vector::get_string() const
{
  std::ostringstream oss;
  oss << "[" << d_[0] << ", "<< d_[1] << ", " << d_[2] << "]";
  return (oss.str());
}

bool
Vector::check_find_orthogonal(Vector& v1, Vector& v2) const
{
  Vector v0(Cross(*this, Vector(1,0,0)));
  if(v0.length2() == 0)
  {
    v0=Cross(*this, Vector(0,1,0));
  }
  v1=Cross(*this, v0);
  double length1 = v1.length();
  if(length1 == 0) return false;
  v1 *= 1./length1;
  v2=Cross(*this, v1);
  double length2 = v2.length();
  if(length2 == 0) return false;
  v2 *= 1./length2;
  return true;
}

Vector
Vector::normal() const
{
  Vector v(*this);
  v.normalize();
  return v;
}

Vector
Vector::safe_normal() const
{
   Vector v(*this);
   v.safe_normalize();
   return v;
}

std::istream& SCIRun::Core::Geometry::operator>>( std::istream& is, Vector& v)
{
  double x, y, z;
  char st;
  is >> st >> x >> st >> y >> st >> z >> st;
  v=Vector(x,y,z);
  return is;
}

void
Vector::rotz90(const int c)
{
  // Rotate by c*90 degrees counter clockwise
  switch(c%4)
  {
    case 0:
    // 0 degrees, do nothing
    break;
    case 1:
    // 90 degrees
    {
      double newx=-d_[1];
      d_[1]=d_[0];
      d_[0]=newx;
    }
    break;
    case 2:
    // 180 degrees
    d_[0]=-d_[0];
    d_[1]=-d_[1];
    break;
    case 3:
    // 270 degrees
    {
      double newy=-d_[0];
      d_[0]=d_[1];
      d_[1]=newy;
    }
    break;
  }
}

void
SCIRun::Core::Geometry::Pio(Piostream& stream, Vector& p)
{
  stream.begin_cheap_delim();
  double x,y,z;
  if (! stream.reading())
  {
    x = p.x();
    y = p.y();
    z = p.z();
  }
  Pio(stream, x);
  Pio(stream, y);
  Pio(stream, z);
  if (stream.reading())
  {
    p.x(x);
    p.y(y);
    p.z(z);
  }
  stream.end_cheap_delim();
}


const std::string&
SCIRun::Vector_get_h_file_path()
{
  static const std::string path(TypeDescription::cc_to_h(__FILE__));
  return path;
}

const TypeDescription* SCIRun::Core::Geometry::get_type_description(Vector*)
{
  static TypeDescription* td = 0;
  if(!td){
    td = new TypeDescription("Vector", Vector_get_h_file_path(),
				"SCIRun", TypeDescription::DATA_E);
  }
  return td;
}

Vector SCIRun::Core::Geometry::vectorFromString(const std::string& str)
{
  std::istringstream istr(str);
  Vector v;
  istr >> v;
  return v;
}
