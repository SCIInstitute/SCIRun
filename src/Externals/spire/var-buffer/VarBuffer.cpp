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


#include <cstdlib>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include "VarBuffer.hpp"

namespace spire {

VarBuffer::VarBuffer() :
mBuffer(1024),
mBufferSize(1024)
{
  mSerializer.reset(new spire::BSerialize(getBuffer(), mBufferSize));
}

VarBuffer::VarBuffer(size_t size) :
mBuffer(size),
mBufferSize(size)
{
  mSerializer.reset(new spire::BSerialize(getBuffer(), mBufferSize));
}

void VarBuffer::clear()
{
  mSerializer->setOffset(0);
}

void VarBuffer::writeBytes(const char* bytes, size_t numBytes)
{
  while (mSerializer->getOffset() + numBytes > mBufferSize)
    resize();

  mSerializer->writeBytes(bytes, numBytes);
}

void VarBuffer::writeNullTermString(const char* str)
{
  size_t stringLength = std::strlen(str);

  while (mSerializer->getOffset() + stringLength + 1 > mBufferSize)
    resize();

  mSerializer->writeNullTermString(str);
}

void VarBuffer::resize()
{
  mBufferSize *= 2;
  mBuffer.resize(mBufferSize);

  size_t bufferOffset = mSerializer->getOffset();

  mSerializer.reset(new spire::BSerialize(getBuffer(), mBufferSize));
  mSerializer->setOffset(bufferOffset);
}

} // namespace spire
