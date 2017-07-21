
#include "BSerialize.hpp"

namespace CPM_BSERIALIZE_NS {

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
  for (int i = mOffset; i < mMsgSize; ++i)
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

} // namespace CPM_BSERIALIZE_NS

