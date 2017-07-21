#ifndef IAUNS_BSERIALIZE_H_
#define IAUNS_BSERIALIZE_H_

#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "src/BSerializeDetail.hpp"

namespace CPM_BSERIALIZE_NS {

class BSerialize
{
public:
  /// Use this constructor for either reading or writing.
  BSerialize(char* msg, size_t msgSize);

  /// Read \p numBytes from the serialization buffer.
  char* readBytes(size_t numBytes);

  /// Write \p numBytes to the serialization buffer.
  void writeBytes(const char* bytes, size_t numBytes);

  /// Reads a null-terminated string. Returns a pointer to the string in-memory.
  const char* readNullTermString();

  /// Writes a null-terminated string.
  void writeNullTermString(const char* str);

  /// Types allowed for reading and writing:
  /// const char*, std::string, int8_t, uint8_t, int16_t, uint16_t, int32_t,
  /// uint32_t, int64_t, uint64_t, float, double.
  /// @{
  template <typename T>
  T read()
  {
    return detail::SerializeType<T>::read(mMsg, mMsgSize, &mOffset);
  }

  /// Returns true if the type was successfully written to the buffer. Returns
  /// false if there was not enough room to place the type in the buffer.
  template <typename T>
  bool write(const T& val)
  {
    return detail::SerializeType<T>::write(mMsg, mMsgSize, &mOffset, val);
  }
  /// @}

  /// Retrieves the current buffer offset. If we are writing to the buffer,
  /// then this is the size of the data currently written. If we are reading,
  /// then this is the read position.
  size_t getOffset()                {return mOffset;}

  /// Sets the offset to a given value. This can be used to advance a read
  /// buffer, or skip over memory in a write buffer.
  void setOffset(size_t newOffset)  {mOffset = newOffset;}

  /// Returns the number of bytes left in the buffer.
  size_t getBytesLeft() {return mMsgSize - mOffset;}

private:

  char*   mMsg;
  size_t  mMsgSize;
  size_t  mOffset;
};

} // namespace CPM_BSERIALIZE_NS

#endif

