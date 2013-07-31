/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
template class GenericField<SQMesh, CFDintBasis,    FData2d<int,SQMesh> >;
template class GenericField<SQMesh, CFDlonglongBasis,FData2d<long long,SQMesh> >;
template class GenericField<SQMesh, CFDshortBasis,  FData2d<short,SQMesh> >;
template class GenericField<SQMesh, CFDcharBasis,   FData2d<char,SQMesh> >;
template class GenericField<SQMesh, CFDuintBasis,   FData2d<unsigned int,
							     SQMesh> >; 
template class GenericField<SQMesh, CFDushortBasis, FData2d<unsigned short,
							     SQMesh> >;
template class GenericField<SQMesh, CFDucharBasis,  FData2d<unsigned char,
							     SQMesh> >;
template class GenericField<SQMesh, CFDulongBasis,  FData2d<unsigned long,
							    SQMesh> >;

//Linear
template class GenericField<SQMesh, FDQTensorBasis, FData2d<Tensor,SQMesh> >;
template class GenericField<SQMesh, FDQVectorBasis, FData2d<Vector,SQMesh> >;
template class GenericField<SQMesh, FDQdoubleBasis, FData2d<double,SQMesh> >;
template class GenericField<SQMesh, FDQfloatBasis,  FData2d<float,SQMesh> >;
template class GenericField<SQMesh, FDQintBasis,    FData2d<int,SQMesh> >;
template class GenericField<SQMesh, FDQlonglongBasis,FData2d<long long,SQMesh> >;
template class GenericField<SQMesh, FDQshortBasis,  FData2d<short,SQMesh> >;
template class GenericField<SQMesh, FDQcharBasis,   FData2d<char,SQMesh> >;
template class GenericField<SQMesh, FDQuintBasis,   FData2d<unsigned int,
							    SQMesh> >; 
template class GenericField<SQMesh, FDQushortBasis, FData2d<unsigned short,
							    SQMesh> >;
template class GenericField<SQMesh, FDQucharBasis,  FData2d<unsigned char,
							    SQMesh> >;
template class GenericField<SQMesh, FDQulongBasis,  FData2d<unsigned long,
							    SQMesh> >;

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

//Linear
typedef HexTrilinearLgn<Tensor>                FDHTensorBasis;
typedef HexTrilinearLgn<Vector>                FDHVectorBasis;
typedef HexTrilinearLgn<double>                FDHdoubleBasis;
typedef HexTrilinearLgn<float>                 FDHfloatBasis;
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
template class GenericField<SHMesh, CFDintBasis,    FData3d<int,SHMesh> >;
template class GenericField<SHMesh, CFDlonglongBasis,FData3d<long long,SHMesh> >;
template class GenericField<SHMesh, CFDshortBasis,  FData3d<short,SHMesh> >;
template class GenericField<SHMesh, CFDcharBasis,   FData3d<char,SHMesh> >;
template class GenericField<SHMesh, CFDuintBasis,   FData3d<unsigned int,
							     SHMesh> >;
template class GenericField<SHMesh, CFDushortBasis, FData3d<unsigned short,
							     SHMesh> >;
template class GenericField<SHMesh, CFDucharBasis,  FData3d<unsigned char,
							     SHMesh> >;
template class GenericField<SHMesh, CFDulongBasis,  FData3d<unsigned long,
							     SHMesh> >;

//Linear
template class GenericField<SHMesh, FDHTensorBasis, FData3d<Tensor,SHMesh> >;
template class GenericField<SHMesh, FDHVectorBasis, FData3d<Vector,SHMesh> >;
template class GenericField<SHMesh, FDHdoubleBasis, FData3d<double,SHMesh> >;
template class GenericField<SHMesh, FDHfloatBasis,  FData3d<float,SHMesh> >;
template class GenericField<SHMesh, FDHintBasis,    FData3d<int,SHMesh> >;
template class GenericField<SHMesh, FDHlonglongBasis,FData3d<long long,SHMesh> >;
template class GenericField<SHMesh, FDHshortBasis,  FData3d<short,SHMesh> >;
template class GenericField<SHMesh, FDHcharBasis,   FData3d<char,SHMesh> >;
template class GenericField<SHMesh, FDHuintBasis,   FData3d<unsigned int,
							    SHMesh> >;
template class GenericField<SHMesh, FDHushortBasis, FData3d<unsigned short,
							    SHMesh> >;
template class GenericField<SHMesh, FDHucharBasis,  FData3d<unsigned char,
							    SHMesh> >;
template class GenericField<SHMesh, FDHulongBasis,  FData3d<unsigned long,
							    SHMesh> >;


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