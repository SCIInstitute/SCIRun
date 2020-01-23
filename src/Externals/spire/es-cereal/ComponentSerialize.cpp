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


#include "ComponentSerialize.hpp"
#include <tny/tny.hpp>

namespace spire {

ComponentSerialize::~ComponentSerialize()
{
  if (mTnyRoot != NULL && mDeserializing == false)
  {
    Tny_free(mTnyRoot);
  }
}

void ComponentSerialize::prepareForNewComponent(int32_t componentIndex)
{
  if (mDeserializing)
  {
    std::cerr << "es-cereal: Logic error - prepareForNewComponent called when deserializing." << std::endl;
  }

  mLastIndex = -1;

  if (mTnyRoot != NULL && mDeserializing == false)
  {
    Tny_free(mTnyRoot);
  }

  mTnyRoot = Tny_add(NULL, TNY_DICT, NULL, NULL, 0);

  if (componentIndex != -1)
  {
    // Add __cindex to the dictionary.
    const char* cindexStr = "__cindex";
    mTnyRoot = Tny_add(mTnyRoot, TNY_INT32, const_cast<char*>(cindexStr), &componentIndex, 0);
  }
}

Tny* ComponentSerialize::getSerializedObject()
{
  return mTnyRoot->root;
}

Tny* ComponentSerialize::getTypeHeader()
{
  // Build the type header (order is important!)
  Tny* root = Tny_add(NULL, TNY_DICT, NULL, NULL, 0);

  for (HeaderItem& item : mHeader)
  {
    root = Tny_add(root, TNY_BIN, const_cast<char*>(item.name.c_str()),
                   static_cast<void*>(const_cast<char*>(item.basicTypeName.c_str())),
                   item.basicTypeName.size() + 1);  // + 1 for the \0.
  }

  return root->root;
}

} // namespace spire
