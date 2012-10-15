/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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


#include <Core/GeometryPrimitives/Point.h>

#include <iostream>
#include <sstream>

using namespace SCIRun::Core::Geometry;

std::string Point::get_string() const
{
  std::ostringstream oss;
  oss << "[" << d_[0] << ", "<< d_[1] << ", " << d_[2] << "]";
  return (oss.str());
}

int Point::operator==(const Point& p) const
{
  return p.d_[0] == d_[0] && p.d_[1] == d_[1] && p.d_[2] == d_[2];
}

int Point::operator!=(const Point& p) const
{
  return p.d_[0] != d_[0] || p.d_[1] != d_[1] || p.d_[2] != d_[2];
}

Point::Point(double x, double y, double z, double w)
{
  if(w==0)
  {
    d_[0]=0.0;
    d_[1]=0.0;
    d_[2]=0.0;
  } 
  else 
  {
    d_[0]=x/w;
    d_[1]=y/w;
    d_[2]=z/w;
  }
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
Point AffineCombination(const Point& p1, double w1,
			const Point& p2, double w2)
{
  return Point(p1.d_[0]*w1+p2.d_[0]*w2,
		 p1.d_[1]*w1+p2.d_[1]*w2,
		 p1.d_[2]*w1+p2.d_[2]*w2);
}

Point AffineCombination(const Point& p1, double w1,
			const Point& p2, double w2,
			const Point& p3, double w3)
{
  return Point(p1.d_[0]*w1+p2.d_[0]*w2+p3.d_[0]*w3,
		 p1.d_[1]*w1+p2.d_[1]*w2+p3.d_[1]*w3,
		 p1.d_[2]*w1+p2.d_[2]*w2+p3.d_[2]*w3);
}

Point AffineCombination(const Point& p1, double w1,
			const Point& p2, double w2,
			const Point& p3, double w3,
			const Point& p4, double w4)
{
  return Point(p1.d_[0]*w1+p2.d_[0]*w2+p3.d_[0]*w3+p4.d_[0]*w4,
		 p1.d_[1]*w1+p2.d_[1]*w2+p3.d_[1]*w3+p4.d_[1]*w4,
		 p1.d_[2]*w1+p2.d_[2]*w2+p3.d_[2]*w3+p4.d_[2]*w4);
}
#endif

std::ostream& operator<<( std::ostream& os, const Point& p )
{
  os << '[' << p.x() << ' ' << p.y() << ' ' << p.z() << ']';
  return os;
}

std::istream& operator>>( std::istream& is, Point& v)
{
  double x, y, z;
  char st;
  is >> st >> x >> st >> y >> st >> z >> st;
  v = Point(x,y,z);
  return is;
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
Point 
centroid(const std::vector<Point>& points)
{
	double cx = 0.0;
	double cy = 0.0;
	double cz = 0.0;

  size_t size = points.size();
  if (size)
  {
    for (size_t i = 0; i < size; i++) 
    {
      cx += points[i].x();
      cy += points[i].y();
      cz += points[i].z();
    }
    return Point(cx/size, cy/size, cz/size);
  }
  else
  {
    return Point(0.0,0.0,0.0);
  }
}

void 
Pio(Piostream& stream, Point& p)
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
Point_get_h_file_path() 
{
  static const std::string path(TypeDescription::cc_to_h(__FILE__));
  return path;
}

const TypeDescription* get_type_description(Point*)
{
  static TypeDescription* td = 0;
  if(!td){
    td = new TypeDescription("Point", Point_get_h_file_path(), 
				"SCIRun", TypeDescription::DATA_E);
  }
  return td;
}
#endif