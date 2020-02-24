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


/*
 * Manual template instantiations
 */


/*
 * These aren't used by Datatypes directly, but since they are used in
 * a lot of different modules, we instantiate them here to avoid bloat
 *
 * Find the bloaters with:
find . -name "*.ii" -print | xargs cat | sort | uniq -c | sort -nr | more
 */

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

typedef StructCurveMesh<CrvLinearLgn<Point> > CMesh;
PersistentTypeID backwards_compat_SCM("StructCurveMesh", "Mesh",
				      CMesh::maker, CMesh::maker);
template class StructCurveMesh<CrvLinearLgn<Point> >;

//NoData
template class GenericField<CMesh, NDBasis, std::vector<double> >;


//Constant
template class GenericField<CMesh, CFDTensorBasis, std::vector<Tensor> >;
template class GenericField<CMesh, CFDVectorBasis, std::vector<Vector> >;
template class GenericField<CMesh, CFDdoubleBasis, std::vector<double> >;
template class GenericField<CMesh, CFDfloatBasis,  std::vector<float> >;
template class GenericField<CMesh, CFDcomplexBasis, std::vector<complex> >;
template class GenericField<CMesh, CFDintBasis,    std::vector<int> >;
template class GenericField<CMesh, CFDlonglongBasis,std::vector<long long> >;
template class GenericField<CMesh, CFDshortBasis,  std::vector<short> >;
template class GenericField<CMesh, CFDcharBasis,   std::vector<char> >;
template class GenericField<CMesh, CFDuintBasis,   std::vector<unsigned int> >;
template class GenericField<CMesh, CFDushortBasis, std::vector<unsigned short> >;
template class GenericField<CMesh, CFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<CMesh, CFDulongBasis,  std::vector<unsigned long> >;

//Linear
template class GenericField<CMesh, FDCrTensorBasis, std::vector<Tensor> >;
template class GenericField<CMesh, FDCrVectorBasis, std::vector<Vector> >;
template class GenericField<CMesh, FDCrdoubleBasis, std::vector<double> >;
template class GenericField<CMesh, FDCrfloatBasis,  std::vector<float> >;
template class GenericField<CMesh, FDCrcomplexBasis, std::vector<complex> >;
template class GenericField<CMesh, FDCrintBasis,    std::vector<int> >;
template class GenericField<CMesh, FDCrlonglongBasis,std::vector<long long> >;
template class GenericField<CMesh, FDCrshortBasis,  std::vector<short> >;
template class GenericField<CMesh, FDCrcharBasis,   std::vector<char> >;
template class GenericField<CMesh, FDCruintBasis,   std::vector<unsigned int> >;
template class GenericField<CMesh, FDCrushortBasis, std::vector<unsigned short> >;
template class GenericField<CMesh, FDCrucharBasis,  std::vector<unsigned char> >;
template class GenericField<CMesh, FDCrulongBasis,  std::vector<unsigned long> >;


PersistentTypeID
backwards_compat_SCFT("StructCurveField<Tensor>", "Field",
		      GenericField<CMesh, FDCrTensorBasis,
		      std::vector<Tensor> >::maker,
		      GenericField<CMesh, CFDTensorBasis,
		      std::vector<Tensor> >::maker);
PersistentTypeID
backwards_compat_SCFV("StructCurveField<Vector>", "Field",
		      GenericField<CMesh, FDCrVectorBasis,
		      std::vector<Vector> >::maker,
		      GenericField<CMesh, CFDVectorBasis,
		      std::vector<Vector> >::maker);
PersistentTypeID
backwards_compat_SCFd("StructCurveField<double>", "Field",
		      GenericField<CMesh, FDCrdoubleBasis,
		      std::vector<double> >::maker,
		      GenericField<CMesh, CFDdoubleBasis,
		      std::vector<double> >::maker,
		      GenericField<CMesh, NDBasis,
		      std::vector<double> >::maker);
PersistentTypeID
backwards_compat_SCFf("StructCurveField<float>", "Field",
		      GenericField<CMesh, FDCrfloatBasis,
		      std::vector<float> >::maker,
		      GenericField<CMesh, CFDfloatBasis,
		      std::vector<float> >::maker);
PersistentTypeID
backwards_compat_SCFco("StructCurveField<complex>", "Field",
          GenericField<CMesh, FDCrcomplexBasis,
          std::vector<complex> >::maker,
          GenericField<CMesh, CFDcomplexBasis,
          std::vector<complex> >::maker);
PersistentTypeID
backwards_compat_SCFi("StructCurveField<int>", "Field",
		      GenericField<CMesh, FDCrintBasis,
		      std::vector<int> >::maker,
		      GenericField<CMesh, CFDintBasis,
		      std::vector<int> >::maker);
PersistentTypeID
backwards_compat_SCFs("StructCurveField<short>", "Field",
		      GenericField<CMesh, FDCrshortBasis,
		      std::vector<short> >::maker,
		      GenericField<CMesh, CFDshortBasis,
		      std::vector<short> >::maker);
PersistentTypeID
backwards_compat_SCFc("StructCurveField<char>", "Field",
		      GenericField<CMesh, FDCrcharBasis,
		      std::vector<char> >::maker,
		      GenericField<CMesh, CFDcharBasis,
		      std::vector<char> >::maker);
PersistentTypeID
backwards_compat_SCFui("StructCurveField<unsigned_int>", "Field",
		       GenericField<CMesh, FDCruintBasis,
		       std::vector<unsigned int> >::maker,
		       GenericField<CMesh, CFDuintBasis,
		       std::vector<unsigned int> >::maker);
PersistentTypeID
backwards_compat_SCFus("StructCurveField<unsigned_short>", "Field",
		       GenericField<CMesh, FDCrushortBasis,
		       std::vector<unsigned short> >::maker,
		       GenericField<CMesh, CFDushortBasis,
		       std::vector<unsigned short> >::maker);
PersistentTypeID
backwards_compat_SCFuc("StructCurveField<unsigned_char>", "Field",
		       GenericField<CMesh, FDCrucharBasis,
		       std::vector<unsigned char> >::maker,
		       GenericField<CMesh, CFDucharBasis,
		       std::vector<unsigned char> >::maker);
PersistentTypeID
backwards_compat_SCFul("StructCurveField<unsigned_long>", "Field",
		       GenericField<CMesh, FDCrulongBasis,
		       std::vector<unsigned long> >::maker,
		       GenericField<CMesh, CFDulongBasis,
		       std::vector<unsigned long> >::maker);

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

}
