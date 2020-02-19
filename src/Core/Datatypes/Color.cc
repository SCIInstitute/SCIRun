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


///@todo Documentation Core/Datatypes/Color.cc

#include <sstream>
#include <Core/Datatypes/Color.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

using namespace SCIRun::Core::Datatypes;

ColorRGB::ColorRGB()
  : r_(1.0), g_(1.0), b_(1.0), a_(1.0)
{
}

ColorRGB::ColorRGB(double v)
  : r_(v), g_(v), b_(v), a_(1.0)
{
}

ColorRGB::ColorRGB(double r, double g, double b)
  : r_(r), g_(g), b_(b), a_(1.0)
{
}

ColorRGB::ColorRGB(double r, double g, double b, double a)
  : r_(r), g_(g), b_(b), a_(a)
{
}

ColorRGB::ColorRGB(const std::string& rgb) : r_(1.0), g_(1.0), b_(1.0), a_(1.0)
{
  if (!rgb.empty())
  {
    try
    {
      static boost::regex r("Color\\((.+),(.+),(.+)\\)");
      boost::smatch what;
      regex_match(rgb, what, r);
      r_ = boost::lexical_cast<double>(what[1]);
      g_ = boost::lexical_cast<double>(what[2]);
      b_ = boost::lexical_cast<double>(what[3]);
    }
    catch (...)
    {
      //error results in white (so you can see things in viewer):
      r_ = g_ = b_ = 1.0f;
    }
  }
}

ColorRGB::ColorRGB(unsigned int rgbHexValue) :
  r_(((rgbHexValue & 0xFF0000) >> 16) / 255.0),
  g_(((rgbHexValue & 0xFF00) >> 8) / 255.0),
  b_((rgbHexValue & 0xFF) / 255.0),
  a_(1.0)
{
}

std::string ColorRGB::toString() const
{
  std::ostringstream ostr;
  ostr << *this;
  return ostr.str();
}

std::ostream& SCIRun::Core::Datatypes::operator<<(std::ostream& out, const ColorRGB& color)
{
  out << "Color(" << color.r() << "," << color.g() << "," << color.b() << ")";
  return out;
}

bool ViewSceneFeedback::matchesWithModuleId(const std::string& modId) const
{
  auto toMatch = GeometryObject::delimiter + modId + GeometryObject::delimiter;
  return selectionName.find(toMatch) != std::string::npos;
}
