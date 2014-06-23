/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
/// @todo Documentation Core/Datatypes/DatatypeFwd.h
#ifndef CORE_DATATYPES_DATATYPE_FWD_H
#define CORE_DATATYPES_DATATYPE_FWD_H 

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/any.hpp>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  class Datatype;
  typedef boost::shared_ptr<Datatype> DatatypeHandle;
  typedef boost::shared_ptr<const Datatype> DatatypeConstHandle;
  typedef boost::optional<DatatypeHandle> DatatypeHandleOption;

  template <typename T>
  class Scalar;

  typedef Scalar<int> Int32;
  typedef Scalar<double> Double;

  class String;
  class GeometryObject;
  class ColorMap;
  class Bundle;

  typedef boost::shared_ptr<String> StringHandle;
  typedef boost::shared_ptr<GeometryObject> GeometryHandle;
}}

  class Field;
  class Mesh;
  typedef boost::shared_ptr<Field> FieldHandle;
  typedef boost::shared_ptr<Mesh> MeshHandle;
  typedef std::vector<FieldHandle> FieldList;
}


#endif
