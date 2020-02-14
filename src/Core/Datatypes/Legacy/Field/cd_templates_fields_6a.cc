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
///@brief Manual template instantiations
///


////
///  These aren't used by Datatypes directly, but since they are used in
///  a lot of different modules, we instantiate them here to avoid bloat
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

//Linear
typedef HexTrilinearLgn<Tensor>                FDHTensorBasis;
typedef HexTrilinearLgn<Vector>                FDHVectorBasis;
typedef HexTrilinearLgn<double>                FDHdoubleBasis;
typedef HexTrilinearLgn<float>                 FDHfloatBasis;
typedef HexTrilinearLgn<complex>               FDHcomplexBasis;
typedef HexTrilinearLgn<int>                   FDHintBasis;
typedef HexTrilinearLgn<long long>             FDHlonglongBasis;
typedef HexTrilinearLgn<short>                 FDHshortBasis;
typedef HexTrilinearLgn<char>                  FDHcharBasis;
typedef HexTrilinearLgn<unsigned int>          FDHuintBasis;
typedef HexTrilinearLgn<unsigned short>        FDHushortBasis;
typedef HexTrilinearLgn<unsigned char>         FDHucharBasis;
typedef HexTrilinearLgn<unsigned long>         FDHulongBasis;

typedef StructHexVolMesh<HexTrilinearLgn<Point> > SHMesh;
PersistentTypeID backwards_compat_SHVM("StructHexVolMesh", "Mesh",
				       SHMesh::maker, SHMesh::maker);
template class StructHexVolMesh<HexTrilinearLgn<Point> >;

//NoData
template class GenericField<SHMesh, NDBasis, FData3d<double,SHMesh> >;

//Constant
template class GenericField<SHMesh, CFDTensorBasis, FData3d<Tensor,SHMesh> >;
template class GenericField<SHMesh, CFDVectorBasis, FData3d<Vector,SHMesh> >;
template class GenericField<SHMesh, CFDdoubleBasis, FData3d<double,SHMesh> >;
template class GenericField<SHMesh, CFDfloatBasis,  FData3d<float,SHMesh> >;
template class GenericField<SHMesh, CFDcomplexBasis, FData3d<complex, SHMesh> >;
template class GenericField<SHMesh, CFDintBasis,    FData3d<int,SHMesh> >;
template class GenericField<SHMesh, CFDlonglongBasis,FData3d<long long,SHMesh> >;
template class GenericField<SHMesh, CFDshortBasis,  FData3d<short,SHMesh> >;
template class GenericField<SHMesh, CFDcharBasis,   FData3d<char,SHMesh> >;
template class GenericField<SHMesh, CFDuintBasis,   FData3d<unsigned int,SHMesh> >;
template class GenericField<SHMesh, CFDushortBasis, FData3d<unsigned short,SHMesh> >;
template class GenericField<SHMesh, CFDucharBasis,  FData3d<unsigned char,SHMesh> >;
template class GenericField<SHMesh, CFDulongBasis,  FData3d<unsigned long,SHMesh> >;

//Linear
template class GenericField<SHMesh, FDHTensorBasis, FData3d<Tensor,SHMesh> >;
template class GenericField<SHMesh, FDHVectorBasis, FData3d<Vector,SHMesh> >;
template class GenericField<SHMesh, FDHdoubleBasis, FData3d<double,SHMesh> >;
template class GenericField<SHMesh, FDHfloatBasis,  FData3d<float,SHMesh> >;
template class GenericField<SHMesh, FDHcomplexBasis, FData3d<complex, SHMesh> >;
template class GenericField<SHMesh, FDHintBasis,    FData3d<int,SHMesh> >;
template class GenericField<SHMesh, FDHlonglongBasis,FData3d<long long,SHMesh> >;
template class GenericField<SHMesh, FDHshortBasis,  FData3d<short,SHMesh> >;
template class GenericField<SHMesh, FDHcharBasis,   FData3d<char,SHMesh> >;
template class GenericField<SHMesh, FDHuintBasis,   FData3d<unsigned int,SHMesh> >;
template class GenericField<SHMesh, FDHushortBasis, FData3d<unsigned short,SHMesh> >;
template class GenericField<SHMesh, FDHucharBasis,  FData3d<unsigned char,SHMesh> >;
template class GenericField<SHMesh, FDHulongBasis,  FData3d<unsigned long,SHMesh> >;


PersistentTypeID
backwards_compat_SHVFT("StructHexVolField<Tensor>", "Field",
		       GenericField<SHMesh, FDHTensorBasis,
		       FData3d<Tensor, SHMesh> >::maker,
		       GenericField<SHMesh, CFDTensorBasis,
		       FData3d<Tensor, SHMesh> >::maker);
PersistentTypeID
backwards_compat_SHVFV("StructHexVolField<Vector>", "Field",
		       GenericField<SHMesh, FDHVectorBasis,
		       FData3d<Vector, SHMesh> >::maker,
		       GenericField<SHMesh, CFDVectorBasis,
		       FData3d<Vector, SHMesh> >::maker);
PersistentTypeID
backwards_compat_SHVFd("StructHexVolField<double>", "Field",
		       GenericField<SHMesh, FDHdoubleBasis,
		       FData3d<double, SHMesh> >::maker,
		       GenericField<SHMesh, CFDdoubleBasis,
		       FData3d<double, SHMesh> >::maker,
		       GenericField<SHMesh, NDBasis,
		       FData3d<double, SHMesh> >::maker);
PersistentTypeID
backwards_compat_SHVFf("StructHexVolField<float>", "Field",
		       GenericField<SHMesh, FDHfloatBasis,
		       FData3d<float, SHMesh> >::maker,
		       GenericField<SHMesh, CFDfloatBasis,
		       FData3d<float, SHMesh> >::maker);
PersistentTypeID
          backwards_compat_SHVFco("StructHexVolField<complex>", "Field",
          GenericField<SHMesh, FDHcomplexBasis,
          FData3d<complex, SHMesh> >::maker,
          GenericField<SHMesh, CFDcomplexBasis,
          FData3d<complex, SHMesh> >::maker);
PersistentTypeID
backwards_compat_SHVFi("StructHexVolField<int>", "Field",
		       GenericField<SHMesh, FDHintBasis,
		       FData3d<int, SHMesh> >::maker,
		       GenericField<SHMesh, CFDintBasis,
		       FData3d<int, SHMesh> >::maker);
PersistentTypeID
backwards_compat_SHVFs("StructHexVolField<short>", "Field",
		       GenericField<SHMesh, FDHshortBasis,
		       FData3d<short, SHMesh> >::maker,
		       GenericField<SHMesh, CFDshortBasis,
		       FData3d<short, SHMesh> >::maker);
PersistentTypeID
backwards_compat_SHVFc("StructHexVolField<char>", "Field",
		       GenericField<SHMesh, FDHcharBasis,
		       FData3d<char, SHMesh> >::maker,
		       GenericField<SHMesh, CFDcharBasis,
		       FData3d<char, SHMesh> >::maker);
PersistentTypeID
backwards_compat_SHVFui("StructHexVolField<unsigned_int>", "Field",
			GenericField<SHMesh, FDHuintBasis,
			FData3d<unsigned int, SHMesh> >::maker,
			GenericField<SHMesh, CFDuintBasis,
			FData3d<unsigned int, SHMesh> >::maker);
PersistentTypeID
backwards_compat_SHVFus("StructHexVolField<unsigned_short>", "Field",
			GenericField<SHMesh, FDHushortBasis,
			FData3d<unsigned short, SHMesh> >::maker,
			GenericField<SHMesh, CFDushortBasis,
			FData3d<unsigned short, SHMesh> >::maker);
PersistentTypeID
backwards_compat_SHVFuc("StructHexVolField<unsigned_char>", "Field",
			GenericField<SHMesh, FDHucharBasis,
			FData3d<unsigned char, SHMesh> >::maker,
			GenericField<SHMesh, CFDucharBasis,
			FData3d<unsigned char, SHMesh> >::maker);
PersistentTypeID
backwards_compat_SHVFul("StructHexVolField<unsigned_long>", "Field",
			GenericField<SHMesh, FDHulongBasis,
			FData3d<unsigned long, SHMesh> >::maker,
			GenericField<SHMesh, CFDulongBasis,
			FData3d<unsigned long, SHMesh> >::maker);

}
