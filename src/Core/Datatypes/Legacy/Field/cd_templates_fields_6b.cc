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
/// Manual template instantiations
///


///
/// These aren't used by Datatypes directly, but since they are used in
/// a lot of different modules, we instantiate them here to avoid bloat
///
/// Find the bloaters with:
///   find . -name "*.ii" -print | xargs cat | sort | uniq -c | sort -nr | more
///

#include <Core/Basis/NoData.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Basis/HexTrilinearLgn.h>
#include <Core/Basis/QuadBilinearLgn.h>
#include <Core/Datatypes/Legacy/Field/StructCurveMesh.h>
#include <Core/Datatypes/Legacy/Field/StructQuadSurfMesh.h>
#include <Core/Datatypes/Legacy/Field/StructHexVolMesh.h>
#include <Core/Containers/FData.h>
#include <Core/Datatypes/Legacy/Field/GenericField.h>

using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;

namespace SCIRun {

//NoData
typedef NoDataBasis<double>                 NDBasis;

//Linear
typedef ConstantBasis<Tensor>                CFDTensorBasis;
typedef ConstantBasis<Vector>                CFDVectorBasis;
typedef ConstantBasis<double>                CFDdoubleBasis;
typedef ConstantBasis<float>                 CFDfloatBasis;
typedef ConstantBasis<complex>               CFDcomplexBasis;
typedef ConstantBasis<int>                   CFDintBasis;
typedef ConstantBasis<long long>             CFDlonglongBasis;
typedef ConstantBasis<short>                 CFDshortBasis;
typedef ConstantBasis<char>                  CFDcharBasis;
typedef ConstantBasis<unsigned int>          CFDuintBasis;
typedef ConstantBasis<unsigned short>        CFDushortBasis;
typedef ConstantBasis<unsigned char>         CFDucharBasis;
typedef ConstantBasis<unsigned long>         CFDulongBasis;

//Linear
typedef CrvLinearLgn<Tensor>                FDCrTensorBasis;
typedef CrvLinearLgn<Vector>                FDCrVectorBasis;
typedef CrvLinearLgn<double>                FDCrdoubleBasis;
typedef CrvLinearLgn<float>                 FDCrfloatBasis;
typedef CrvLinearLgn<complex>               FDCrcomplexBasis;
typedef CrvLinearLgn<int>                   FDCrintBasis;
typedef CrvLinearLgn<long long>             FDCrlonglongBasis;
typedef CrvLinearLgn<short>                 FDCrshortBasis;
typedef CrvLinearLgn<char>                  FDCrcharBasis;
typedef CrvLinearLgn<unsigned int>          FDCruintBasis;
typedef CrvLinearLgn<unsigned short>        FDCrushortBasis;
typedef CrvLinearLgn<unsigned char>         FDCrucharBasis;
typedef CrvLinearLgn<unsigned long>         FDCrulongBasis;

//Linear
typedef QuadBilinearLgn<Tensor>                FDQTensorBasis;
typedef QuadBilinearLgn<Vector>                FDQVectorBasis;
typedef QuadBilinearLgn<double>                FDQdoubleBasis;
typedef QuadBilinearLgn<float>                 FDQfloatBasis;
typedef QuadBilinearLgn<complex>               FDQcomplexBasis;
typedef QuadBilinearLgn<int>                   FDQintBasis;
typedef QuadBilinearLgn<long long>             FDQlonglongBasis;
typedef QuadBilinearLgn<short>                 FDQshortBasis;
typedef QuadBilinearLgn<char>                  FDQcharBasis;
typedef QuadBilinearLgn<unsigned int>          FDQuintBasis;
typedef QuadBilinearLgn<unsigned short>        FDQushortBasis;
typedef QuadBilinearLgn<unsigned char>         FDQucharBasis;
typedef QuadBilinearLgn<unsigned long>         FDQulongBasis;

typedef StructQuadSurfMesh<QuadBilinearLgn<Point> > SQMesh;
PersistentTypeID backwards_compat_SQM("StructQuadSurfMesh", "Mesh",
				      SQMesh::maker, SQMesh::maker);
template class StructQuadSurfMesh<QuadBilinearLgn<Point> >;

//NoData
template class GenericField<SQMesh, NDBasis, FData2d<double,SQMesh> >;

//Constant
template class GenericField<SQMesh, CFDTensorBasis, FData2d<Tensor,SQMesh> >;
template class GenericField<SQMesh, CFDVectorBasis, FData2d<Vector,SQMesh> >;
template class GenericField<SQMesh, CFDdoubleBasis, FData2d<double,SQMesh> >;
template class GenericField<SQMesh, CFDfloatBasis,  FData2d<float,SQMesh> >;
template class GenericField<SQMesh, CFDcomplexBasis, FData2d<complex, SQMesh> >;
template class GenericField<SQMesh, CFDintBasis,    FData2d<int,SQMesh> >;
template class GenericField<SQMesh, CFDlonglongBasis,FData2d<long long,SQMesh> >;
template class GenericField<SQMesh, CFDshortBasis,  FData2d<short,SQMesh> >;
template class GenericField<SQMesh, CFDcharBasis,   FData2d<char,SQMesh> >;
template class GenericField<SQMesh, CFDuintBasis,   FData2d<unsigned int,SQMesh> >;
template class GenericField<SQMesh, CFDushortBasis, FData2d<unsigned short,SQMesh> >;
template class GenericField<SQMesh, CFDucharBasis,  FData2d<unsigned char,SQMesh> >;
template class GenericField<SQMesh, CFDulongBasis,  FData2d<unsigned long,SQMesh> >;

//Linear
template class GenericField<SQMesh, FDQTensorBasis, FData2d<Tensor,SQMesh> >;
template class GenericField<SQMesh, FDQVectorBasis, FData2d<Vector,SQMesh> >;
template class GenericField<SQMesh, FDQdoubleBasis, FData2d<double,SQMesh> >;
template class GenericField<SQMesh, FDQfloatBasis,  FData2d<float,SQMesh> >;
template class GenericField<SQMesh, FDQcomplexBasis, FData2d<complex, SQMesh> >;
template class GenericField<SQMesh, FDQintBasis,    FData2d<int,SQMesh> >;
template class GenericField<SQMesh, FDQlonglongBasis,FData2d<long long,SQMesh> >;
template class GenericField<SQMesh, FDQshortBasis,  FData2d<short,SQMesh> >;
template class GenericField<SQMesh, FDQcharBasis,   FData2d<char,SQMesh> >;
template class GenericField<SQMesh, FDQuintBasis,   FData2d<unsigned int,SQMesh> >;
template class GenericField<SQMesh, FDQushortBasis, FData2d<unsigned short,SQMesh> >;
template class GenericField<SQMesh, FDQucharBasis,  FData2d<unsigned char,SQMesh> >;
template class GenericField<SQMesh, FDQulongBasis,  FData2d<unsigned long,SQMesh> >;

PersistentTypeID
backwards_compat_SQSFT("StructQuadSurfField<Tensor>", "Field",
		       GenericField<SQMesh, FDQTensorBasis,
		       FData2d<Tensor, SQMesh> >::maker,
		       GenericField<SQMesh, CFDTensorBasis,
		       FData2d<Tensor, SQMesh> >::maker);
PersistentTypeID
backwards_compat_SQSFV("StructQuadSurfField<Vector>", "Field",
		       GenericField<SQMesh, FDQVectorBasis,
		       FData2d<Vector, SQMesh> >::maker,
		       GenericField<SQMesh, CFDVectorBasis,
		       FData2d<Vector, SQMesh> >::maker);
PersistentTypeID
backwards_compat_SQSFd("StructQuadSurfField<double>", "Field",
		       GenericField<SQMesh, FDQdoubleBasis,
		       FData2d<double, SQMesh> >::maker,
		       GenericField<SQMesh, CFDdoubleBasis,
		       FData2d<double, SQMesh> >::maker,
		       GenericField<SQMesh, NDBasis,
		       FData2d<double, SQMesh> >::maker);
PersistentTypeID
backwards_compat_SQSFf("StructQuadSurfField<float>", "Field",
		       GenericField<SQMesh, FDQfloatBasis,
		       FData2d<float, SQMesh> >::maker,
		       GenericField<SQMesh, CFDfloatBasis,
		       FData2d<float, SQMesh> >::maker);
PersistentTypeID
backwards_compat_SQSFco("StructQuadSurfField<complex>", "Field",
          GenericField<SQMesh, FDQcomplexBasis,
          FData2d<complex, SQMesh> >::maker,
          GenericField<SQMesh, CFDcomplexBasis,
          FData2d<complex, SQMesh> >::maker);
PersistentTypeID
backwards_compat_SQSFi("StructQuadSurfField<int>", "Field",
		       GenericField<SQMesh, FDQintBasis,
		       FData2d<int, SQMesh> >::maker,
		       GenericField<SQMesh, CFDintBasis,
		       FData2d<int, SQMesh> >::maker);
PersistentTypeID
backwards_compat_SQSFs("StructQuadSurfField<short>", "Field",
		       GenericField<SQMesh, FDQshortBasis,
		       FData2d<short, SQMesh> >::maker,
		       GenericField<SQMesh, CFDshortBasis,
		       FData2d<short, SQMesh> >::maker);
PersistentTypeID
backwards_compat_SQSFc("StructQuadSurfField<char>", "Field",
		       GenericField<SQMesh, FDQcharBasis,
		       FData2d<char, SQMesh> >::maker,
		       GenericField<SQMesh, CFDcharBasis,
		       FData2d<char, SQMesh> >::maker);
PersistentTypeID
backwards_compat_SQSFui("StructQuadSurfField<unsigned_int>", "Field",
			GenericField<SQMesh, FDQuintBasis,
			FData2d<unsigned int, SQMesh> >::maker,
			GenericField<SQMesh, CFDuintBasis,
			FData2d<unsigned int, SQMesh> >::maker);
PersistentTypeID
backwards_compat_SQSFus("StructQuadSurfField<unsigned_short>", "Field",
			GenericField<SQMesh, FDQushortBasis,
			FData2d<unsigned short, SQMesh> >::maker,
			GenericField<SQMesh, CFDushortBasis,
			FData2d<unsigned short, SQMesh> >::maker);
PersistentTypeID
backwards_compat_SQSFuc("StructQuadSurfField<unsigned_char>", "Field",
			GenericField<SQMesh, FDQucharBasis,
			FData2d<unsigned char, SQMesh> >::maker,
			GenericField<SQMesh, CFDucharBasis,
			FData2d<unsigned char, SQMesh> >::maker);
PersistentTypeID
backwards_compat_SQSFul("StructQuadSurfField<unsigned_long>", "Field",
			GenericField<SQMesh, FDQulongBasis,
			FData2d<unsigned long, SQMesh> >::maker,
			GenericField<SQMesh, CFDulongBasis,
			FData2d<unsigned long, SQMesh> >::maker);
}
