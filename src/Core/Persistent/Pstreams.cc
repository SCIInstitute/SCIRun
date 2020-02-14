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
///@file  Pstreams.cc
///@brief Reading/writing persistent objects
///
///@authors
///       Steven G. Parker,
///     Modified by:
///       Michelle Miller
///       Department of Computer Science
///       University of Utah
///@date April 1994 Modified:
///     Thu Feb 19 17:04:59 MST 1998
///

#include <Core/Persistent/Pstreams.h>
#include <Core/Logging/LoggerInterface.h>
#include <Core/Utils/Legacy/StringUtil.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#include <teem/air.h>
#include <teem/nrrd.h>
#include <zlib.h>
#endif

#include <string.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef _WIN32
#  include <io.h>
#endif

using namespace SCIRun::Core::Logging;

namespace SCIRun {

// BinaryPiostream -- portable
  BinaryPiostream::BinaryPiostream(const std::string& filename, Direction dir,
    const int& v, LoggerHandle pr)
    : Piostream(dir, v, filename, pr),
    fp_(0)
  {
    if (v == -1) // no version given so use PERSISTENT_VERSION
      version_ = PERSISTENT_VERSION;
    else
      version_ = v;

    if (dir==Read)
    {
      fp_ = fopen (filename.c_str(), "rb");
      if (!fp_)
      {
        reporter_->error("Error opening file: " + filename + " for reading.");
        err = true;
        return;
      }

      // Old versions had headers of size 12.
      if (version() == 1)
      {
        char hdr[12];
        if (!fread(hdr, 1, 12, fp_))
        {
          reporter_->error("Header read failed.");
          err = true;
          return;
        }
      }
      else
      {
        // Versions > 1 have size of 16 to account for endianness in
        // header (LIT | BIG).
        char hdr[16];
        if (!fread(hdr, 1, 16, fp_))
        {
          reporter_->error("Header read failed.");
          err = true;
          return;
        }
      }
    }
    else
    {
      fp_ = fopen(filename.c_str(), "wb");
      if (!fp_)
      {
        reporter_->error("Error opening file '" + filename + "' for writing.");
        err = true;
        return;
      }

      if (version() > 1)
      {
        // write out 16 bytes, but we need 17 for \0
        char hdr[17];
        sprintf(hdr, "SCI\nBIN\n%03d\n%s", version_, endianness());

        if (!fwrite(hdr, 1, 16, fp_))
        {
          reporter_->error("Header write failed.");
          err = true;
          return;
        }
      }
      else
      {
        // write out 12 bytes, but we need 13 for \0
        char hdr[13];
        sprintf(hdr, "SCI\nBIN\n%03d\n", version_);
        if (!fwrite(hdr, 1, 13, fp_))
        {
          reporter_->error("Header write failed.");
          err = true;
          return;
        }
      }
    }
  }


BinaryPiostream::BinaryPiostream(int fd, Direction dir, const int& v,
                                 LoggerHandle pr)
  : Piostream(dir, v, "", pr),
    fp_(0)
{
  if (v == -1) // No version given so use PERSISTENT_VERSION.
    version_ = PERSISTENT_VERSION;
  else
    version_ = v;

  if (dir == Read)
  {
    fp_ = fdopen (fd, "rb");
    if (!fp_)
    {
      reporter_->error("Error opening socket " + to_string(fd) +
                       " for reading.");
      err = true;
      return;
    }

    // Old versions had headers of size 12.
    if (version() == 1)
    {
      char hdr[12];

      // read header
      if (!fread(hdr, 1, 12, fp_))
      {
        reporter_->error("Header read failed.");
        err = true;
        return;
      }
    }
    else
    {
      // Versions > 1 have size of 16 to account for endianness in
      // header (LIT | BIG).
      char hdr[16];
      if (!fread(hdr, 1, 16, fp_))
      {
        reporter_->error("Header read failed.");
        err = true;
        return;
      }
    }
  }
  else
  {
    fp_ = fdopen(fd, "wb");
    if (!fp_)
    {
      reporter_->error("Error opening socket " + to_string(fd) +
                       " for writing.");
      err = true;
      return;
    }

    if (version() > 1)
    {
      char hdr[16];
      sprintf(hdr, "SCI\nBIN\n%03d\n%s", version_, endianness());

      if (!fwrite(hdr, 1, 16, fp_))
      {
	reporter_->error("Header write failed.");
	err = true;
	return;
      }
    }
    else
    {
      char hdr[12];
      sprintf(hdr, "SCI\nBIN\n%03d\n", version_);
      if (!fwrite(hdr, 1, 12, fp_))
      {
	reporter_->error("Header write failed.");
	err = true;
	return;
      }
    }
  }
}


BinaryPiostream::~BinaryPiostream()
{
  if (fp_) fclose(fp_);
}

void
BinaryPiostream::reset_post_header()
{
  if (! reading()) return;

  fseek(fp_, 0, SEEK_SET);

  if (version() == 1)
  {
    // Old versions had headers of size 12.
    char hdr[12];
    // read header
    fread(hdr, 1, 12, fp_);
  }
  else
  {
    // Versions > 1 have size of 16 to account for endianness in
    // header (LIT | BIG).
    char hdr[16];
    // read header
    fread(hdr, 1, 16, fp_);
  }
}

const char *
BinaryPiostream::endianness()
{
  /// @todo SCIRUN4_CODE_TO_BE_ENABLED_LATER
  //if (airMyEndian == airEndianLittle)
    return "LIT\n";
  //else
    //return "BIG\n";

}


template <class T>
inline void
BinaryPiostream::gen_io(T& data, const char *iotype)
{
  if (err) return;
  if (dir==Read)
  {
    if (!fread(&data, sizeof(data), 1, fp_))
    {
      err = true;
      reporter_->error(std::string("BinaryPiostream error reading ") +
                       iotype + ".");
    }
  }
  else
  {
    if (!fwrite(&data, sizeof(data), 1, fp_))
    {
      err = true;
      reporter_->error(std::string("BinaryPiostream error writing ") +
                       iotype + ".");
    }
  }
}


void
BinaryPiostream::io(char& data)
{
  if (version() == 1)
  {
    // xdr_char
    int tmp;
    tmp = data;
    io(tmp);
    data = tmp;
  }
  else
  {
    gen_io(data, "char");
  }
}


void
BinaryPiostream::io(signed char& data)
{
  if (version() == 1)
  {
    // Wrote as short, still xdr int eventually.
    short tmp = data;
    io(tmp);
    data = tmp;
  }
  else
  {
    gen_io(data, "signed char");
  }
}


void
BinaryPiostream::io(unsigned char& data)
{
  if (version() == 1)
  {
    // xdr_u_char
    unsigned int tmp;
    tmp = data;
    io(tmp);
    data = tmp;
  }
  else
  {
    gen_io(data, "unsigned char");
  }
}


void
BinaryPiostream::io(short& data)
{
  if (version() == 1)
  {
    // xdr_short
    int tmp;
    tmp = data;
    io(tmp);
    data = tmp;
  }
  else
  {
    gen_io(data, "short");
  }
}


void
BinaryPiostream::io(unsigned short& data)
{
  if (version() == 1)
  {
    // xdr_u_short
    unsigned int tmp;
    tmp = data;
    io(tmp);
    data = tmp;
  }
  else
  {
    gen_io(data, "unsigned short");
  }
}


void
BinaryPiostream::io(int& data)
{
  gen_io(data, "int"); // xdr_int
}


void
BinaryPiostream::io(unsigned int& data)
{
  gen_io(data, "unsigned int"); // xdr_u_int
}


void
BinaryPiostream::io(long& data)
{
  if (version() == 1)
  {
    // xdr_long
    // Note that xdr treats longs as 4 byte numbers.
    int tmp;
    tmp = data;
    io(tmp);
    data = tmp;
  }
  else
  {
    // For 32bits for now (we need to update this later in the next
    // version of Pio)
    int tmp = data;
    gen_io(tmp, "long");
    data = tmp;
  }
}


void
BinaryPiostream::io(unsigned long& data)
{
  if (version() == 1)
  {
    // xdr_u_long
    // Note that xdr treats longs as 4 byte numbers.
    unsigned int tmp;
    tmp = data;
    io(tmp);
    data = tmp;
  }
  else
  {
    // For 32bits for now (we need to update this later in the next
    // version of Pio)
    unsigned int tmp = data;
    gen_io(tmp, "unsigned long");
    data = tmp;
  }
}


void
BinaryPiostream::io(long long& data)
{
  gen_io(data, "long long"); // xdr_longlong_t
}


void
BinaryPiostream::io(unsigned long long& data)
{
  gen_io(data, "unsigned long long"); //xdr_u_longlong_t
}


void
BinaryPiostream::io(double& data)
{
  gen_io(data, "double"); // xdr_double
}


void
BinaryPiostream::io(float& data)
{
  gen_io(data, "float"); // xdr_float
}


void
BinaryPiostream::io(std::string& data)
{
  // xdr_wrapstring
  if (err) return;
  unsigned int chars = 0;
  if (dir == Write)
  {
    if (version() == 1)
    {
      // An xdr string is 4 byte int for string size, followed by the
      // characters without the null termination, then padded back out
      // to the 4 byte boundary with zeros.
      chars = data.size();
      io(chars);
      if (!fwrite(data.c_str(), sizeof(char), chars, fp_)) err = true;

      // Pad data out to 4 bytes.
      int extra = chars % 4;
      if (extra)
      {
        static const char pad[4] = {0, 0, 0, 0};
        if (!fwrite(pad, sizeof(char), 4 - extra, fp_)) err = true;
      }
    }
    else
    {
      const char* p = data.c_str();
      chars = static_cast<int>(strlen(p)) + 1;
      io(chars);
      if (!fwrite(p, sizeof(char), chars, fp_)) err = true;
    }
  }
  if (dir == Read)
  {
    // Read in size.
    io(chars);

    if (version() == 1)
    {
      // Create buffer which is multiple of 4.
      int extra = 4 - (chars%4);
      if (extra == 4)
        extra = 0;
      unsigned int buf_size = chars + extra;
      data = "";
      if (buf_size)
      {
        char* buf = new char[buf_size];

        // Read in data plus padding.
        if (!fread(buf, sizeof(char), buf_size, fp_))
        {
          err = true;
          delete [] buf;
          return;
        }

        // Only use actual size of string.
        for (unsigned int i=0; i<chars; i++)
          data += buf[i];
        delete [] buf;
      }
    }
    else
    {
      char* buf = new char[chars];
      fread(buf, sizeof(char), chars, fp_);
      data = std::string(buf);
      delete[] buf;
    }
  }
}


bool
BinaryPiostream::block_io(void *data, size_t s, size_t nmemb)
{
  if (err || version() == 1) { return false; }
  if (dir == Read)
  {
    const size_t did = fread(data, s, nmemb, fp_);
    if (did != nmemb)
    {
      err = true;
      reporter_->error("BinaryPiostream error reading block io.");
    }
  }
  else
  {
    const size_t did = fwrite(data, s, nmemb, fp_);
    if (did != nmemb)
    {
      err = true;
      reporter_->error("BinaryPiostream error writing block io.");
    }
  }
  return true;
}



////
// BinarySwapPiostream -- portable
// Piostream used when endianness of machine and file don't match
BinarySwapPiostream::BinarySwapPiostream(const std::string& filename, Direction dir,
                                         const int& v, LoggerHandle pr)
  : BinaryPiostream(filename, dir, v, pr)
{
}


BinarySwapPiostream::BinarySwapPiostream(int fd, Direction dir, const int& v,
                                         LoggerHandle pr)
  : BinaryPiostream(fd, dir, v, pr)
{
}


BinarySwapPiostream::~BinarySwapPiostream()
{
}


const char *
BinarySwapPiostream::endianness()
{
  /// @todo SCIRUN4_CODE_TO_BE_ENABLED_LATER
  //if (airMyEndian == airEndianLittle)
    return "LIT\n";
  //else
    //return "BIG\n";
}

template <class T>
inline void
BinarySwapPiostream::gen_io(T& data, const char *iotype)
{
  if (err) return;
  if (dir==Read)
  {
    unsigned char tmp[sizeof(data)];
    if (!fread(tmp, sizeof(data), 1, fp_))
    {
      err = true;
      reporter_->error(std::string("BinaryPiostream error reading ") +
                       iotype + ".");
      return;
    }
    unsigned char *cdata = reinterpret_cast<unsigned char *>(&data);
    for (unsigned int i = 0; i < sizeof(data); i++)
    {
      cdata[i] = tmp[sizeof(data)-i-1];
    }
  }
  else
  {
#if 0
    unsigned char tmp[sizeof(data)];
    unsigned char *cdata = reinterpret_cast<unsigned char *>(&data);
    for (unsigned int i = 0; i < sizeof(data); i++)
    {
      tmp[i] = cdata[sizeof(data)-i-1];
    }
    if (!fwrite(tmp, sizeof(data), 1, fp_))
    {
      err = true;
      reporter_->error(std::string("BinaryPiostream error writing ") +
                       iotype + ".");
    }
#else
    if (!fwrite(&data, sizeof(data), 1, fp_))
    {
      err = true;
      reporter_->error(std::string("BinaryPiostream error writing ") +
                       iotype + ".");
    }
#endif
  }
}



void
BinarySwapPiostream::io(short& data)
{
  if (version() == 1)
  {
    // xdr_short
    int tmp;
    tmp = data;
    io(tmp);
    data = tmp;
  }
  else
  {
    gen_io(data, "short");
  }
}


void
BinarySwapPiostream::io(unsigned short& data)
{
  if (version() == 1)
  {
    // xdr_u_short
    unsigned int tmp;
    tmp = data;
    io(tmp);
    data = tmp;
  }
  else
  {
    gen_io(data, "unsigned short");
  }
}


void
BinarySwapPiostream::io(int& data)
{
  gen_io(data, "int");
}


void
BinarySwapPiostream::io(unsigned int& data)
{
  gen_io(data, "unsigned int");
}


void
BinarySwapPiostream::io(long& data)
{
  if (version() == 1)
  {
    // xdr_long
    // Note that xdr treats longs as 4 byte numbers.
    int tmp;
    tmp = data;
    io(tmp);
    data = tmp;
  }
  else
  {
    gen_io(data, "long");
  }
}


void
BinarySwapPiostream::io(unsigned long& data)
{
  if (version() == 1)
  {
    // xdr_u_long
    // Note that xdr treats longs as 4 byte numbers.
    unsigned int tmp;
    tmp = data;
    io(tmp);
    data = tmp;
  }
  else
  {
    gen_io(data, "unsigned long");
  }
}


void
BinarySwapPiostream::io(long long& data)
{
  gen_io(data, "long long");
}


void
BinarySwapPiostream::io(unsigned long long& data)
{
  gen_io(data, "unsigned long long");
}


void
BinarySwapPiostream::io(double& data)
{
  gen_io(data, "double");
}


void
BinarySwapPiostream::io(float& data)
{
  gen_io(data, "float");
}



TextPiostream::TextPiostream(const std::string& filename, Direction dir,
                             LoggerHandle pr)
  : Piostream(dir, -1, filename, pr),
    ownstreams_p_(true)
{
  if (dir==Read)
  {
    ostr=0;
    istr=new std::ifstream(filename.c_str());
    if (!istr)
    {
      reporter_->error("Error opening file: " + filename + " for reading.");
      err = true;
      return;
    }
    char hdr[12];
    istr->read(hdr, 8);
    if (!*istr)
    {
      reporter_->error("Error reading header of file: " + filename);
      err = true;
      return;
    }
    int c=8;
    while (*istr && c < 12)
    {
      hdr[c]=istr->get();
      if (hdr[c] == '\n')
	break;
      c++;
    }
    if (!readHeader(reporter_, filename, hdr, "ASC", version_, file_endian))
    {
      reporter_->error("Error parsing header of file: " + filename);
      err = true;
      return;
    }
  }
  else
  {
    istr=0;
    ostr = new std::ofstream(filename.c_str(), std::ios_base::binary | std::ios_base::out);
    std::ostream& out=*ostr;
    if (!out)
    {
      reporter_->error("Error opening file: " + filename + " for writing.");
      err = true;
      return;
    }
    out << "SCI\nASC\n" << PERSISTENT_VERSION << "\n";
    version_ = PERSISTENT_VERSION;
  }
}


TextPiostream::TextPiostream(std::istream *strm, LoggerHandle pr)
  : Piostream(Read, -1, "", pr),
    istr(strm),
    ostr(0),
    ownstreams_p_(false)
{
  char hdr[12];
  istr->read(hdr, 8);
  if (!*istr)
  {
    reporter_->error("Error reading header of istream.");
    err = true;
    return;
  }
  int c=8;
  while (*istr && c < 12)
  {
    hdr[c]=istr->get();
    if (hdr[c] == '\n')
      break;
    c++;
  }
  if (!readHeader(reporter_, "istream", hdr, "ASC", version_, file_endian))
  {
    reporter_->error("Error parsing header of istream.");
    err = true;
    return;
  }
}


TextPiostream::TextPiostream(std::ostream *strm, LoggerHandle pr)
  : Piostream(Write, -1, "", pr),
    istr(0),
    ostr(strm),
    ownstreams_p_(false)
{
  std::ostream& out=*ostr;
  out << "SCI\nASC\n" << PERSISTENT_VERSION << "\n";
  version_ = PERSISTENT_VERSION;
}


TextPiostream::~TextPiostream()
{
  if (ownstreams_p_)
  {
    delete istr;
    delete ostr;
  }
}

void
TextPiostream::reset_post_header()
{
  if (! reading()) return;
  istr->seekg(0, std::ios::beg);

  char hdr[12];
  istr->read(hdr, 8);
  int c=8;
  while (*istr && c < 12)
  {
    hdr[c]=istr->get();
    if (hdr[c] == '\n')
      break;
    c++;
  }
}

void
TextPiostream::ioString(bool do_quotes, std::string& str)
{
  if (do_quotes)
  {
    io(str);
  }
  else
  {
    if (dir==Read)
    {
      char buf[1000];
      char* p=buf;
      int n=0;
      std::istream& in=*istr;
      for(;;)
      {
	char c;
	in.get(c);
	if (!in)
        {
	  reporter_->error("String input failed.");
	  char buf[100];
	  in.clear();
	  in.getline(buf, 100);
	  reporter_->error(std::string("Rest of line is: ") + buf);
	  err = true;
	  break;
	}
	if (c == ' ')
	  break;
	else
	  *p++=c;
	if (n++ > 998)
        {
	  reporter_->error("String too long.");
	  char buf[100];
	  in.clear();
	  in.getline(buf, 100);
	  reporter_->error(std::string("Rest of line is: ") + buf);
	  err = true;
	  break;
	}
      }
      *p=0;
      str = std::string(buf);
    }
    else
    {
      *ostr << str << " ";
    }
  }
}


std::string
TextPiostream::peek_class()
{
  // if we have the name already do nothing
  if (have_peekname_)
  {
    return peekname_;
  }
  else
  {
    expect('{');
    ioString(false, peekname_);
    have_peekname_ = true;
    return peekname_;
  }
}


int
TextPiostream::begin_class(const std::string& classname, int current_version)
{
  if (err) return -1;
  int version = current_version;
  std::string gname;
  if (dir==Write)
  {
    gname=classname;
    *ostr << '{';
    ioString(false, gname);
  }
  else if (dir==Read && have_peekname_)
  {
    gname = peekname_;
  }
  else
  {
    expect('{');
    ioString(false, gname);
  }
  have_peekname_ = false;

  if (dir==Read)
  {

  }
  io(version);

  if (dir == Read && version > current_version)
  {
    err = true;
    reporter_->error("File too new.  " + classname + " has version " +
                     to_string(version) +
                     ", but this scirun build is at version " +
                     to_string(current_version) + ".");
  }

  return version;
}


void
TextPiostream::end_class()
{
  if (err) return;

  if (dir==Read)
  {
    expect('}');
    expect('\n');
  }
  else
  {
    *ostr << "}\n";
  }
}


void
TextPiostream::begin_cheap_delim()
{
  if (err) return;
  if (dir==Read)
  {
    expect('{');
  }
  else
  {
    *ostr << "{";
  }
}


void
TextPiostream::end_cheap_delim()
{
  if (err) return;
  if (dir==Read)
  {
    expect('}');
  }
  else
  {
    *ostr << "}";
  }
}


void
TextPiostream::io(bool& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      reporter_->error("Error reading bool.");
      char buf[100];
      in.clear();
      in.getline(buf, 100);
      reporter_->error(std::string("Rest of line is: ") + buf);
      err = true;
      return;
    }
    next_entry();
  }
  else
  {
    *ostr << data << " ";
  }
}


void
TextPiostream::io(char& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      reporter_->error("Error reading char.");
      char buf[100];
      in.clear();
      in.getline(buf, 100);
      reporter_->error(std::string("Rest of line is: ") + buf);
      err = true;
      return;
    }
    next_entry();
  }
  else
  {
    *ostr << data << " ";
  }
}


void
TextPiostream::io(signed char& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      reporter_->error("Error reading signed char.");
      char buf[100];
      in.clear();
      in.getline(buf, 100);
      reporter_->error(std::string("Rest of line is: ") + buf);
      err = true;
      return;
    }
    next_entry();
  }
  else
  {
    *ostr << data << " ";
  }
}


void
TextPiostream::io(unsigned char& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      reporter_->error("Error reading unsigned char.");
      char buf[100];
      in.clear();
      in.getline(buf, 100);
      reporter_->error(std::string("Rest of line is: ") + buf);
      err = true;
      return;
    }
    next_entry();
  }
  else
  {
    *ostr << data << " ";
  }
}


void
TextPiostream::io(short& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      reporter_->error("Error reading short.");
      char buf[100];
      in.clear();
      in.getline(buf, 100);
      reporter_->error(std::string("Rest of line is: ") + buf);
      err = true;
      return;
    }
    next_entry();
  }
  else
  {
    *ostr << data << " ";
  }
}


void
TextPiostream::io(unsigned short& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      reporter_->error("Error reading unsigned short.");
      char buf[100];
      in.clear();
      in.getline(buf, 100);
      reporter_->error(std::string("Rest of line is: ") + buf);
      err = true;
      return;
    }
    next_entry();
  }
  else
  {
    *ostr << data << " ";
  }
}


void
TextPiostream::io(int& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      // Backwards compatibility
      unsigned int udata;
      in >> udata;
      if (!in)
      {
        reporter_->error("Error reading int.");
        char buf[100];
        in.clear();
        in.getline(buf, 100);
        reporter_->error(std::string("Rest of line is: ") + buf);
        err = true;
        return;
      }
      data = static_cast<int>(udata);
    }
    next_entry();
  }
  else
  {
    *ostr << data << " ";
  }
}


void
TextPiostream::io(unsigned int& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      reporter_->error("Error reading unsigned int.");
      char buf[100];
      in.clear();
      in.getline(buf, 100);
      reporter_->error(std::string("Rest of line is: ") + buf);
      err = true;
      return;
    }
    next_entry();
  }
  else
  {
    *ostr << data << " ";
  }
}


void
TextPiostream::io(long& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      reporter_->error("Error reading long.");
      char buf[100];
      in.clear();
      in.getline(buf, 100);
      reporter_->error(std::string("Rest of line is: ") + buf);
      err = true;
      return;
    }
    next_entry();
  }
  else
  {
    *ostr << data << " ";
  }
}


void
TextPiostream::io(unsigned long& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      reporter_->error("Error reading unsigned long.");
      char buf[100];
      in.clear();
      in.getline(buf, 100);
      reporter_->error(std::string("Rest of line is: ") + buf);
      err = true;
      return;
    }
    next_entry();
  }
  else
  {
    *ostr << data << " ";
  }
}


void
TextPiostream::io(long long& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      reporter_->error("Error reading long long.");
      char buf[100];
      in.clear();
      in.getline(buf, 100);
      reporter_->error(std::string("Rest of line is: ") + buf);
      err = true;
      return;
    }
    next_entry();
  }
  else
  {
    *ostr << data << " ";
  }
}


void
TextPiostream::io(unsigned long long& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      reporter_->error("Error reading unsigned long long.");

      char buf[100];
      in.clear();
      in.getline(buf, 100);
      reporter_->error(std::string("Rest of line is: ") + buf);
      err = true;
      return;
    }
    next_entry();
  }
  else
  {
    *ostr << data << " ";
  }
}

void airToLower(char* p)
{
  for ( ; *p; ++p) *p = tolower(*p);
}

void
TextPiostream::io(double& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;

    if (!in)
    {
      char ibuf[5];
      in.clear();
      in.get(ibuf,4);
      // Set the last character to null for airToLower.
      ibuf[3] = '\0';
      // Make sure the comparison is case insensitive.
      airToLower(ibuf);

      if (strcmp(ibuf,"nan")==0)
      {
        data = std::numeric_limits<double>::quiet_NaN();
      }
      else if (strcmp(ibuf,"inf")==0)
      {
        data = std::numeric_limits<double>::infinity();
      }
      else
      {
        in.clear();
        in.get(&(ibuf[3]),2);
        // Set the last character to null for airToLower.
        ibuf[4] = '\0';
        airToLower(ibuf);
        if (strcmp(ibuf,"-inf")==0)
        {
          data = -std::numeric_limits<double>::infinity();
        }
        else
        {
          char buf[100];
          reporter_->error("Error reading double.");
          in.clear();
          in.getline(buf, 100);
          reporter_->error(std::string("Rest of line is: ") + ibuf + buf);
          err = true;
          return;
        }
      }
    }
    next_entry();
  }
  else
  {
    std::ostream& out=*ostr;
    out.precision(16);
    out << data << " ";
  }
}


void
TextPiostream::io(float& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    in >> data;
    if (!in)
    {
      char ibuf[5];
      in.clear();
      in.get(ibuf,4);
      // Set the last character to null for airToLower.
      ibuf[3] = '\0';
      // Make sure the comparison is case insensitive.
      airToLower(ibuf);
      if (strcmp(ibuf,"nan")==0)
      {
        data = std::numeric_limits<float>::quiet_NaN();
      }
      else if (strcmp(ibuf,"inf")==0)
      {
        data = std::numeric_limits<float>::infinity();
      }
      else
      {
        in.clear();
        in.get(&(ibuf[3]),2);
        // Set the last character to null for airToLower.
        ibuf[4] = '\0';
        airToLower(ibuf);
        if (strcmp(ibuf,"-inf")==0)
        {
          data = -std::numeric_limits<float>::infinity();
        }
        else
        {
          char buf[100];
          reporter_->error("Error reading float.");
          in.clear();
          in.getline(buf, 100);
          reporter_->error(std::string("Rest of line is: ") + ibuf + buf);
          err = true;
          return;
        }
      }
    }
    next_entry();
  }
  else
  {
    std::ostream& out=*ostr;
    out.precision(16);
    out << data << " ";
  }
}


void
TextPiostream::io(std::string& data)
{
  if (err) return;
  if (dir==Read)
  {
    std::istream& in=*istr;
    expect('"');
    char buf[1000];
    char* p=buf;
    int n=0;
    for(;;)
    {
      char c;
      in.get(c);
      if (!in)
      {
        reporter_->error("String input failed.");
        char buf[100];
        in.clear();
        in.getline(buf, 100);
        reporter_->error(std::string("Rest of line is: ") + buf);
        err = true;
        return;
      }
      if (c == '"')
        break;
      else
        *p++=c;
      if (n++ > 998)
      {
        reporter_->error("String too long.");
        char buf[100];
        in.clear();
        in.getline(buf, 100);
        reporter_->error(std::string("Rest of line is: ") + buf);
        err = true;
        break;
      }
    }
    *p=0;
    next_entry();
    data=std::string(buf);
  }
  else
  {
    *ostr << "\"" << data << "\" ";
  }
}


void
TextPiostream::next_entry()
{
   if (err) return;
   std::istream& in=*istr;

   if (!in)
  {
    reporter_->error("Read in expect failed (before read).");
    char buf[100];
    in.clear();
    in.getline(buf, 100);
    reporter_->error(std::string("Rest of line is: ") + buf);
    in.clear();
    in.getline(buf, 100);
    reporter_->error(std::string("Next line is: ") + buf);
    err = true;
    return;
  }
  char c;
  in.get(c);
  if ((c!= ' ')&&(c!='\t')&&(c!='\r')&&(c!='\n')&&(c!=',')&&(c!=';')&&(c!='}'))
  {
    err = true;
    reporter_->error(std::string("Persistent Object Stream: Expected a separation character, got '") + c + "'.");
    char buf[100];
    in.clear();
    in.getline(buf, 100);
    reporter_->error(std::string("Rest of line is: ") + buf);
    reporter_->error("Object is not intact.");
    return;
  }

  while ((c == ' ')||(c == '\t')||(c == '\n')||(c == '\r'))
  {
    in.get(c);
  }

  if ((c == ',')||(c == ';'))
  {
    in.get(c);
    while ((c == ' ')||(c == '\t')||(c == '\n')||(c == '\r'))
    {
      in.get(c);
    }
  }

  in.unget();
}

void
TextPiostream::expect(char expected)
{
  if (err) return;
  std::istream& in=*istr;
  if (!in)
  {
    reporter_->error("Read in expect failed (before read).");
    char buf[100];
    in.clear();
    in.getline(buf, 100);
    reporter_->error(std::string("Rest of line is: ") + buf);
    in.clear();
    in.getline(buf, 100);
    reporter_->error(std::string("Next line is: ") + buf);
    err = true;
    return;
  }
  char c;
  in.get(c);
  if (!in)
  {
    reporter_->error("Read in expect failed (after read).");
    char buf[100];
    in.clear();
    in.getline(buf, 100);
    reporter_->error(std::string("Rest of line is: ") + buf);
    in.clear();
    in.getline(buf, 100);
    reporter_->error(std::string("Next line is: ") + buf);
    err = true;
    return;
  }
  if (c != expected)
  {
    err = true;
    reporter_->error(std::string("Persistent Object Stream: Expected '") +
                     expected + "', got '" + c + "'.");
    char buf[100];
    in.clear();
    in.getline(buf, 100);
    reporter_->error(std::string("Rest of line is: ") + buf);
    reporter_->error("Object is not intact.");
    return;
  }
}


void
TextPiostream::emit_pointer(int& have_data, int& pointer_id)
{
  if (dir==Read)
  {
    std::istream& in=*istr;
    char c;
    in.get(c);
    if (in && c=='%')
      have_data=0;
    else if (in && c=='@')
      have_data=1;
    else
    {
      reporter_->error("Error reading pointer...");
      err = true;
    }
    in >> pointer_id;
    if (!in)
    {
      reporter_->error("Error reading pointer id.");
      err = true;
      return;
    }
    expect(' ');
  }
  else
  {
    std::ostream& out=*ostr;
    if (have_data)
      out << '@';
    else
      out << '%';
    out << pointer_id << " ";
  }
}


bool
TextPiostream::eof() {
  if (dir == Write) {
    return false;
  }

  std::istream &in = *istr;
  in.peek();
  return in.eof();
}



// FastPiostream is a non portable binary output.
FastPiostream::FastPiostream(const std::string& filename, Direction dir,
                             LoggerHandle pr)
  : Piostream(dir, -1, filename, pr),
    fp_(0)
{
  if (dir==Read)
  {
    fp_ = fopen (filename.c_str(), "rb");
    if (!fp_)
    {
      reporter_->error("Error opening file: " + filename + " for reading.");
      err = true;
      return;
    }
    char hdr[12];
    size_t chars_read = fread(hdr, sizeof(char), 12, fp_);
    if (chars_read != 12)
    {
      reporter_->error("Error reading header from: " + filename);
      err = true;
      return;
    }
    readHeader(reporter_, filename, hdr, "FAS", version_, file_endian);
  }
  else
  {
    fp_ = fopen(filename.c_str(), "wb");
    if (!fp_)
    {
      reporter_->error("Error opening file: " + filename + " for writing.");
      err = true;
      return;
    }
    version_ = PERSISTENT_VERSION;
    if (version() > 1)
    {
      char hdr[16];
      //if (airMyEndian == airEndianLittle)
        sprintf(hdr, "SCI\nFAS\n%03d\nLIT\n", version_);
      //else
        //sprintf(hdr, "SCI\nFAS\n%03d\nBIG\n", version_);
      // write the header
      size_t wrote = fwrite(hdr, sizeof(char), 16, fp_);
      if (wrote != 16)
      {
        reporter_->error("Error writing header to: " + filename);
        err = true;
        return;
      }
    }
    else
    {
      char hdr[12];
      sprintf(hdr, "SCI\nFAS\n%03d\n", version_);
      // write the header
      size_t wrote = fwrite(hdr, sizeof(char), 12, fp_);
      if (wrote != 12)
      {
        reporter_->error("Error writing header to: " + filename);
        err = true;
        return;
      }
    }
  }
}


FastPiostream::FastPiostream(int fd, Direction dir, LoggerHandle pr)
: Piostream(dir, -1, "", pr),
fp_(0)
{
  if (dir==Read)
  {
    fp_ = fdopen (fd, "rb");
    if (!fp_)
    {
      reporter_->error("Error opening socket: " + to_string(fd) +
        " for reading.");
      err = true;
      return;
    }
    char hdr[12];
    size_t chars_read = fread(hdr, sizeof(char), 12, fp_);
    if (chars_read != 12)
    {
      reporter_->error("Error reading header from socket: " +
        to_string(fd) + ".");
      err = true;
      return;
    }
    readHeader(reporter_, "socket", hdr, "FAS", version_, file_endian);
  }
  else
  {
    fp_=fdopen(fd, "wb");
    if (!fp_)
    {
      reporter_->error("Error opening socket: " + to_string(fd) +
        " for writing.");
      err = true;
      return;
    }
    version_ = PERSISTENT_VERSION;
    if (version() > 1)
    {
      char hdr[16];
      //if (airMyEndian == airEndianLittle)
        sprintf(hdr, "SCI\nFAS\n%03d\nLIT\n", version_);
      //else
        //sprintf(hdr, "SCI\nFAS\n%03d\nBIG\n", version_);
      // write the header
      size_t wrote = fwrite(hdr, sizeof(char), 16, fp_);
      if (wrote != 16)
      {
        reporter_->error("Error writing header to: " + to_string(fd) + ".");
        err = true;
        return;
      }
    }
    else
    {
      char hdr[12];
      sprintf(hdr, "SCI\nFAS\n%03d\n", version_);
      // write the header
      size_t wrote = fwrite(hdr, sizeof(char), 12, fp_);
      if (wrote != 12)
      {
        reporter_->error("Error writing header to socket: " +
          to_string(fd) + ".");
        err = true;
        return;
      }
    }
  }
}


FastPiostream::~FastPiostream()
{
  if (fp_) fclose(fp_);
}


void
FastPiostream::reset_post_header()
{
  if (! reading()) return;

  if (version() == 1)
  {
    // Old versions had headers of size 12.
    fseek(fp_, 13, SEEK_SET);
  }
  else
  {
    // Versions > 1 have size of 16 to account for endianness in
    // header (LIT | BIG).
    fseek(fp_, 17, SEEK_SET);
  }
}


template <class T>
inline void
FastPiostream::gen_io(T& data, const char *iotype)
{
  if (err) return;
  size_t expect = 1;
  size_t did = 0;
  if (dir == Read)
  {
    did = fread(&data, sizeof(data), 1, fp_);
    if (expect != did && !feof(fp_))
    {
      err = true;
      reporter_->error(std::string("FastPiostream error reading ") + iotype + ".");
    }
  }
  else
  {
    did = fwrite(&data, sizeof(data), 1, fp_);
    if (expect != did)
    {
      err = true;
      reporter_->error(std::string("FastPiostream error writing ") + iotype + ".");
    }
  }
}


void
FastPiostream::io(bool& data)
{
  gen_io(data, "bool");
}


void
FastPiostream::io(char& data)
{
  gen_io(data, "char");
}


void
FastPiostream::io(signed char& data)
{
  gen_io(data, "signed char");
}


void
FastPiostream::io(unsigned char& data)
{
  gen_io(data, "unsigned char");
}


void
FastPiostream::io(short& data)
{
  gen_io(data, "short");
}


void
FastPiostream::io(unsigned short& data)
{
  gen_io(data, "unsigned short");
}


void
FastPiostream::io(int& data)
{
  gen_io(data, "int");
}


void
FastPiostream::io(unsigned int& data)
{
  gen_io(data, "unsigned int");
}


void
FastPiostream::io(long& data)
{
  gen_io(data, "long");
}


void
FastPiostream::io(unsigned long& data)
{
  gen_io(data, "unsigned long");
}


void
FastPiostream::io(long long& data)
{
  gen_io(data, "long long");
}


void
FastPiostream::io(unsigned long long& data)
{
  gen_io(data, "unsigned long long");
}


void
FastPiostream::io(double& data)
{
  gen_io(data, "double");
}


void
FastPiostream::io(float& data)
{
  gen_io(data, "float");
}


void
FastPiostream::io(std::string& data)
{
  if (err) return;
  unsigned int chars = 0;
  if (dir==Write)
  {
    const char* p=data.c_str();
    chars = static_cast<int>(strlen(p)) + 1;
    fwrite(&chars, sizeof(unsigned int), 1, fp_);
    fwrite(p, sizeof(char), chars, fp_);
  }
  if (dir==Read)
  {
    fread(&chars, sizeof(unsigned int), 1, fp_);
    char* buf = new char[chars];
    fread(buf, sizeof(char), chars, fp_);
    data=std::string(buf);
    delete[] buf;
  }
}


bool
FastPiostream::block_io(void *data, size_t s, size_t nmemb)
{
  if (dir == Read)
  {
    const size_t did = fread(data, s, nmemb, fp_);
    if (did != nmemb)
    {
      err = true;
      reporter_->error("FastPiostream error reading block io.");
    }
  }
  else
  {
    const size_t did = fwrite(data, s, nmemb, fp_);
    if (did != nmemb)
    {
      err = true;
      reporter_->error("FastPiostream error writing block io.");
    }
  }
  return true;
}


} // End namespace SCIRun
