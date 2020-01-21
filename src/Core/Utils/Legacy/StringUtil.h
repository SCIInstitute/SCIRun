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


/// @todo Documentation Core/Utils/Legacy/StringUtil.h

#ifndef CORE_UTIL_STRINGUTIL_H
#define CORE_UTIL_STRINGUTIL_H 1

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

#include <Core/Utils/Legacy/share.h>

namespace SCIRun {

SCISHARE bool from_string(const std::string &str, double &value);
SCISHARE bool from_string(const std::string &str, float &value);
SCISHARE bool from_string(const std::string &str, int &value);
SCISHARE bool from_string(const std::string &str, unsigned int &value);
SCISHARE bool from_string(const std::string &str, long &value);
SCISHARE bool from_string(const std::string &str, unsigned long &value);
SCISHARE bool from_string(const std::string &str, long long &value);
SCISHARE bool from_string(const std::string &str, unsigned long long &value);

template <class T>
bool multiple_from_string(const std::string &str, std::vector<T> &values)
{
  values.clear();

  std::string data = str;
  for (size_t j=0; j<data.size(); j++)
    if ((data[j] == '\t')||(data[j] == '\r')||(data[j] == '\n')||(data[j]=='"')) data[j] = ' ';

  std::vector<std::string> nums;
  for (size_t p=0;p<data.size();)
  {
    while((data[p] == ' ')&&(p<data.size())) p++;
    if (p >= data.size()) break;

    std::string::size_type next_space = data.find(' ',p);
    if (next_space == std::string::npos) next_space = data.size();
    T value;
    if (from_string(data.substr(p,next_space-p), value)) values.push_back(value);
    p = next_space;

    if (p >= data.size()) break;
  }
  if (values.size() > 0) return (true);
  return (false);
}

template <class T>
bool from_string(const std::string &str, T &value)
{
  std::istringstream iss(str+"  ");
  iss.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);
  try
  {
    iss >> value;
    return (true);
  }
  catch (...)
  {
    return (false);
  }
}


inline bool string_to_bool(const std::string &str, bool &result)
{ return(from_string(str,result)); }
inline bool string_to_int(const std::string &str, int &result)
{ return(from_string(str,result)); }
inline bool string_to_double(const std::string &str, double &result)
{ return(from_string(str,result)); }
inline bool string_to_unsigned_long(const std::string &str, unsigned long &result)
{ return(from_string(str,result)); }


template <class T>
std::string to_string(T val)
{
  std::ostringstream oss;
  oss << val;
  return (oss.str());
}

template <class T>
std::string to_string(T val,int precision)
{
  std::ostringstream oss;
  oss.precision(precision);
  oss << val;
  return (oss.str());
}

SCISHARE std::string string_toupper(std::string);
SCISHARE std::string string_tolower(std::string);

//////////
/// Remove directory name
inline std::string basename(const std::string &path)
{ return (path.substr(path.rfind('/')+1)); }

//////////
/// Return directory name
inline std::string pathname(const std::string &path)
{ return (path.substr(0, path.rfind('/')+1)); }

// Split a string into multiple parts, separated by the character sep
SCISHARE std::vector<std::string> split_string(const std::string& str, char sep);

/////////
/// C++ify a string, turn newlines into \n, use \t, \r, \\ \", etc.
SCISHARE std::string string_Cify(const std::string &str);

//////////
/// Unsafe cast from string to char *, used to export strings to C functions.
SCISHARE char* ccast_unsafe(const std::string &str);

// replaces all occurances of 'substr' in 'str' with 'replacement'
SCISHARE std::string replace_substring(std::string str,
                                       const std::string &substr,
                                       const std::string &replacement);

// Returns true if 'str' ends with the string 'substr'
SCISHARE bool ends_with(const std::string &str, const std::string &substr);

// Remove spaces from start of string
SCISHARE void strip_spaces(std::string& str);

// Remove spaces from start of string
SCISHARE void strip_surrounding_spaces(std::string& str);


} // End namespace SCIRun

#endif
