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


/*
 *  Pstream.h: reading/writing persistent objects
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   April 1994
 *
 */

#ifndef SCI_project_Pstream_h
#define SCI_project_Pstream_h 1

#include <Core/Persistent/Persistent.h>
#include <cstdio>
#include <iosfwd>

#include <Core/Persistent/share.h>

namespace SCIRun {

class SCISHARE BinaryPiostream : public Piostream {
protected:
  FILE* fp_;

  virtual const char *endianness();
  void reset_post_header() override;
private:
  template <class T> void gen_io(T&, const char *);

public:
  BinaryPiostream(const std::string& filename, Direction dir,
                  const int& v = -1, Core::Logging::LoggerHandle pr = Core::Logging::LoggerHandle());
  BinaryPiostream(int fd, Direction dir, const int& v = -1,
                  Core::Logging::LoggerHandle pr = Core::Logging::LoggerHandle());
  virtual ~BinaryPiostream();

  void io(char&) override;
  void io(signed char&) override;
  void io(unsigned char&) override;
  void io(short&) override;
  void io(unsigned short&) override;
  void io(int&) override;
  void io(unsigned int&) override;
  void io(long&) override;
  void io(unsigned long&) override;
  void io(long long&) override;
  void io(unsigned long long&) override;
  void io(double&) override;
  void io(float&) override;
  void io(std::string& str) override;

  bool supports_block_io() override { return (version() > 1); }
  bool block_io(void*, size_t, size_t) override;
};


class SCISHARE BinarySwapPiostream : public BinaryPiostream {
protected:
  const char *endianness() override;
private:
  template <class T> void gen_io(T&, const char *);

public:
  BinarySwapPiostream(const std::string& filename, Direction d,
                      const int& v = -1, Core::Logging::LoggerHandle pr = Core::Logging::LoggerHandle());
  BinarySwapPiostream(int fd, Direction dir, const int& v = -1,
                      Core::Logging::LoggerHandle pr = Core::Logging::LoggerHandle());
  virtual ~BinarySwapPiostream();

  void io(short&) override;
  void io(unsigned short&) override;
  void io(int&) override;
  void io(unsigned int&) override;
  void io(long&) override;
  void io(unsigned long&) override;
  void io(long long&) override;
  void io(unsigned long long&) override;
  void io(double&) override;
  void io(float&) override;

  bool supports_block_io() override { return false; }
  bool block_io(void*, size_t, size_t) override { return false; }
};


class SCISHARE TextPiostream : public Piostream {
private:
  std::istream* istr;
  std::ostream* ostr;
  bool ownstreams_p_;

  void expect(char);
  void next_entry();
  void emit_pointer(int&, int&) override;
  void ioString(bool do_quotes, std::string& str);
protected:
  void reset_post_header() override;
public:
  TextPiostream(const std::string& filename, Direction dir,
                Core::Logging::LoggerHandle pr = Core::Logging::LoggerHandle());
  TextPiostream(std::istream *strm, Core::Logging::LoggerHandle pr = Core::Logging::LoggerHandle());
  TextPiostream(std::ostream *strm, Core::Logging::LoggerHandle pr = Core::Logging::LoggerHandle());
  virtual ~TextPiostream();

  std::string peek_class() override;
  int begin_class(const std::string& name, int) override;
  void end_class() override;
  void begin_cheap_delim() override;
  void end_cheap_delim() override;

  void io(bool&) override;
  void io(char&) override;
  void io(signed char&) override;
  void io(unsigned char&) override;
  void io(short&) override;
  void io(unsigned short&) override;
  void io(int&) override;
  void io(unsigned int&) override;
  void io(long&) override;
  void io(unsigned long&) override;
  void io(long long&) override;
  void io(unsigned long long&) override;
  void io(double&) override;
  void io(float&) override;
  void io(std::string& str) override;
  bool eof() override;
};


/// The Fast stream is binary, its results can only safely be used
/// on the architecture where the file is generated.
class SCISHARE FastPiostream : public Piostream {
private:
  FILE* fp_;

  void report_error(const char *);
  template <class T> void gen_io(T&, const char *);
protected:
  void reset_post_header() override;
public:
  FastPiostream(const std::string& filename, Direction dir,
                Core::Logging::LoggerHandle pr = Core::Logging::LoggerHandle());
  FastPiostream(int fd, Direction dir,
                Core::Logging::LoggerHandle pr = Core::Logging::LoggerHandle());
  virtual ~FastPiostream();

  void io(bool&) override;
  void io(char&) override;
  void io(signed char&) override;
  void io(unsigned char&) override;
  void io(short&) override;
  void io(unsigned short&) override;
  void io(int&) override;
  void io(unsigned int&) override;
  void io(long&) override;
  void io(unsigned long&) override;
  void io(long long&) override;
  void io(unsigned long long&) override;
  void io(double&) override;
  void io(float&) override;
  void io(std::string& str) override;

  bool supports_block_io() override { return true; }
  bool block_io(void*, size_t, size_t) override;
};


} // End namespace SCIRun


#endif
