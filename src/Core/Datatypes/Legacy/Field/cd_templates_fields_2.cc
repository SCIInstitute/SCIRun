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


/// @todo Documentation Core/Datatypes/Legacy/Field/cd_templates_fields_2.cc

#include <Core/Persistent/PersistentSTL.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/NoData.h>
#include <Core/Datatypes/Legacy/Field/ScanlineMesh.h>
#include <Core/Datatypes/Legacy/Field/PointCloudMesh.h>
#include <Core/Datatypes/Legacy/Field/GenericField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;

//NoData
typedef NoDataBasis<double>                  NDBasis;

//Constant
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

typedef ScanlineMesh<CrvLinearLgn<Point> > SLMesh;

PersistentTypeID backwards_compat_SLM("ScanlineMesh", "Mesh", SLMesh::maker, SLMesh::maker);

namespace SCIRun {

  template class ScanlineMesh<CrvLinearLgn<Point> >;

//NoData
template class GenericField<SLMesh, NDBasis, std::vector<double> >;

//Constant
template class GenericField<SLMesh, CFDTensorBasis, std::vector<Tensor> >;
template class GenericField<SLMesh, CFDVectorBasis, std::vector<Vector> >;
template class GenericField<SLMesh, CFDdoubleBasis, std::vector<double> >;
template class GenericField<SLMesh, CFDfloatBasis,  std::vector<float> >;
template class GenericField<SLMesh, CFDcomplexBasis, std::vector<complex> >;
template class GenericField<SLMesh, CFDintBasis,    std::vector<int> >;
template class GenericField<SLMesh, CFDlonglongBasis,std::vector<long long> >;
template class GenericField<SLMesh, CFDshortBasis,  std::vector<short> >;
template class GenericField<SLMesh, CFDcharBasis,   std::vector<char> >;
template class GenericField<SLMesh, CFDuintBasis,   std::vector<unsigned int> >;
template class GenericField<SLMesh, CFDushortBasis, std::vector<unsigned short> >;
template class GenericField<SLMesh, CFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<SLMesh, CFDulongBasis,  std::vector<unsigned long> >;

}

typedef PointCloudMesh<ConstantBasis<Point> > PCMesh;
PersistentTypeID backwards_compat_PCM("PointCloudMesh", "Mesh",
				      PCMesh::maker, PCMesh::maker);

namespace SCIRun {
template class PointCloudMesh<ConstantBasis<Point> >;

//NoData
template class GenericField<PCMesh, NDBasis, std::vector<double> >;

//Constant
template class GenericField<PCMesh, CFDTensorBasis, std::vector<Tensor> >;
template class GenericField<PCMesh, CFDVectorBasis, std::vector<Vector> >;
template class GenericField<PCMesh, CFDdoubleBasis, std::vector<double> >;
template class GenericField<PCMesh, CFDfloatBasis,  std::vector<float> >;
template class GenericField<PCMesh, CFDcomplexBasis, std::vector<complex> >;
template class GenericField<PCMesh, CFDintBasis,    std::vector<int> >;
template class GenericField<PCMesh, CFDlonglongBasis,std::vector<long long> >;
template class GenericField<PCMesh, CFDshortBasis,  std::vector<short> >;
template class GenericField<PCMesh, CFDcharBasis,   std::vector<char> >;
template class GenericField<PCMesh, CFDuintBasis,   std::vector<unsigned int> >;
template class GenericField<PCMesh, CFDushortBasis, std::vector<unsigned short> >;
template class GenericField<PCMesh, CFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<PCMesh, CFDulongBasis,  std::vector<unsigned long> >;
}

PersistentTypeID
backwards_compat_PCFT("PointCloudField<Tensor>", "Field",
		      GenericField<PCMesh, CFDTensorBasis,
		      std::vector<Tensor> >::maker,
		      GenericField<PCMesh, CFDTensorBasis,
		      std::vector<Tensor> >::maker);
PersistentTypeID
backwards_compat_PCFV("PointCloudField<Vector>", "Field",
		      GenericField<PCMesh, CFDVectorBasis,
		      std::vector<Vector> >::maker,
		      GenericField<PCMesh, CFDVectorBasis,
		      std::vector<Vector> >::maker);
PersistentTypeID
backwards_compat_PCFd("PointCloudField<double>", "Field",
		      GenericField<PCMesh, CFDdoubleBasis,
		      std::vector<double> >::maker,
		      GenericField<PCMesh, NDBasis,
		      std::vector<double> >::maker);
PersistentTypeID
backwards_compat_PCFf("PointCloudField<float>", "Field",
		      GenericField<PCMesh, CFDfloatBasis,
		      std::vector<float> >::maker,
		      GenericField<PCMesh, CFDfloatBasis,
		      std::vector<float> >::maker);
PersistentTypeID
backwards_compat_PCFco("PointCloudField<complex>", "Field",
          GenericField<PCMesh, CFDcomplexBasis,
          std::vector<complex> >::maker,
          GenericField<PCMesh, CFDcomplexBasis,
          std::vector<complex> >::maker);
PersistentTypeID
backwards_compat_PCFi("PointCloudField<int>", "Field",
		      GenericField<PCMesh, CFDintBasis,
		      std::vector<int> >::maker,
		      GenericField<PCMesh, CFDintBasis,
		      std::vector<int> >::maker);
PersistentTypeID
backwards_compat_PCFs("PointCloudField<short>", "Field",
		      GenericField<PCMesh, CFDshortBasis,
		      std::vector<short> >::maker,
		      GenericField<PCMesh, CFDshortBasis,
		      std::vector<short> >::maker);
PersistentTypeID
backwards_compat_PCFc("PointCloudField<char>", "Field",
		      GenericField<PCMesh, CFDcharBasis,
		      std::vector<char> >::maker,
		      GenericField<PCMesh, CFDcharBasis,
		      std::vector<char> >::maker);
PersistentTypeID
backwards_compat_PCFui("PointCloudField<unsigned_int>", "Field",
		       GenericField<PCMesh, CFDuintBasis,
		       std::vector<unsigned int> >::maker,
		       GenericField<PCMesh, CFDuintBasis,
		       std::vector<unsigned int> >::maker);
PersistentTypeID
backwards_compat_PCFus("PointCloudField<unsigned_short>", "Field",
		       GenericField<PCMesh, CFDushortBasis,
		       std::vector<unsigned short> >::maker,
		       GenericField<PCMesh, CFDushortBasis,
		       std::vector<unsigned short> >::maker);
PersistentTypeID
backwards_compat_PCFuc("PointCloudField<unsigned_char>", "Field",
		       GenericField<PCMesh, CFDucharBasis,
		       std::vector<unsigned char> >::maker,
		       GenericField<PCMesh, CFDucharBasis,
		       std::vector<unsigned char> >::maker);
PersistentTypeID
backwards_compat_PCFul("PointCloudField<unsigned_long>", "Field",
		       GenericField<PCMesh, CFDulongBasis,
		       std::vector<unsigned long> >::maker,
		       GenericField<PCMesh, CFDulongBasis,
		       std::vector<unsigned long> >::maker);
