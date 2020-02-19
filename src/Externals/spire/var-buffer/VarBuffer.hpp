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


#ifndef SPIRE_VARBUFFER_HPP
#define SPIRE_VARBUFFER_HPP

#include <es-log/trace-log.h>
#include <cstdint>
#include <memory>
#include <vector>
#include <bserialize/BSerialize.hpp>
#include <spire/scishare.h>

namespace spire {

/// This class is only for writing to a variable sized buffer. It is a thin
/// layer over the BSerialize code. To read data from a buffer, simply wrap
/// the buffer using BSerialize. There is no need to automatically extend
/// the buffer size when reading.
class SCISHARE VarBuffer final
{
public:
  /// Initialize an empty buffer.
  VarBuffer();

  // Preallocate the variable buffer to the preset size.
  explicit VarBuffer(size_t size);

  // todo Add flag which will swap bytes when reading out of or into buffer.

  // Writes numBytes of bytes.
  void writeBytes(const char* bytes, size_t numBytes);

  /// Writes a null terminated string.
  void writeNullTermString(const char* str);

  template <typename T>
  void write(const T& val)
  {
    // Attempt to write the value to BSerialize. If a runtime exception is
    // caught, then we need to resize the buffer.
    while (mSerializer->write(val) == false)
    {
      resize();
    }
  }

  template <typename T>
  inline void writeUnsafe(const T& val)
  {
    mSerializer->writeUnsafe(val);
  }


  /// Clears all data currently written to the var buffer.
  void clear();

  /// Retrieves raw buffer contents.
  char* getBuffer()         {return mBuffer.empty() ? 0 : &mBuffer[0];}

  /// Retrieves the current size of the buffer (size of all data currently
  /// written to the buffer).
  size_t getBufferSize() const {return mSerializer->getOffset();}

  /// Retrieves currently allocated size of the buffer.
  size_t getAllocatedSize() const {return mBufferSize;}

private:

  void resize();

  static bool serializeFloat(char* msg, int msgLen, int* offset_out, float in);

  static bool serializeUInt16(char* msg, int msgLen, int* offset_out, uint16_t in);
  static uint16_t deserializeUInt16(const char* msg, int msgLen, int* offset_out);

  std::vector<char> mBuffer;      ///< buffer
  size_t            mBufferSize;  ///< Absolute size of mBuffer in bytes.

  std::unique_ptr<spire::BSerialize> mSerializer;
};

} // namespace spire

#endif
