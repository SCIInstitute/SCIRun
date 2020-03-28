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


/// @todo Documentation Core/Datatypes/MatrixIO.h

#ifndef CORE_DATATYPES_MATRIX_IO_H
#define CORE_DATATYPES_MATRIX_IO_H

#include <Core/Utils/Exception.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>
#include <vector>
#include <ostream>
#include <istream>
#include <boost/algorithm/string/predicate.hpp>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  // see http://stackoverflow.com/questions/7477978/nan-ascii-i-o-with-visual-c
  template <typename T>
  struct FloatNaNHelper
  {
    T& value;
    explicit FloatNaNHelper(T& f) : value(f) { }
    operator const T&() const { return value; }
  };


  //TODO: template partial spec for double/float/etc.
  template <typename T>
  std::istream& operator>>(std::istream& in, FloatNaNHelper<T>& f)
  {
    return in;
  }

  template <>
  inline std::istream& operator>>(std::istream& in, FloatNaNHelper<double>& f)
  {
    if (in >> f.value)
      return in;

    in.clear();
    std::string str;
    if (!(in >> str))
      return in;

    if (boost::iequals(str, "NaN"))
      f.value = std::numeric_limits<double>::quiet_NaN();
    else
      in.setstate(std::ios::badbit);

    return in;
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

      while (!reader.eof())
      {
        T val;
        reader >> val;

        if (reader.fail())
        {
          FloatNaNHelper<T> helper(val);
          reader >> helper;

          if (reader.fail())
          {
            //THROW_INVALID_ARGUMENT("Matrix reading failed: stream failed");
            break;
          }
        }

        lineData.push_back(val);
      }

      if (!lineData.empty())
      {
        if (!values.empty() && values.back().size() != lineData.size())
          THROW_INVALID_ARGUMENT("Improper format of matrix text stream: not every line contains the same amount of numbers.");

        values.push_back(lineData);
      }
    }

    m.resize(values.size(), values.empty() ? 0 : values[0].size());
    m.setZero();
    for (int i = 0; i < m.rows(); ++i)
    {
      for (int j = 0; j < m.cols(); ++j)
      {
        m(i,j) = values[i][j];
      }
    }

    return istr;
  }

  template <typename T>
  std::string matrix_to_string(const T& m)
  {
    std::ostringstream o;
    o << m;
    return o.str();
  }

  template <typename T>
  std::string sparse_matrix_to_string(const T& m)
  {
    std::ostringstream o;
    o << m.castForPrinting();
    return o.str();
  }

#define DENSEMATRIX_VERSION 4

  template <typename T>
  void DenseMatrixGeneric<T>::io(Piostream& stream)
  {
    int version=stream.begin_class("DenseMatrix", DENSEMATRIX_VERSION);
    // Do the base class first...
    MatrixIOBase::io(stream);

    if (version < 4)
    {
      int nrows = static_cast<int>(this->rows());
      int ncols = static_cast<int>(this->cols());
      stream.io(nrows);
      stream.io(ncols);
      this->resize(nrows, ncols);
    }
    else
    {
      long long nrows = static_cast<long long>(this->rows());
      long long ncols = static_cast<long long>(this->cols());
      stream.io(nrows);
      stream.io(ncols);
      if(stream.reading())
      {
        this->resize(nrows, ncols);
      }
    }

    stream.begin_cheap_delim();

    int split;
    if (stream.reading())
    {
      if (version > 2)
      {
        split = this->separate_raw_;
        Pio(stream, split);
        if (this->separate_raw_)
        {
          Pio(stream, this->raw_filename_);
          FILE *f=fopen(this->raw_filename_.c_str(), "r");
          if (f)
          {
            auto result = fread(this->data(), sizeof(T), this->rows() * this->cols(), f);
            if (result != this->get_dense_size())
            {
              //TODO: test
              if (false)
                BOOST_THROW_EXCEPTION(SCIRun::Core::ExceptionBase() << FileNotFound("error reading matrix file"));
            }
            fclose(f);
          }
          else
          {
            const std::string errmsg = "Error reading separated file '" + this->raw_filename_ + "'";
            std::cerr << errmsg << "\n";
            BOOST_THROW_EXCEPTION(SCIRun::Core::ExceptionBase() << FileNotFound(errmsg));
          }
        }
      }
      else
      {
        this->separate_raw_ = false;
      }
      split = this->separate_raw_;
    }
    else
    {     // writing
      std::string filename = this->raw_filename_;
      split = this->separate_raw_;
      if (split)
      {
        if (filename == "")
        {
          if (stream.file_name.c_str())
          {
            size_t pos = stream.file_name.rfind('.');
            if (pos == std::string::npos) filename = stream.file_name + ".raw";
            else filename = stream.file_name.substr(0,pos) + ".raw";
          }
          else
          {
            split=0;
          }
        }
      }
      Pio(stream, split);
      if (split)
      {
        Pio(stream, filename);
        FILE *f = fopen(filename.c_str(), "w");
        fwrite(this->data(), sizeof(T), this->rows() * this->cols(), f);
        fclose(f);
      }
    }

    if (!split)
    {
      size_t block_size = this->rows() * this->cols();
      if (!stream.block_io(this->data(), sizeof(T), block_size))
      {
        for (size_t i = 0; i < block_size; i++)
        {
          stream.io(this->data()[i]);
        }
      }
    }
    stream.end_cheap_delim();
    stream.end_class();
  }


#define SPARSEROWMATRIX_VERSION 2

  template <typename T>
  void SparseRowMatrixGeneric<T>::io(Piostream& stream)
  {
    int version = stream.begin_class("SparseRowMatrix", SPARSEROWMATRIX_VERSION);
    // Do the base class first...
    MatrixBase<T>::io(stream);

    if (version < 2)
    {
      int r = static_cast<int>(this->nrows());
      int c = static_cast<int>(this->ncols());
      int n = static_cast<int>(this->nonZeros());
      stream.io(r);
      stream.io(c);
      stream.io(n);
      this->resize(r,c);
      this->resizeNonZeros(n);
    }
    else
    {
      auto r = this->nrows();
      Pio_size(stream, r);
      auto c = this->ncols();
      Pio_size(stream, c);
      auto n = this->nonZeros();
      Pio_size(stream, n);
      if (stream.reading())
      {
        this->resize(r,c);
        this->resizeNonZeros(n);
      }
    }

    stream.begin_cheap_delim();
    Pio_index(stream, this->outerIndexPtr(), this->nrows() + 1);
    stream.end_cheap_delim();

    stream.begin_cheap_delim();
    Pio_index(stream, this->innerIndexPtr(), this->nonZeros());
    stream.end_cheap_delim();

    stream.begin_cheap_delim();
    Pio(stream, this->valuePtr(), this->nonZeros());
    stream.end_cheap_delim();

    stream.end_class();
  }

#define COLUMNMATRIX_VERSION 3

  template <typename T>
  void DenseColumnMatrixGeneric<T>::io(Piostream& stream)
  {
    int version = stream.begin_class("ColumnMatrix", COLUMNMATRIX_VERSION);

    if (version > 1)
    {
      // New version inherits from Matrix
      MatrixBase<T>::io(stream);
    }

    if (version < 3)
    {
      int nrows = static_cast<int>(this->nrows());
      stream.io(nrows);
      this->resize(static_cast<size_type>(nrows), 1);
    }
    else
    {
      long long nrows= static_cast<long long>(this->nrows());
      stream.io(nrows);
      if (stream.reading())
      {
        this->resize(static_cast<size_type>(nrows), 1);
      }
    }

    if (!stream.block_io(this->data(), sizeof(T), this->nrows()))
    {
      for (size_t i=0; i<this->nrows(); i++)
        stream.io(this->data()[i]);
    }
    stream.end_class();
  }

}}}


#endif
