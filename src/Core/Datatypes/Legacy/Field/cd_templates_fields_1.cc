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


/// @todo Documentation Core/Datatypes/Legacy/Field/cd_templates_fields_1.cc

#include <Core/Persistent/PersistentSTL.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Basis/NoData.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/QuadBilinearLgn.h>
#include <Core/Datatypes/Legacy/Field/ImageMesh.h>
#include <Core/Datatypes/Legacy/Field/QuadSurfMesh.h>
#include <Core/Containers/FData.h>
#include <Core/Datatypes/Legacy/Field/GenericField.h>


using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Basis;


namespace SCIRun {

typedef ImageMesh<QuadBilinearLgn<Point> > IMesh;

template class ImageMesh<QuadBilinearLgn<Point> >;

//NoData
typedef NoDataBasis<double>               NDBasis;

//Constant
typedef ConstantBasis<Tensor>             CFDTensorBasis;
typedef ConstantBasis<Vector>             CFDVectorBasis;
typedef ConstantBasis<double>             CFDdoubleBasis;
typedef ConstantBasis<float>              CFDfloatBasis;
typedef ConstantBasis<complex>            CFDcomplexBasis;
typedef ConstantBasis<int>                CFDintBasis;
typedef ConstantBasis<long long>          CFDlonglongBasis;
typedef ConstantBasis<short>              CFDshortBasis;
typedef ConstantBasis<char>               CFDcharBasis;
typedef ConstantBasis<unsigned int>       CFDuintBasis;
typedef ConstantBasis<unsigned short>     CFDushortBasis;
typedef ConstantBasis<unsigned char>      CFDucharBasis;
typedef ConstantBasis<unsigned long>      CFDulongBasis;

//Linear
typedef QuadBilinearLgn<Tensor>             FDTensorBasis;
typedef QuadBilinearLgn<Vector>             FDVectorBasis;
typedef QuadBilinearLgn<double>             FDdoubleBasis;
typedef QuadBilinearLgn<float>              FDfloatBasis;
typedef QuadBilinearLgn<complex>            FDcomplexBasis;
typedef QuadBilinearLgn<int>                FDintBasis;
typedef QuadBilinearLgn<long long>          FDlonglongBasis;
typedef QuadBilinearLgn<short>              FDshortBasis;
typedef QuadBilinearLgn<char>               FDcharBasis;
typedef QuadBilinearLgn<unsigned int>       FDuintBasis;
typedef QuadBilinearLgn<unsigned short>     FDushortBasis;
typedef QuadBilinearLgn<unsigned char>      FDucharBasis;
typedef QuadBilinearLgn<unsigned long>      FDulongBasis;


//NoData
template class GenericField<IMesh, NDBasis, FData2d<double, IMesh> >;
template class GenericField<IMesh, NoDataBasis<int>, FData2d<int, IMesh> >;

//Constant
template class GenericField<IMesh, CFDTensorBasis, FData2d<Tensor, IMesh> >;
template class GenericField<IMesh, CFDVectorBasis, FData2d<Vector, IMesh> >;
template class GenericField<IMesh, CFDdoubleBasis, FData2d<double, IMesh> >;
template class GenericField<IMesh, CFDfloatBasis,  FData2d<float, IMesh> >;
template class GenericField<IMesh, CFDcomplexBasis, FData2d<complex, IMesh> >;
template class GenericField<IMesh, CFDintBasis,    FData2d<int, IMesh> >;
template class GenericField<IMesh, CFDlonglongBasis, FData2d<long long, IMesh> >;
template class GenericField<IMesh, CFDshortBasis,  FData2d<short, IMesh> >;
template class GenericField<IMesh, CFDcharBasis,   FData2d<char, IMesh> >;
template class GenericField<IMesh, CFDuintBasis,   FData2d<unsigned int, IMesh> >;
template class GenericField<IMesh, CFDushortBasis, FData2d<unsigned short, IMesh> >;
template class GenericField<IMesh, CFDucharBasis,  FData2d<unsigned char, IMesh> >;
template class GenericField<IMesh, CFDulongBasis,  FData2d<unsigned long, IMesh> >;

//Linear
template class GenericField<IMesh, FDTensorBasis, FData2d<Tensor, IMesh> >;
template class GenericField<IMesh, FDVectorBasis, FData2d<Vector, IMesh> >;
template class GenericField<IMesh, FDdoubleBasis, FData2d<double, IMesh> >;
template class GenericField<IMesh, FDfloatBasis,  FData2d<float, IMesh> >;
template class GenericField<IMesh, FDcomplexBasis, FData2d<complex, IMesh> >;
template class GenericField<IMesh, FDintBasis,    FData2d<int, IMesh> >;
template class GenericField<IMesh, FDlonglongBasis, FData2d<long long, IMesh> >;
template class GenericField<IMesh, FDshortBasis,  FData2d<short, IMesh> >;
template class GenericField<IMesh, FDcharBasis,   FData2d<char, IMesh> >;
template class GenericField<IMesh, FDuintBasis,   FData2d<unsigned int, IMesh> >;
template class GenericField<IMesh, FDushortBasis, FData2d<unsigned short, IMesh> >;
template class GenericField<IMesh, FDucharBasis,  FData2d<unsigned char, IMesh> >;
template class GenericField<IMesh, FDulongBasis,  FData2d<unsigned long, IMesh> >;
}

PersistentTypeID backwards_compat_IM("ImageMesh", "Mesh",
  IMesh::maker, IMesh::maker);

PersistentTypeID
backwards_compat_IFT("ImageField<Tensor>", "Field",
		     GenericField<IMesh, FDTensorBasis,
		     FData2d<Tensor, IMesh> >::maker,
		     GenericField<IMesh, CFDTensorBasis,
		     FData2d<Tensor, IMesh> >::maker);
PersistentTypeID
backwards_compat_IFV("ImageField<Vector>", "Field",
		     GenericField<IMesh, FDVectorBasis,
		     FData2d<Vector, IMesh> >::maker,
		     GenericField<IMesh, CFDVectorBasis,
		     FData2d<Vector, IMesh> >::maker);
PersistentTypeID
backwards_compat_IFd("ImageField<double>", "Field",
		     GenericField<IMesh, FDdoubleBasis,
		     FData2d<double, IMesh> >::maker,
		     GenericField<IMesh, CFDdoubleBasis,
		     FData2d<double, IMesh> >::maker,
		     GenericField<IMesh, NDBasis,
		     FData2d<double, IMesh> >::maker);
PersistentTypeID
backwards_compat_IFf("ImageField<float>", "Field",
		     GenericField<IMesh, FDfloatBasis,
		     FData2d<float, IMesh> >::maker,
		     GenericField<IMesh, CFDfloatBasis,
		     FData2d<float, IMesh> >::maker);
PersistentTypeID
backwards_compat_IFco("ImageField<complex>", "Field",
         GenericField<IMesh, FDcomplexBasis,
         FData2d<complex, IMesh> >::maker,
         GenericField<IMesh, CFDcomplexBasis,
         FData2d<complex, IMesh> >::maker);
PersistentTypeID
backwards_compat_IFi("ImageField<int>", "Field",
		     GenericField<IMesh, FDintBasis,
		     FData2d<int, IMesh> >::maker,
		     GenericField<IMesh, CFDintBasis,
		     FData2d<int, IMesh> >::maker);
PersistentTypeID
backwards_compat_IFs("ImageField<short>", "Field",
		     GenericField<IMesh, FDshortBasis,
		     FData2d<short, IMesh> >::maker,
		     GenericField<IMesh, CFDshortBasis,
		     FData2d<short, IMesh> >::maker);
PersistentTypeID
backwards_compat_IFc("ImageField<char>", "Field",
		     GenericField<IMesh, FDcharBasis,
		     FData2d<char, IMesh> >::maker,
		     GenericField<IMesh, CFDcharBasis,
		     FData2d<char, IMesh> >::maker);
PersistentTypeID
backwards_compat_IFui("ImageField<unsigned_int>", "Field",
		      GenericField<IMesh, FDuintBasis,
		      FData2d<unsigned int, IMesh> >::maker,
		      GenericField<IMesh, CFDuintBasis,
		      FData2d<unsigned int, IMesh> >::maker);
PersistentTypeID
backwards_compat_IFus("ImageField<unsigned_short>", "Field",
		      GenericField<IMesh, FDushortBasis,
		      FData2d<unsigned short, IMesh> >::maker,
		      GenericField<IMesh, CFDushortBasis,
		      FData2d<unsigned short, IMesh> >::maker);
PersistentTypeID
backwards_compat_IFuc("ImageField<unsigned_char>", "Field",
		      GenericField<IMesh, FDucharBasis,
		      FData2d<unsigned char, IMesh> >::maker,
		      GenericField<IMesh, CFDucharBasis,
		      FData2d<unsigned char, IMesh> >::maker);
PersistentTypeID
backwards_compat_IFul("ImageField<unsigned_long>", "Field",
		      GenericField<IMesh, FDulongBasis,
		      FData2d<unsigned long, IMesh> >::maker,
		      GenericField<IMesh, CFDulongBasis,
		      FData2d<unsigned long, IMesh> >::maker);
