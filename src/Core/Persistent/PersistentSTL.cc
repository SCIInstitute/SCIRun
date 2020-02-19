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
///@file   PersistentSTL.cc
///@brief  Persistent i/o for STL containers
///@author Michael Callahan
///        Department of Computer Science
///        University of Utah
///@date   March 2001
///

#include <iostream>
#include <Core/Persistent/Persistent.h>
#include <Core/Persistent/PersistentSTL.h>

namespace SCIRun {

  template <> void Pio(Piostream& stream, std::vector<bool>& data)
  {
    int version = stream.begin_class("STLVector", STLVECTOR_VERSION);

    size_type size;

    if (version  < 3)
    {
      int sz = static_cast<int>(data.size());
      Pio(stream,sz);
      size = static_cast<size_type>(sz);
    }
    else
    {
      Pio_size(stream,size);
    }

    if(stream.reading()){
      data.resize(size);
    }

    for (int i = 0; i < size; i++)
    {
      bool b;
      Pio(stream, b);
      data[i] = b;
    }

    stream.end_class();
  }



  template <class T>
  static inline void block_pio(Piostream &stream, std::vector<T> &data)
  {
    int version;
    if (stream.reading() && stream.peek_class() == "Array1")
    {
      version = stream.begin_class("Array1", STLVECTOR_VERSION);
    }
    else
    {
      version = stream.begin_class("STLVector", STLVECTOR_VERSION);
    }

    size_type size;

    if (version  < 3)
    {
      int sz = static_cast<int>(data.size());
      Pio(stream,sz);
      size = static_cast<size_type>(sz);
    }
    else
    {
      Pio_size(stream,size);
    }

    if(stream.reading()){
      data.resize(size);
    }

    if (data.size() && !stream.block_io(&data.front(), sizeof(T), data.size()))
    {
      for (int i = 0; i < size; i++)
      {
        Pio(stream, data[i]);
      }
    }

    stream.end_class();
  }


  template <>
  void Pio(Piostream& stream, std::vector<char>& data)
  {
    block_pio(stream, data);
  }

  template <>
  void Pio(Piostream& stream, std::vector<unsigned char>& data)
  {
    block_pio(stream, data);
  }

  template <>
  void Pio(Piostream& stream, std::vector<short>& data)
  {
    block_pio(stream, data);
  }

  template <>
  void Pio(Piostream& stream, std::vector<unsigned short>& data)
  {
    block_pio(stream, data);
  }

  template <>
  void Pio(Piostream& stream, std::vector<int>& data)
  {
    block_pio(stream, data);
  }

  template <>
  void Pio(Piostream& stream, std::vector<unsigned int>& data)
  {
    block_pio(stream, data);
  }

  template <>
  void Pio(Piostream& stream, std::vector<long long>& data)
  {
    block_pio(stream, data);
  }

  template <>
  void Pio(Piostream& stream, std::vector<unsigned long long>& data)
  {
    block_pio(stream, data);
  }

  template <>
  void Pio(Piostream& stream, std::vector<float>& data)
  {
    block_pio(stream, data);
  }

  template <>
  void Pio(Piostream& stream, std::vector<double>& data)
  {
    block_pio(stream, data);
  }


  /// Export indices 32/64 bit compatible
  void Pio_index(Piostream& stream, std::vector<index_type>& data)
  {
    if (stream.reading())
    {
      if ((stream.peek_class() == "Array1")||(stream.peek_class() == "STLVector"))
      {
        std::vector<unsigned int> old_indices;
        Pio(stream,old_indices);
        data.resize(old_indices.size());
        for (size_t i=0;i<old_indices.size();i++)
          data[i] = static_cast<index_type>(old_indices[i]);
        return;
      }
      else if (stream.peek_class() == "STLIndexVector")
      {
        stream.begin_class("STLIndexVector",1);
        long long size;
        stream.io(size);
        long long index_size;
        stream.io(index_size);

        if ((index_size == 4)&&(sizeof(index_type)==4))
        {
          data.resize(static_cast<size_t>(size));
          if (size > 0)
          {
            if (stream.supports_block_io())
              stream.block_io(&data.front(), 4, static_cast<size_t>(size));
            else
            {
              for (long long i = 0; i < size; i++)
              {
                stream.io(data[i]);
              }
            }
          }
        }
        else if ((index_size == 4)&&(sizeof(index_type)==8))
        {
          std::vector<unsigned int> indices;
          indices.resize(static_cast<size_t>(size));
          if (size > 0)
          {
            if (stream.supports_block_io())
              stream.block_io(&indices.front(), 4, static_cast<size_t>(size));
            else
            {
              for (unsigned int i = 0; i < size; i++)
              {
                stream.io(indices[i]);
              }
            }

            data.resize(static_cast<size_t>(size));
            for (unsigned int i = 0; i < size; i++)
            {
              data[i] = static_cast<index_type>(indices[i]);
            }
          }
        }
        else if ((index_size == 8)&&(sizeof(index_type)==4))
        {
          std::vector<long long> indices;
          indices.resize(static_cast<size_t>(size));
          if (size > 0)
          {
            if (stream.supports_block_io())
              stream.block_io(&indices.front(), 8, static_cast<size_t>(size));
            else
              for (long long i = 0; i < size; i++)
              {
                stream.io(indices[i]);
              }

            data.resize(static_cast<size_t>(size));
            for (long long i = 0; i < size; i++)
            {
              data[i] = static_cast<index_type>(indices[i]);
            }
          }
        }
        if ((index_size == 8)&&(sizeof(index_type)==8))
        {
          data.resize(static_cast<size_t>(size));
          if (size > 0)
          {
            if (stream.supports_block_io())
              stream.block_io(&data.front(), 8, static_cast<size_t>(size));
            else
            {
              for (long long i = 0; i < size; i++)
              {
                stream.io(data[i]);
              }
            }
          }
        }
        stream.end_class();
        return;
      }
      else
      {
        std::cerr << "Encountered an unknown class for storing indices\n";
        data.resize(0);
        return;
      }
    }
    else
    {
      stream.begin_class("STLIndexVector",1);
      long long size = static_cast<long long>(data.size());
      stream.io(size);
      long long index_size = sizeof(index_type);
      stream.io(index_size);

      if (stream.supports_block_io() && data.size() > 0 )
        stream.block_io(&(data.front()),sizeof(index_type),static_cast<size_t>(size));
      else
        for (index_type i=0; i< size; i++)
          stream.io(data[i]);

      stream.end_class();
      return;
    }
  }


} // End namespace SCIRun
