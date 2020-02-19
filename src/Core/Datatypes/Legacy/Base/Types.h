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


/// @todo Documentation Core/Datatypes/Legacy/Base/Types.h

#ifndef CORE_DATATYPES_LEGACY_TYPES_H
#define CORE_DATATYPES_LEGACY_TYPES_H 1

namespace SCIRun {

// Depending on whether SCIRun is 64bits, the indices into the fields are
// altered to make sure we can address really large memory chunks.

typedef long long       index_type;
typedef long long       size_type;

// Mask type ensures the user that the bits mean something
// The numeric value should not be used

typedef unsigned int    mask_type;

// Mesh information types, that work with FieldInformation and
// CreateMesh/CreateField

enum mesh_info_type {
      POINTCLOUDMESH_E = 1,
      CURVEMESH_E,
      TRISURFMESH_E,
      QUADSURFMESH_E,
      TETVOLMESH_E,
      PRISMVOLMESH_E,
      HEXVOLMESH_E,
      SCANLINEMESH_E,
      IMAGEMESH_E,
      LATVOLMESH_E,
      STRUCTCURVEMESH_E,
      STRUCTQUADSURFMESH_E,
      STRUCTHEXVOLMESH_E };

// How the data is interpolated over the elements

enum databasis_info_type {
      NODATA_E =  -1,
      CONSTANTDATA_E = 0,
      LINEARDATA_E = 1,
      QUADRATICDATA_E = 2,
      CUBICDATA_E = 3 };

// The order of the underlying elements

enum meshbasis_info_type {
      LINEARMESH_E = 1,
      QUADRATICMESH_E = 2,
      CUBICMESH_E = 3 };

// The type of data stored in the field

enum data_info_type {
      NONE_E = 0,
      CHAR_E,
      UNSIGNED_CHAR_E,
      SHORT_E,
      UNSIGNED_SHORT_E,
      INT_E,
      UNSIGNED_INT_E,
      LONGLONG_E,
      UNSIGNED_LONGLONG_E,
      FLOAT_E,
      DOUBLE_E,
      VECTOR_E,
      TENSOR_E } ;

}


#endif
