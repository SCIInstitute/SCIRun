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


#ifndef SPIRE_CEREALTYPESERIALIZE_HPP
#define SPIRE_CEREALTYPESERIALIZE_HPP

#include <es-log/trace-log.h>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <spire/scishare.h>

struct _Tny;
typedef _Tny Tny;

namespace spire {

// Cereal serialize type detail
namespace CST_detail
{
  // Basic types stored in a dictionary (TNY_DICT).
 SCISHARE bool inBool(Tny* root, const char* name, bool& b);
 SCISHARE bool inInt8(Tny* root, const char* name, int8_t& c);
 SCISHARE bool inUInt8(Tny* root, const char* name, uint8_t& c);
 SCISHARE bool inInt32(Tny* root, const char* name, int32_t& v);
 SCISHARE bool inUInt32(Tny* root, const char* name, uint32_t& v);
 SCISHARE bool inInt64(Tny* root, const char* name, int64_t& v);
 SCISHARE bool inUInt64(Tny* root, const char* name, uint64_t& v);
 SCISHARE bool inFloat(Tny* root, const char* name, float& v);
 SCISHARE bool inDouble(Tny* root, const char* name, double& v);
 SCISHARE bool inString(Tny* root, const char* name, char* str, size_t maxSize);
 SCISHARE bool inStringStd(Tny* root, const char* name, std::string& str);
 SCISHARE bool inBinary(Tny* root, const char* name, void* data, size_t size);
 SCISHARE bool inBinaryMalloc(Tny* root, const char* name, void** data);

  SCISHARE Tny* outBool(Tny* root, const char* name, const bool& b);
  SCISHARE Tny* outInt8(Tny* root, const char* name, const int8_t& c);
  SCISHARE Tny* outUInt8(Tny* root, const char* name, const uint8_t& c);
  SCISHARE Tny* outInt32(Tny* root, const char* name, const int32_t& v);
  SCISHARE Tny* outUInt32(Tny* root, const char* name, const uint32_t& v);
  SCISHARE Tny* outInt64(Tny* root, const char* name, const int64_t& v);
  SCISHARE Tny* outUInt64(Tny* root, const char* name, const uint64_t& v);
  SCISHARE Tny* outFloat(Tny* root, const char* name, const float& v);
  SCISHARE Tny* outDouble(Tny* root, const char* name, const double& v);
  SCISHARE Tny* outString(Tny* root, const char* name, const char* str);
  SCISHARE Tny* outBinary(Tny* root, const char* name, const void* data, size_t size);
  SCISHARE Tny* outBinaryMalloc(Tny* root, const char* name, const void* data, size_t size);

  // Basic types stored in an array (TNY_ARRAY).
  SCISHARE Tny* inBoolArray(Tny* root, bool& b);
  SCISHARE Tny* inInt8Array(Tny* root, int8_t& c);
  SCISHARE Tny* inUInt8Array(Tny* root, uint8_t& c);
  SCISHARE Tny* inInt32Array(Tny* root, int32_t& v);
  SCISHARE Tny* inUInt32Array(Tny* root, uint32_t& v);
  SCISHARE Tny* inInt64Array(Tny* root, int64_t& v);
  SCISHARE Tny* inUInt64Array(Tny* root, uint64_t& v);
  SCISHARE Tny* inFloatArray(Tny* root, float& v);
  SCISHARE Tny* inDoubleArray(Tny* root, double& v);
  SCISHARE Tny* inStringArray(Tny* root, char* str, size_t maxSize);
  SCISHARE Tny* inBinaryArray(Tny* root, void* data, size_t size);
  SCISHARE Tny* inBinaryMallocArray(Tny* root, void** data);

  SCISHARE Tny* outBoolArray(Tny* root, const bool& b);
  SCISHARE Tny* outInt8Array(Tny* root, const int8_t& c);
  SCISHARE Tny* outUInt8Array(Tny* root, const uint8_t& c);
  SCISHARE Tny* outInt32Array(Tny* root, const int32_t& v);
  SCISHARE Tny* outUInt32Array(Tny* root, const uint32_t& v);
  SCISHARE Tny* outInt64Array(Tny* root, const int64_t& v);
  SCISHARE Tny* outUInt64Array(Tny* root, const uint64_t& v);
  SCISHARE Tny* outFloatArray(Tny* root, const float& v);
  SCISHARE Tny* outDoubleArray(Tny* root, const double& v);
  SCISHARE Tny* outStringArray(Tny* root, const char* str);
  SCISHARE Tny* outBinaryArray(Tny* root, const void* data, size_t size);
  SCISHARE Tny* outBinaryMallocArray(Tny* root, const void* data, size_t size);
}

template <typename T>
class CerealSerializeType
{
public:
  typedef int Type;

  static_assert(true, "es-cereal: CerealSerializeType type specialization not defined.");
  static bool in(Tny* root, const char* name, Type& v);
  static Tny* out(Tny* root, const char* name, const Type& v);
  static const char* getTypeName();
};

template<>
class CerealSerializeType<bool>
{
public:
  typedef bool Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inBool(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outBool(root, name, v);}
  static const char* getTypeName()    {return "bool";}
};

template<>
class CerealSerializeType<int8_t>
{
public:
  typedef int8_t Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inInt8(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outInt8(root, name, v);}
  static const char* getTypeName()    {return "int8";}
};

template<>
class CerealSerializeType<uint8_t>
{
public:
  typedef uint8_t Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inUInt8(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outUInt8(root, name, v);}
  static const char* getTypeName()    {return "uint8";}
};

template<>
class CerealSerializeType<int32_t>
{
public:
  typedef int32_t Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inInt32(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outInt32(root, name, v);}
  static const char* getTypeName()    {return "int32";}
};

template<>
class CerealSerializeType<uint32_t>
{
public:
  typedef uint32_t Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inUInt32(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outUInt32(root, name, v);}
  static const char* getTypeName()    {return "uint32";}
};

template<>
class CerealSerializeType<int64_t>
{
public:
  typedef int64_t Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inInt64(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outInt64(root, name, v);}
  static const char* getTypeName()    {return "int64";}
};

template<>
class CerealSerializeType<uint64_t>
{
public:
  typedef uint64_t Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inUInt64(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outUInt64(root, name, v);}
  static const char* getTypeName()    {return "uint64";}
};

template<>
class CerealSerializeType<float>
{
public:
  typedef float Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inFloat(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outFloat(root, name, v);}
  static const char* getTypeName()    {return "float";}
};

template<>
class CerealSerializeType<double>
{
public:
  typedef double Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inDouble(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outDouble(root, name, v);}
  static const char* getTypeName()    {return "double";}
};

template<>
class CerealSerializeType<std::string>
{
public:
  typedef std::string Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inStringStd(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outString(root, name, v.c_str());}
  static const char* getTypeName()    {return "string";}
};

} // namespace spire

#endif
