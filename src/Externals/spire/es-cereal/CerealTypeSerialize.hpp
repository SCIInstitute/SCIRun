#ifndef IAUNS_CEREALTYPESERIALIZE_HPP
#define IAUNS_CEREALTYPESERIALIZE_HPP

#include <cstdint>
#include <cstddef>
#include <cstring>

struct _Tny;
typedef _Tny Tny;

namespace CPM_ES_CEREAL_NS {

// Cereal serialize type detail
namespace CST_detail
{
  // Basic types stored in a dictionary (TNY_DICT).
  bool inBool(Tny* root, const char* name, bool& b);
  bool inInt8(Tny* root, const char* name, int8_t& c);
  bool inUInt8(Tny* root, const char* name, uint8_t& c);
  bool inInt32(Tny* root, const char* name, int32_t& v);
  bool inUInt32(Tny* root, const char* name, uint32_t& v);
  bool inInt64(Tny* root, const char* name, int64_t& v);
  bool inUInt64(Tny* root, const char* name, uint64_t& v);
  bool inFloat(Tny* root, const char* name, float& v);
  bool inDouble(Tny* root, const char* name, double& v);
  bool inString(Tny* root, const char* name, char* str, size_t maxSize);
  bool inStringStd(Tny* root, const char* name, std::string& str);
  bool inBinary(Tny* root, const char* name, void* data, size_t size);
  bool inBinaryMalloc(Tny* root, const char* name, void** data);

  Tny* outBool(Tny* root, const char* name, const bool& b);
  Tny* outInt8(Tny* root, const char* name, const int8_t& c);
  Tny* outUInt8(Tny* root, const char* name, const uint8_t& c);
  Tny* outInt32(Tny* root, const char* name, const int32_t& v);
  Tny* outUInt32(Tny* root, const char* name, const uint32_t& v);
  Tny* outInt64(Tny* root, const char* name, const int64_t& v);
  Tny* outUInt64(Tny* root, const char* name, const uint64_t& v);
  Tny* outFloat(Tny* root, const char* name, const float& v);
  Tny* outDouble(Tny* root, const char* name, const double& v);
  Tny* outString(Tny* root, const char* name, const char* str);
  Tny* outBinary(Tny* root, const char* name, const void* data, size_t size);
  Tny* outBinaryMalloc(Tny* root, const char* name, const void* data, size_t size);

  // Basic types stored in an array (TNY_ARRAY).
  Tny* inBoolArray(Tny* root, bool& b);
  Tny* inInt8Array(Tny* root, int8_t& c);
  Tny* inUInt8Array(Tny* root, uint8_t& c);
  Tny* inInt32Array(Tny* root, int32_t& v);
  Tny* inUInt32Array(Tny* root, uint32_t& v);
  Tny* inInt64Array(Tny* root, int64_t& v);
  Tny* inUInt64Array(Tny* root, uint64_t& v);
  Tny* inFloatArray(Tny* root, float& v);
  Tny* inDoubleArray(Tny* root, double& v);
  Tny* inStringArray(Tny* root, char* str, size_t maxSize);
  Tny* inBinaryArray(Tny* root, void* data, size_t size);
  Tny* inBinaryMallocArray(Tny* root, void** data);

  Tny* outBoolArray(Tny* root, const bool& b);
  Tny* outInt8Array(Tny* root, const int8_t& c);
  Tny* outUInt8Array(Tny* root, const uint8_t& c);
  Tny* outInt32Array(Tny* root, const int32_t& v);
  Tny* outUInt32Array(Tny* root, const uint32_t& v);
  Tny* outInt64Array(Tny* root, const int64_t& v);
  Tny* outUInt64Array(Tny* root, const uint64_t& v);
  Tny* outFloatArray(Tny* root, const float& v);
  Tny* outDoubleArray(Tny* root, const double& v);
  Tny* outStringArray(Tny* root, const char* str);
  Tny* outBinaryArray(Tny* root, const void* data, size_t size);
  Tny* outBinaryMallocArray(Tny* root, const void* data, size_t size);
}

template <typename T>
class CerealSerializeType
{
public:
  typedef int Type;

  static_assert(true, "cpm-es-cereal: CerealSerializeType type specialization not defined.");
  static bool in(Tny* root, const char* name, Type& v);
  static Tny* out(Tny* root, const char* name, const Type& v);
  static const char* getTypeName();
};

template<>
class CerealSerializeType<bool>
{
public:
  typedef bool Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inBool(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outBool(root, name, v);}
  static const char* getTypeName()    {return "bool";}
};

template<>
class CerealSerializeType<int8_t>
{
public:
  typedef int8_t Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inInt8(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outInt8(root, name, v);}
  static const char* getTypeName()    {return "int8";}
};

template<>
class CerealSerializeType<uint8_t>
{
public:
  typedef uint8_t Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inUInt8(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outUInt8(root, name, v);}
  static const char* getTypeName()    {return "uint8";}
};

template<>
class CerealSerializeType<int32_t>
{
public:
  typedef int32_t Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inInt32(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outInt32(root, name, v);}
  static const char* getTypeName()    {return "int32";}
};

template<>
class CerealSerializeType<uint32_t>
{
public:
  typedef uint32_t Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inUInt32(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outUInt32(root, name, v);}
  static const char* getTypeName()    {return "uint32";}
};

template<>
class CerealSerializeType<int64_t>
{
public:
  typedef int64_t Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inInt64(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outInt64(root, name, v);}
  static const char* getTypeName()    {return "int64";}
};

template<>
class CerealSerializeType<uint64_t>
{
public:
  typedef uint64_t Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inUInt64(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outUInt64(root, name, v);}
  static const char* getTypeName()    {return "uint64";}
};

template<>
class CerealSerializeType<float>
{
public:
  typedef float Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inFloat(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outFloat(root, name, v);}
  static const char* getTypeName()    {return "float";}
};

template<>
class CerealSerializeType<double>
{
public:
  typedef double Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inDouble(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outDouble(root, name, v);}
  static const char* getTypeName()    {return "double";}
};

template<>
class CerealSerializeType<std::string>
{
public:
  typedef std::string Type;

  static bool in(Tny* root, const char* name, Type& v)        {return CST_detail::inStringStd(root, name, v);}
  static Tny* out(Tny* root, const char* name, const Type& v) {return CST_detail::outString(root, name, v.c_str());}
  static const char* getTypeName()    {return "string";}
};

} // namespace CPM_ES_CEREAL_NS

#endif 
