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


///
///@file  TypeName.h
///@brief specializations of template<class T>
///       find_type_name() function for build-in
///       and simple types not deriving from Core::Datatype
///
///@author
///       Alexei Samsonov
///       Department of Computer Science
///       University of Utah
///@date  December 2000
///
///

#include <string>

#include <Core/Datatypes/Legacy/Base/TypeName.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Point.h>

namespace SCIRun{

using namespace SCIRun::Core::Geometry;

//////////
// Template function specializations for built-in types

//////////
// Floating-point types
template<> std::string find_type_name(float*)
{
  static const std::string name = "float";
  return name;
}

template<> std::string find_type_name(double*)
{
  static const std::string name = "double";
  return name;
}

template<> std::string find_type_name(std::complex<double>*)
{
  static const std::string name = "complex";
  return name;
}

template<> std::string find_type_name(long double*)
{
  static const std::string name = "long_double";
  return name;
}

//////////
// Integer types
template<> std::string find_type_name(short*)
{
  static const std::string name = "short";
  return name;
}

template<> std::string find_type_name(unsigned short*)
{
  static const std::string name = "unsigned_short";
  return name;
}

template<> std::string find_type_name(int*)
{
  static const std::string name = "int";
  return name;
}

template<> std::string find_type_name(unsigned int*)
{
  static const std::string name = "unsigned_int";
  return name;
}

template<> std::string find_type_name(long*)
{
  static const std::string name = "long";
  return name;
}

template<> std::string find_type_name(unsigned long*)
{
  static const std::string name = "unsigned_long";
  return name;
}

template<> std::string find_type_name(long long*)
{
  static const std::string name = "long_long";
  return name;
}

template<> std::string find_type_name(unsigned long long*)
{
  static const std::string name = "unsigned_long_long";
  return name;
}

//////////
// Character types
template<> std::string find_type_name(char*)
{
  static const std::string name = "char";
  return name;
}

template<> std::string find_type_name(unsigned char*)
{
  static const std::string name = "unsigned_char";
  return name;
}

//////////
// Boolean type
template<> std::string find_type_name(bool*)
{
  static const std::string name = "bool";
  return name;
}

//////////
// Template function specializations for some primitives
template<> std::string find_type_name(Vector*)
{
  static const std::string name = "Vector";
  return name;
}

template<> std::string find_type_name(Point*)
{
  static const std::string name = "Point";
  return name;
}

template<> std::string find_type_name(Transform*)
{
  static const std::string name = "Transform";
  return name;
}

template<> std::string find_type_name(std::string*)
{
  static const std::string name = "string";
  return name;
}


#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
template<> std::string find_type_name(LockingHandle< Matrix<double> > *)
{
  static const std::string name = TypeNameGenerator::make_template_id("LockingHandle", "Matrix");
  return name;
}

template<> std::string find_type_name(LockingHandle<NrrdData> *)
{
  static const std::string name = TypeNameGenerator::make_template_id("LockingHandle", "NrrdData");
  return name;
}

template<> std::string find_type_name(LockingHandle<Field> *)
{
  static const std::string name = TypeNameGenerator::make_template_id("LockingHandle", "Field");
  return name;
}

template<> std::string find_type_name(LockingHandle<String> *)
{
  static const std::string name = TypeNameGenerator::make_template_id("LockingHandle", "String");
  return name;
}
#endif

} // namespace SCIRun
