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


#ifndef SPIRE_BSERIALIZEDETAIL_HPP
#define SPIRE_BSERIALIZEDETAIL_HPP

#include <es-log/trace-log.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <stdexcept>
#include <spire/scishare.h>

namespace spire {

namespace detail
{

template <typename T>
T readTypeFromMemory(char* msg, size_t msgSize, size_t* offset_out)
{
  size_t offset = *offset_out;
  if (offset + sizeof(T) > msgSize)
  {
    throw std::runtime_error("Read passed end of buffer");
    return 0;
  }

  const char* offsetPtr = msg + offset;

  *offset_out += sizeof(T);

  T out;
  std::memcpy(&out, offsetPtr, sizeof(T));

  return out;
}

template <typename T>
bool writeTypeToMemory(char* msg, size_t msgSize, size_t* offset_out, const T& v)
{
  size_t offset = *offset_out;
  if (offset + sizeof(T) > msgSize)
  {
    return false;
  }

  // Copy the integer into the memory buffer.
  char* offsetPtr = msg + offset;
  std::memcpy(offsetPtr, &v, sizeof(T));

  *offset_out += sizeof(T);

  return true;
}

template <typename T>
inline void writeTypeToMemoryUnsafe(char* msg, size_t& offset_out, const T& v)
{
  std::memcpy(msg + offset_out, &v, sizeof(T));
  offset_out += sizeof(T);
}

template <typename T>
class SerializeType
{
public:
  static_assert(true, "BSerialize: type specialization not defined.");
  typedef int Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out);
  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& v);
};

//------------------------------------------------------------------------------
// INT8_T
//------------------------------------------------------------------------------
template<>
class SerializeType<int8_t>
{
public:
  typedef int8_t Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return readTypeFromMemory<Type>(msg, msgSize, offset_out); }

  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return writeTypeToMemory<Type>(msg, msgSize, offset_out, in); }
};

//------------------------------------------------------------------------------
// UINT8_T
//------------------------------------------------------------------------------
template<>
class SerializeType<uint8_t>
{
public:
  typedef uint8_t Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return readTypeFromMemory<Type>(msg, msgSize, offset_out); }

  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return writeTypeToMemory<Type>(msg, msgSize, offset_out, in); }
};

//------------------------------------------------------------------------------
// INT16_T
//------------------------------------------------------------------------------
template<>
class SerializeType<int16_t>
{
public:
  typedef int16_t Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return readTypeFromMemory<Type>(msg, msgSize, offset_out); }

  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return writeTypeToMemory<Type>(msg, msgSize, offset_out, in); }
};

//------------------------------------------------------------------------------
// UINT16_T
//------------------------------------------------------------------------------
template<>
class SerializeType<uint16_t>
{
public:
  typedef uint16_t Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return readTypeFromMemory<Type>(msg, msgSize, offset_out); }

  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return writeTypeToMemory<Type>(msg, msgSize, offset_out, in); }
};

//------------------------------------------------------------------------------
// INT32_T
//------------------------------------------------------------------------------
template<>
class SerializeType<int32_t>
{
public:
  typedef int32_t Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return readTypeFromMemory<Type>(msg, msgSize, offset_out); }

  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return writeTypeToMemory<Type>(msg, msgSize, offset_out, in); }
};

//------------------------------------------------------------------------------
// UINT32_T
//------------------------------------------------------------------------------
template<>
class SerializeType<uint32_t>
{
public:
  typedef uint32_t Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return readTypeFromMemory<Type>(msg, msgSize, offset_out); }

  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return writeTypeToMemory<Type>(msg, msgSize, offset_out, in); }
};

//------------------------------------------------------------------------------
// INT64_T
//------------------------------------------------------------------------------
template<>
class SerializeType<int64_t>
{
public:
  typedef int64_t Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return readTypeFromMemory<Type>(msg, msgSize, offset_out); }

  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return writeTypeToMemory<Type>(msg, msgSize, offset_out, in); }
};

//------------------------------------------------------------------------------
// UINT64_T
//------------------------------------------------------------------------------
template<>
class SerializeType<uint64_t>
{
public:
  typedef uint64_t Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return readTypeFromMemory<Type>(msg, msgSize, offset_out); }

  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return writeTypeToMemory<Type>(msg, msgSize, offset_out, in); }
};

//------------------------------------------------------------------------------
// float
//------------------------------------------------------------------------------
template<>
class SerializeType<float>
{
public:
  typedef float Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return readTypeFromMemory<Type>(msg, msgSize, offset_out); }

  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return writeTypeToMemory<Type>(msg, msgSize, offset_out, in); }

//  static bool writeUnsafe(char* msg, size_t* offset_out, const Type& in)
//  { return writeTypeToMemoryUnsafe<Type>(msg, offset_out, in); }
};

//------------------------------------------------------------------------------
// double
//------------------------------------------------------------------------------
template<>
class SerializeType<double>
{
public:
  typedef double Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return readTypeFromMemory<Type>(msg, msgSize, offset_out); }

  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return writeTypeToMemory<Type>(msg, msgSize, offset_out, in); }
};

//------------------------------------------------------------------------------
// BOOL
//------------------------------------------------------------------------------
template<>
class SerializeType<bool>
{
public:
  typedef bool Type;
  static bool read(char* msg, size_t msgSize, size_t* offset_out)
  { return readTypeFromMemory<Type>(msg, msgSize, offset_out); }

  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& v)
  { return writeTypeToMemory<Type>(msg, msgSize, offset_out, v); }
};

//------------------------------------------------------------------------------
// Strings
//------------------------------------------------------------------------------
template<>
class SerializeType<const char*>
{
public:
  typedef const char* Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  {
    int32_t stringLength = SerializeType<int32_t>::read(msg, msgSize, offset_out);

    if (*offset_out + stringLength + 1 > msgSize)
    {
      throw std::runtime_error("Read passed end of buffer");
      return NULL;
    }

    const char* ret = msg + *offset_out;

    *offset_out += stringLength + 1;

    return ret;
  }

  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  {
    if (in != NULL)
    {
      int32_t stringLength = (int)std::strlen(in);
      SerializeType<int32_t>::write(msg, msgSize, offset_out, stringLength);

      if (*offset_out + stringLength + 1 > msgSize)
      {
        return false;
      }

      strcpy(msg + *offset_out, in);
      *offset_out += stringLength + 1;  // +1 for the null terminator.
    }
    else
    {
      // We have a zero length string.
      SerializeType<int32_t>::write(msg, msgSize, offset_out, 0);
      *offset_out += 1;                 // +1 for null terminator.
    }
    return true;
  }
};

template<>
class SerializeType<std::string>
{
public:
  typedef std::string Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return std::string(SerializeType<const char*>::read(msg, msgSize, offset_out)); }
  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return SerializeType<const char*>::write(msg, msgSize, offset_out, in.c_str()); }
};

template<>
class SerializeType<std::string&>
{
public:
  typedef std::string Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return SerializeType<const char*>::read(msg, msgSize, offset_out); }
  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return SerializeType<const char*>::write(msg, msgSize, offset_out, in.c_str()); }
};

template<>
class SerializeType<const std::string&>
{
public:
  typedef std::string Type;
  static Type read(char* msg, size_t msgSize, size_t* offset_out)
  { return SerializeType<const char*>::read(msg, msgSize, offset_out); }
  static bool write(char* msg, size_t msgSize, size_t* offset_out, const Type& in)
  { return SerializeType<const char*>::write(msg, msgSize, offset_out, in.c_str()); }
};


} // namespace detail

} // namespace spire

#endif
