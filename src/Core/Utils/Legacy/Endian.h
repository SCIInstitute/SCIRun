/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
///@file  Endian.h
///@brief Detect host endianness and provide tools to swap bytes if necessary.
///
///@author
///       Kurt Zimmerman
///       Department of Computer Science
///       University of Utah
///@date  January 2001
///

#ifndef SCI_Endianness_h
#define SCI_Endianness_h

#include <sci_defs/config_defs.h>

//#if HAVE_INTTYPES_H
//#  include <inttypes.h>
//#endif
#ifdef _WIN32
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#endif

#include <string>

#include <Core/Utils/Legacy/share.h>

namespace SCIRun {

#define SWAP_2(u2)/* IronDoc macro to swap two byte quantity */ \
  { unsigned char* _p = reinterpret_cast<unsigned char*>(&(u2)); \
    unsigned char _c =   *_p; *_p = _p[1]; _p[1] = _c; }
#define SWAP_4(u4)/* IronDoc macro to swap four byte quantity */ \
  { unsigned char* _p = reinterpret_cast<unsigned char*>(&(u4)); \
    unsigned char  _c =   *_p; *_p = _p[3]; _p[3] = _c; \
                   _c = *++_p; *_p = _p[1]; _p[1] = _c; }
#define SWAP_8(u8)/* IronDoc macro to swap eight byte quantity */ \
  { unsigned char* _p = reinterpret_cast<unsigned char*>(&(u8)); \
    unsigned char  _c =   *_p; *_p = _p[7]; _p[7] = _c; \
                   _c = *++_p; *_p = _p[5]; _p[5] = _c; \
                   _c = *++_p; *_p = _p[3]; _p[3] = _c; \
                   _c = *++_p; *_p = _p[1]; _p[1] = _c; }

SCISHARE void swapbytes(bool& i);
SCISHARE void swapbytes(int8_t& i);
SCISHARE void swapbytes(uint8_t& i);
SCISHARE void swapbytes(int16_t& i);
SCISHARE void swapbytes(uint16_t& i);
SCISHARE void swapbytes(int32_t& i);
SCISHARE void swapbytes(uint32_t& i);
SCISHARE void swapbytes(int64_t& i);
SCISHARE void swapbytes(uint64_t& i);
SCISHARE void swapbytes(float& i);
SCISHARE void swapbytes(double& i);
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
SCISHARE void swapbytes(Point &i);
SCISHARE void swapbytes(Vector &i);
#endif

SCISHARE bool isBigEndian();

SCISHARE bool isLittleEndian();
 
SCISHARE std::string endianness();



} //end namespace SCIRun
#endif
