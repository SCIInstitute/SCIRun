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
///@file   PersistentSTL.h
///@brief  Persistent i/o for STL containers
///
///@author David Hart, Alexei Samsonov
///        Department of Computer Science
///        University of Utah
///@date   Mar. 2000, Dec 2000
///

#ifndef SCI_project_PersistentSTL_h
#define SCI_project_PersistentSTL_h 1

#include <Core/Persistent/Persistent.h>
#include <Core/Persistent/share.h>
#include <map>
#include <vector>
#include <list>


namespace SCIRun {

#define MAP_VERSION 1

// Persistent IO for maps
template <class Key, class Data>
void
Pio(Piostream& stream, std::map<Key, Data>& data );

//////////
// Persistent IO of vector containers
template <class T>
void Pio(Piostream& stream, std::vector<T>& data);

template<class T,class S>
void
Pio( Piostream &stream, std::pair<T,S>& pair);


SCISHARE void
Pio_index(Piostream& stream, std::vector<index_type>& data);

//////////
// Persistent io for maps
template <class Key, class Data>
inline void
Pio(Piostream& stream, std::map<Key, Data>& data) {

  stream.begin_class("Map", MAP_VERSION);

				// if reading from stream
  if (stream.reading()) {
				// get map size
    int n;
    Pio(stream, n);
				// read elements
    Key k;
    Data d;
    for (int i = 0; i < n; i++) {
      Pio(stream, k);
      Pio(stream, d);
      data[k] = d;
    }

  }
				// if writing to stream
  else {
				// write map size
    int n = data.size();
    Pio(stream, n);
				// write elements
    for (auto iter = data.begin(); iter != data.end(); iter++) {
				// have to copy iterator elements,
				// since passing them directly in a
				// call to Pio can be invalid because
				// Pio passes data by reference
      Key ik = (*iter).first;
      Data dk = (*iter).second;
      Pio(stream, ik);
      Pio(stream, dk);
    }

  }

  stream.end_class();

}

//////////
// PIO for vectors
#define STLVECTOR_VERSION 2



// Optimize  heavily used in the field classes.
template <>
SCISHARE void Pio(Piostream& stream, std::vector<bool>& data);
template <>
SCISHARE void Pio(Piostream& stream, std::vector<char>& data);
template <>
SCISHARE void Pio(Piostream& stream, std::vector<unsigned char>& data);
template <>
SCISHARE void Pio(Piostream& stream, std::vector<short>& data);
template <>
SCISHARE void Pio(Piostream& stream, std::vector<unsigned short>& data);
template <>
SCISHARE void Pio(Piostream& stream, std::vector<int>& data);
template <>
SCISHARE void Pio(Piostream& stream, std::vector<unsigned int>& data);
template <>
SCISHARE void Pio(Piostream& stream, std::vector<long long>& data);
template <>
SCISHARE void Pio(Piostream& stream, std::vector<unsigned long long>& data);
template <>
SCISHARE void Pio(Piostream& stream, std::vector<float>& data);
template <>
SCISHARE void Pio(Piostream& stream, std::vector<double>& data);

template <class T>
void Pio(Piostream& stream, std::vector<T>& data)
{
  if (stream.reading() && stream.peek_class() == "Array1")
  {
    stream.begin_class("Array1", STLVECTOR_VERSION);
  }
  else
  {
    stream.begin_class("STLVector", STLVECTOR_VERSION);
  }

  int size=static_cast<int>(data.size());
  stream.io(size);

  if(stream.reading()){
    data.resize(size);
  }

  for (int i = 0; i < size; i++)
  {
    Pio(stream, data[i]);
  }

  stream.end_class();
}

template <class T>
void Pio(Piostream& stream, std::vector<T*>& data)
{
  if (stream.reading() && stream.peek_class() == "Array1")
  {
    stream.begin_class("Array1", STLVECTOR_VERSION);
  }
  else
  {
    stream.begin_class("STLVector", STLVECTOR_VERSION);
  }

  int size=static_cast<int>(data.size());
  stream.io(size);

  if(stream.reading()){
    data.resize(size);
  }

  for (int i = 0; i < size; i++)
  {
    Pio(stream, *data[i]);
  }

  stream.end_class();
}



//////////
// PIO for lists
#define STLLIST_VERSION 1

template <class T>
void Pio(Piostream& stream, std::list<T>& data)
{
  stream.begin_cheap_delim();

  int size=data.size();
  stream.io(size);

  if(stream.reading()){
    data.resize(size);
  }

  for (typename std::list<T>::iterator ii=data.begin(); ii!=data.end(); ii++)
    Pio(stream, *ii);

  stream.end_cheap_delim();
}


//////////
// PIO for pair
#define STLPAIR_VERSION 1

template <class T,class S>
void Pio(Piostream& stream, std::pair<T,S>& data)
{
  stream.begin_class("STLPair", STLPAIR_VERSION);

  Pio(stream, data.first);
  Pio(stream, data.second);

  stream.end_class();
}

} // End namespace SCIRun

#endif // SCI_project_PersistentSTL_h
