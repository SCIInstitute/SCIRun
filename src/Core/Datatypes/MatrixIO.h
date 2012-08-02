/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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


#ifndef CORE_DATATYPES_MATRIX_IO_H
#define CORE_DATATYPES_MATRIX_IO_H 

#include <Core/Datatypes/Matrix.h>
#include <ostream>
#include <istream>

namespace SCIRun {
namespace Domain {
namespace Datatypes {

  template <typename T>
  std::ostream& operator<<(std::ostream& o, const DenseMatrixGeneric<T>& m)
  {
    for (size_t i = 0; i < m.nrows(); ++i)
    {
      for (size_t j = 0; j < m.ncols(); ++j)
      {
        o << m(i,j) << " ";
      }
      o << "\n";
    }
    return o;
  }

  template <typename T>
  std::istream& operator>>(std::istream& istr, DenseMatrixGeneric<T>& m)
  {
    std::vector<std::vector<T> > values;

    std::string line;

    while (!std::getline(istr, line, '\n').eof()) 
    {
      std::istringstream reader(line);

      std::vector<T> lineData;

      while(!reader.eof()) 
      {
        T val;
        reader >> val;

        if(reader.fail())
          break;

        lineData.push_back(val);
      }

      if (!lineData.empty())
        values.push_back(lineData);
    }

    m.clear();
    m.resize(values.size(), values[0].size());
    for (size_t i = 0; i < m.nrows(); ++i)
    {
      for (size_t j = 0; j < m.ncols(); ++j)
      {
        m(i,j) = values[i][j];
      }
    }

    return istr;
  }

  template <typename T>
  std::string matrix_to_string(const DenseMatrixGeneric<T>& m)
  {
    std::ostringstream o;
    o << m;
    return o.str();
  }

}}}


#endif
