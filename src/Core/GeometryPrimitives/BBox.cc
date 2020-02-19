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


#include <cmath>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/Persistent/Persistent.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;

void
BBox::extend_disk(const Point& cen, const Vector& normal, double r)
{
  if (normal.length2() < 1.e-12) { extend(cen); return; }
  Vector n(normal);
  n.safe_normalize();
  double x = std::sqrt(1-n.x())*r;
  double y = std::sqrt(1-n.y())*r;
  double z = std::sqrt(1-n.z())*r;
  extend(cen+Vector(x,y,z));
  extend(cen-Vector(x,y,z));
}

// if diff is 1.0, the two bboxes have to have about 50% overlap each for x,y,z
// if diff is 0.0, they have to have 100% overlap
bool
BBox::is_similar_to(const BBox &b, double diff) const
{
  if (!is_valid_ || !b.valid()) return false;
  Vector acceptable_diff=((diagonal()+b.diagonal())/2.0)*diff;
  Vector min_diff(cmin_-b.get_min());
  min_diff = Vector(fabs(min_diff.x()),fabs(min_diff.y()),fabs(min_diff.z()));
  if (min_diff.x()>acceptable_diff.x()) return false;
  if (min_diff.y()>acceptable_diff.y()) return false;
  if (min_diff.z()>acceptable_diff.z()) return false;
  Vector max_diff(cmax_ - b.get_max());
  max_diff = Vector(fabs(max_diff.x()),fabs(max_diff.y()),fabs(max_diff.z()));
  if (max_diff.x()>acceptable_diff.x()) return false;
  if (max_diff.y()>acceptable_diff.y()) return false;
  if (max_diff.z()>acceptable_diff.z()) return false;
  return true;
}

void
BBox::translate(const Vector &v)
{
  cmin_+=v;
  cmax_+=v;
}

void
BBox::scale(double s, const Vector&o)
{
  cmin_-=o;
  cmax_-=o;
  cmin_*=s;
  cmax_*=s;
  cmin_+=o;
  cmax_+=o;
}

bool
BBox::overlaps(const BBox & bb) const
{
  if( bb.cmin_.x() > cmax_.x() || bb.cmax_.x() < cmin_.x())
    return false;
  else if( bb.cmin_.y() > cmax_.y() || bb.cmax_.y() < cmin_.y())
    return false;
  else if( bb.cmin_.z() > cmax_.z() || bb.cmax_.z() < cmin_.z())
    return false;

  return true;
}

bool
BBox::overlaps_inside(const BBox & bb) const
{
  if( bb.cmin_.x() >= cmax_.x() || bb.cmax_.x() <= cmin_.x())
    return false;
  else if( bb.cmin_.y() >= cmax_.y() || bb.cmax_.y() <= cmin_.y())
    return false;
  else if( bb.cmin_.z() >= cmax_.z() || bb.cmax_.z() <= cmin_.z())
    return false;

  return true;
}

bool
BBox::intersect(const Point& origin, const Vector& dir, Point& hitPoint) const
{
  Vector t1 = (cmin_ - origin) / dir;
  Vector t2 = (cmax_ - origin) / dir;
  Vector tn = Min(t1, t2);
  Vector tf = Max(t1, t2);
  double tnear = tn.maxComponent();
  double tfar = tf.minComponent();
  if (tnear <= tfar)
  {
    hitPoint = origin + dir*tnear;
    return true;
  }
  else
  {
    return false;
  }
}

void SCIRun::Core::Geometry::Pio(Piostream &stream, BBox & box)
{
  stream.begin_cheap_delim();

  // Store the valid flag as an int to be backwards compatible.
  int tmp = box.valid();
  Point min = box.get_min();
  Point max = box.get_max();
  Pio(stream, tmp);
  if (tmp)
  {
    Pio(stream, min);
    Pio(stream, max);
  }

  if(stream.reading())
  {
    box.set_valid(tmp);
    if (tmp)
    {
      box.extend(min);
      box.extend(max);
    }
  }
  stream.end_cheap_delim();
}

std::ostream& SCIRun::Core::Geometry::operator<<(std::ostream& out, const BBox& b)
{
  return out << b.get_min() << " : " << b.get_max();
}
