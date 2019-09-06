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
