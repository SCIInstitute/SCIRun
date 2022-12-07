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


#include <stdlib.h>         // For C's free
#include <cstring>

#include "CerealCore.hpp"
#include <tny/tny.hpp>

namespace spire {

CerealCore::CerealCore()
{
}

CerealCore::~CerealCore()
{
}

std::tuple<void*, size_t> CerealCore::dumpTny(Tny* tny)
{
  void* data = nullptr;
  size_t dataSize = Tny_dumps(tny, &data);
  return std::make_tuple(data, dataSize);
}

Tny* CerealCore::loadTny(void* data, size_t dataSize)
{
  return Tny_loads(data, dataSize);
}

void CerealCore::freeTnyDataPtr(void* ptr)
{
  free(ptr);
}

// serializeAllComponents and serializeEntity are the same function with a
// different ComponentSerialize call. Figure out a way to fix this.
Tny* CerealCore::serializeAllComponents()
{
  // Build dictionary whose keys correspond to the names of the components.
	Tny* root = Tny_add(nullptr, TNY_DICT, nullptr, nullptr, 0);
  Tny* cur = root;
  for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
  {
    // Build a new component array of dictionaries from this heap.
    ComponentSerializeInterface* heap =
        dynamic_cast<ComponentSerializeInterface*>(it->second);

    if (heap->isSerializable())
    {
      Tny* serializedHeap = heap->serialize(*this);

      // Add the serialized heap as a Tny object. Then free serializedHeap.
      // When a TNY_OBJ is added, it is deep copied and not moved.
      cur = Tny_add(cur, TNY_OBJ, const_cast<char*>(heap->getComponentName()), serializedHeap, 0);

      if (cur == nullptr)
      {
        std::cerr << "es-cereal: Failed to serialize all components." << std::endl;
        std::cerr << "Failed on component: " << heap->getComponentName() << std::endl;
        throw std::runtime_error("Failed serialization");
      }

      // Clean up the heap.
      Tny_free(serializedHeap);
    }
  }

  return root;
}

// serializeAllComponents and serializeEntity are the same function with a
// different ComponentSerialize call. Figure out a way to fix this.
Tny* CerealCore::serializeEntity(uint64_t entityID)
{
  // Build dictionary whose keys correspond to the names of the components.
	Tny* root = Tny_add(nullptr, TNY_DICT, nullptr, nullptr, 0);
  Tny* cur = root;
  for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
  {
    // Build a new component array of dictionaries from this heap.
    ComponentSerializeInterface* heap =
        dynamic_cast<ComponentSerializeInterface*>(it->second);

    if (heap->isSerializable())
    {
      Tny* serializedHeap = heap->serializeEntity(*this, entityID);

      // Add the serialized heap as a Tny object. Then free serializedHeap.
      // When a TNY_OBJ is added, it is deep copied and not moved.
      cur = Tny_add(cur, TNY_OBJ, const_cast<char*>(heap->getComponentName()), serializedHeap, 0);

      if (cur == nullptr)
      {
        std::cerr << "es-cereal: Failed to serialize all components." << std::endl;
        std::cerr << "Failed on component: " << heap->getComponentName() << std::endl;
        throw std::runtime_error("Failed serialization");
      }

      // Clean up the heap.
      Tny_free(serializedHeap);
    }
  }

  return root;
}

// deserializeComponentMerge and deserializeComponentCreate are the same
// function with a different ComponentSerialize call. Figure out a way to
// fix this.
void CerealCore::deserializeComponentMerge(Tny* root, bool copyExisting)
{
  if (root == nullptr)
  {
    std::cerr << "es-cereal: deserializeComponentMerge root is NULL" << std::endl;
    throw std::runtime_error("Tny root NULL");
    return;
  }

  /// Root should be a dictionary.
  if (root->type != TNY_DICT)
  {
    std::cerr << "es-cereal: Unexpected Tny type to deserializeComponentMerge." << std::endl;
    throw std::runtime_error("Unexpected Tny type");
    return;
  }

  Tny* cur = root;

  // Iterate through the dictionary, using the dictionary keys of the elements
  // to lookup the correct component containers. Complain if we don't find
  // the correct component container. But do not throw an exception, as this
  // could be a very common case.
  while (Tny_hasNext(cur))
  {
    cur = Tny_next(cur);

    if (cur->type != TNY_OBJ)
    {
      std::cerr << "es-cereal: Unexpected Tny type deserializing heap." << std::endl;
      throw std::runtime_error("Unexpected Tny type");
      return;
    }

    const char* heapName = cur->key;

    // Search for the correct system name in our current set of registered
    // systems. This is not efficient. If more speed is needed, use a map with
    // hashed strings.
    bool foundHeap = false;
    for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
    {
      ComponentSerializeInterface* heap = dynamic_cast<ComponentSerializeInterface*>(it->second);
      if (std::strcmp(heap->getComponentName(), heapName) == 0)
      {
        // Found the appropriate container, obtain Tny object and
        heap->deserializeMerge(*this, cur->value.tny, copyExisting);
        foundHeap = true;
        break;
      }
    }

    if (foundHeap != true)
    {
      std::cerr << "es-cereal: Warning - Unable to find heap with key: " << heapName << std::endl;
      return;
    }
  }
}

// deserializeComponentMerge and deserializeComponentCreate are the same
// function with a different ComponentSerialize call. Figure out a way to
// fix this.
void CerealCore::deserializeComponentCreate(Tny* root)
{
  if (root == nullptr)
  {
    std::cerr << "es-cereal: deserializeComponentMerge root is NULL" << std::endl;
    throw std::runtime_error("Tny root NULL");
    return;
  }

  /// Root should be a dictionary.
  if (root->type != TNY_DICT)
  {
    std::cerr << "es-cereal: Unexpected Tny type to deserializeComponentMerge." << std::endl;
    throw std::runtime_error("Unexpected Tny type");
    return;
  }

  Tny* cur = root;

  // Iterate through the dictionary, using the dictionary keys of the elements
  // to lookup the correct component containers. Complain if we don't find
  // the correct component container. But do not throw an exception, as this
  // could be a very common case.
  while (Tny_hasNext(cur))
  {
    cur = Tny_next(cur);

    if (cur->type != TNY_OBJ)
    {
      std::cerr << "es-cereal: Unexpected Tny type deserializing heap." << std::endl;
      throw std::runtime_error("Unexpected Tny type");
      return;
    }

    const char* heapName = cur->key;

    // Search for the correct system name in our current set of registered
    // systems. This is not efficient. If more speed is needed, use a map with
    // hashed strings.
    bool foundHeap = false;
    for (auto it = mComponents.begin(); it != mComponents.end(); ++it)
    {
      ComponentSerializeInterface* heap = dynamic_cast<ComponentSerializeInterface*>(it->second);
      if (std::strcmp(heap->getComponentName(), heapName) == 0)
      {
        // Found the appropriate container, obtain Tny object and
        heap->deserializeCreate(*this, cur->value.tny);
        foundHeap = true;
        break;
      }
    }

    if (foundHeap != true)
    {
      std::cerr << "es-cereal: Warning - Unable to find heap with key: " << heapName << std::endl;
      return;
    }
  }
}

}
