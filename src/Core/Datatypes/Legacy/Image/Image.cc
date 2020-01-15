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
///@file  Image.cc
///
///@author
///   Sourced from MeshPort.cc by David Weinstein
///   Department of Computer Science
///   University of Utah
///

#include <Core/Datatypes/Image.h>
#include <Core/Persistent/Persistent.h>

namespace SCIRun {

PersistentTypeID Image::type_id("Image", "Datatype", 0);

ColorImage::ColorImage(int xres, int yres)
: imagedata(yres, xres)
{
}

ColorImage::~ColorImage()
{
}

DepthImage::DepthImage(int xres, int yres)
: depthdata(yres, xres)
{
}

DepthImage::~DepthImage()
{
}

Image::Image(int xres, int yres)
: xr(xres), yr(yres)
{
  if(xres && yres)
  {
    rows=new float*[yres];
    float* p=new float[xres*yres*2];
    for(int y=0;y<yres;y++)
    {
	    rows[y]=p;
	    p+=xres*2;
    }
  }
  else
  {
    rows=0;
  }
}

Image::~Image()
{
  if(rows)
  {
    delete[] rows[0];
    delete[] rows;
  }
}

int Image::xres() const
{
  return xr;
}

int Image::yres() const
{
  return yr;
}

Image* Image::clone()
{
  return new Image(*this);
}

Image::Image(const Image& copy)
: Datatype(copy), xr(copy.xr), yr(copy.yr)
{
  if(xr && yr)
  {
    rows=new float*[yr];
    float* p=new float[xr*yr*2];
    for(int y=0;y<yr;y++)
    {
      rows[y]=p;
      p+=xr*2;
      for(int x=0;x<xr*2;x++)
        rows[y][x]=copy.rows[y][x];
    }
  }
}

#define IMAGE_VERSION 1

void Image::io(Piostream& stream)
{
  stream.begin_class("Image", IMAGE_VERSION);
  stream.io(xr);
  stream.io(yr);
  if(stream.reading())
  {
    if(rows)
    {
	    delete[] rows[0];
	    delete[] rows;
    }

    rows=new float*[yr];
    float* p=new float[xr*yr*2];
    for(int y=0;y<yr;y++)
    {
	    rows[y]=p;
	    p+=xr*2;
    }
  }
  int n=xr*yr*2;
  float* p=rows[0];
  for(int i=0;i<n;i++) stream.io(*p++);
  stream.end_class();
}

float Image::max_abs()
{
  float max=0;
  float* p=rows[0];
  for(int y=0;y<yr;y++)
  {
    for(int x=0;x<xr;x++)
    {
	    float r=*p;
	    if(r>max) max=r;
	    p+=2;
    }
  }
  return max;
}

} // End namespace SCIRun
