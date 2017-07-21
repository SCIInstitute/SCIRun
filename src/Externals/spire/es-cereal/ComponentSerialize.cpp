#include "ComponentSerialize.hpp"
#include <tny/tny.hpp>

namespace CPM_ES_CEREAL_NS {

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
    std::cerr << "cpm-es-cereal: Logic error - prepareForNewComponent called when deserializing." << std::endl;
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

} // namespace CPM_ES_CEREAL_NS

