#ifndef UTILITY_H
#define UTILITY_H

#include <sstream>

template <class Point>
std::string to_string(const Point& p)
{
  std::ostringstream ostr;
  ostr << "QPoint(" << p.x() << "," << p.y() << ")";
  return ostr.str();
}

#endif