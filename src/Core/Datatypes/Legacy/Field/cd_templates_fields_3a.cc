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


/// @todo Documentation Core/Datatypes/Legacy/Field/cd_templates_fields_3a.cc

#include <Core/Persistent/PersistentSTL.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Basis/NoData.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/TetQuadraticLgn.h>
#include <Core/Basis/HexTriquadraticLgn.h>
#include <Core/Basis/PrismLinearLgn.h>
#include <Core/Datatypes/Legacy/Field/PrismVolMesh.h>
#include <Core/Datatypes/Legacy/Field/TetVolMesh.h>
#include <Core/Datatypes/Legacy/Field/LatVolMesh.h>
#include <Core/Datatypes/Legacy/Field/GenericField.h>


using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Basis;

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

//Linear
typedef PrismLinearLgn<Tensor>                PFDTensorBasis;
typedef PrismLinearLgn<Vector>                PFDVectorBasis;
typedef PrismLinearLgn<double>                PFDdoubleBasis;
typedef PrismLinearLgn<float>                 PFDfloatBasis;
typedef PrismLinearLgn<complex>               PFDcomplexBasis;
typedef PrismLinearLgn<int>                   PFDintBasis;
typedef PrismLinearLgn<long long>             PFDlonglongBasis;
typedef PrismLinearLgn<short>                 PFDshortBasis;
typedef PrismLinearLgn<char>                  PFDcharBasis;
typedef PrismLinearLgn<unsigned int>          PFDuintBasis;
typedef PrismLinearLgn<unsigned short>        PFDushortBasis;
typedef PrismLinearLgn<unsigned char>         PFDucharBasis;
typedef PrismLinearLgn<unsigned long>         PFDulongBasis;

typedef PrismVolMesh<PrismLinearLgn<Point> > PVMesh;
PersistentTypeID backwards_compat_PVM("PrismVolMesh", "Mesh", PVMesh::maker, PVMesh::maker);

namespace SCIRun {

template class PrismVolMesh<PrismLinearLgn<Point> >;


//NoData
template class GenericField<PVMesh, NDBasis, std::vector<double> >;

//Constant
template class GenericField<PVMesh, CFDTensorBasis, std::vector<Tensor> >;
template class GenericField<PVMesh, CFDVectorBasis, std::vector<Vector> >;
template class GenericField<PVMesh, CFDdoubleBasis, std::vector<double> >;
template class GenericField<PVMesh, CFDfloatBasis,  std::vector<float> >;
template class GenericField<PVMesh, CFDcomplexBasis, std::vector<complex> >;
template class GenericField<PVMesh, CFDintBasis,    std::vector<int> >;
template class GenericField<PVMesh, CFDlonglongBasis,std::vector<long long> >;
template class GenericField<PVMesh, CFDshortBasis,  std::vector<short> >;
template class GenericField<PVMesh, CFDcharBasis,   std::vector<char> >;
template class GenericField<PVMesh, CFDuintBasis,   std::vector<unsigned int> >;
template class GenericField<PVMesh, CFDushortBasis, std::vector<unsigned short> >;
template class GenericField<PVMesh, CFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<PVMesh, CFDulongBasis,  std::vector<unsigned long> >;

//Linear
template class GenericField<PVMesh, PFDTensorBasis, std::vector<Tensor> >;
template class GenericField<PVMesh, PFDVectorBasis, std::vector<Vector> >;
template class GenericField<PVMesh, PFDdoubleBasis, std::vector<double> >;
template class GenericField<PVMesh, PFDfloatBasis,  std::vector<float> >;
template class GenericField<PVMesh, PFDcomplexBasis, std::vector<complex> >;
template class GenericField<PVMesh, PFDintBasis,    std::vector<int> >;
template class GenericField<PVMesh, PFDlonglongBasis,std::vector<long long> >;
template class GenericField<PVMesh, PFDshortBasis,  std::vector<short> >;
template class GenericField<PVMesh, PFDcharBasis,   std::vector<char> >;
template class GenericField<PVMesh, PFDuintBasis,   std::vector<unsigned int> >;
template class GenericField<PVMesh, PFDushortBasis, std::vector<unsigned short> >;
template class GenericField<PVMesh, PFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<PVMesh, PFDulongBasis,  std::vector<unsigned long> >;

}

PersistentTypeID
backwards_compat_PVFT("PrismVolField<Tensor>", "Field",
		      GenericField<PVMesh, PFDTensorBasis,
		      std::vector<Tensor> >::maker,
		      GenericField<PVMesh, CFDTensorBasis,
		      std::vector<Tensor> >::maker);
PersistentTypeID
backwards_compat_PVFV("PrismVolField<Vector>", "Field",
		      GenericField<PVMesh, PFDVectorBasis,
		      std::vector<Vector> >::maker,
		      GenericField<PVMesh, CFDVectorBasis,
		      std::vector<Vector> >::maker);
PersistentTypeID
backwards_compat_PVFd("PrismVolField<double>", "Field",
		      GenericField<PVMesh, PFDdoubleBasis,
		      std::vector<double> >::maker,
		      GenericField<PVMesh, CFDdoubleBasis,
		      std::vector<double> >::maker,
		      GenericField<PVMesh, NDBasis,
		      std::vector<double> >::maker);
PersistentTypeID
backwards_compat_PVFf("PrismVolField<float>", "Field",
		      GenericField<PVMesh, PFDfloatBasis,
		      std::vector<float> >::maker,
		      GenericField<PVMesh, CFDfloatBasis,
		      std::vector<float> >::maker);
PersistentTypeID
backwards_compat_PVFco("PrismVolField<complex>", "Field",
          GenericField<PVMesh, PFDcomplexBasis,
          std::vector<complex> >::maker,
          GenericField<PVMesh, CFDcomplexBasis,
          std::vector<complex> >::maker);
PersistentTypeID
backwards_compat_PVFi("PrismVolField<int>", "Field",
		      GenericField<PVMesh, PFDintBasis,
		      std::vector<int> >::maker,
		      GenericField<PVMesh, CFDintBasis,
		      std::vector<int> >::maker);
PersistentTypeID
backwards_compat_PVFs("PrismVolField<short>", "Field",
		      GenericField<PVMesh, PFDshortBasis,
		      std::vector<short> >::maker,
		      GenericField<PVMesh, CFDshortBasis,
		      std::vector<short> >::maker);
PersistentTypeID
backwards_compat_PVFc("PrismVolField<char>", "Field",
		      GenericField<PVMesh, PFDcharBasis,
		      std::vector<char> >::maker,
		      GenericField<PVMesh, CFDcharBasis,
		      std::vector<char> >::maker);
PersistentTypeID
backwards_compat_PVFui("PrismVolField<unsigned_int>", "Field",
		       GenericField<PVMesh, PFDuintBasis,
		       std::vector<unsigned int> >::maker,
		       GenericField<PVMesh, CFDuintBasis,
		       std::vector<unsigned int> >::maker);
PersistentTypeID
backwards_compat_PVFus("PrismVolField<unsigned_short>", "Field",
		       GenericField<PVMesh, PFDushortBasis,
		       std::vector<unsigned short> >::maker,
		       GenericField<PVMesh, CFDushortBasis,
		       std::vector<unsigned short> >::maker);
PersistentTypeID
backwards_compat_PVFuc("PrismVolField<unsigned_char>", "Field",
		       GenericField<PVMesh, PFDucharBasis,
		       std::vector<unsigned char> >::maker,
		       GenericField<PVMesh, CFDucharBasis,
		       std::vector<unsigned char> >::maker);
PersistentTypeID
backwards_compat_PVFul("PrismVolField<unsigned_long>", "Field",
		       GenericField<PVMesh, PFDulongBasis,
		       std::vector<unsigned long> >::maker,
		       GenericField<PVMesh, CFDulongBasis,
		       std::vector<unsigned long> >::maker);
