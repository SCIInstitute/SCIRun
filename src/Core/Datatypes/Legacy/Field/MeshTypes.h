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
///@file  MeshTypes.h
///@brief Forward declarations of Mesh get_type_description.
///
///@author
///       Michael Callahan
///       Department of Computer Science
///       University of Utah
///@date  January 2005
///
///@details
///  The GCC 3.4 compiler will not compile the GenericField template
///  without these being declared ahead of time.  It will compile one
///  field include because the mesh is included before GenericField.
///  However if you include two field files it will fail because the
///  second one is not declared before the GenericField.h insertion
///  point.  It fails in order_type_description.
///

#ifndef Datatypes_MeshTypes_h
#define Datatypes_MeshTypes_h

namespace SCIRun {

class CurveMeshNode;
class CurveMeshEdge;
const TypeDescription *get_type_description(CurveMeshNode *);
const TypeDescription *get_type_description(CurveMeshEdge *);

class HexVolMeshNode;
class HexVolMeshCell;
const TypeDescription *get_type_description(HexVolMeshNode *);
const TypeDescription *get_type_description(HexVolMeshCell *);

class ImageMeshNode;
class ImageMeshFace;
const TypeDescription *get_type_description(ImageMeshNode *);
const TypeDescription *get_type_description(ImageMeshFace *);

class LatVolMeshNode;
class LatVolMeshCell;
const TypeDescription *get_type_description(LatVolMeshNode *);
const TypeDescription *get_type_description(LatVolMeshCell *);

class MaskedLatVolMeshNode;
class MaskedLatVolMeshCell;
const TypeDescription *get_type_description(MaskedLatVolMeshNode *);
const TypeDescription *get_type_description(MaskedLatVolMeshCell *);

class PointCloudMeshNode;
const TypeDescription *get_type_description(PointCloudMeshNode *);

class PrismVolMeshNode;
class PrismVolMeshCell;
const TypeDescription *get_type_description(PrismVolMeshNode *);
const TypeDescription *get_type_description(PrismVolMeshCell *);

class QuadraticLatVolMeshNode;
const TypeDescription *get_type_description(QuadraticLatVolMeshNode *);

class QuadSurfMeshNode;
class QuadSurfMeshFace;
const TypeDescription *get_type_description(QuadSurfMeshNode *);
const TypeDescription *get_type_description(QuadSurfMeshFace *);

class ScanlineMeshNode;
class ScanlineMeshEdge;
const TypeDescription *get_type_description(ScanlineMeshNode *);
const TypeDescription *get_type_description(ScanlineMeshEdge *);

class TetVolMeshNode;
class TetVolMeshCell;
const TypeDescription *get_type_description(TetVolMeshNode *);
const TypeDescription *get_type_description(TetVolMeshCell *);

class TriSurfMeshNode;
class TriSurfMeshFace;
const TypeDescription *get_type_description(TriSurfMeshNode *);
const TypeDescription *get_type_description(TriSurfMeshFace *);

} // end namespace SCIRun

#endif // Datatypes_MeshTypes_h
