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


/*
 *@file  TypeName.h
 *@brief template to return name of argument type;
 *       used in PIO of templatized types
 *
 *@author
 *   Alexei Samsonov
 *   Department of Computer Science
 *   University of Utah
 *@date  December 2000
 *
 */

#ifndef TYPENAME_H
#define TYPENAME_H

#include <string>
#include <vector>
#include <map>
#include <complex>
#include <sstream>

#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Core/Datatypes/Legacy/Base/share.h>


namespace SCIRun {

class TypeNameGenerator
{
/// @todo: callers of these can be compressed using macros.  worth it?
public:
  static std::string make_template_id(const std::string& templateName, const std::string& templateParam)
  {
    std::ostringstream o;
    o << templateName << leftAngleBracket << templateParam << rightAngleBracket;
    return o.str();
  }

  static std::string make_template_id(const std::string& templateName,
    const std::string& templateParam1,
    const std::string& templateParam2)
  {
    std::ostringstream o;
    o << templateName << leftAngleBracket
      << templateParam1 << comma
      << templateParam2 << rightAngleBracket;
    return o.str();
  }

  static std::string make_template_id(const std::string& templateName,
    const std::string& templateParam1,
    const std::string& templateParam2,
    const std::string& templateParam3)
  {
    std::ostringstream o;
    o << templateName << leftAngleBracket
      << templateParam1 << comma
      << templateParam2 << comma
      << templateParam3 << rightAngleBracket;
    return o.str();
  }

private:
  static const char leftAngleBracket = '<';
  static const char comma = ',';
  static const char rightAngleBracket = '>';
};


//////////
// Function to return name of type of its argument
template <class T> std::string find_type_name(T*)
{
  return T::type_name(-1);
}


template<class T, class S> std::string find_type_name( std::pair<T,S> *);

class IntVector;
class NrrdData;

using complex = std::complex<double>;

template<> SCISHARE std::string find_type_name(float*);
template<> SCISHARE std::string find_type_name(double*);
template<> SCISHARE std::string find_type_name(complex*);
template<> SCISHARE std::string find_type_name(long double*);
template<> SCISHARE std::string find_type_name(short*);
template<> SCISHARE std::string find_type_name(unsigned short*);
template<> SCISHARE std::string find_type_name(int*);
template<> SCISHARE std::string find_type_name(unsigned int*);
template<> SCISHARE std::string find_type_name(long*);
template<> SCISHARE std::string find_type_name(unsigned long*);
template<> SCISHARE std::string find_type_name(long long*);
template<> SCISHARE std::string find_type_name(unsigned long long*);
template<> SCISHARE std::string find_type_name(char*);
template<> SCISHARE std::string find_type_name(unsigned char*);
template<> SCISHARE std::string find_type_name(bool*);
template<> SCISHARE std::string find_type_name(SCIRun::Core::Geometry::Vector*);
template<> SCISHARE std::string find_type_name(IntVector*);
template<> SCISHARE std::string find_type_name(SCIRun::Core::Geometry::Point*);
template<> SCISHARE std::string find_type_name(SCIRun::Core::Geometry::Transform*);
template<> SCISHARE std::string find_type_name(std::string*);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
template<> SCISHARE std::string find_type_name(LockingHandle< Matrix<double> > *);
template<> SCISHARE std::string find_type_name(LockingHandle<NrrdData> *);
template<> SCISHARE std::string find_type_name(LockingHandle<Field> *);
template<> SCISHARE std::string find_type_name(LockingHandle<String> *);
#endif

//////////
// Function overloading for templates
template<class T> class Array1;
template<class T> class Array2;

template <class T>
std::string find_type_name(Array1<T>*)
{
  static const std::string name = TypeNameGenerator::make_template_id("Array1", find_type_name(static_cast<T*>(nullptr)));
  return name;
}

template <class T> std::string find_type_name(Array2<T>*)
{
  static const std::string name = TypeNameGenerator::make_template_id("Array2", find_type_name(static_cast<T*>(nullptr)));
  return name;
}

template <class T> std::string find_type_name(std::vector<T>*)
{
  static const std::string name = TypeNameGenerator::make_template_id("vector", find_type_name(static_cast<T*>(nullptr)));
  return name;
}

template<class T, class S> std::string find_type_name( std::pair<T,S> *)
{
  static const std::string name = TypeNameGenerator::make_template_id("pair", find_type_name(static_cast<T*>(nullptr)), find_type_name(static_cast<S*>(nullptr)));
  return name;
}

} // namespace SCIRun

#endif
