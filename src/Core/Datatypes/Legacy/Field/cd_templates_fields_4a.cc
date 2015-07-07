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
/// @todo Documentation Core/Datatypes/Legacy/Field/cd_templates_fields_4.cc

#include <Core/Persistent/PersistentSTL.h>

#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/NoData.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Datatypes/Legacy/Field/TriSurfMesh.h>
#include <Core/Datatypes/Legacy/Field/CurveMesh.h>
#include <Core/Datatypes/Legacy/Field/GenericField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Basis;
using namespace SCIRun::Core::Geometry;

//Linear
typedef TriLinearLgn<Tensor>                FDTensorBasis;
typedef TriLinearLgn<Vector>                FDVectorBasis;
typedef TriLinearLgn<double>                FDdoubleBasis;
typedef TriLinearLgn<float>                 FDfloatBasis;
typedef TriLinearLgn<int>                   FDintBasis;
typedef TriLinearLgn<long long>             FDlonglongBasis;
typedef TriLinearLgn<short>                 FDshortBasis;
typedef TriLinearLgn<char>                  FDcharBasis;
typedef TriLinearLgn<unsigned int>          FDuintBasis;
typedef TriLinearLgn<unsigned short>        FDushortBasis;
typedef TriLinearLgn<unsigned char>         FDucharBasis;
typedef TriLinearLgn<unsigned long>         FDulongBasis;

typedef TriSurfMesh<TriLinearLgn<Point> > TSMesh;


namespace SCIRun
{
template class TriSurfMesh<TriLinearLgn<Point> >;

//Linear
template class GenericField<TSMesh, FDTensorBasis, std::vector<Tensor> >;       
template class GenericField<TSMesh, FDVectorBasis, std::vector<Vector> >;       
template class GenericField<TSMesh, FDdoubleBasis, std::vector<double> >;       
template class GenericField<TSMesh, FDfloatBasis,  std::vector<float> >;        
template class GenericField<TSMesh, FDintBasis,    std::vector<int> >;
template class GenericField<TSMesh, FDlonglongBasis,std::vector<long long> >;
template class GenericField<TSMesh, FDshortBasis,  std::vector<short> >;        
template class GenericField<TSMesh, FDcharBasis,   std::vector<char> >;         
template class GenericField<TSMesh, FDuintBasis,   std::vector<unsigned int> >; 
template class GenericField<TSMesh, FDushortBasis, std::vector<unsigned short> >;
template class GenericField<TSMesh, FDucharBasis,  std::vector<unsigned char> >;
template class GenericField<TSMesh, FDulongBasis,  std::vector<unsigned long> >;
}

//Linear
typedef CrvLinearLgn<Tensor>                CrFDTensorBasis;
typedef CrvLinearLgn<Vector>                CrFDVectorBasis;
typedef CrvLinearLgn<double>                CrFDdoubleBasis;
typedef CrvLinearLgn<float>                 CrFDfloatBasis;
typedef CrvLinearLgn<int>                   CrFDintBasis;
typedef CrvLinearLgn<long long>             CrFDlonglongBasis;
typedef CrvLinearLgn<short>                 CrFDshortBasis;
typedef CrvLinearLgn<char>                  CrFDcharBasis;
typedef CrvLinearLgn<unsigned int>          CrFDuintBasis;
typedef CrvLinearLgn<unsigned short>        CrFDushortBasis;
typedef CrvLinearLgn<unsigned char>         CrFDucharBasis;
typedef CrvLinearLgn<unsigned long>         CrFDulongBasis;

typedef CurveMesh<CrvLinearLgn<Point> > CMesh;

namespace SCIRun {

  template class CurveMesh<CrvLinearLgn<Point> >;
  
//Linear
template class GenericField<CMesh, CrFDTensorBasis, std::vector<Tensor> >;       
template class GenericField<CMesh, CrFDVectorBasis, std::vector<Vector> >;       
template class GenericField<CMesh, CrFDdoubleBasis, std::vector<double> >;       
template class GenericField<CMesh, CrFDfloatBasis,  std::vector<float> >;        
template class GenericField<CMesh, CrFDintBasis,    std::vector<int> >;
template class GenericField<CMesh, CrFDlonglongBasis,std::vector<long long> >;
template class GenericField<CMesh, CrFDshortBasis,  std::vector<short> >;        
template class GenericField<CMesh, CrFDcharBasis,   std::vector<char> >;         
template class GenericField<CMesh, CrFDuintBasis,   std::vector<unsigned int> >; 
template class GenericField<CMesh, CrFDushortBasis, std::vector<unsigned short> >;
template class GenericField<CMesh, CrFDucharBasis,  std::vector<unsigned char> >;
template class GenericField<CMesh, CrFDulongBasis,  std::vector<unsigned long> >;
}

