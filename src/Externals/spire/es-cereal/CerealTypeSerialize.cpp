#include <iostream>

#include "CerealTypeSerialize.hpp"
#include <tny/tny.hpp>

namespace CPM_ES_CEREAL_NS {
namespace CST_detail {

template <typename T>
Tny* tnyGenericOut(Tny* root, const char* name, const T& v, TnyType type)
{
  T* ptr = const_cast<T*>(&v);
  return Tny_add(root, type, const_cast<char*>(name), static_cast<void*>(ptr), 0);
}

template <typename T>
bool tny8In(Tny* root, const char* name, T& v)
{
  Tny* obj = Tny_get(root, name);
  if (obj != NULL)
  {
    if (obj->type == TNY_CHAR)
    {
      v = *reinterpret_cast<T*>(&obj->value.chr);
      return true;
    }
    else
    {
      std::cerr << "cpm-es-cereal: Mismatched Tny types for " << name << "!" << std::endl;
      std::cerr << "Expected TNY_CHAR (" << TNY_CHAR << ") got (" << obj->type << ")" << std::endl;
      return false;
    }
  }
  else
  {
#ifdef CPM_ES_CEREAL_VERBOSE_OUTPUT
    // This is unlikely an error when we use delta compression.
    std::cerr << "cpm-es-cereal: Unable to find " << name << " in Tny dictionary." << std::endl;
#endif
    return false;
  }
}

template <typename T>
bool tny32In(Tny* root, const char* name, T& v)
{
  Tny* obj = Tny_get(root, name);
  if (obj != NULL)
  {
    if (obj->type == TNY_INT32)
    {
      v = *reinterpret_cast<T*>(&obj->value.num);
      return true;
    }
    else
    {
      std::cerr << "cpm-es-cereal: Mismatched Tny types for " << name << "!" << std::endl;
      std::cerr << "Expected TNY_INT32 (" << TNY_INT32 << ") got (" << obj->type << ")" << std::endl;
      return false;
    }
  }
  else
  {
#ifdef CPM_ES_CEREAL_VERBOSE_OUTPUT
    std::cerr << "cpm-es-cereal: Unable to find " << name << " in Tny dictionary." << std::endl;
#endif
    return false;
  }
}

template <typename T>
bool tny64In(Tny* root, const char* name, T& v)
{
  Tny* obj = Tny_get(root, name);
  if (obj != NULL)
  {
    if (obj->type == TNY_INT64)
    {
      v = *reinterpret_cast<T*>(&obj->value.num);
      return true;
    }
    else
    {
      std::cerr << "cpm-es-cereal: Mismatched Tny types for " << name << "!" << std::endl;
      std::cerr << "Expected TNY_INT64 (" << TNY_INT64 << ") got (" << obj->type << ")" << std::endl;
      return false;
    }
  }
  else
  {
#ifdef CPM_ES_CEREAL_VERBOSE_OUTPUT
    std::cerr << "cpm-es-cereal: Unable to find " << name << " in Tny dictionary." << std::endl;
#endif
    return false;
  }
}

bool inBool(Tny* root, const char* name, bool& ch)
{
  Tny* obj = Tny_get(root, name);
  if (obj != NULL)
  {
    if (obj->type == TNY_CHAR)
    {
      ch = (obj->value.chr != 0);
      return true;
    }
    else
    {
      std::cerr << "cpm-es-cereal: Mismatched Tny types for " << name << "!" << std::endl;
      std::cerr << "Expected TNY_CHAR (" << TNY_CHAR << ") got (" << obj->type << ")" << std::endl;
      return false;
    }
  }
  else
  {
#ifdef CPM_ES_CEREAL_VERBOSE_OUTPUT
    std::cerr << "cpm-es-cereal: Unable to find " << name << " in Tny dictionary." << std::endl;
#endif
    return false;
  }
}

Tny* outBool(Tny* root, const char* name, const bool& v)      {return tnyGenericOut(root, name, v, TNY_CHAR);}

bool inInt8(Tny* root, const char* name, int8_t& c)           {return tny8In(root, name, c);}
Tny* outInt8(Tny* root, const char* name, const int8_t& c)    {return tnyGenericOut(root, name, c, TNY_CHAR);}

bool inUInt8(Tny* root, const char* name, uint8_t& c)         {return tny8In(root, name, c);}
Tny* outUInt8(Tny* root, const char* name, const uint8_t& c)  {return tnyGenericOut(root, name, c, TNY_CHAR);}

bool inInt32(Tny* root, const char* name, int32_t& v)         {return tny32In(root, name, v);}
Tny* outInt32(Tny* root, const char* name, const int32_t& v)  {return tnyGenericOut(root, name, v, TNY_INT32);}

bool inUInt32(Tny* root, const char* name, uint32_t& v)        {return tny32In(root, name, v);}
Tny* outUInt32(Tny* root, const char* name, const uint32_t& v) {return tnyGenericOut(root, name, v, TNY_INT32);}

bool inInt64(Tny* root, const char* name, int64_t& v)         {return tny64In(root, name, v);}
Tny* outInt64(Tny* root, const char* name, const int64_t& v)  {return tnyGenericOut(root, name, v, TNY_INT64);}

bool inUInt64(Tny* root, const char* name, uint64_t& v)        {return tny64In(root, name, v);}
Tny* outUInt64(Tny* root, const char* name, const uint64_t& v) {return tnyGenericOut(root, name, v, TNY_INT64);}

bool inFloat(Tny* root, const char* name, float& v)           {return tny32In(root, name, v);}
Tny* outFloat(Tny* root, const char* name, const float& v)    {return tnyGenericOut(root, name, v, TNY_INT32);}

bool inDouble(Tny* root, const char* name, double& v)         {return tny64In(root, name, v);}
Tny* outDouble(Tny* root, const char* name, const double& v)  {return tnyGenericOut(root, name, v, TNY_INT64);}

bool inBinary(Tny* root, const char* name, void* data, size_t size)
{
  Tny* obj = Tny_get(root, name);
  if (obj != NULL)
  {
    if (obj->type == TNY_BIN)
    {
      if (obj->size <= size)
      {
        std::memcpy(data, obj->value.ptr, obj->size);
        return true;
      }
      else
      {
        std::cerr << "cpm-es-cereal: Memory for binary block too small for: " << name << std::endl;
        std::cerr << "Size of incoming binary block: " << obj->size << " size of memory: " << size << std::endl;
        return false;
      }
    }
    else
    {
      std::cerr << "cpm-es-cereal: Mismatched Tny types for " << name << "!" << std::endl;
      std::cerr << "Expected TNY_BIN (" << TNY_BIN << ") got (" << obj->type << ")" << std::endl;
      return false;
    }
  }
  else
  {
#ifdef CPM_ES_CEREAL_VERBOSE_OUTPUT
    std::cerr << "cpm-es-cereal: Unable to find " << name << " in Tny dictionary." << std::endl;
#endif
    return false;
  }
}

Tny* outBinary(Tny* root, const char* name, const void* data, size_t size)
{
  void* ptr = const_cast<void*>(data);
  return Tny_add(root, TNY_BIN, const_cast<char*>(name), ptr, size);
}

bool inString(Tny* root, const char* name, char* str, size_t maxSize)
{
  return inBinary(root, name, static_cast<void*>(str), maxSize);
}

bool inStringStd(Tny* root, const char* name, std::string& str)
{
  char* data = nullptr;
  bool res = inBinaryMalloc(root, name, reinterpret_cast<void**>(&data));
  str = data;
  std::free(data);
  return res;
}

Tny* outString(Tny* root, const char* name, const char* str)
{
  int length = std::strlen(str);
  return outBinary(root, name, static_cast<const void*>(str), length + 1);   // include null
}

bool inBinaryMalloc(Tny* root, const char* name, void** data)
{
  Tny* obj = Tny_get(root, name);
  if (obj != NULL)
  {
    if (obj->type == TNY_BIN)
    {
      *data = std::malloc(obj->size);
      if (*data != NULL)
      {
        std::memcpy(*data, obj->value.ptr, obj->size);
        return true;
      }
      else
      {
        std::cerr << "cpm-es-cereal: Failed to allocate memory for " << name << " of size " << obj->size << std::endl;
        return false;
      }
    }
    else
    {
      std::cerr << "cpm-es-cereal: Mismatched Tny types for " << name << "!" << std::endl;
      std::cerr << "Expected TNY_BIN (" << TNY_BIN << ") got (" << obj->type << ")" << std::endl;
      return false;
    }
  }
  else
  {
#ifdef CPM_ES_CEREAL_VERBOSE_OUTPUT
    std::cerr << "cpm-es-cereal: Unable to find " << name << " in Tny dictionary." << std::endl;
#endif
    return false;
  }
}

Tny* outBinaryMalloc(Tny* root, const char* name, const void* data, size_t size)
{
  return outBinary(root, name, data, size);
}



//------------------------------------------------------------------------------
// TNY_ARRAY implementation
//------------------------------------------------------------------------------

template <typename T>
Tny* tnyGenericOutArray(Tny* root, const T& v, TnyType type)
{
  T* ptr = const_cast<T*>(&v);
  return Tny_add(root, type, NULL, static_cast<void*>(ptr), 0);
}

template <typename T>
Tny* tny8InArray(Tny* root, T& v)
{
  if (root->type == TNY_CHAR)
    v = *reinterpret_cast<T*>(&root->value.chr);
  else
    std::cerr << "Expected TNY_CHAR (" << TNY_CHAR << ") got (" << root->type << ")" << std::endl;

  if (Tny_hasNext(root))
    return Tny_next(root);
  else
    return root;
}

template <typename T>
Tny* tny32InArray(Tny* root, T& v)
{
  if (root->type == TNY_INT32)
    v = *reinterpret_cast<T*>(&root->value.num);
  else
    std::cerr << "Expected TNY_INT32 (" << TNY_INT32 << ") got (" << root->type << ")" << std::endl;

  if (Tny_hasNext(root))
    return Tny_next(root);
  else
    return root;
}

template <typename T>
Tny* tny64InArray(Tny* root, T& v)
{
  if (root->type == TNY_INT64)
    v = *reinterpret_cast<T*>(&root->value.num);
  else
    std::cerr << "Expected TNY_INT64 (" << TNY_INT64 << ") got (" << root->type << ")" << std::endl;

  if (Tny_hasNext(root))
    return Tny_next(root);
  else
    return root;
}

Tny* inBoolArray(Tny* root, bool& ch)
{
  if (root->type == TNY_CHAR)
    ch = (root->value.chr != 0);
  else
    std::cerr << "Expected TNY_CHAR (" << TNY_CHAR << ") got (" << root->type << ")" << std::endl;

  if (Tny_hasNext(root))
    return Tny_next(root);
  else
    return root;
}

Tny* outBoolArray(Tny* root, const bool& v)       {return tnyGenericOutArray(root, v, TNY_CHAR);}

Tny* inInt8Array(Tny* root, int8_t& c)            {return tny8InArray(root, c);}
Tny* outInt8Array(Tny* root, const int8_t& c)     {return tnyGenericOutArray(root, c, TNY_CHAR);}

Tny* inUInt8Array(Tny* root, uint8_t& c)          {return tny8InArray(root, c);}
Tny* outUInt8Array(Tny* root, const uint8_t& c)   {return tnyGenericOutArray(root, c, TNY_CHAR);}

Tny* inInt32Array(Tny* root, int32_t& v)          {return tny32InArray(root, v);}
Tny* outInt32Array(Tny* root, const int32_t& v)   {return tnyGenericOutArray(root, v, TNY_INT32);}

Tny* inUInt32Array(Tny* root, uint32_t& v)        {return tny32InArray(root, v);}
Tny* outUInt32Array(Tny* root, const uint32_t& v) {return tnyGenericOutArray(root, v, TNY_INT32);}

Tny* inInt64Array(Tny* root, int64_t& v)          {return tny64InArray(root, v);}
Tny* outInt64Array(Tny* root, const int64_t& v)   {return tnyGenericOutArray(root, v, TNY_INT64);}

Tny* inUInt64Array(Tny* root, uint64_t& v)        {return tny64InArray(root, v);}
Tny* outUInt64Array(Tny* root, const uint64_t& v) {return tnyGenericOutArray(root, v, TNY_INT64);}

Tny* inFloatArray(Tny* root, float& v)            {return tny32InArray(root, v);}
Tny* outFloatArray(Tny* root, const float& v)     {return tnyGenericOutArray(root, v, TNY_INT32);}

Tny* inDoubleArray(Tny* root, double& v)          {return tny64InArray(root, v);}
Tny* outDoubleArray(Tny* root, const double& v)   {return tnyGenericOutArray(root, v, TNY_INT64);}

Tny* inBinaryArray(Tny* root, void* data, size_t size)
{
  if (root->type == TNY_BIN)
  {
    if (root->size <= size)
    {
      std::memcpy(data, root->value.ptr, root->size);
    }
    else
    {
      std::cerr << "cpm-es-cereal: Memory for binary block too small. " << std::endl;
      std::cerr << "Size of incoming binary block: " << root->size << " size of memory: " << size << std::endl;
    }
  }
  else
  {
    std::cerr << "cpm-es-cereal: Expected TNY_BIN (" << TNY_BIN << ") got (" << root->type << ")" << std::endl;
  }

  if (Tny_hasNext(root))
    return Tny_next(root);
  else
    return root;
}

Tny* outBinaryArray(Tny* root, const void* data, size_t size)
{
  void* ptr = const_cast<void*>(data);
  return Tny_add(root, TNY_BIN, NULL, ptr, size);
}

Tny* inStringArray(Tny* root, char* str, size_t maxSize)
{
  return inBinaryArray(root, static_cast<void*>(str), maxSize);
}

Tny* outStringArray(Tny* root, const char* str)
{
  int length = std::strlen(str);
  return outBinaryArray(root, static_cast<const void*>(str), length + 1);   // include null
}

Tny* inBinaryMallocArray(Tny* root, void** data)
{
  if (root->type == TNY_BIN)
  {
    *data = std::malloc(root->size);
    if (*data != NULL)
      std::memcpy(*data, root->value.ptr, root->size);
    else
      std::cerr << "cpm-es-cereal: Failed to allocate memory for size " << root->size << std::endl;
  }
  else
  {
    std::cerr << "cpm-es-cereal: Mismatched Tny types!" << std::endl;
    std::cerr << "Expected TNY_BIN (" << TNY_BIN << ") got (" << root->type << ")" << std::endl;
  }

  if (Tny_hasNext(root))
    return Tny_next(root);
  else
    return root;
}

Tny* outBinaryMallocArray(Tny* root, const void* data, size_t size)
{
  return outBinaryArray(root, data, size);
}




} // namespace CST_detail
} // namespace CPM_ES_CEREAL_NS

