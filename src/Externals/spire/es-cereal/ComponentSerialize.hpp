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


#ifndef SPIRE_COMMON_COMPONENTSERIALIZE_HPP
#define SPIRE_COMMON_COMPONENTSERIALIZE_HPP

#include <es-log/trace-log.h>
#include <entity-system/ESCoreBase.hpp>
#include "CerealTypeSerialize.hpp"
#include <spire/scishare.h>

struct _Tny;
typedef _Tny Tny;

namespace spire {

// Idea to speed up serialization:
// Add integer block alongside every component. This will denote the offsets
// into the component heap header of the component. This will be in pairs:
// (component offset, header offset)
// This will make deserialization much faster. We don't have a dictionary
// lookup.


// Class that supports basic type serialization with the tny library.
class SCISHARE ComponentSerialize
{
public:

  // This class will be instantiated inside of the static functions.
  // The instance members in this class are only used when the serialize
  // member function is called.
  ComponentSerialize(spire::ESCoreBase& core, bool deserializing) :
    mLastIndex(-1),
    mDeserializing(deserializing),
    mTnyRoot(nullptr),
    mCore(core)
  {
    if (deserializing) mHeader.reserve(15);
  }

  // This is generally the only function that you will care about in this class.
  template <typename T>
  void serialize(const char* name, T& v)
  {
    // Based on whether we are serializing in / out, we will either accept the
    // value of the type or set the value of the type.

    // Using template specialization we will select the appropriate context
    // under which we will serialize the type.
    if (isDeserializing() == true)
    {
      // Find the name in our current component dictionary and serialize.
      // Use template specialization to turn Tny object into appropriate type.
      CerealSerializeType<T>::in(mTnyRoot, name, v);
    }
    else
    {
      ++mLastIndex;

      // Check mLastIndex (if it exists), and see if it has same name
      // as the object we are trying to serialize.
      bool searchForName = true;
      if (mLastIndex < static_cast<int>(mHeader.size()))
      {
        if (mHeader[mLastIndex].name == name)
        {
          searchForName = false;
        }
      }

      if (searchForName)
      {
        bool foundName = false;
        for (HeaderItem& item : mHeader)
        {
          if (item.name == name)
          {
            foundName = true;
            break;
          }
        }

        if (!foundName)
        {
          // Add the name to header.
          mHeader.push_back(HeaderItem(name, CerealSerializeType<T>::getTypeName()));
        }
      }

      // Insert the name along with the Tny object serialized from the
      // appropriate type.
      mTnyRoot = CerealSerializeType<T>::out(mTnyRoot, name, v);
    }
  }

  virtual ~ComponentSerialize();

  /// Prepares this class for a new component. Only called when serializing.
  void prepareForNewComponent(int32_t componentIndex = -1);

  /// Retrieves the Tny object that represents the current state of
  /// serialization.
  Tny* getSerializedObject();

  /// True if deserializing into variables.
  bool isDeserializing()        {return mDeserializing;}

  /// Sets the root element to use for deserialization.
  void setDeserializeRoot(Tny* root) {mTnyRoot = root;}

  /// Constructs a header containing the real types of elements.
  Tny* getTypeHeader();

  /// Retrieves the core that is currently responsible for creating this
  /// serialization class.
  spire::ESCoreBase& getCore()  {return mCore;}

  // A header doesn't need to be built for the components. It's just for
  // determining explicit type is being serialized remotely. Later, it could
  // be used for speeding up deserialization alongside component header that
  // has pairs of integers that denotes 1) a range inside of the serialize
  // function (in case the serialize function has conditional statements), and
  // 2) a range inside of the header indicating which serialization type is
  // coming next.
  struct HeaderItem
  {
    HeaderItem(const char* nameIn, const char* typeNameIn) :
        name(nameIn),
        basicTypeName(typeNameIn)
    {}

    std::string name;             ///< Name of the component.
    std::string basicTypeName;    ///< Used for external programs to identify types.
                                  ///< Tny doesn't contain many basic types.
  };

private:

  int                     mLastIndex;     ///< Last memoized index inside mHeader.
  std::vector<HeaderItem> mHeader;        ///< Deserialize header.

  bool                    mDeserializing; ///< True if we are serializing into variables.
  Tny*                    mTnyRoot;       ///< When serializing in, this is the source.

  spire::ESCoreBase&  mCore;          ///< ESCore.
};

/// Interface defining what a ComponentHeap must implement in order to properly
/// serialize the component system.
class ComponentSerializeInterface
{
public:
  virtual Tny* serialize(spire::ESCoreBase& core) = 0;
  virtual Tny* serializeEntity(spire::ESCoreBase& core, uint64_t entity) = 0;
  virtual void deserializeMerge(spire::ESCoreBase& core, Tny* root, bool copyExisting) = 0;
  virtual void deserializeCreate(spire::ESCoreBase& core, Tny* root) = 0;
  virtual bool isSerializable() const {return true;}

  virtual const char* getComponentName() const = 0;
};

} // namespace spire

#endif
