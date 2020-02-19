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


#ifndef SPIRE_CEREALCORE_HPP
#define SPIRE_CEREALCORE_HPP

#include <es-log/trace-log.h>
#include <set>
#include <iostream>
#include <stdexcept>
#include <entity-system/ESCoreBase.hpp>

#include "CerealHeap.hpp"
#include "ComponentSerialize.hpp"
#include <spire/scishare.h>

struct _Tny;
typedef _Tny Tny;

namespace spire {

class SCISHARE CerealCore : public spire::ESCoreBase
{
public:
  CerealCore();
  virtual ~CerealCore();

  /// Converts a Tny pointer into a pointer to a data location (void*) and
  /// a size of the pointed to memory location. The memory is allocated
  /// with 'malloc' (not std::malloc) inside of the Tny library. I'm unsure
  /// as to the compatibility of these two functions. To be on the safe side,
  /// use the non-namespaced free from C's stdlib.h when freeing this memory.
  /// Or just use the free Tny data pointer function given below.
  /// The caller is responsible for calling C's free, or freeTnyDataPtr on
  /// the returned void*.
  static std::tuple<void*, size_t> dumpTny(Tny* tny);

  /// Accepts a pointer to Tny data and the size of the Tny data, and
  /// then converts the inputs into a Tny pointer which can be given to
  /// any one of the deserialize functions below. The data pointer is not
  /// touched and it is not freed. Will return NULL if the data is invalid,
  /// or their was a failure.
  /// The caller is responsible for calling Tny_free on the returned Tny*.
  static Tny* loadTny(void* data, size_t dataSize);

  /// Uses the correct 'free()' function to free the data pointer returned
  /// by dumpTny.
  static void freeTnyDataPtr(void* ptr);

  /// Serializes all components.
  /// The caller is responsible for calling Tny_free on the returned Tny*.
  Tny* serializeAllComponents();

  /// Serializes a single entity into CerealSerialize.
  /// The caller is responsible for calling Tny_free on the returned Tny*.
  Tny* serializeEntity(uint64_t entityID);

  /// Serializes a Tny pointer as if it were an entity. Useful in constructing
  /// change sets. Output can be used in conjunction with
  /// deserializeComponentMerge. The caller is responsible for calling Tny_free
  /// on the returned Tny*.
  /// \param value            Tny dictionary of values that is to be serialized.
  /// \param entityID         EntityID to associate with serialized data.
  /// \param componentIndex   If not -1, then this is the index within the
  ///                         array of entityIDs to place this serialized value.
  template <typename T>
  Tny* serializeValue(T& value, uint64_t entityID, int32_t componentIndex = -1)
  {
    // Grab the CerealHeap based off of the
    spire::BaseComponentContainer* cont = ensureComponentArrayExists<T, CerealHeap<T>>();

    // Convert value into a TNY_DICT, then call the necessary function to
    // slap on a valid serialization header.
    CerealHeap<T>* heap = dynamic_cast<CerealHeap<T>*>(cont);
    if (heap->isSerializable() == false)
    {
      std::cerr << "Attempting to explicitly serialize value from non-serializable component." << std::endl;
      throw std::runtime_error("Non-serializable component.");
      return nullptr;
    }
    Tny* val = heap->serializeValue(*this, value, entityID, componentIndex);

    // Add val to a dictionary which contains the component's name.
    Tny* root = Tny_add(NULL, TNY_DICT, NULL, NULL, 0);
    root = Tny_add(root, TNY_OBJ, const_cast<char*>(heap->getComponentName()), val, 0);

    // Get rid of value since a deep copy was made.
    Tny_free(val);

    // Pass back the root.
    return root->root;
  }

  /// Deserializes all components given a Tny root. Will merge all pre-existing
  /// components with components found inside of Tny root. Will not create new
  /// components. Only components that currently exist in the component
  /// system will be updated. Renormalization is required after calling.
  /// Accepts Tny output from serializeAllComponents and serializeEntity
  /// (anything that serializes components). This function does not call
  /// Tny_free. If \p copyExisting is true, then the existing element is
  /// copied into the memory of the new value, then new values are serialized
  /// over the top of the copied values. This is useful if you fine grained
  /// delta compression is being used.
  void deserializeComponentMerge(Tny* root, bool copyExisting);

  /// Create components from serialized data. Creates components regardless of
  /// the existence of any other components. Renormalization is required after
  /// calling this function in order to add the components. Accepts Tny output
  /// from serializeAllComponents and serializeEntity (anything that serializes
  /// components). This function does not call Tny_free.
  void deserializeComponentCreate(Tny* root);

  /// Registers a component. This builds a component heap if one is not already
  /// present. This is not strictly mandatory, but will help avoid errors if you
  /// are deserializing a saved state and have not used all of the components
  /// in the system yet.
  template <typename T>
  void registerComponent()
  {
    spire::BaseComponentContainer* system = ensureComponentArrayExists<T, CerealHeap<T>>();
    const char* name = dynamic_cast<CerealHeap<T>*>(system)->getComponentName();

    // Ensure there are no duplicate component names.
    if (std::get<1>(mComponentNames.insert(std::string(name))) == false)
    {
      std::cerr << "es-cereal: Component with duplicate name." << " Name: " << name << std::endl;
      throw std::runtime_error("es-cereal: Component with duplicate name.");
      return;
    }

    mComponentIDNameMap.insert(std::make_pair(spire::TemplateID<T>::getID(), std::string(name)));
  }

  template <typename T>
  void addComponent(uint64_t entityID, const T& component)
  {
    spire::BaseComponentContainer* componentContainer
        = getComponentContainer(spire::TemplateID<T>::getID());
    if (componentContainer == nullptr)
    {
      std::cerr << "es-cereal: Warning - addComponent called but component has not been registered yet!" << std::endl;
      std::cerr << "es-cereal: Component - " << T::getName() << std::endl;
    }

    coreAddComponent<T, CerealHeap<T>>(entityID, component);
  }

  template <typename T>
  size_t addStaticComponent(T&& component)
  {
    spire::BaseComponentContainer* componentContainer
        = getComponentContainer(spire::getESTypeID<T>());
    if (componentContainer == nullptr)
    {
      std::cerr << "es-cereal: Warning - addStaticComponent called but component has not been registered yet!" << std::endl;
      std::cerr << "es-cereal: Component - " << std::decay<T>::type::getName() << std::endl;
    }

    return coreAddStaticComponent<T, CerealHeap<typename std::decay<T>::type>>(std::forward<T>(component));
  }

  /// Marks the entire component container as non-serializable. This is useful
  /// in a limited number of cases, such as static components, where you want
  /// to have a pointer to a unique class whose implementation *depends on
  /// the platform*.
  template <typename T>
  void disableComponentSerialization()
  {
    // Grab the CerealHeap based off of the
    spire::BaseComponentContainer* cont = ensureComponentArrayExists<T, CerealHeap<T>>();

    CerealHeap<T>* heap = dynamic_cast<CerealHeap<T>*>(cont);
    heap->setSerializable(false);
  }

  /// Ensures the component container exists and returns the container.
  template <typename T>
  CerealHeap<T>* getOrCreateComponentContainer()
  {
    spire::BaseComponentContainer* cont = ensureComponentArrayExists<T, CerealHeap<T>>();
    return dynamic_cast<CerealHeap<T>*>(cont);
  }

protected:

  /// Set containing names of all components registered this far. Used to ensure
  /// no name conflicts are registered.
  std::set<std::string>           mComponentNames;
  std::map<uint64_t, std::string> mComponentIDNameMap;
};

} // namespace spire

#endif
