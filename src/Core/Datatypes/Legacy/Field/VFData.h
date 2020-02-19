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


#ifndef CORE_DATATYPES_VFDATA_H
#define CORE_DATATYPES_VFDATA_H 1

#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/Containers/Array2.h>
#include <Core/Containers/Array3.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/CastFData.h>
#include <string>
#include <vector>
#include <complex>
#include <float.h>

#include <Core/Datatypes/Legacy/Field/share.h>

#define VFDATA_ACCESS_DECLARATION(type) \
  virtual void get_value(type &val, VMesh::index_type idx) const; \
  virtual void set_value(const type &val, VMesh::index_type idx); \
  virtual void get_evalue(type &val, VMesh::index_type idx) const; \
  virtual void set_evalue(const type &val, VMesh::index_type idx); \
  virtual void get_values(type *ptr, VMesh::size_type sz, VMesh::size_type offset) const; \
  virtual void set_values(const type *ptr, VMesh::size_type sz, VMesh::size_type offset); \
  virtual void get_evalues(type *ptr, VMesh::size_type sz, VMesh::size_type offset) const; \
  virtual void set_evalues(const type *ptr, VMesh::size_type sz, VMesh::size_type offset); \
  virtual void set_all_values(const type &val); \
  virtual void get_weighted_value(type &val, const VMesh::index_type* idx, const VMesh::weight_type* w, VMesh::size_type sz) const;  \
  virtual void get_weighted_evalue(type &val, const VMesh::index_type* idx, const VMesh::weight_type* w, VMesh::size_type sz) const; \
  virtual void get_values(type *ptr, VMesh::Node::array_type& nodes) const; \
  virtual void get_values(type *ptr, VMesh::Elem::array_type& elems) const; \
  virtual void set_values(const type *ptr, VMesh::Node::array_type& nodes); \
  virtual void set_values(const type *ptr, VMesh::Elem::array_type& elems); \
  virtual void get_values(type *ptr, index_type* idx, size_type size) const; \
  virtual void set_values(const type *ptr, index_type* idx, size_type size); \


#define VFDATA_ACCESS_DECLARATION2(type) \
  virtual void interpolate(type &val, VMesh::ElemInterpolate &interp, type defval = (static_cast<type>(0))) const; \
  virtual void minterpolate(std::vector<type> &val, VMesh::MultiElemInterpolate &interp, type defval = (static_cast<type>(0))) const; \
  virtual void gradient(StackVector<type,3> &val, VMesh::ElemGradient &interp, type defval = (static_cast<type>(0))) const; \
  virtual void mgradient(std::vector<StackVector<type,3> > &val, VMesh::MultiElemGradient &interp, type defval = (static_cast<type>(0))) const; \

#define VFDATA_FUNCTION_DECLARATION(type) \
  SCISHARE VFData* CreateVFData(std::vector<type>& fdata, std::vector<type>& lfdata, std::vector<std::vector<type> >& hfdata); \
  SCISHARE VFData* CreateVFData(Array2<type>& fdata, std::vector<type>& lfdata, std::vector<std::vector<type> >& hfdata); \
  SCISHARE VFData* CreateVFData(Array3<type>& fdata, std::vector<type>& lfdata, std::vector<std::vector<type> >& hfdata);


namespace SCIRun {

/// Interface class to data stored in field.
/// The VField class has a pointer to this class and hence can call the
/// appropriate function from the table listed by this class. This class
/// works with references to the original class and hence the additional
/// overhead should be small.
class SCISHARE VFData {
public:
  virtual ~VFData() {}

  virtual VMesh::size_type fdata_size() const;
  virtual VMesh::size_type efdata_size() const;

  virtual void resize_fdata(VMesh::dimension_type dim);
  virtual void resize_efdata(VMesh::dimension_type dim);

  virtual void* fdata_pointer() const;
  virtual void* efdata_pointer() const;

  VFDATA_ACCESS_DECLARATION(char)
  VFDATA_ACCESS_DECLARATION(unsigned char)
  VFDATA_ACCESS_DECLARATION(short)
  VFDATA_ACCESS_DECLARATION(unsigned short)
  VFDATA_ACCESS_DECLARATION(int)
  VFDATA_ACCESS_DECLARATION(unsigned int)
  VFDATA_ACCESS_DECLARATION(long long)
  VFDATA_ACCESS_DECLARATION(unsigned long long)
  VFDATA_ACCESS_DECLARATION(float)
  VFDATA_ACCESS_DECLARATION(double)
  VFDATA_ACCESS_DECLARATION(std::complex<double>)
  VFDATA_ACCESS_DECLARATION(Core::Geometry::Vector)
  VFDATA_ACCESS_DECLARATION(Core::Geometry::Tensor)


  VFDATA_ACCESS_DECLARATION2(char)
  VFDATA_ACCESS_DECLARATION2(int)
  VFDATA_ACCESS_DECLARATION2(float)
  VFDATA_ACCESS_DECLARATION2(double)
  VFDATA_ACCESS_DECLARATION2(std::complex<double>)
  VFDATA_ACCESS_DECLARATION2(Core::Geometry::Vector)
  VFDATA_ACCESS_DECLARATION2(Core::Geometry::Tensor)


  /// Copy a value without needing to know the type
  virtual void copy_value(VFData* fdata,
                          VMesh::index_type vidx,
                          VMesh::index_type idx);

  virtual void copy_values(VFData* fdata,
                           VMesh::index_type vidx,
                           VMesh::index_type idx,
                           VMesh::size_type num);

  /// Copy a weighted value without needing to know the type
  virtual void copy_weighted_value(VFData* fdata,
                                   const VMesh::index_type* vidx,
                                   const VMesh::weight_type* vw,
                                   VMesh::size_type sz,
                                   VMesh::index_type idx);

  /// Copy a edge value without needing to know the type
  virtual void copy_evalue(VFData* fdata,
                           VMesh::index_type vidx,
                           VMesh::index_type idx);

  virtual void copy_evalues(VFData* fdata,
                            VMesh::index_type vidx,
                            VMesh::index_type idx,
                            VMesh::size_type num);

  /// Copy a weighted edge value without needing to know the type
  virtual void copy_weighted_evalue(VFData* fdata,
                                    const VMesh::index_type* vidx,
                                    const VMesh::weight_type* vw,
                                    VMesh::size_type sz,
                                    VMesh::index_type idx);

  /// Copy values from one FData array to another FData array
  virtual void copy_values(VFData* fdata);
  virtual void copy_evalues(VFData* fdata);

  virtual bool min(double& val, VMesh::index_type& idx) const;
  virtual bool max(double& val, VMesh::index_type& idx) const;
  virtual bool minmax(double& min, VMesh::index_type& idxmin,
                      double& max, VMesh::index_type& idxmax) const;

  virtual VMesh::size_type size();
};



// Functions to instantiate the FData interface

/// In case we encounter an unknow datatype.
/// We could have this function return a fuly functional interface, in which
/// case after the field is created in the dynamic compilation process it would
/// have this part of the virtual interface. It would be compiled with the first
/// instantiation of the field. However for the moment we ignore that case and
/// do not provide virtual interfaces for unknown data types.
template<class FDATA, class LFDATA, class HFDATA>
inline VFData* CreateVFData(FDATA& /*fdata*/, LFDATA& /*lfdata*/, HFDATA& /*hfdata*/)
{
  return nullptr;
}

/// Pre instantiated versions, these should cover most of SCIRun needs
/// As these are declared as functions, dynamic compilation should not instantiate
/// these once more, but link agianst the dynamic library. This should reduce the
/// amount of time spend in dynamic compilation and keep dynamically compiled files
/// small while providing a full virtual framework in parallel.


VFDATA_FUNCTION_DECLARATION(char)
VFDATA_FUNCTION_DECLARATION(unsigned char)
VFDATA_FUNCTION_DECLARATION(short)
VFDATA_FUNCTION_DECLARATION(unsigned short)
VFDATA_FUNCTION_DECLARATION(int)
VFDATA_FUNCTION_DECLARATION(unsigned int)
VFDATA_FUNCTION_DECLARATION(long long)
VFDATA_FUNCTION_DECLARATION(unsigned long long)
VFDATA_FUNCTION_DECLARATION(float)
VFDATA_FUNCTION_DECLARATION(double)
VFDATA_FUNCTION_DECLARATION(std::complex<double>)
VFDATA_FUNCTION_DECLARATION(Core::Geometry::Vector)
VFDATA_FUNCTION_DECLARATION(Core::Geometry::Tensor)


} // end namespace

#endif
