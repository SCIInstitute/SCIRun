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


#include "CerealHeap.hpp"

namespace spire {

namespace heap_detail {

bool checkTnyType(Tny* root, TnyType type)
{
  if (root == NULL)
  {
    std::cerr << "es-cereal: deserialize root is NULL" << std::endl;
    throw std::runtime_error("NULL ptr");
    return false;
  }

  if (root->type != type)
  {
    std::cerr << "es-cereal: Unexpected Tny type to deserialize." << std::endl;
    throw std::runtime_error("Unexpected Tny type");
    return false;
  }

  return true;
}

Tny* addSerializedComponent(Tny* cur, Tny* component, uint64_t entityID)
{
  // A TNY_INT64 is really an UINT64
  cur = Tny_add(cur, TNY_INT64, NULL, static_cast<void*>(&entityID), 0);
  cur = Tny_add(cur, TNY_OBJ, NULL, component, 0);
  return cur;
}

Tny* writeSerializedHeap(ComponentSerialize& s, Tny* compArray)
{
  // The heap header will contain all information regarding values.
  Tny* root = Tny_add(NULL, TNY_ARRAY, NULL, NULL, 0);

  // Retrieve header indicating the types that have been serialized.
  Tny* typeHeader = s.getTypeHeader();
  root = Tny_add(root, TNY_OBJ, NULL, typeHeader, 0);

  // Add all serialized data.
  root = Tny_add(root, TNY_OBJ, NULL, compArray, 0);

  Tny_free(typeHeader);

  return root;
}

Tny* readSerializedHeap(ComponentSerialize& /* s */, Tny* root,
                        std::vector<ComponentSerialize::HeaderItem>& typeHeaders)
{
  if (!heap_detail::checkTnyType(root, TNY_ARRAY)) return nullptr;
  if (!Tny_hasNext(root)) return nullptr;
  root = Tny_next(root);
  if (root->type != TNY_OBJ) return nullptr;

  Tny* typeHeader = root->value.tny;

  // Populate typeHeaders
  std::string name;
  std::string typeName;
  while (Tny_hasNext(typeHeader))
  {
    typeHeader = Tny_next(typeHeader);
    if (typeHeader->type != TNY_BIN) return nullptr;

    // Read the name from key, and the string is binary inside of the Tny obj.
    name = typeHeader->key;
    typeName = reinterpret_cast<const char*>(typeHeader->value.ptr);

    typeHeaders.push_back(
        ComponentSerialize::HeaderItem(name.c_str(), typeName.c_str()));
  }

  if (!Tny_hasNext(root)) return nullptr;
  root = Tny_next(root);
  if (root->type != TNY_OBJ) return nullptr;

  Tny* components = root->value.tny;

  if (components->type != TNY_ARRAY) return nullptr;

  return components;
}

} // namespace heap_detail

}
