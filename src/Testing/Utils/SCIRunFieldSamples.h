/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

/// Utility file containing empty and very small fields (no data).


namespace SCIRun
{

namespace TestUtils
{

// TODO: more field and basis types needed

inline
FieldHandle EmptyTetVolFieldConstantBasis()
{
  FieldInformation fi(TETVOLMESH_E, CONSTANTDATA_E, DOUBLE_E);
  return( CreateField(fi) );
}

inline
FieldHandle EmptyTetVolFieldLinearBasis()
{
  FieldInformation fi(TETVOLMESH_E, LINEARDATA_E, DOUBLE_E);
  return( CreateField(fi) );
}

inline
FieldHandle EmptyTriSurfFieldConstantBasis()
{
  FieldInformation fi(TRISURFMESH_E, CONSTANTDATA_E, DOUBLE_E);
  return( CreateField(fi) );
}

inline
FieldHandle EmptyTriSurfFieldLinearBasis()
{
  FieldInformation fi(TRISURFMESH_E, LINEARDATA_E, DOUBLE_E);
  return( CreateField(fi) );
}


FieldHandle CubeTetVolConstantBasis();
FieldHandle CubeTetVolLinearBasis();

FieldHandle TetrahedronTetVolConstantBasis();
FieldHandle TetrahedronTetVolLinearBasis();

FieldHandle TriangleTriSurfConstantBasis();
FieldHandle TriangleTriSurfLinearBasis();

FieldHandle CubeTriSurfConstantBasis();
FieldHandle CubeTriSurfLinearBasis();

FieldHandle TetrahedronTriSurfConstantBasis();
FieldHandle TetrahedronTriSurfLinearBasis();

}}

#endif