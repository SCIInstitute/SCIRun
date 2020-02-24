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


/// @todo Documentation Core/Datatypes/Legacy/Field/cd_templates_fields_5.cc

#include <Core/Persistent/PersistentSTL.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Basis/NoData.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/HexTricubicHmt.h>
#include <Core/Basis/HexTricubicHmtScaleFactors.h>
#include <Core/Basis/HexTricubicHmtScaleFactorsEdges.h>
#include <Core/Basis/HexTrilinearLgn.h>
#include <Core/Datatypes/Legacy/Field/HexVolMesh.h>
#include <Core/Datatypes/Legacy/Field/GenericField.h>

using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;

namespace SCIRun {

//NoData
typedef NoDataBasis<double>                NDBasis;

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
typedef HexTrilinearLgn<Tensor>                FDTensorBasis;
typedef HexTrilinearLgn<Vector>                FDVectorBasis;
typedef HexTrilinearLgn<double>                FDdoubleBasis;
typedef HexTrilinearLgn<float>                 FDfloatBasis;
typedef HexTrilinearLgn<complex>               FDcomplexBasis;
typedef HexTrilinearLgn<int>                   FDintBasis;
typedef HexTrilinearLgn<long long>             FDlonglongBasis;
typedef HexTrilinearLgn<short>                 FDshortBasis;
typedef HexTrilinearLgn<char>                  FDcharBasis;
typedef HexTrilinearLgn<unsigned int>          FDuintBasis;
typedef HexTrilinearLgn<unsigned short>        FDushortBasis;
typedef HexTrilinearLgn<unsigned char>         FDucharBasis;
typedef HexTrilinearLgn<unsigned long>         FDulongBasis;

typedef HexVolMesh<HexTrilinearLgn<Point> > HVMesh;
PersistentTypeID backwards_compat_HVM("HexVolMesh", "Mesh",
				      HVMesh::maker, HVMesh::maker);

template class HexVolMesh<HexTrilinearLgn<Point> >;

//NoData
template class GenericField<HVMesh, NDBasis, std::vector<double> >;

//Constant
template class GenericField<HVMesh, CFDTensorBasis, std::vector<Tensor> >;
template class GenericField<HVMesh, CFDVectorBasis, std::vector<Vector> >;
template class GenericField<HVMesh, CFDdoubleBasis, std::vector<double> >;
template class GenericField<HVMesh, CFDfloatBasis,  std::vector<float> >;
template class GenericField<HVMesh, CFDcomplexBasis, std::vector<complex> >;
template class GenericField<HVMesh, CFDintBasis,    std::vector<int> >;
template class GenericField<HVMesh, CFDlonglongBasis,std::vector<long long> >;
template class GenericField<HVMesh, CFDshortBasis,  std::vector<short> >;
template class GenericField<HVMesh, CFDcharBasis,   std::vector<char> >;
template class GenericField<HVMesh, CFDuintBasis,   std::vector<unsigned int> >;
template class GenericField<HVMesh, CFDushortBasis, std::vector<unsigned short> >;
template class GenericField<HVMesh, CFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<HVMesh, CFDulongBasis,  std::vector<unsigned long> >;

//Linear
template class GenericField<HVMesh, FDTensorBasis, std::vector<Tensor> >;
template class GenericField<HVMesh, FDVectorBasis, std::vector<Vector> >;
template class GenericField<HVMesh, FDdoubleBasis, std::vector<double> >;
template class GenericField<HVMesh, FDfloatBasis,  std::vector<float> >;
template class GenericField<HVMesh, FDcomplexBasis, std::vector<complex> >;
template class GenericField<HVMesh, FDintBasis,    std::vector<int> >;
template class GenericField<HVMesh, FDlonglongBasis,std::vector<long long> >;
template class GenericField<HVMesh, FDshortBasis,  std::vector<short> >;
template class GenericField<HVMesh, FDcharBasis,   std::vector<char> >;
template class GenericField<HVMesh, FDuintBasis,   std::vector<unsigned int> >;
template class GenericField<HVMesh, FDushortBasis, std::vector<unsigned short> >;
template class GenericField<HVMesh, FDucharBasis,  std::vector<unsigned char> >;
template class GenericField<HVMesh, FDulongBasis,  std::vector<unsigned long> >;


PersistentTypeID
backwards_compat_HVFT("HexVolField<Tensor>", "Field",
		      GenericField<HVMesh, FDTensorBasis,
		      std::vector<Tensor> >::maker,
		      GenericField<HVMesh, CFDTensorBasis,
		      std::vector<Tensor> >::maker);
PersistentTypeID
backwards_compat_HVFV("HexVolField<Vector>", "Field",
		      GenericField<HVMesh, FDVectorBasis,
		      std::vector<Vector> >::maker,
		      GenericField<HVMesh, CFDVectorBasis,
		      std::vector<Vector> >::maker);
PersistentTypeID
backwards_compat_HVFd("HexVolField<double>", "Field",
		      GenericField<HVMesh, FDdoubleBasis,
		      std::vector<double> >::maker,
		      GenericField<HVMesh, CFDdoubleBasis,
		      std::vector<double> >::maker,
		      GenericField<HVMesh, NDBasis,
		      std::vector<double> >::maker);
PersistentTypeID
backwards_compat_HVFf("HexVolField<float>", "Field",
		      GenericField<HVMesh, FDfloatBasis,
		      std::vector<float> >::maker,
		      GenericField<HVMesh, CFDfloatBasis,
		      std::vector<float> >::maker);
PersistentTypeID
backwards_compat_HVFco("HexVolField<complex>", "Field",
          GenericField<HVMesh, FDcomplexBasis,
          std::vector<complex> >::maker,
          GenericField<HVMesh, CFDcomplexBasis,
          std::vector<complex> >::maker);
PersistentTypeID
backwards_compat_HVFi("HexVolField<int>", "Field",
		      GenericField<HVMesh, FDintBasis,
		      std::vector<int> >::maker,
		      GenericField<HVMesh, CFDintBasis,
		      std::vector<int> >::maker);
PersistentTypeID
backwards_compat_HVFs("HexVolField<short>", "Field",
		      GenericField<HVMesh, FDshortBasis,
		      std::vector<short> >::maker,
		      GenericField<HVMesh, CFDshortBasis,
		      std::vector<short> >::maker);
PersistentTypeID
backwards_compat_HVFc("HexVolField<char>", "Field",
		      GenericField<HVMesh, FDcharBasis,
		      std::vector<char> >::maker,
		      GenericField<HVMesh, CFDcharBasis,
		      std::vector<char> >::maker);
PersistentTypeID
backwards_compat_HVFui("HexVolField<unsigned_int>", "Field",
		       GenericField<HVMesh, FDuintBasis,
		       std::vector<unsigned int> >::maker,
		       GenericField<HVMesh, CFDuintBasis,
		       std::vector<unsigned int> >::maker);
PersistentTypeID
backwards_compat_HVFus("HexVolField<unsigned_short>", "Field",
		       GenericField<HVMesh, FDushortBasis,
		       std::vector<unsigned short> >::maker,
		       GenericField<HVMesh, CFDushortBasis,
		       std::vector<unsigned short> >::maker);
PersistentTypeID
backwards_compat_HVFuc("HexVolField<unsigned_char>", "Field",
		       GenericField<HVMesh, FDucharBasis,
		       std::vector<unsigned char> >::maker,
		       GenericField<HVMesh, CFDucharBasis,
		       std::vector<unsigned char> >::maker);
PersistentTypeID
backwards_compat_HVFul("HexVolField<unsigned_long>", "Field",
		       GenericField<HVMesh, FDulongBasis,
		       std::vector<unsigned long> >::maker,
		       GenericField<HVMesh, CFDulongBasis,
		       std::vector<unsigned long> >::maker);


}
