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

#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/Datatypes/Legacy/Field/FieldIndex.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#include <Core/Datatypes/Legacy/Matrix/ColumnMatrixFunctions.h>
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>
#include <Core/Datatypes/Legacy/Matrix/MatrixTypeConverter.h>
#include <Core/Persistent/PersistentSTL.h>
#endif

#include <Core/Datatypes/Legacy/Base/PropertyManager.h>

#include <Core/Datatypes/Legacy/Field/GenericField.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

namespace SCIRun
{
  namespace Core
  {
    namespace Datatypes
    {
      template class MatrixBase<double>;
      template class SparseRowMatrixGeneric<double>;
      template class DenseColumnMatrixGeneric<double>;
    }
  }
}


//Index types
const TypeDescription* get_type_description(NodeIndex<int>*);
const TypeDescription* get_type_description(EdgeIndex<int>*);
const TypeDescription* get_type_description(FaceIndex<int>*);
const TypeDescription* get_type_description(CellIndex<int>*);

const TypeDescription* get_type_description(std::vector<NodeIndex<int> >*);
const TypeDescription* get_type_description(std::vector<EdgeIndex<int> >*);
const TypeDescription* get_type_description(std::vector<FaceIndex<int> >*);
const TypeDescription* get_type_description(std::vector<CellIndex<int> >*);

namespace SCIRun {
// Property types
template class Property<bool>;
template class Property<int>;
template class Property<unsigned int>;
template class Property<std::string>;
template class Property<double>;
template class Property<float>;
template class Property<Array1<double> >;
template class Property<Array1<Tensor> >;
template class Property<std::pair<int,double> >;
template class Property<std::pair<double,double> >;
template class Property<std::pair<float,float> >;
template class Property<std::pair<unsigned int,unsigned int> >;
template class Property<std::pair<int,int> >;
template class Property<std::pair<unsigned short,unsigned short> >;
template class Property<std::pair<short,short> >;
template class Property<std::pair<unsigned char,unsigned char> >;
template class Property<std::pair<char,char> >;
template class Property<std::vector<std::pair<std::string,Tensor> > >;
template class Property<std::vector<std::pair<int,double> > >;
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
template class Property<LockingHandle< Matrix<double> > >;
template class Property<LockingHandle<NrrdData> >;
template class Property<NodeIndex<unsigned int> >;
#endif
