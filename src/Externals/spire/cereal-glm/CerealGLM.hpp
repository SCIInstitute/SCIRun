#ifndef SPIRE_CEREAL_GLM_CEREALTYPESERIALIZE_HPP
#define SPIRE_CEREAL_GLM_CEREALTYPESERIALIZE_HPP

#include <cstdint>
#include <cstddef>
#include <es-cereal/CerealTypeSerialize.hpp>
#include <glm/glm.hpp>
#include <tny/tny.hpp>

namespace spire {

template<>
class CerealSerializeType<glm::vec3>
{
public:
  typedef glm::vec3 Type;

  static bool in(Tny* root, const char* name, glm::vec3& v)
  {
    Tny* obj = Tny_get(root, name);
    if (obj)
    {
      if (obj->type == TNY_OBJ)
      {
        obj = obj->value.tny;
        obj = Tny_next(obj);
        obj = CST_detail::inFloatArray(obj, v.x);
        obj = CST_detail::inFloatArray(obj, v.y);
        obj = CST_detail::inFloatArray(obj, v.z);
        return true;
      }
      else
      {
        std::cerr << "cereal-glm: Wrong object type." << std::endl;
        return false;
      }
    }
    else
    {
      std::cerr << "cereal-glm: Unable to find " << name << " in dictionary." << std::endl;
      return false;
    }
  }
  static Tny* out(Tny* root, const char* name, const glm::vec3& v)
  {
    Tny* obj = Tny_add(NULL, TNY_ARRAY, NULL, NULL, 0);

    obj = CST_detail::outFloatArray(obj, v.x);
    obj = CST_detail::outFloatArray(obj, v.y);
    obj = CST_detail::outFloatArray(obj, v.z);

    root = Tny_add(root, TNY_OBJ, const_cast<char*>(name), obj, 0);

    Tny_free(obj);

    return root;
  }
  static const char* getTypeName()    {return "glm::vec3";}
};

template<>
class CerealSerializeType<glm::vec4>
{
public:
  typedef glm::vec4 Type;

  static bool in(Tny* root, const char* name, glm::vec4& v)
  {
    Tny* obj = Tny_get(root, name);
    if (obj)
    {
      if (obj->type == TNY_OBJ)
      {
        obj = obj->value.tny;
        obj = Tny_next(obj);
        obj = CST_detail::inFloatArray(obj, v.x);
        obj = CST_detail::inFloatArray(obj, v.y);
        obj = CST_detail::inFloatArray(obj, v.z);
        obj = CST_detail::inFloatArray(obj, v.w);
        return true;
      }
      else
      {
        std::cerr << "cereal-glm: Wrong object type." << std::endl;
        return false;
      }
    }
    else
    {
      std::cerr << "cereal-glm: Unable to find " << name << " in dictionary." << std::endl;
      return false;
    }
  }
  static Tny* out(Tny* root, const char* name, const glm::vec4& v)
  {
    Tny* obj = Tny_add(NULL, TNY_ARRAY, NULL, NULL, 0);

    obj = CST_detail::outFloatArray(obj, v.x);
    obj = CST_detail::outFloatArray(obj, v.y);
    obj = CST_detail::outFloatArray(obj, v.z);
    obj = CST_detail::outFloatArray(obj, v.w);

    root = Tny_add(root, TNY_OBJ, const_cast<char*>(name), obj, 0);

    Tny_free(obj);

    return root;
  }
  static const char* getTypeName()    {return "glm::vec4";}
};

template<>
class CerealSerializeType<glm::mat4>
{
public:
  typedef glm::mat4 Type;

  static bool in(Tny* root, const char* name, glm::mat4& m)
  {
    Tny* obj = Tny_get(root, name);
    if (obj)
    {
      if (obj->type == TNY_OBJ)
      {
        obj = obj->value.tny;
        obj = Tny_next(obj);
        // COLUMN / ROW -- instead of the normal mathematical row/column
        obj = CST_detail::inFloatArray(obj, m[0][0]); obj = CST_detail::inFloatArray(obj, m[1][0]); obj = CST_detail::inFloatArray(obj, m[2][0]); obj = CST_detail::inFloatArray(obj, m[3][0]);
        obj = CST_detail::inFloatArray(obj, m[0][1]); obj = CST_detail::inFloatArray(obj, m[1][1]); obj = CST_detail::inFloatArray(obj, m[2][1]); obj = CST_detail::inFloatArray(obj, m[3][1]);
        obj = CST_detail::inFloatArray(obj, m[0][2]); obj = CST_detail::inFloatArray(obj, m[1][2]); obj = CST_detail::inFloatArray(obj, m[2][2]); obj = CST_detail::inFloatArray(obj, m[3][2]);
        obj = CST_detail::inFloatArray(obj, m[0][3]); obj = CST_detail::inFloatArray(obj, m[1][3]); obj = CST_detail::inFloatArray(obj, m[2][3]); obj = CST_detail::inFloatArray(obj, m[3][3]);
        return true;
      }
      else
      {
        std::cerr << "cereal-glm: Wrong object type." << std::endl;
        return false;
      }
    }
    else
    {
      std::cerr << "cereal-glm: Unable to find " << name << " in dictionary." << std::endl;
      return false;
    }
  }
  static Tny* out(Tny* root, const char* name, const glm::mat4& m)
  {
    Tny* obj = Tny_add(NULL, TNY_ARRAY, NULL, NULL, 0);

    obj = CST_detail::outFloatArray(obj, m[0][0]); obj = CST_detail::outFloatArray(obj, m[1][0]); obj = CST_detail::outFloatArray(obj, m[2][0]); obj = CST_detail::outFloatArray(obj, m[3][0]);
    obj = CST_detail::outFloatArray(obj, m[0][1]); obj = CST_detail::outFloatArray(obj, m[1][1]); obj = CST_detail::outFloatArray(obj, m[2][1]); obj = CST_detail::outFloatArray(obj, m[3][1]);
    obj = CST_detail::outFloatArray(obj, m[0][2]); obj = CST_detail::outFloatArray(obj, m[1][2]); obj = CST_detail::outFloatArray(obj, m[2][2]); obj = CST_detail::outFloatArray(obj, m[3][2]);
    obj = CST_detail::outFloatArray(obj, m[0][3]); obj = CST_detail::outFloatArray(obj, m[1][3]); obj = CST_detail::outFloatArray(obj, m[2][3]); obj = CST_detail::outFloatArray(obj, m[3][3]);

    root = Tny_add(root, TNY_OBJ, const_cast<char*>(name), obj, 0);

    Tny_free(obj);

    return root;
  }
  static const char* getTypeName()    {return "glm::mat4";}
};


template<>
class CerealSerializeType<glm::mat3>
{
public:
  typedef glm::mat3 Type;

  static bool in(Tny* root, const char* name, glm::mat3& m)
  {
    Tny* obj = Tny_get(root, name);
    if (obj)
    {
      if (obj->type == TNY_OBJ)
      {
        obj = obj->value.tny;
        obj = Tny_next(obj);
        // COLUMN / ROW -- instead of the normal mathematical row/column
        obj = CST_detail::inFloatArray(obj, m[0][0]); obj = CST_detail::inFloatArray(obj, m[1][0]); obj = CST_detail::inFloatArray(obj, m[2][0]);
        obj = CST_detail::inFloatArray(obj, m[0][1]); obj = CST_detail::inFloatArray(obj, m[1][1]); obj = CST_detail::inFloatArray(obj, m[2][1]);
        obj = CST_detail::inFloatArray(obj, m[0][2]); obj = CST_detail::inFloatArray(obj, m[1][2]); obj = CST_detail::inFloatArray(obj, m[2][2]);
        return true;
      }
      else
      {
        std::cerr << "cereal-glm: Wrong object type." << std::endl;
        return false;
      }
    }
    else
    {
      std::cerr << "cereal-glm: Unable to find " << name << " in dictionary." << std::endl;
      return false;
    }
  }
  static Tny* out(Tny* root, const char* name, const glm::mat3& m)
  {
    Tny* obj = Tny_add(NULL, TNY_ARRAY, NULL, NULL, 0);

    obj = CST_detail::outFloatArray(obj, m[0][0]); obj = CST_detail::outFloatArray(obj, m[1][0]); obj = CST_detail::outFloatArray(obj, m[2][0]);
    obj = CST_detail::outFloatArray(obj, m[0][1]); obj = CST_detail::outFloatArray(obj, m[1][1]); obj = CST_detail::outFloatArray(obj, m[2][1]);
    obj = CST_detail::outFloatArray(obj, m[0][2]); obj = CST_detail::outFloatArray(obj, m[1][2]); obj = CST_detail::outFloatArray(obj, m[2][2]);

    root = Tny_add(root, TNY_OBJ, const_cast<char*>(name), obj, 0);

    Tny_free(obj);

    return root;
  }
  static const char* getTypeName()    {return "glm::mat3";}
};

} // namespace spire

#endif
