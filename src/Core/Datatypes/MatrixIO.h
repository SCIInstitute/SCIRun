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

#include <Core/Utils/Exception.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Base/PropertyManager.h>
#include <vector>
#include <ostream>
#include <istream>

namespace SCIRun {
namespace Core {
namespace Datatypes {

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
          //THROW_INVALID_ARGUMENT("Matrix reading failed: stream failed");
          break;
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

    m.resize(values.size(), values[0].size());
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
      //this->nrows_ = static_cast<size_type>(nrows);
      //this->ncols_ = static_cast<size_type>(ncols);
    }
    else
    {
      long long nrows = static_cast<long long>(this->rows());
      long long ncols = static_cast<long long>(this->cols());
      stream.io(nrows);
      stream.io(ncols);
      this->resize(nrows, ncols);
      //this->nrows_ = static_cast<size_type>(nrows);
      //this->ncols_ = static_cast<size_type>(ncols);
    }

    if(stream.reading())
    {
      //already resized above

      //data = new double*[this->rows()];
      //double* tmp = new double[this->rows() * this->cols()];
      //dataptr_=tmp;
      //for (index_type i = 0; i < this->rows(); i++)
      //{
      //  data[i] = tmp;
      //  tmp += this->cols();
      //}
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
            fread(this->data(), sizeof(T), this->rows() * this->cols(), f);
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
      //this->nrows() = static_cast<size_type>(r);
      //this->ncols() = static_cast<size_type>(c);
      //this->nonZeros() = static_cast<size_type>(n);
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
      std::cout << "SPARSE resizing to " << r << " x " << c << std::endl;
      this->resize(r,c);
      std::cout << "    and " << n << " nnz" << std::endl;
      this->resizeNonZeros(n);
    }

    if (stream.reading())
    {
      //resizing done above 

      //data_.reset(new T[nnz_]);
      //columns_.reset(new index_type[nnz_]);
      //rows_.reset(new index_type[this->nrows_+1]);
    }

    stream.begin_cheap_delim();  
    Pio(stream, this->outerIndexPtr(), this->outerSize());
    stream.end_cheap_delim();

    stream.begin_cheap_delim();
    Pio(stream, this->innerIndexPtr(), this->nonZeros());
    stream.end_cheap_delim();

    stream.begin_cheap_delim();

    std::cout << "NNZ array before: " << std::endl;
    std::copy(this->valuePtr(), this->valuePtr() + this->nonZeros(), std::ostream_iterator<T>(std::cout, " "));

    Pio(stream, this->valuePtr(), this->nonZeros());

    std::cout << "NNZ array after: " << std::endl;
    std::copy(this->valuePtr(), this->valuePtr() + this->nonZeros(), std::ostream_iterator<T>(std::cout, " "));

    stream.end_cheap_delim();

    stream.end_class();
  }

}}}


#endif
