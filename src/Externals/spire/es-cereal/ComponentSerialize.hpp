#ifndef IAUNS_COMMON_COMPONENTSERIALIZE_HPP
#define IAUNS_COMMON_COMPONENTSERIALIZE_HPP

#include <entity-system/ESCoreBase.hpp>
#include "CerealTypeSerialize.hpp"

struct _Tny;
typedef _Tny Tny;

namespace CPM_ES_CEREAL_NS {

// Idea to speed up serialization:
// Add integer block alongside every component. This will denote the offsets
// into the component heap header of the component. This will be in pairs:
// (component offset, header offset)
// This will make deserialization much faster. We don't have a dictionary
// lookup.


// Class that supports basic type serialization with the tny library.
class ComponentSerialize
{
public:

  // This class will be instantiated inside of the static functions.
  // The instance members in this class are only used when the serialize
  // member function is called.
  ComponentSerialize(CPM_ES_NS::ESCoreBase& core, bool deserializing) :
    mDeserializing(deserializing),
    mLastIndex(-1),
    mTnyRoot(NULL),
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
      if (mLastIndex < mHeader.size())
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
  CPM_ES_NS::ESCoreBase& getCore()  {return mCore;}

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

  CPM_ES_NS::ESCoreBase&  mCore;          ///< ESCore.
};

/// Interface defining what a ComponentHeap must implement in order to properly
/// serialize the component system.
class ComponentSerializeInterface
{
public:
  virtual Tny* serialize(CPM_ES_NS::ESCoreBase& core) = 0;
  virtual Tny* serializeEntity(CPM_ES_NS::ESCoreBase& core, uint64_t entity) = 0;
  virtual void deserializeMerge(CPM_ES_NS::ESCoreBase& core, Tny* root, bool copyExisting) = 0;
  virtual void deserializeCreate(CPM_ES_NS::ESCoreBase& core, Tny* root) = 0;
  virtual bool isSerializable() {return true;}

  virtual const char* getComponentName() = 0;
};

} // namespace CPM_ES_CEREAL_NS

#endif 
