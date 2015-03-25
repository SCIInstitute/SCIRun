/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
typedef ConstantBasis<int>                   CFDintBasis;
typedef ConstantBasis<long long>             CFDlonglongBasis;
typedef ConstantBasis<short>                 CFDshortBasis;
typedef ConstantBasis<char>                  CFDcharBasis;
typedef ConstantBasis<unsigned int>          CFDuintBasis;
typedef ConstantBasis<unsigned short>        CFDushortBasis;
typedef ConstantBasis<unsigned char>         CFDucharBasis;
typedef ConstantBasis<unsigned long>         CFDulongBasis;

//Linear
typedef CrvLinearLgn<Tensor>                FDTensorBasis;
typedef CrvLinearLgn<Vector>                FDVectorBasis;
typedef CrvLinearLgn<double>                FDdoubleBasis;
typedef CrvLinearLgn<float>                 FDfloatBasis;
typedef CrvLinearLgn<int>                   FDintBasis;
typedef CrvLinearLgn<long long>             FDlonglongBasis;
typedef CrvLinearLgn<short>                 FDshortBasis;
typedef CrvLinearLgn<char>                  FDcharBasis;
typedef CrvLinearLgn<unsigned int>          FDuintBasis;
typedef CrvLinearLgn<unsigned short>        FDushortBasis;
typedef CrvLinearLgn<unsigned char>         FDucharBasis;
typedef CrvLinearLgn<unsigned long>         FDulongBasis;

typedef ScanlineMesh<CrvLinearLgn<Point> > SLMesh;

PersistentTypeID backwards_compat_SLM("ScanlineMesh", "Mesh",
				      SLMesh::maker, SLMesh::maker);

namespace SCIRun {
  
  template class ScanlineMesh<CrvLinearLgn<Point> >;

//NoData
template class GenericField<SLMesh, NDBasis, std::vector<double> >;

//Linear
template class GenericField<SLMesh, CFDTensorBasis, std::vector<Tensor> >;       
template class GenericField<SLMesh, CFDVectorBasis, std::vector<Vector> >;       
template class GenericField<SLMesh, CFDdoubleBasis, std::vector<double> >;       
template class GenericField<SLMesh, CFDfloatBasis,  std::vector<float> >;        
template class GenericField<SLMesh, CFDintBasis,    std::vector<int> >;
template class GenericField<SLMesh, CFDlonglongBasis,std::vector<long long> >;
template class GenericField<SLMesh, CFDshortBasis,  std::vector<short> >;        
template class GenericField<SLMesh, CFDcharBasis,   std::vector<char> >;         
template class GenericField<SLMesh, CFDuintBasis,   std::vector<unsigned int> >; 
template class GenericField<SLMesh, CFDushortBasis, std::vector<unsigned short> >;
template class GenericField<SLMesh, CFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<SLMesh, CFDulongBasis,  std::vector<unsigned long> >;

//Linear
template class GenericField<SLMesh, FDTensorBasis, std::vector<Tensor> >;       
template class GenericField<SLMesh, FDVectorBasis, std::vector<Vector> >;       
template class GenericField<SLMesh, FDdoubleBasis, std::vector<double> >;       
template class GenericField<SLMesh, FDfloatBasis,  std::vector<float> >;
template class GenericField<SLMesh, FDintBasis,    std::vector<int> >;          
template class GenericField<SLMesh, FDlonglongBasis,std::vector<long long> >;          
template class GenericField<SLMesh, FDshortBasis,  std::vector<short> >;        
template class GenericField<SLMesh, FDcharBasis,   std::vector<char> >;         
template class GenericField<SLMesh, FDuintBasis,   std::vector<unsigned int> >; 
template class GenericField<SLMesh, FDushortBasis, std::vector<unsigned short> >;
template class GenericField<SLMesh, FDucharBasis,  std::vector<unsigned char> >;
template class GenericField<SLMesh, FDulongBasis,  std::vector<unsigned long> >;
}

PersistentTypeID 
backwards_compat_SLFT("ScanlineField<Tensor>", "Field",
		      GenericField<SLMesh, FDTensorBasis, 
		      std::vector<Tensor> >::maker, 
		      GenericField<SLMesh, CFDTensorBasis, 
		      std::vector<Tensor> >::maker);
PersistentTypeID 
backwards_compat_SLFV("ScanlineField<Vector>", "Field",
		      GenericField<SLMesh, FDVectorBasis, 
		      std::vector<Vector> >::maker, 
		      GenericField<SLMesh, CFDVectorBasis, 
		      std::vector<Vector> >::maker);
PersistentTypeID 
backwards_compat_SLFd("ScanlineField<double>", "Field",
		      GenericField<SLMesh, FDdoubleBasis, 
		      std::vector<double> >::maker, 
		      GenericField<SLMesh, CFDdoubleBasis, 
		      std::vector<double> >::maker,
		      GenericField<SLMesh, NDBasis, 
		      std::vector<double> >::maker);
PersistentTypeID 
backwards_compat_SLFf("ScanlineField<float>", "Field",
		      GenericField<SLMesh, FDfloatBasis, 
		      std::vector<float> >::maker, 
		      GenericField<SLMesh, CFDfloatBasis, 
		      std::vector<float> >::maker);
PersistentTypeID 
backwards_compat_SLFi("ScanlineField<int>", "Field",
		      GenericField<SLMesh, FDintBasis, 
		      std::vector<int> >::maker, 
		      GenericField<SLMesh, CFDintBasis, 
		      std::vector<int> >::maker);
PersistentTypeID 
backwards_compat_SLFs("ScanlineField<short>", "Field",
		      GenericField<SLMesh, FDshortBasis, 
		      std::vector<short> >::maker, 
		      GenericField<SLMesh, CFDshortBasis, 
		      std::vector<short> >::maker);
PersistentTypeID 
backwards_compat_SLFc("ScanlineField<char>", "Field",
		      GenericField<SLMesh, FDcharBasis, 
		      std::vector<char> >::maker, 
		      GenericField<SLMesh, CFDcharBasis, 
		      std::vector<char> >::maker);
PersistentTypeID 
backwards_compat_SLFui("ScanlineField<unsigned_int>", "Field",
		       GenericField<SLMesh, FDuintBasis, 
		       std::vector<unsigned int> >::maker, 
		       GenericField<SLMesh, CFDuintBasis, 
		       std::vector<unsigned int> >::maker);
PersistentTypeID 
backwards_compat_SLFus("ScanlineField<unsigned_short>", "Field",
		       GenericField<SLMesh, FDushortBasis, 
		       std::vector<unsigned short> >::maker, 
		       GenericField<SLMesh, CFDushortBasis, 
		       std::vector<unsigned short> >::maker);
PersistentTypeID 
backwards_compat_SLFuc("ScanlineField<unsigned_char>", "Field",
		       GenericField<SLMesh, FDucharBasis, 
		       std::vector<unsigned char> >::maker, 
		       GenericField<SLMesh, CFDucharBasis, 
		       std::vector<unsigned char> >::maker);
PersistentTypeID 
backwards_compat_SLFul("ScanlineField<unsigned_long>", "Field",
		       GenericField<SLMesh, FDulongBasis, 
		       std::vector<unsigned long> >::maker, 
		       GenericField<SLMesh, CFDulongBasis, 
		       std::vector<unsigned long> >::maker);

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
