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
///    @file   FData.h
///    @author Martin Cole
///    @date   Wed Apr 28 09:45:51 2004
///
///    @brief  Taken from old LatVolField.h ...


#ifndef CORE_CONTAINERS_FDATA_H
#define CORE_CONTAINERS_FDATA_H 1

#include <Core/Utils/Legacy/TypeDescription.h>
#include <Core/Datatypes/Legacy/Base/Types.h>
#include <Core/Datatypes/Legacy/Base/TypeName.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Containers/Array3.h>
#include <Core/Containers/Array2.h>
#include <Core/Math/MiscMath.h>

#include <Core/Utils/Legacy/Assert.h>
#include <string>

namespace SCIRun {

template <class Data, class Msh>
class FData3d : public Array3<Data> {
public:
  typedef Data value_type;
  typedef Data * iterator;
  typedef const Data * const_iterator;

  iterator begin() { return &(*this)(0,0,0); }
  iterator end() { return &((*this)(this->dim1()-1,this->dim2()-1,this->dim3()-1))+1; }
  const_iterator begin() const { return &(*this)(0,0,0); }
  const_iterator end() const { return &((*this)(this->dim1()-1,this->dim2()-1,this->dim3()-1))+1; }


  FData3d() : Array3<Data>() {}
  FData3d(int) : Array3<Data>() {}
  FData3d(const FData3d& data) : Array3<Data>(data) {}
  virtual ~FData3d();

  const value_type &operator[](typename Msh::Cell::index_type idx) const
  { return operator()(idx.k_,idx.j_,idx.i_); }
  const value_type &operator[](typename Msh::Face::index_type idx) const
  { return operator()(0, 0, idx); }
  const value_type &operator[](typename Msh::Edge::index_type idx) const
  { return operator()(0, 0, idx); }
  const value_type &operator[](typename Msh::Node::index_type idx) const
  { return operator()(idx.k_,idx.j_,idx.i_); }
  const value_type &operator[](VMesh::index_type idx) const
  { return Array3<Data>::operator()(idx); }

  value_type &operator[](typename Msh::Cell::index_type idx)
  { return operator()(idx.k_,idx.j_,idx.i_); }
  value_type &operator[](typename Msh::Face::index_type idx)
  { return operator()(0, 0, idx); }
  value_type &operator[](typename Msh::Edge::index_type idx)
  { return operator()(0, 0, idx); }
  value_type &operator[](typename Msh::Node::index_type idx)
  { return operator()(idx.k_,idx.j_,idx.i_); }
  value_type &operator[](VMesh::index_type idx)
  { return Array3<Data>::operator()(idx); }

  void resize(const typename Msh::Node::size_type &size)
  { Array3<Data>::resize(size.k_, size.j_, size.i_); Array3<Data>::zero(); }
  void resize(const typename Msh::Edge::size_type &size)
  { Array3<Data>::resize(1, 1, size); Array3<Data>::zero();  }
  void resize(const typename Msh::Face::size_type &size)
  { Array3<Data>::resize(1, 1, size); Array3<Data>::zero();  }
  void resize(const typename Msh::Cell::size_type &size)
  { Array3<Data>::resize(size.k_, size.j_, size.i_); Array3<Data>::zero();  }
  void resize(const size_t size)
  { Array3<Data>::resize(1,1,size); Array3<Data>::zero();  }

  unsigned int size() const { return this->dim1() * this->dim2() * this->dim3(); }

  static const std::string type_name(int n = -1);
  const TypeDescription* get_type_description(int n) const;
};


template <class Data, class Msh>
FData3d<Data, Msh>::~FData3d()
{
}


template <class Data, class Msh>
const std::string
FData3d<Data, Msh>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 2);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1), type_name(2));
    return name;
  }
  else if (n == 0)
  {
    return "FData3d";
  }
  else if (n == 1)
  {
    return find_type_name(static_cast<Data *>(0));
  }
  else
  {
    return find_type_name(static_cast<Msh *>(0));
  }
}

template <class Data, class Msh>
const TypeDescription*
FData3d<Data, Msh>::get_type_description(int n) const
{
  ASSERT((n >= -1) && n <= 2);

  static std::string name(type_name(0));
  static std::string namesp("SCIRun");
  static std::string path(__FILE__);
  const TypeDescription *sub1 = get_type_description(static_cast<Data*>(0));
  const TypeDescription *sub2 = get_type_description(static_cast<Msh*>(0));

  if (n == -1) {
    static TypeDescription* tdn1 = 0;
    if (tdn1 == 0) {
      TypeDescription::td_vec *subs = new TypeDescription::td_vec(2);
      (*subs)[0] = sub1;
      (*subs)[1] = sub2;
      tdn1 = new TypeDescription(name, subs, path, namesp,
				    TypeDescription::CONTAINER_E);
    }
    return tdn1;
  }
  else if(n == 0) {
    static TypeDescription* tdn0 = 0;
    if (tdn0 == 0) {
      tdn0 = new TypeDescription(name, 0, path, namesp,
				    TypeDescription::CONTAINER_E);
    }
    return tdn0;
  }
  else if(n == 1) {
    return sub1;
  }
  return sub2;
}

template <class Data, class Msh>
const TypeDescription*
get_type_description(FData3d<Data, Msh>*)
{
  static std::string name(FData3d<Data, Msh>::type_name(0));
  static std::string namesp("SCIRun");
  static std::string path(__FILE__);
  const TypeDescription *sub1 = get_type_description(static_cast<Data*>(0));
  const TypeDescription *sub2 = SCIRun::get_type_description(static_cast<Msh*>(0));

  static TypeDescription* tdn1 = 0;
  if (tdn1 == 0) {
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(2);
    (*subs)[0] = sub1;
    (*subs)[1] = sub2;
    tdn1 = new TypeDescription(name, subs, path, namesp,
				  TypeDescription::CONTAINER_E);
  }
  return tdn1;
}



template <class Data, class Msh>
class FData2d : public Array2<Data> {
public:
  typedef Data value_type;
  typedef Data * iterator;
  typedef Data const * const_iterator;

  iterator begin() { return &(*this)(0,0); }
  iterator end() { return &((*this)(this->dim1()-1,this->dim2()-1))+1; }
  const_iterator begin() const { return &(*this)(0,0); }
  const_iterator end() const { return &((*this)(this->dim1()-1,this->dim2()-1))+1; }

  FData2d() : Array2<Data>() {}
  FData2d(int) : Array2<Data>() {}
  FData2d(const FData2d& data) : Array2<Data>(data) {}
  virtual ~FData2d();

  const value_type &operator[](typename Msh::Cell::index_type idx) const
  { return operator()(0, idx); }
  const value_type &operator[](typename Msh::Face::index_type idx) const
  { return operator()(idx.j_, idx.i_); }
  const value_type &operator[](typename Msh::Edge::index_type idx) const
  { return operator()(0, idx); }
  const value_type &operator[](typename Msh::Node::index_type idx) const
  { return operator()(idx.j_, idx.i_); }
  const value_type &operator[](VMesh::index_type idx) const
  { return Array2<Data>::operator()(static_cast<unsigned int>(idx)); }

  value_type &operator[](typename Msh::Cell::index_type idx)
  { return operator()(0, idx); }
  value_type &operator[](typename Msh::Face::index_type idx)
  { return operator()(idx.j_, idx.i_); }
  value_type &operator[](typename Msh::Edge::index_type idx)
  { return operator()(0, idx); }
  value_type &operator[](typename Msh::Node::index_type idx)
  { return operator()(idx.j_, idx.i_); }
  value_type &operator[](VMesh::index_type idx)
  { return Array2<Data>::operator()(static_cast<unsigned int>(idx)); }

  void resize(const typename Msh::Node::size_type &size)
  { Array2<Data>::resize(size.j_, size.i_); Array2<Data>::zero(); }
  void resize(const typename Msh::Edge::size_type &size)
  { Array2<Data>::resize(1, size); Array2<Data>::zero(); }
  void resize(const typename Msh::Face::size_type &size)
  { Array2<Data>::resize(size.j_, size.i_); Array2<Data>::zero(); }
  void resize(const typename Msh::Cell::size_type &size)
  { Array2<Data>::resize(1, size); Array2<Data>::zero(); }
  void resize(const size_t size)
  { Array2<Data>::resize(1,size); Array2<Data>::zero(); }

  unsigned int size() const { return this->dim1() * this->dim2(); }

  static const std::string type_name(int n = -1);
};


template <class Data, class Msh>
FData2d<Data, Msh>::~FData2d()
{
}


template <class Data, class Msh>
const std::string
FData2d<Data, Msh>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 2);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1), type_name(2));
    return name;
  }
  else if (n == 0)
  {
    return "FData2d";
  }
  else if (n == 1)
  {
    return find_type_name(static_cast<Data *>(0));
  }
  else
  {
    return find_type_name(static_cast<Msh *>(0));
  }
}

template <class Data, class Msh>
const TypeDescription*
get_type_description(FData2d<Data, Msh>*)
{
  static std::string name(FData2d<Data, Msh>::type_name(0));
  static std::string namesp("SCIRun");
  static std::string path(__FILE__);
  const TypeDescription *sub1 = get_type_description(static_cast<Data*>(0));
  const TypeDescription *sub2 = get_type_description(static_cast<Msh*>(0));

  static TypeDescription* tdn1 = 0;
  if (tdn1 == 0) {
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(2);
    (*subs)[0] = sub1;
    (*subs)[1] = sub2;
    tdn1 = new TypeDescription(name, subs, path, namesp,
				  TypeDescription::CONTAINER_E);
  }
  return tdn1;
}







template <class Data>
class FData1d : public Array1<Data> {
public:
  // import index_type
  typedef typename Array1<Data>::index_type index_type;
  typedef typename Array1<Data>::size_type  size_type;

  typedef Data value_type;
  typedef Data * iterator;
  typedef Data const * const_iterator;

  iterator begin() { return &(*this)(0); }
  iterator end() { return &((*this)(this->dim1()-1))+1; }
  const_iterator begin() const { return &(*this)(0); }
  const_iterator end() const { return &((*this)(this->dim1()-1))+1; }

  FData1d() : Array1<Data>() {}
  FData1d(int) : Array1<Data>() {}
  FData1d(const FData1d& data) { Array1<Data>::copy(data); }
  virtual ~FData1d();

  const value_type &operator[](index_type idx) const
  { return operator()(idx); }
  value_type &operator[](index_type idx)
  { return operator()(idx); }

  void resize(const size_type size)
  { Array1<Data>::resize(size); }

  size_type size() const { return this->dim1(); }

  static const std::string type_name(int n = -1);
};


template <class Data>
FData1d<Data>::~FData1d()
{
}


template <class Data>
const std::string
FData1d<Data>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 2);
  if (n == -1)
  {
    static const std::string name = TypeNameGenerator::make_template_id(type_name(0), type_name(1));
    return name;
  }
  else if (n == 0)
  {
    return "FData1d";
  }
  else
  {
    return find_type_name(static_cast<Data *>(0));
  }
}

template <class Data>
const TypeDescription*
get_type_description(FData1d<Data>*)
{
  static std::string name(FData1d<Data>::type_name(0));
  static std::string namesp("SCIRun");
  static std::string path(__FILE__);
  const TypeDescription *sub1 = get_type_description(static_cast<Data*>(0));

  static TypeDescription* tdn1 = 0;
  if (tdn1 == 0)
  {
    TypeDescription::td_vec *subs = new TypeDescription::td_vec(1);
    (*subs)[0] = sub1;
    tdn1 = new TypeDescription(name, subs, path, namesp,
				  TypeDescription::CONTAINER_E);
  }
  return tdn1;
}


} // end namespace SCIRun

#endif // Containers_FData_h
