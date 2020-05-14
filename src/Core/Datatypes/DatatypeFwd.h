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


#ifndef CORE_DATATYPES_DATATYPE_FWD_H
#define CORE_DATATYPES_DATATYPE_FWD_H

#include <vector>
#include <Core/Utils/SmartPointers.h>
#include <boost/optional.hpp>
// ReSharper disable once CppUnusedIncludeDirective
#include <boost/make_shared.hpp>
#include <Core/Datatypes/MatrixFwd.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  class Datatype;
  typedef SharedPointer<Datatype> DatatypeHandle;
  typedef SharedPointer<const Datatype> DatatypeConstHandle;
  typedef boost::optional<DatatypeHandle> DatatypeHandleOption;

  class Scalar;
  class Double;
  class Int32;
  class String;
  class GeometryObject;
  class ColorMap;
  class Bundle;
  class MetadataObject;

  typedef SharedPointer<String> StringHandle;
  typedef SharedPointer<GeometryObject> GeometryBaseHandle;
  typedef SharedPointer<ColorMap> ColorMapHandle;
  typedef SharedPointer<Bundle> BundleHandle;
}}

  class Field;
  class Mesh;
  typedef SharedPointer<Field> FieldHandle;
  typedef SharedPointer<Mesh> MeshHandle;
  typedef std::vector<FieldHandle> FieldList;

  class NrrdData;
  typedef SharedPointer<NrrdData> NrrdDataHandle;

  namespace Core {
  namespace Datatypes {
    typedef NrrdData NrrdDataType;
  }}
}


#endif
