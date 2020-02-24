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


#include <Core/Basis/TetLinearLgn.h>

using namespace SCIRun::Core::Basis;

double TetLinearLgnUnitElement::unit_vertices[4][3] =
  {{0,0,0}, {1,0,0}, {0,1,0}, {0,0,1}};
int TetLinearLgnUnitElement::unit_edges[6][2] =
  {{0,1}, {1,2}, {2,0}, {0,3}, {1,3}, {2,3}};
int TetLinearLgnUnitElement::unit_faces[4][3] =
  {{3,2,1}, {0,2,3}, {3,1,0}, {0,1,2}};

double TetLinearLgnUnitElement::unit_face_normals[4][3] =
  { {-5.773503e-01, -5.773503e-01, -5.773503e-01},
    {1.000000e+00, 0.000000e+00, 0.000000e+00},
    {0.000000e+00, 1.000000e+00, 0.000000e+00},
    {0.000000e+00, 0.000000e+00, 1.000000e+00} };

double TetLinearLgnUnitElement::unit_face_areas[4] =
  { 0.866, 0.5, 0.5, 0.5 };

double TetLinearLgnUnitElement::unit_center[3] = { 0.25, 0.25, 0.25 };
