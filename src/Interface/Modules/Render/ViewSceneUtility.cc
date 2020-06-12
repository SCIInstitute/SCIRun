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

#include <Interface/Modules/Render/ViewSceneUtility.h>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <sstream>

using namespace SCIRun::Render::Gui;

glm::quat ViewSceneUtility::stringToQuat(const std::string &s)
{
  double w, x, y, z;
  w = 1.0;
  x = y = z = 0.0f;
  if (!s.empty())
  {
    try
    {
      static boost::regex r("Quaternion\\((.+),\\s?(.+),\\s?(.+),\\s?(.+)\\)");
      boost::smatch what;
      regex_match(s, what, r);
      w = boost::lexical_cast<double>(what[1]);
      x = boost::lexical_cast<double>(what[2]);
      y = boost::lexical_cast<double>(what[3]);
      z = boost::lexical_cast<double>(what[4]);
    }
    catch (...)
    {
      // keeps as default view
    }
  }
  return glm::normalize(glm::quat(w, x, y, z));
}

std::string ViewSceneUtility::quatToString(const glm::quat &q)
{
  return "Quaternion(" + std::to_string((double)q.w) + ","
                       + std::to_string((double)q.x) + ","
                       + std::to_string((double)q.y) + ","
                       + std::to_string((double)q.z) + ")";
}
