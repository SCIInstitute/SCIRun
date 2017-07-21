#include <cstdlib>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include "VarBuffer.hpp"

namespace CPM_VAR_BUFFER_NS {

VarBuffer::VarBuffer() :
mBuffer(1024),
    mBufferSize(1024)
{
  mSerializer = std::unique_ptr<CPM_BSERIALIZE_NS::BSerialize>(
    new CPM_BSERIALIZE_NS::BSerialize(getBuffer(), mBufferSize));
}

VarBuffer::VarBuffer(uint32_t size) :
mBuffer(size),
mBufferSize(size)
{
  mSerializer = std::unique_ptr<CPM_BSERIALIZE_NS::BSerialize>(
    new CPM_BSERIALIZE_NS::BSerialize(getBuffer(), mBufferSize));
}

VarBuffer::~VarBuffer()
{
}

void VarBuffer::clear()
{
  mSerializer->setOffset(0);
}

/// Writes \p numBytes of \p bytes.
void VarBuffer::writeBytes(const char* bytes, size_t numBytes)
{
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

  // Record offset before we destroy and recreate the serializer.
  size_t bufferOffset = mSerializer->getOffset();
  mSerializer.reset();

  // Create a new serializer and reset its offset.
  mSerializer = std::unique_ptr<CPM_BSERIALIZE_NS::BSerialize>(
      new CPM_BSERIALIZE_NS::BSerialize(getBuffer(), mBufferSize));
  mSerializer->setOffset(bufferOffset);
}

} // namespace CPM_VAR_BUFFER_NS


