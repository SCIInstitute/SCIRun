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

   Author:          Martin Cole, Frank B. Sachse
   Date:            December 4 2004
*/


#include <Core/Basis/HexTrilinearLgn.h>

using namespace SCIRun::Core::Basis;

HexTrilinearLgnUnitElement::HexTrilinearLgnUnitElement() {}
HexTrilinearLgnUnitElement::~HexTrilinearLgnUnitElement() {}

double HexTrilinearLgnUnitElement::unit_vertices[8][3] =
  { {0.0L, 0.0L, 0.0L}, {1.0L, 0.0L, 0.0L}, {1.0L, 1.0L, 0.0L},
    {0.0L, 1.0L, 0.0L}, {0.0L, 0.0L, 1.0L}, {1.0L, 0.0L, 1.0L},
    {1.0L, 1.0L, 1.0L}, {0.0L, 1.0L, 1.0L} };
int HexTrilinearLgnUnitElement::unit_edges[12][2] =
  {{0,1}, {1,2}, {2,3}, {3,0},
   {4,5}, {5,6}, {6,7}, {7,4},
   {0,4}, {5,1}, {2,6}, {7,3}};

int HexTrilinearLgnUnitElement::unit_faces[6][4] =
  {{0,3,2,1}, {4,5,6,7}, {0,1,5,4},
   {2,3,7,6}, {3,0,4,7}, {1,2,6,5}};

double HexTrilinearLgnUnitElement::unit_face_normals[6][3] =
  { {0.0L, 0.0L, -1.0L}, {0.0L, -1.0L, 0.0L},  {1.0L, 0.0L, 0.0L},
    {0.0L, 1.0L, 0.0L}, {-1.0L, 0.0L, 0.0L},  {0.0L, 0.0L, 1.0L} };

double HexTrilinearLgnUnitElement::unit_center[3] = { 0.5, 0.5, 0.5 };
