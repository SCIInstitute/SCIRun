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
  RENDERER_LOG("VarBuffer ctor (mBufferSize {})", mBufferSize);
  mSerializer.reset(new spire::BSerialize(getBuffer(), mBufferSize));
}

VarBuffer::VarBuffer(uint32_t size) :
mBuffer(size),
mBufferSize(size)
{
  RENDERER_LOG("VarBuffer ctor (mBufferSize {})", size);
  mSerializer.reset(new spire::BSerialize(getBuffer(), mBufferSize));
}

void VarBuffer::clear()
{
  mSerializer->setOffset(0);
}

/// Writes \p numBytes of \p bytes.
void VarBuffer::writeBytes(const char* bytes, size_t numBytes)
{
  RENDERER_LOG("VarBuffer writeBytes (bytes {}, numBytes {})", bytes, numBytes);
  // Resize the buffer if necessary.
  while (mSerializer->getOffset() + numBytes > mBufferSize)
  {
    resize();
  }

  mSerializer->writeBytes(bytes, numBytes);
}

/// Writes a null terminated string.
void VarBuffer::writeNullTermString(const char* str)
{
  RENDERER_LOG("VarBuffer writeNullTermString (str {})", str);

  size_t stringLength = std::strlen(str);

  // Resize the buffer if necessary.
  while (mSerializer->getOffset() + stringLength + 1 > mBufferSize)
  {
    resize();
  }

  mSerializer->writeNullTermString(str);
}

void VarBuffer::resize()
{
  mBufferSize *= 2;
  mBuffer.resize(mBufferSize);

  RENDERER_LOG("VarBuffer resize (oldSize {}, newSize {})", mBufferSize/2, mBufferSize);

  // Record offset before we destroy and recreate the serializer.
  size_t bufferOffset = mSerializer->getOffset();

  // Create a new serializer and reset its offset.
  mSerializer.reset(new spire::BSerialize(getBuffer(), mBufferSize));
  mSerializer->setOffset(bufferOffset);
}

} // namespace spire
