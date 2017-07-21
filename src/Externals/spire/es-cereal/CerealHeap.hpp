#ifndef IAUNS_COMMON_CEREALHEAP_HPP
#define IAUNS_COMMON_CEREALHEAP_HPP

#include <entity-system/ESCoreBase.hpp>
#include <tny/tny.hpp>

/// \todo Add option to suppress type headers and use names only.
///       Type headers should only be used when you want to perform some
///       sort of reflection on your datatypes and figure out what
///       types they really are.

#include "ComponentSerialize.hpp"

namespace CPM_ES_CEREAL_NS {

namespace heap_detail {

bool checkTnyType(Tny* root, TnyType type);
Tny* addSerializedComponent(Tny* cur, Tny* component, uint64_t entityID);
Tny* writeSerializedHeap(ComponentSerialize& s, Tny* compArray);
Tny* readSerializedHeap(ComponentSerialize& s, Tny* compArray,
                        std::vector<ComponentSerialize::HeaderItem>& typeHeaders);
}


template <typename T>
class CerealHeap : public CPM_ES_NS::ComponentContainer<T>, public ComponentSerializeInterface
{
  /// Checking if a function exists at runtime.
  /// http://stackoverflow.com/questions/10957924/is-there-any-way-to-detect-whether-a-function-exists-and-can-be-used-at-compile
  template <typename U>
  struct has_member_serialize
  {
    typedef char yes;
    struct no { char _[2]; };
    template<typename V, bool (V::*)(ComponentSerialize&, uint64_t) = &V::serialize>
    static yes impl( V* );
    static no  impl(...);

    enum { value = sizeof( impl( static_cast<U*>(0) ) ) == sizeof(yes) };
  };

  /// Has getname function implementation
  /// Note: This is different from the serialize function above because
  ///       getName is expected to be static.
  template <typename U>
  struct has_member_getname
  {
    typedef char yes;
    struct no { char _[2]; };
    template<typename V, const char* (*)() = &V::getName>
    static yes impl( V* );
    static no  impl(...);

    enum { value = sizeof( impl( static_cast<U*>(0) ) ) == sizeof(yes) };
  };

public:
  CerealHeap() : mIsSerializable(true)  {}
  virtual ~CerealHeap()                 {}

  Tny* serialize(CPM_ES_NS::ESCoreBase& core) override
  {
    static_assert( has_member_serialize<T>::value,
                  "Component does not have a serialize function with signature: bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize&, uint64_t)" );

    // Build component array.
    Tny* compArray = Tny_add(NULL, TNY_ARRAY, NULL, NULL, 0);

    ComponentSerialize s(core, false);

    for (auto it = CPM_ES_NS::ComponentContainer<T>::mComponents.begin();
         it != CPM_ES_NS::ComponentContainer<T>::mComponents.end(); ++it)
    {
      s.prepareForNewComponent();
      if (it->component.serialize(s, it->sequence))
      {
        compArray = heap_detail::addSerializedComponent(
            compArray, s.getSerializedObject(), it->sequence);
      }
    }

    Tny* root = heap_detail::writeSerializedHeap(s, compArray);

    Tny_free(compArray);

    return root->root;
  }

  /// \todo Add serializeEntityComponent function! Serializes one component,
  ///       of a particular entity, at a particular component index.
  Tny* serializeEntity(CPM_ES_NS::ESCoreBase& core, uint64_t entityID) override
  {
    static_assert( has_member_serialize<T>::value,
                  "Component does not have a serialize function with signature: bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize&, uint64_t)" );

    // Attempt to find entity in our component array. Then serialize all
    // components related to that entity.
    int baseIndex = CPM_ES_NS::ComponentContainer<T>::getComponentItemIndexWithSequence(entityID);
    if (baseIndex == -1)
    {
      std::cerr << "Unable to find entityID " << entityID << " in " << getComponentName() << std::endl;
      return NULL;
    }

    Tny* compArray = Tny_add(NULL, TNY_ARRAY, NULL, NULL, 0);

    ComponentSerialize s(core, false);

    typename CPM_ES_NS::ComponentContainer<T>::ComponentItem* array =
        CPM_ES_NS::ComponentContainer<T>::getComponentArray();
    int i = baseIndex;
    size_t numComponents = CPM_ES_NS::ComponentContainer<T>::getNumComponents();
    while (i != numComponents && array[i].sequence == entityID)
    {
      // Serialize the entity at index 'i'.
      s.prepareForNewComponent();
      if (array[i].component.serialize(s, entityID))
        compArray = heap_detail::addSerializedComponent(compArray, s.getSerializedObject(), entityID);
      ++i;
    }

    Tny* root = heap_detail::writeSerializedHeap(s, compArray);

    Tny_free(compArray);

    return root;
  }

  /// Returns the Tny* dictionary containing value's serialized contents.
  /// for the given entityID and componentIndex.
  Tny* serializeValue(CPM_ES_NS::ESCoreBase& core, T& value, uint64_t entityID, int32_t componentIndex)
  {
    static_assert( has_member_serialize<T>::value,
                  "Component does not have a serialize function with signature: bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize&, uint64_t)" );

    Tny* compArray = Tny_add(NULL, TNY_ARRAY, NULL, NULL, 0);

    ComponentSerialize s(core, false);
    s.prepareForNewComponent();
    if (value.serialize(s, entityID))
      compArray = heap_detail::addSerializedComponent(compArray, s.getSerializedObject(), entityID);

    Tny* root = heap_detail::writeSerializedHeap(s, compArray);

    Tny_free(compArray);

    return root;
  }

  void deserializeMerge(CPM_ES_NS::ESCoreBase& core, Tny* root, bool copyExisting) override
  {
    static_assert( has_member_serialize<T>::value,
                  "Component does not have a serialize function with signature: bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize&, uint64_t)" );
    deserializeMergeInternal(core, root, copyExisting);
  }

  void deserializeCreate(CPM_ES_NS::ESCoreBase& core, Tny* root) override
  {
    static_assert( has_member_serialize<T>::value,
                  "Component does not have a serialize function with signature: bool serialize(CPM_ES_CEREAL_NS::ComponentSerialize&, uint64_t)" );
    deserializeCreateInternal(core, root);
  }

  const char* getComponentName() override
  {
    static_assert( has_member_getname<T>::value,
                  "Component does not have a getName function with signature: static const char* getName()" );
    return T::getName();
  }

  std::string getTypeOfElement(const char* elementName)
  {
    if (mTypeHeaders.size() == 0)
    {
      std::cerr << "cpm-es-cereal: Can't find type name, don't have type data (have you deserialized once?)." << std::endl;
      return std::string();
    }

    for (ComponentSerialize::HeaderItem& item : mTypeHeaders)
    {
      if (item.name == elementName)
      {
        return item.basicTypeName;
      }
    }

    std::cerr << "cpm-es-cereal: Unable to find type name." << std::endl;
    return std::string();
  }

  bool isSerializable() override          {return mIsSerializable;}
  void setSerializable(bool serializable) {mIsSerializable = serializable;}

private:

  void deserializeMergeInternal(CPM_ES_NS::ESCoreBase& core, Tny* root, bool copyExisting)
  {
    /// \xxx  We may be erasing good type headers in preference of partial
    ///       type headers when we merge (delta compression).
    ComponentSerialize s(core, true);

    // Extract header information and grab Tny pointer to actual data.
    mTypeHeaders.clear();
    Tny* components = heap_detail::readSerializedHeap(s, root, mTypeHeaders);
    if (components == nullptr)
    {
      std::cerr << "cpm-es-cereal: Corrupt heap header." << std::endl;
      return;
    }

    T value;
    typename CPM_ES_NS::ComponentContainer<T>::ComponentItem* array = 
        CPM_ES_NS::ComponentContainer<T>::getComponentArray();
    Tny* cur = components;
    int componentIndex = 0;
    uint64_t lastEntityID = 0;
    while (Tny_hasNext(cur))
    {
      cur = Tny_next(cur);

      if (!heap_detail::checkTnyType(cur, TNY_INT64)) return;

      uint64_t entityID = cur->value.num;

      if (!Tny_hasNext(cur))
      {
        std::cerr << "cpm-es-cereal: Unexpected end of header." << std::endl;
        throw std::runtime_error("cpm-es-cereal: Unexpected end of header.");
        return;
      }

      cur = Tny_next(cur);

      if (!heap_detail::checkTnyType(cur, TNY_OBJ)) return;

      if (lastEntityID == entityID)
        ++componentIndex;
      else
        componentIndex = 0;

      // Check to ensure that the entityID exists alongised the correct
      // component ID. These will be used together to add a modification
      // to the current state of the component system.
      int baseIndex = CPM_ES_NS::ComponentContainer<T>::getComponentItemIndexWithSequence(entityID);
      if (baseIndex != -1)
      {
        Tny* obj = cur->value.tny;
        if (!heap_detail::checkTnyType(obj, TNY_DICT)) return;

        // Check to see if __cindex exists inside of the dictionary.
        int trueIndex = 0;
        if (Tny_get(obj, "__cindex") != NULL)
        {
          int32_t serializedIndex = 0;
          CerealSerializeType<int32_t>::in(obj, "__cindex", componentIndex);

          // Compute using the index that was given alongside the object.
          trueIndex = baseIndex + serializedIndex;
        }
        else
        {
          // Use a computed index based on the number of times a component
          // with the same entity ID is deserialized.
          trueIndex = baseIndex + componentIndex;
        }

        if (trueIndex < CPM_ES_NS::ComponentContainer<T>::getNumComponents())
        {
          if (array[trueIndex].sequence == entityID)
          {
            if (copyExisting)
            {
              // Copy the pre-existing component. We can deserialize less data
              // than what is actually in the component. This is important when
              // we are using delta compression.
              value = array[trueIndex].component;
            }

            // We have a valid index and value. Add it as a higher priority
            // item to the modification array.
            s.setDeserializeRoot(obj);
            if (value.serialize(s, entityID))
              CPM_ES_NS::ComponentContainer<T>::modifyIndex(value, trueIndex, 10000);
          }
        }

      }
    }
  }

  void deserializeCreateInternal(CPM_ES_NS::ESCoreBase& core, Tny* root)
  {
    /// \xxx  We may be erasing good type headers in preference of partial
    ///       type headers when we merge (delta compression).
    ComponentSerialize s(core, true);

    // Extract header information and grab Tny pointer to actual data.
    mTypeHeaders.clear();
    Tny* components = heap_detail::readSerializedHeap(s, root, mTypeHeaders);
    if (components == nullptr)
    {
      std::cerr << "cpm-es-cereal: Corrupt heap header." << std::endl;
      return;
    }

    T value;
    Tny* cur = components;
    while (Tny_hasNext(cur))
    {
      cur = Tny_next(cur);

      if (!heap_detail::checkTnyType(cur, TNY_INT64)) return;

      uint64_t entityID = cur->value.num;

      // Ensure next TNY_OBJ is present.
      if (!Tny_hasNext(cur))
      {
        std::cerr << "cpm-es-cereal: Unexpected end of header." << std::endl;
        throw std::runtime_error("cpm-es-cereal: Unexpected end of header.");
        return;
      }

      cur = Tny_next(cur);

      if (!heap_detail::checkTnyType(cur, TNY_OBJ)) return;

      Tny* obj = cur->value.tny;
      s.setDeserializeRoot(obj);
      if (value.serialize(s, entityID))
        CPM_ES_NS::ComponentContainer<T>::addComponent(entityID, value);
    }
  }

  /// Type information that we obtained from deserialization. This contains
  /// what *explicit* type is associated with a particular name.
  std::vector<ComponentSerialize::HeaderItem>   mTypeHeaders;

  ///< Default: true. Set to false if this component should not be serialized.
  bool mIsSerializable;
};

} // namespace CPM_ES_CEREAL_NS

#endif

