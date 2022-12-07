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
///       Modified by
///       Michelle Miller
///       Department of Computer Science
///       University of Utah
///@date  April 1994, Modified
///   Thu Feb 19 17:04:59 MST 1998
///

#include <Core/Persistent/GZstream.h>

#include <Core/Util/StringUtil.h>

#include <teem/air.h>
#include <teem/nrrd.h>
#include <zlib.h>


#include   <fstream>
#include   <iostream>




#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#ifdef _WIN32
#  include <io.h>
#endif

namespace SCIRun {

// BinaryPiostream -- portable
GZPiostream::GZPiostream(const std::string& filename, Direction dir,
                                 const int& v, ProgressReporter *pr)
  : Piostream(dir, v, filename, pr),
    fp_(0)
{
  if (v == -1) // no version given so use PERSISTENT_VERSION
    version_ = PERSISTENT_VERSION;
  else
    version_ = v;

  if (dir==Read)
  {
    fp_ = gzopen (filename.c_str(), "rb");
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
      if (!gzread(fp_, hdr, 12))
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
      if (!gzread(fp_, hdr, 16))
      {
	reporter_->error("Header read failed.");
	err = true;
	return;
      }
    }
  }
  else
  {
    fp_ = gzopen(filename.c_str(), "wb");
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

      if (!gzwrite(fp_, hdr, 16))
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
      if (!gzwrite(fp_, hdr, 13))
      {
	reporter_->error("Header write failed.");
	err = true;
	return;
      }
    }
  }
}


GZPiostream::GZPiostream(int fd, Direction dir, const int& v,
                                 ProgressReporter *pr)
  : Piostream(dir, v, "", pr),
    fp_(0)
{
  if (v == -1) // No version given so use PERSISTENT_VERSION.
    version_ = PERSISTENT_VERSION;
  else
    version_ = v;

  if (dir == Read)
  {
    fp_ = gzdopen (fd, "rb");
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
      if (!gzread(fp_, hdr, 12))
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
      if (!gzread(fp_, hdr, 16))
      {
	reporter_->error("Header read failed.");
	err = true;
	return;
      }
    }
  }
  else
  {
    fp_ = gzdopen(fd, "wb");
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

      if (!gzwrite(fp_, hdr, 16))
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
      if (!gzwrite(fp_, hdr, 12))
      {
	reporter_->error("Header write failed.");
	err = true;
	return;
      }
    }
  }
}


GZPiostream::~GZPiostream()
{
  if (fp_) gzclose(fp_);
}

void
GZPiostream::reset_post_header()
{
  if (! reading()) return;

  gzseek(fp_, 0, SEEK_SET);

  if (version() == 1)
  {
    // Old versions had headers of size 12.
    char hdr[12];
    // read header
    gzread(fp_, hdr, 12);
  }
  else
  {
    // Versions > 1 have size of 16 to account for endianness in
    // header (LIT | BIG).
    char hdr[16];
    // read header
    gzread(fp_, hdr, 16);
  }
}

const char *
GZPiostream::endianness()
{
  if (airMyEndian == airEndianLittle)
    return "LIT\n";
  else
    return "BIG\n";

}


template <class T>
inline void
GZPiostream::gen_io(T& data, const char *iotype)
{
  if (err) return;
  if (dir==Read)
  {
    if (!gzread(fp_, &data, sizeof(data)))
    {
      err = true;
      reporter_->error(std::string("GZPiostream error reading ") +
                       iotype + ".");
    }
  }
  else
  {
    if (!gzwrite(fp_, &data, sizeof(data)))
    {
      err = true;
      reporter_->error(std::string("GZPiostream error writing ") +
                       iotype + ".");
    }
  }
}


void
GZPiostream::io(char& data)
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
GZPiostream::io(signed char& data)
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
GZPiostream::io(unsigned char& data)
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
GZPiostream::io(short& data)
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
GZPiostream::io(unsigned short& data)
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
GZPiostream::io(int& data)
{
  gen_io(data, "int"); // xdr_int
}


void
GZPiostream::io(unsigned int& data)
{
  gen_io(data, "unsigned int"); // xdr_u_int
}


void
GZPiostream::io(long& data)
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
GZPiostream::io(unsigned long& data)
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
GZPiostream::io(long long& data)
{
  gen_io(data, "long long"); // xdr_longlong_t
}


void
GZPiostream::io(unsigned long long& data)
{
  gen_io(data, "unsigned long long"); //xdr_u_longlong_t
}


void
GZPiostream::io(double& data)
{
  gen_io(data, "double"); // xdr_double
}


void
GZPiostream::io(float& data)
{
  gen_io(data, "float"); // xdr_float
}


void
GZPiostream::io(std::string& data)
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
      if (!gzwrite(fp_, data.c_str(), sizeof(char) * chars)) err = true;

      // Pad data out to 4 bytes.
      int extra = chars % 4;
      if (extra)
      {
        static const char pad[4] = {0, 0, 0, 0};
        if (!gzwrite(fp_, pad, sizeof(char) * (4 - extra))) err = true;
      }
    }
    else
    {
      const char* p = data.c_str();
      chars = static_cast<int>(strlen(p)) + 1;
      io(chars);
      if (!gzwrite(fp_, p, sizeof(char) * chars)) err = true;
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
        if (!gzread(fp_, buf, sizeof(char) * buf_size))
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
      gzread(fp_, buf, sizeof(char) * chars);
      data = std::string(buf);
      delete[] buf;
    }
  }
}


bool
GZPiostream::block_io(void *data, size_t s, size_t nmemb)
{
  if (err || version() == 1) { return false; }
  if (dir == Read)
  {
    const size_t did = gzread(fp_, data, s * nmemb);
    if (did != nmemb)
    {
      err = true;
      reporter_->error("GZPiostream error reading block io.");
    }
  }
  else
  {
    const size_t did = gzwrite(fp_, data, s * nmemb);
    if (did != nmemb)
    {
      err = true;
      reporter_->error("GZPiostream error writing block io.");
    }
  }
  return true;
}



////
// GZSwapPiostream -- portable
// Piostream used when endianness of machine and file don't match
GZSwapPiostream::GZSwapPiostream(const std::string& filename, Direction dir,
                                         const int& v, ProgressReporter *pr)
  : GZPiostream(filename, dir, v, pr)
{
}


GZSwapPiostream::GZSwapPiostream(int fd, Direction dir, const int& v,
                                         ProgressReporter *pr)
  : GZPiostream(fd, dir, v, pr)
{
}


GZSwapPiostream::~GZSwapPiostream()
{
}


const char *
GZSwapPiostream::endianness()
{
  if (airMyEndian == airEndianLittle)
    return "LIT\n";
  else
    return "BIG\n";
}

template <class T>
inline void
GZSwapPiostream::gen_io(T& data, const char *iotype)
{
  if (err) return;
  if (dir==Read)
  {
    unsigned char tmp[sizeof(data)];
    if (!gzread(fp_, tmp, sizeof(data)))
    {
      err = true;
      reporter_->error(std::string("GZPiostream error reading ") +
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
    if (!gzwrite(fp_, tmp, sizeof(data)))
    {
      err = true;
      reporter_->error(std::string("GZPiostream error writing ") +
                       iotype + ".");
    }
#else
    if (!gzwrite(fp_, &data, sizeof(data)))
    {
      err = true;
      reporter_->error(std::string("GZPiostream error writing ") +
                       iotype + ".");
    }
#endif
  }
}



void
GZSwapPiostream::io(short& data)
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
GZSwapPiostream::io(unsigned short& data)
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
GZSwapPiostream::io(int& data)
{
  gen_io(data, "int");
}


void
GZSwapPiostream::io(unsigned int& data)
{
  gen_io(data, "unsigned int");
}


void
GZSwapPiostream::io(long& data)
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
GZSwapPiostream::io(unsigned long& data)
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
GZSwapPiostream::io(long long& data)
{
  gen_io(data, "long long");
}


void
GZSwapPiostream::io(unsigned long long& data)
{
  gen_io(data, "unsigned long long");
}


void
GZSwapPiostream::io(double& data)
{
  gen_io(data, "double");
}


void
GZSwapPiostream::io(float& data)
{
  gen_io(data, "float");
}


PiostreamPtr
auto_gzistream(const std::string& filename, ProgressReporter *pr)
{
  gzFile file = gzopen(filename.c_str(), "rb");
  char hdr[16];
  gzread(file, hdr, 16);
  gzclose(file);

  int file_endian, version;
  if (!Piostream::readHeader(pr, filename, hdr, 0, version, file_endian))
  {
    if (pr)
      pr->error("Cannot parse header of file: " + filename);
    else
      std::cerr << "ERROR - Cannot parse header of file: " << filename << std::endl;
    return PiostreamPtr();
  }
  if (version > Piostream::PERSISTENT_VERSION)
  {
    const std::string errmsg = "File '" + filename + "' has version " +
      to_string(version) + ", this build only supports up to version " +
      to_string(Piostream::PERSISTENT_VERSION) + ".";

    if (pr)
      pr->error(errmsg);
    else
      std::cerr << "ERROR - " + errmsg;

    return PiostreamPtr();
  }

  const char m1 = hdr[4];
  const char m2 = hdr[5];
  const char m3 = hdr[6];
  if (m1 == 'B' && m2 == 'I' && m3 == 'N')
  {
    // Old versions of Pio used XDR which always wrote big endian so if
    // the version = 1, readHeader would return BIG, otherwise it will
    // read it from the header.
    int machine_endian = Piostream::Big;
    if (airMyEndian == airEndianLittle) {
      machine_endian = Piostream::Little;
    }

    if (file_endian == machine_endian)
      return PiostreamPtr(new GZPiostream(filename, Piostream::Read, version, pr));
    else
      return PiostreamPtr(new GZSwapPiostream(filename, Piostream::Read, version,pr));
  }
  else
  {
    const std::string msg = "Text based compressed files are not supported.";
    if (pr)
      pr->error(msg);
    else
      std::cerr << "ERROR - " + msg << "\n";
    return PiostreamPtr();
  }

  if (pr)
    pr->error(filename + " is an unknown type!");
  else
    std::cerr << filename << " is an unknown type!" << std::endl;
  return PiostreamPtr();
}



} // End namespace SCIRun
