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

#ifndef LOGGER_H
#define LOGGER_H

#include <boost/shared_ptr.hpp>
#include <QString>

namespace SCIRun {
namespace Gui {

  //TODO move to separate header

  inline QColor to_color(const std::string& str)
  {
    if (str == "red")
      return Qt::red;
    if (str == "blue")
      return Qt::blue;
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
    else
      return Qt::black;
  }

  //TODO: merge this with Core_Logging...
  class Logger
  {
  public:
    virtual ~Logger() {}
    virtual void log(const QString& message) const = 0;
    static boost::shared_ptr<Logger> Instance() { return instance_; }
    static void set_instance(Logger* p) { instance_.reset(p); }
  private:
    static boost::shared_ptr<Logger> instance_;
  };

}
}

#endif