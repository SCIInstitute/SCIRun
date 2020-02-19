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
 *  IgbFiletoMatrix_Plugin.cc
 *
 *  Written by:
 *   Karli Gillette
 *   Department of Bioengineering
 *   University of Utah
 *
 */

#include <Core/IEPlugin/IgbFileToMatrix_Plugin.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Logging/LoggerInterface.h>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Datatypes;

MatrixHandle SCIRun::IgbFileMatrix_reader(LoggerHandle pr, const char *filename)
{
  std::string line;
  std::vector<std::string> strs;
  int x_size=0;
  int t_size=0;
  int count=0;

  std::ifstream myfile(filename);
  if (myfile.is_open())
  {
    for (int i=0; i<5; i++)
    {
      std::getline(myfile,line);
      boost::split(strs,line,boost::is_any_of(":, "));
      size_t sz = line.size();

      for (int p=0;p<sz;p++)
      {
        if (boost::iequals(strs[p], "x"))
        {
          x_size=atoi(strs[p+1].c_str());
          count += 1;
        }
        if (boost::iequals(strs[p], "t"))
        {
          t_size=atoi(strs[p+1].c_str());
          count += 1;
        }
        if (count == 2)
          break;
      }
  	}
    myfile.close();
  }

  std::streamoff length=0;
  std::streamsize numbyts=0;

  std::ifstream is;
  is.open(filename, std::ios::in | std::ios::binary );
  if (is.is_open())
  {
    // get length of file:
    is.seekg(0, std::ios::end);
    length = is.tellg();
    is.seekg(1024, std::ios::beg);
    length -= 1024;

    std::vector<float> vec;
    vec.resize(length);
    is.read((char*)&vec[0],length);

    if (!is)
    {
      numbyts = is.gcount();
      std::cerr << "Error reading binary data. Number of bytes read: " << numbyts << std::endl;
    }
    is.close();

    auto result(boost::make_shared<DenseMatrix>(x_size,t_size));

    for(size_t p=0;p<t_size;p++ )
    {
      for(size_t pp=0;pp<x_size;pp++ )
      {
        (*result)(pp, p) = vec[(p*x_size)+pp];
      }
    }

    return result;
  }
  return nullptr;
}
