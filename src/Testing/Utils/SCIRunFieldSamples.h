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


#ifndef TESTING_UTIL_SCIRUNFIELDSAMPLES
#define TESTING_UTIL_SCIRUNFIELDSAMPLES 1

#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Point.h>

#include <Testing/Utils/share.h>

/// Utility file containing empty and very small fields (no data set, only types).

namespace SCIRun
{

namespace TestUtils
{

// TODO: more field and basis types needed

// TODO: add latvol

inline
FieldHandle EmptyTetVolFieldConstantBasis(data_info_type type)
{
  FieldInformation fi(TETVOLMESH_E, CONSTANTDATA_E, type);
  return( CreateField(fi) );
}

inline
FieldHandle EmptyTetVolFieldLinearBasis(data_info_type type)
{
  FieldInformation fi(TETVOLMESH_E, LINEARDATA_E, type);
  return( CreateField(fi) );
}

inline
FieldHandle EmptyTriSurfFieldConstantBasis(data_info_type type)
{
  FieldInformation fi(TRISURFMESH_E, CONSTANTDATA_E, type);
  return( CreateField(fi) );
}

inline
FieldHandle EmptyTriSurfFieldLinearBasis(data_info_type type)
{
  FieldInformation fi(TRISURFMESH_E, LINEARDATA_E, type);
  return( CreateField(fi) );
}


SCISHARE FieldHandle CubeTetVolConstantBasis(data_info_type type);
SCISHARE FieldHandle CubeTetVolLinearBasis(data_info_type type);

SCISHARE FieldHandle TetrahedronTetVolConstantBasis(data_info_type type);
SCISHARE FieldHandle TetrahedronTetVolLinearBasis(data_info_type type);

SCISHARE FieldHandle TriangleTriSurfConstantBasis(data_info_type type);
SCISHARE FieldHandle TriangleTriSurfLinearBasis(data_info_type type);

SCISHARE FieldHandle CubeTriSurfConstantBasis(data_info_type type);
SCISHARE FieldHandle CubeTriSurfLinearBasis(data_info_type type);

SCISHARE FieldHandle TetrahedronTriSurfConstantBasis(data_info_type type);
SCISHARE FieldHandle TetrahedronTriSurfLinearBasis(data_info_type type);

SCISHARE FieldHandle CreateEmptyLatVol();
SCISHARE FieldHandle CreateEmptyLatVol(size_type sizex, size_type sizey, size_type sizez,
  data_info_type type = DOUBLE_E,
  const Core::Geometry::Point& minb = { -1, -1, -1 }, const Core::Geometry::Point& maxb = {1,1,1});

}}

#endif
