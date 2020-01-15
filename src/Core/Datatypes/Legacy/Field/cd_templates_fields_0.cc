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


/// @todo Documentation Core/Datatypes/Legacy/Field/cd_templates_fields_0.cc

#include <Core/Persistent/PersistentSTL.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/NoData.h>
#include <Core/Basis/HexTrilinearLgn.h>
#include <Core/Datatypes/Legacy/Field/LatVolMesh.h>
#include <Core/Containers/FData.h>
#include <Core/Datatypes/Legacy/Field/GenericField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Basis;

typedef LatVolMesh<HexTrilinearLgn<Point> > LVMesh;

typedef HexTrilinearLgn<Tensor>             FDTensorBasis;
typedef HexTrilinearLgn<Vector>             FDVectorBasis;
typedef HexTrilinearLgn<double>             FDdoubleBasis;
typedef HexTrilinearLgn<std::complex<double>>       FDComplexBasis;
typedef HexTrilinearLgn<float>              FDfloatBasis;
typedef HexTrilinearLgn<int>                FDintBasis;
typedef HexTrilinearLgn<long long>          FDlonglongBasis;
typedef HexTrilinearLgn<short>              FDshortBasis;
typedef HexTrilinearLgn<char>               FDcharBasis;
typedef HexTrilinearLgn<unsigned int>       FDuintBasis;
typedef HexTrilinearLgn<unsigned short>     FDushortBasis;
typedef HexTrilinearLgn<unsigned char>      FDucharBasis;
typedef HexTrilinearLgn<unsigned long>      FDulongBasis;

typedef ConstantBasis<Tensor>             CFDTensorBasis;
typedef ConstantBasis<Vector>             CFDVectorBasis;
typedef ConstantBasis<double>             CFDdoubleBasis;
typedef ConstantBasis<std::complex<double>>             CFComplexdoubleBasis;
typedef ConstantBasis<float>              CFDfloatBasis;
typedef ConstantBasis<int>                CFDintBasis;
typedef ConstantBasis<long long>          CFDlonglongBasis;
typedef ConstantBasis<short>              CFDshortBasis;
typedef ConstantBasis<char>               CFDcharBasis;
typedef ConstantBasis<unsigned int>       CFDuintBasis;
typedef ConstantBasis<unsigned short>     CFDushortBasis;
typedef ConstantBasis<unsigned char>      CFDucharBasis;
typedef ConstantBasis<unsigned long>      CFDulongBasis;

typedef NoDataBasis<double>             NDBasis;

namespace SCIRun
{
  template class LatVolMesh<HexTrilinearLgn<Point> >;

  //NoData
  template class GenericField<LVMesh, NoDataBasis<double>,  FData3d<double, LVMesh> >;
  template class GenericField<LVMesh, NoDataBasis<int>,  FData3d<int, LVMesh> >;

  //Constant
  template class GenericField<LVMesh, CFDTensorBasis,  FData3d<Tensor, LVMesh> >;
  template class GenericField<LVMesh, CFDVectorBasis,  FData3d<Vector, LVMesh> >;
  template class GenericField<LVMesh, CFComplexdoubleBasis, FData3d<std::complex<double>, LVMesh> >;
  template class GenericField<LVMesh, CFDdoubleBasis, FData3d<double, LVMesh> >;
  template class GenericField<LVMesh, CFDfloatBasis,   FData3d<float, LVMesh> >;
  template class GenericField<LVMesh, CFDintBasis,     FData3d<int, LVMesh> >;
  template class GenericField<LVMesh, CFDlonglongBasis,FData3d<long long, LVMesh> >;
  template class GenericField<LVMesh, CFDshortBasis,   FData3d<short, LVMesh> >;
  template class GenericField<LVMesh, CFDcharBasis,    FData3d<char, LVMesh> >;
  template class GenericField<LVMesh, CFDuintBasis,
    FData3d<unsigned int, LVMesh> >;
  template class GenericField<LVMesh, CFDushortBasis,
    FData3d<unsigned short, LVMesh> >;
  template class GenericField<LVMesh, CFDucharBasis,
    FData3d<unsigned char, LVMesh> >;
  template class GenericField<LVMesh, CFDulongBasis,
    FData3d<unsigned long, LVMesh> >;

  //Linear
  template class GenericField<LVMesh, FDTensorBasis,   FData3d<Tensor, LVMesh> >;
  template class GenericField<LVMesh, FDVectorBasis,   FData3d<Vector, LVMesh> >;
  template class GenericField<LVMesh, FDdoubleBasis,   FData3d<double, LVMesh> >;
  template class GenericField<LVMesh, FDComplexBasis,  FData3d<std::complex<double>, LVMesh> >;
  template class GenericField<LVMesh, FDfloatBasis,    FData3d<float, LVMesh> >;
  template class GenericField<LVMesh, FDintBasis,      FData3d<int, LVMesh> >;
  template class GenericField<LVMesh, FDlonglongBasis, FData3d<long long, LVMesh> >;
  template class GenericField<LVMesh, FDshortBasis,    FData3d<short, LVMesh> >;
  template class GenericField<LVMesh, FDcharBasis,     FData3d<char, LVMesh> >;
  template class GenericField<LVMesh, FDuintBasis,     FData3d<unsigned int, LVMesh> >;
  template class GenericField<LVMesh, FDushortBasis,   FData3d<unsigned short, LVMesh> >;
  template class GenericField<LVMesh, FDucharBasis,    FData3d<unsigned char, LVMesh> >;
  template class GenericField<LVMesh, FDulongBasis,    FData3d<unsigned long, LVMesh> >;
}

PersistentTypeID backwards_compat_LVM("LatVolMesh", "Mesh",
  LVMesh::maker,
  LVMesh::maker);

PersistentTypeID
backwards_compat_LVFT("LatVolField<Tensor>", "Field",
		      GenericField<LVMesh, FDTensorBasis,
		      FData3d<Tensor, LVMesh> >::maker,
		      GenericField<LVMesh, CFDTensorBasis,
		      FData3d<Tensor, LVMesh> >::maker);
PersistentTypeID
backwards_compat_LVFV("LatVolField<Vector>", "Field",
		      GenericField<LVMesh, FDVectorBasis,
		      FData3d<Vector, LVMesh> >::maker,
		      GenericField<LVMesh, CFDVectorBasis,
		      FData3d<Vector, LVMesh> >::maker);
PersistentTypeID
backwards_compat_LVFd("LatVolField<double>", "Field",
		      GenericField<LVMesh, FDdoubleBasis,
		      FData3d<double, LVMesh> >::maker,
		      GenericField<LVMesh, CFDdoubleBasis,
		      FData3d<double, LVMesh> >::maker,
		      GenericField<LVMesh, NDBasis,
		      FData3d<double, LVMesh> >::maker);
PersistentTypeID
backwards_compat_LVFcd("LatVolField<complex>", "Field",
          GenericField<LVMesh, FDComplexBasis,
          FData3d<std::complex<double>, LVMesh> >::maker,
          GenericField<LVMesh, CFComplexdoubleBasis,
          FData3d<std::complex<double>, LVMesh> >::maker,
          GenericField<LVMesh, NDBasis,
          FData3d<std::complex<double>, LVMesh> >::maker);
PersistentTypeID
backwards_compat_LVFf("LatVolField<float>", "Field",
		      GenericField<LVMesh, FDfloatBasis,
		      FData3d<float, LVMesh> >::maker,
		      GenericField<LVMesh, CFDfloatBasis,
		      FData3d<float, LVMesh> >::maker);
PersistentTypeID
backwards_compat_LVFi("LatVolField<int>", "Field",
		      GenericField<LVMesh, FDintBasis,
		      FData3d<int, LVMesh> >::maker,
		      GenericField<LVMesh, CFDintBasis,
		      FData3d<int, LVMesh> >::maker);
PersistentTypeID
backwards_compat_LVFs("LatVolField<short>", "Field",
		      GenericField<LVMesh, FDshortBasis,
		      FData3d<short, LVMesh> >::maker,
		      GenericField<LVMesh, CFDshortBasis,
		      FData3d<short, LVMesh> >::maker);
PersistentTypeID
backwards_compat_LVFc("LatVolField<char>", "Field",
		      GenericField<LVMesh, FDcharBasis,
		      FData3d<char, LVMesh> >::maker,
		      GenericField<LVMesh, CFDcharBasis,
		      FData3d<char, LVMesh> >::maker);
PersistentTypeID
backwards_compat_LVFui("LatVolField<unsigned_int>", "Field",
		       GenericField<LVMesh, FDuintBasis,
		       FData3d<unsigned int, LVMesh> >::maker,
		       GenericField<LVMesh, CFDuintBasis,
		       FData3d<unsigned int, LVMesh> >::maker);
PersistentTypeID
backwards_compat_LVFus("LatVolField<unsigned_short>", "Field",
		       GenericField<LVMesh, FDushortBasis,
		       FData3d<unsigned short, LVMesh> >::maker,
		       GenericField<LVMesh, CFDushortBasis,
		       FData3d<unsigned short, LVMesh> >::maker);
PersistentTypeID
backwards_compat_LVFuc("LatVolField<unsigned_char>", "Field",
		       GenericField<LVMesh, FDucharBasis,
		       FData3d<unsigned char, LVMesh> >::maker,
		       GenericField<LVMesh, CFDucharBasis,
		       FData3d<unsigned char, LVMesh> >::maker);
PersistentTypeID
backwards_compat_LVFul("LatVolField<unsigned_long>", "Field",
		       GenericField<LVMesh, FDulongBasis,
		       FData3d<unsigned long, LVMesh> >::maker,
		       GenericField<LVMesh, CFDulongBasis,
		       FData3d<unsigned long, LVMesh> >::maker);
