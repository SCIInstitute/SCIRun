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


///
///@file Image.h
///
///@author
///      Sourced from MeshPort.cc by David Weinstein
///      Department of Computer Science
///      University of Utah
///

#ifndef SCI_DATATYPES_IMAGE_H
#define SCI_DATATYPES_IMAGE_H 1

#include <Core/Datatypes/Datatype.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Containers/Array2.h>
#include <Core/Datatypes/Color.h>

#include <Core/Datatypes/share.h>

namespace SCIRun {


class Image;
typedef LockingHandle<Image> ImageHandle;

class SCISHARE Image : public Datatype {
  /* Complex... */
public:
  float** rows;
  int xr, yr;
  Image(int xres, int yres);
  Image(const Image&);
  virtual ~Image();
  int xres() const;
  int yres() const;

  inline float getr(int x, int y) {
    return rows[y][x*2];
  }
  inline void set(int x, int y, float r, float i) {
    rows[y][x*2]=r;
    rows[y][x*2+1]=i;
  }
  float max_abs();

  virtual Image* clone();

  // Persistent representation...
  virtual void io(Piostream&);
  static PersistentTypeID type_id;
  virtual std::string dynamic_type_name() const { return type_id.type; }
};

class SCISHARE ColorImage {
public:
  ColorImage(int xres, int yres);
  ~ColorImage();
  Array2<Color> imagedata;
  inline Color& get_pixel(int x, int y) {
    return imagedata(y,x);
  }
  inline void put_pixel(int x, int y, const Color& pixel) {
    imagedata(y,x)=pixel;
  }
  int xres() const;
  int yres() const;
};

class SCISHARE DepthImage {
public:
  DepthImage(int xres, int yres);
  ~DepthImage();
  Array2<double> depthdata;
  double get_depth(int x, int y) {
    return depthdata(y,x);
  }
  inline void put_pixel(int x, int y, double depth) {
    depthdata(y,x)=depth;
  }
  int xres() const;
  int yres() const;
};

} // End namespace SCIRun


#endif
