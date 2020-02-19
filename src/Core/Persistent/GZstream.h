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
///@file  Pstream.h
///@brief reading/writing persistent objects
///
///@author
///       Michael Callahan
///       Department of Computer Science
///       University of Utah
///@date  June 2007
///

#ifndef SCI_project_GZstream_h
#define SCI_project_GZstream_h 1

#include <Core/Persistent/Persistent.h>
#include <stdio.h>

#include <zlib.h>


#include <iosfwd>


#include <Core/Persistent/share.h>

namespace SCIRun {


class SCISHARE GZPiostream : public Piostream {
protected:
  gzFile fp_;

  virtual const char *endianness();
  virtual void reset_post_header();
private:
  template <class T> void gen_io(T&, const char *);

public:
  GZPiostream(const std::string& filename, Direction dir,
                  const int& v = -1, ProgressReporter *pr = 0);
  GZPiostream(int fd, Direction dir, const int& v = -1,
                  ProgressReporter *pr = 0);
  virtual ~GZPiostream();

  virtual void io(char&);
  virtual void io(signed char&);
  virtual void io(unsigned char&);
  virtual void io(short&);
  virtual void io(unsigned short&);
  virtual void io(int&);
  virtual void io(unsigned int&);
  virtual void io(long&);
  virtual void io(unsigned long&);
  virtual void io(long long&);
  virtual void io(unsigned long long&);
  virtual void io(double&);
  virtual void io(float&);
  virtual void io(std::string& str);

  virtual bool supports_block_io() { return (version() > 1); }
  virtual bool block_io(void*, size_t, size_t);
};


class SCISHARE GZSwapPiostream : public GZPiostream {
protected:
  virtual const char *endianness();
private:
  template <class T> void gen_io(T&, const char *);

public:
  GZSwapPiostream(const std::string& filename, Direction d,
                      const int& v = -1, ProgressReporter *pr = 0);
  GZSwapPiostream(int fd, Direction dir, const int& v = -1,
                      ProgressReporter *pr = 0);
  virtual ~GZSwapPiostream();

  virtual void io(short&);
  virtual void io(unsigned short&);
  virtual void io(int&);
  virtual void io(unsigned int&);
  virtual void io(long&);
  virtual void io(unsigned long&);
  virtual void io(long long&);
  virtual void io(unsigned long long&);
  virtual void io(double&);
  virtual void io(float&);

  virtual bool supports_block_io() { return false; }
  virtual bool block_io(void*, size_t, size_t) { return false; }
};



PiostreamPtr
auto_gzistream(const std::string& filename, ProgressReporter *pr);


} // End namespace SCIRun


#endif
