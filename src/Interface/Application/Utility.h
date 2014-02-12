/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#ifndef UTILITY_H
#define UTILITY_H

#include <sstream>

namespace SCIRun {

template <class Point>
std::string to_string(const Point& p)
{
  std::ostringstream ostr;
  ostr << "QPoint(" << p.x() << "," << p.y() << ")";
  return ostr.str();
}

namespace Gui 
{
  //TODO un-inline

  inline QColor to_color(const std::string& str)
  {
    if (str == "red")
      return Qt::red;
    if (str == "blue")
      return Qt::blue;
    if (str == "darkBlue")
      return Qt::darkBlue;
    if (str == "cyan")
      return Qt::cyan;
    if (str == "darkCyan")
      return Qt::darkCyan;
    if (str == "darkGreen")
      return Qt::darkGreen;
    if (str == "cyan")
      return Qt::cyan;
    if (str == "magenta")
      return Qt::magenta;
    if (str == "white")
      return Qt::white;
    if (str == "yellow")
      return Qt::yellow;
    if (str == "darkYellow")
      return Qt::darkYellow;
    if (str == "lightGray")
      return Qt::lightGray;
    if (str == "darkGray")
      return Qt::darkGray;
    if (str == "black")
      return Qt::black;
    if (str == "purple")
      return Qt::darkMagenta;
    else
      return Qt::black;
  }

  inline QAction* separatorAction(QWidget* parent)
  {
    auto sep = new QAction(parent);
    sep->setSeparator(true);
    return sep;
  }

  inline QAction* disabled(QAction* action)
  {
    action->setEnabled(false);
    return action;
  }


  inline std::ostream& operator<<(std::ostream& o, const QPointF& p)
  {
    return o << "[" << p.x() << "," << p.y() << "]";
  }
}

}

#endif