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

//Linear
typedef CrvLinearLgn<Tensor>                FDTensorBasis;
typedef CrvLinearLgn<Vector>                FDVectorBasis;
typedef CrvLinearLgn<double>                FDdoubleBasis;
typedef CrvLinearLgn<float>                 FDfloatBasis;
typedef CrvLinearLgn<complex>               FDcomplexBasis;
typedef CrvLinearLgn<int>                   FDintBasis;
typedef CrvLinearLgn<long long>             FDlonglongBasis;
typedef CrvLinearLgn<short>                 FDshortBasis;
typedef CrvLinearLgn<char>                  FDcharBasis;
typedef CrvLinearLgn<unsigned int>          FDuintBasis;
typedef CrvLinearLgn<unsigned short>        FDushortBasis;
typedef CrvLinearLgn<unsigned char>         FDucharBasis;
typedef CrvLinearLgn<unsigned long>         FDulongBasis;

typedef ScanlineMesh<CrvLinearLgn<Point> > SLMesh;

namespace SCIRun {

  template class ScanlineMesh<CrvLinearLgn<Point> >;

//Linear
template class GenericField<SLMesh, FDTensorBasis, std::vector<Tensor> >;
template class GenericField<SLMesh, FDVectorBasis, std::vector<Vector> >;
template class GenericField<SLMesh, FDdoubleBasis, std::vector<double> >;
template class GenericField<SLMesh, FDfloatBasis,  std::vector<float> >;
template class GenericField<SLMesh, FDcomplexBasis, std::vector<complex> >;
template class GenericField<SLMesh, FDintBasis,    std::vector<int> >;
template class GenericField<SLMesh, FDlonglongBasis,std::vector<long long> >;
template class GenericField<SLMesh, FDshortBasis,  std::vector<short> >;
template class GenericField<SLMesh, FDcharBasis,   std::vector<char> >;
template class GenericField<SLMesh, FDuintBasis,   std::vector<unsigned int> >;
template class GenericField<SLMesh, FDushortBasis, std::vector<unsigned short> >;
template class GenericField<SLMesh, FDucharBasis,  std::vector<unsigned char> >;
template class GenericField<SLMesh, FDulongBasis,  std::vector<unsigned long> >;
}

namespace SCIRun {
template class PointCloudMesh<ConstantBasis<Point> >;

}
