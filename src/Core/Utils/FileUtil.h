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


/// @todo Documentation Core/Utils/FileUtil.h

#ifndef CORE_UTILS_FILEUTIL_H
#define CORE_UTILS_FILEUTIL_H

#include <sstream>
#include <Core/Utils/share.h>

namespace SCIRun
{
namespace Core
{

inline bool fileContainsString(const std::string& filename, const std::string& str)
{
  std::ifstream input(filename.c_str());
  std::string line;

  while (std::getline(input, line))
  {
    auto index = line.find(str);
    if (index != std::string::npos && line.find('\0') > index)
      return true;
  }

  return false;
}

inline void replaceDelimitersWithWhitespace(std::string& line)
{
  /// @todo this code doesn't work under VS2010 - check with VS2012

  // replace comma's, tabs etc. with white spaces
//  for (auto &c : line)
//  {
//    if ( (c == '\t') || (c == ',') || (c == '"') )
//      c = ' ';
//  }
	for (auto it = line.begin(); it != line.end(); it++)
	{
		char c = *it;
        if ( (c == '\t') || (c == ',') || (c == '"') )
          c = ' ';
	}
}

inline bool lineStartsWithComment(const std::string& line)
{
  if ( line.empty() ) return false;

  if ( (line[0] == '#') || (line[0] == '%') ) return true;

  return false;
}

}}

#endif
