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


#include "BSerialize.hpp"

namespace spire {

/// Use this constructor for either reading or writing.
BSerialize::BSerialize(char* msg, size_t msgSize) :
  mMsg(msg),
  mMsgSize(msgSize),
  mOffset(0)
{}

char* BSerialize::readBytes(size_t numBytes)
{
  if (mOffset + numBytes > mMsgSize)
  {
    throw std::runtime_error("Read passed end of buffer");
    return nullptr;
  }

  char* offsetPtr = mMsg + mOffset;

  mOffset += numBytes;

  return offsetPtr;
}

void BSerialize::writeBytes(const char* bytes, size_t numBytes)
{
  if (mOffset + numBytes > mMsgSize)
  {
    throw std::runtime_error("BSerialize: Write passed end of buffer");
    return;
  }

  char* offsetPtr = mMsg + mOffset;
  std::memcpy(offsetPtr, bytes, numBytes);

  mOffset += numBytes;
}

const char* BSerialize::readNullTermString()
{
  int nullIndex = -1;
  size_t size = 0;
  for (size_t i = mOffset; i < mMsgSize; ++i)
  {
    ++size;
    if (mMsg[i] == '\0')
    {
      nullIndex = i;
      break;
    }
  }

  if (nullIndex != -1)
  {
    return readBytes(size);
  }
  else
  {
    throw std::runtime_error("BSerialize: Failed to find null terminator.");
    return nullptr;
  }
}

void BSerialize::writeNullTermString(const char* str)
{
  size_t length = std::strlen(str);
  writeBytes(str, length + 1);
}

} // namespace spire
