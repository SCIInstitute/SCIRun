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


/// @todo Documentation Core/Utils/Legacy/StringUtil.cc

#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Utils/Legacy/Assert.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits>

namespace SCIRun {

char *
ccast_unsafe(const std::string &str)
{
  char *result = const_cast<char *>(str.c_str());
  ASSERT(result);
  return result;
}


std::vector<std::string>
split_string(const std::string& str, char sep)
{
  std::vector<std::string> result;
  std::string s(str);
  while(s != "")
  {
    std::string::size_type first = s.find(sep);
    if(first < s.size())
    {
      if (first)
      {
        result.push_back(s.substr(0, first));
      }
      s = s.substr(first+1);
    }
    else
    {
      result.push_back(s);
      break;
    }
  }
  return result;
}


/////////
// C++ify a string, turn newlines into \n, use \t, \r, \\ \", etc.
std::string
string_Cify(const std::string &str)
{
  std::string result("");
  for (std::string::size_type i = 0; i < str.size(); i++)
  {
    switch(str[i])
    {
    case '\n':
      result.push_back('\\');
      result.push_back('n');
      break;

    case '\r':
      result.push_back('\\');
      result.push_back('r');
      break;

    case '\t':
      result.push_back('\\');
      result.push_back('t');
      break;

    case '"':
      result.push_back('\\');
      result.push_back('"');
      break;

    case '\\':
      result.push_back('\\');
      result.push_back('\\');
      break;

    default:
      result.push_back(str[i]);
    }
  }
  return result;
}


// replaces all occurrences of 'substr' in 'str' with 'replacement'
std::string
replace_substring(std::string str,
		  const std::string &substr,
		  const std::string &replacement)
{
  std::string::size_type pos;
  do {
    pos = str.find(substr);
    if (pos != std::string::npos)
      str = str.replace(str.begin()+pos,
                        str.begin()+pos+substr.length(),
                        replacement);
  } while (pos != std::string::npos);
  return str;
}


bool
ends_with(const std::string &str, const std::string &substr)
{
  return str.rfind(substr) == str.size()-substr.size();
}


std::string
string_toupper(std::string str)
{
  std::string::iterator iter = str.begin();
  std::string::iterator iend = str.end();
  for (; iter != iend; ++iter)
    *iter = toupper(*iter);
  return str;
}

std::string
string_tolower(std::string str)
{
  std::string::iterator iter = str.begin();
  std::string::iterator iend = str.end();
  for (; iter != iend; ++iter)
    *iter = tolower(*iter);
  return str;
}

bool from_string(const std::string &str, double &value)
{
  std::string dstr = str + "\0";
  strip_spaces(dstr);

  // if empty just return
  if (dstr.empty()) return (false);

  const double nan = std::numeric_limits<double>::quiet_NaN();
  const double inf = std::numeric_limits<double>::infinity();
  // Handle special cases: nan, inf, and -inf
  if (dstr[0] == 'n' || dstr[0] == 'N')
  {
    if (dstr.compare(0,3,"nan") == 0)
    {
      value = nan; return (true);
    }
    else if (dstr.compare(0,3,"NaN") == 0)
    {
      value = nan; return (true);
    }
    else if (dstr.compare(0,3,"Nan") == 0)
    {
      value = nan; return (true);
    }
    else if (dstr.compare(0,3,"NAN") == 0)
    {
      value = nan; return (true);
    }
  }
  else if (dstr[0] == 'i' || dstr[0] == 'I')
  {
    if (dstr.compare(0,3,"inf") == 0)
    {
      value = inf; return (true);
    }
    else if (dstr.compare(0,3,"Inf") == 0)
    {
      value = inf; return (true);
    }
    else if (dstr.compare(0,3,"INF") == 0)
    {
      value = inf; return (true);
    }
  }
  else if (dstr.size() > 1 && dstr[0] == '-' && (dstr[1] == 'i' || dstr[1] == 'I'))
  {
    if (dstr.compare(0,4,"-inf") == 0)
    {
      value = -inf; return (true);
    }
    else if (dstr.compare(0,4,"-Inf") == 0)
    {
      value = -inf; return (true);
    }
    else if (dstr.compare(0,4,"-INF") == 0)
    {
      value = -inf; return (true);
    }
  }

  // Default conversion
  char* eptr;
  value = strtod(&(dstr[0]),&eptr);
  if (eptr == &(dstr[0])) return (false);
  return (true);
}

bool from_string(const std::string &str, float &value)
{
  std::string dstr = str + "\0";
  strip_spaces(dstr);

  // if empty just return
  if (dstr.empty()) return (false);

  const float nan = std::numeric_limits<float>::quiet_NaN();
  const float inf = std::numeric_limits<float>::infinity();
  // Handle special cases: nan, inf, and -inf
  if (dstr[0] == 'n' || dstr[0] == 'N')
  {
    if (dstr.compare(0,3,"nan") == 0)
    {
      value = nan; return (true);
    }
    else if (dstr.compare(0,3,"NaN") == 0)
    {
      value = nan; return (true);
    }
    else if (dstr.compare(0,3,"Nan") == 0)
    {
      value = nan; return (true);
    }
    else if (dstr.compare(0,3,"NAN") == 0)
    {
      value = nan; return (true);
    }
  }
  else if (dstr[0] == 'i' || dstr[0] == 'I')
  {
    if (dstr.compare(0,3,"inf") == 0)
    {
      value = inf; return (true);
    }
    else if (dstr.compare(0,3,"Inf") == 0)
    {
      value = inf; return (true);
    }
    else if (dstr.compare(0,3,"INF") == 0)
    {
      value = inf; return (true);
    }
  }
  else if (dstr.size() > 1 && dstr[0] == '-' && (dstr[1] == 'i' || dstr[1] == 'I'))
  {
    if (dstr.compare(0,4,"-inf") == 0)
    {
      value = -inf; return (true);
    }
    else if (dstr.compare(0,4,"-Inf") == 0)
    {
      value = -inf; return (true);
    }
    else if (dstr.compare(0,4,"-INF") == 0)
    {
      value = -inf; return (true);
    }
  }

  // Handle normal numbers
  char *eptr;
  double tempValue = strtod(&(dstr[0]),&eptr);
  value = (float) tempValue;
  if (eptr == &(dstr[0])) return (false);
  return (true);
}

bool from_string(const std::string &str, int &value)
{
  std::string dstr = str+ "\0";
  char *eptr;
  value = static_cast<int>(strtol(&(dstr[0]),&eptr,0));
  if (eptr == &(dstr[0])) return (false);
  return (true);
}

bool from_string(const std::string &str, unsigned int &value)
{
  std::string dstr = str+ "\0";
  char *eptr;
  value = static_cast<unsigned int>(strtol(&(dstr[0]),&eptr,0));
  if (eptr == &(dstr[0])) return (false);
  return (true);
}

bool from_string(const std::string &str, long &value)
{
  std::string dstr = str+ "\0";
  char *eptr;
  value = static_cast<long>(strtol(&(dstr[0]),&eptr,0));
  if (eptr == &(dstr[0])) return (false);
  return (true);
}

bool from_string(const std::string &str, unsigned long &value)
{
  std::string dstr = str+ "\0";
  char *eptr;
  value = static_cast<unsigned long>(strtol(&(dstr[0]),&eptr,0));
  if (eptr == &(dstr[0])) return (false);
  return (true);
}

bool from_string(const std::string &str, long long &value)
{
  std::string dstr = str+ "\0";
  char *eptr;
  value = static_cast<long long>(strtol(&(dstr[0]),&eptr,0));
  if (eptr == &(dstr[0])) return (false);
  return (true);
}

bool from_string(const std::string &str, unsigned long long &value)
{
  std::string dstr = str+ "\0";
  char *eptr;
  value = static_cast<unsigned long long>(strtol(&(dstr[0]),&eptr,0));
  if (eptr == &(dstr[0])) return (false);
  return (true);
}


// Strip out space at the start and at the end of the string
void
strip_spaces(std::string& str)
{
  size_t esize = str.size();
  size_t idx = 0;

  // Strip out spaces at the start of the str
  while((idx <esize)&&((str[idx] == ' ') ||(str[idx] == '\t')||
        (str[idx] == '\n') || (str[idx] == '\r')||
        (str[idx] == '\f') || (str[idx] == '\v'))) idx++;

  // Get the substring without spaces at the start or at the end
  str = str.substr(idx,(str.size()-idx));
}


// Strip out space at the start and at the end of the string
void
strip_surrounding_spaces(std::string& str)
{
  size_t esize = str.size();
  size_t idx = 0;

  // Strip out spaces at the start of the str
  while((idx <esize)&&((str[idx] == ' ') ||(str[idx] == '\t')||
        (str[idx] == '\n') || (str[idx] == '\r')||
        (str[idx] == '\f') || (str[idx] == '\v'))) idx++;

  size_t ridx = 0;
  if (str.size()) ridx = str.size()-1;

  // Strip out spaces at the end of the str
  while((ridx > 0)&&((str[ridx] == ' ') ||(str[ridx] == '\t')||
        (str[ridx] == '\n') || (str[ridx] == '\r')||
        (str[ridx] == '\f') || (str[ridx] == '\v'))) ridx--;

  // Get the substring without spaces at the start or at the end
  str = str.substr(idx,(ridx-idx+1));
}

} // End namespace SCIRun
