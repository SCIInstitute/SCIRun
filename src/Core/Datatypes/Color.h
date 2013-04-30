/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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


#ifndef CORE_DATATYPES_COLORRGB_H
#define CORE_DATATYPES_COLORRGB_H 

#include <iosfwd>
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  class SCISHARE ColorRGB 
  {
  private:
    int r_, g_, b_;
  public:
    ColorRGB();
    ColorRGB(int, int, int);

    inline bool operator==(const ColorRGB& c) const {
      return ((r_==c.r_)&&(g_==c.g_)&&(b_==c.b_));
    }

    inline bool operator!=(const ColorRGB& c) const {
      return !(*this == c);
    }

    inline int r() const {return r_;}
    inline int g() const {return g_;}
    inline int b() const {return b_;}
  };

  typedef boost::shared_ptr<ColorRGB> ColorRGBHandle;

  SCISHARE std::ostream& operator<<(std::ostream& out, const ColorRGB& color);

}}}


#endif
