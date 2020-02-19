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


#include <Core/Basis/PrismLinearLgn.h>

using namespace SCIRun::Core::Basis;

double PrismLinearLgnUnitElement::unit_vertices[6][3] =
  {{0,0,0}, {1,0,0}, {0,1,0},
   {0,0,1}, {1,0,1}, {0,1,1}};

int PrismLinearLgnUnitElement::unit_edges[9][2] =
  {{0,1}, {0,2}, {0,3},
   {1,2}, {1,4}, {2,5},
   {3,4}, {3,5}, {4,5}};

int PrismLinearLgnUnitElement::unit_faces[5][4] =
  {{0,1,2,-1}, {5,4,3,-1}, {4,5,2,1}, {3,4,1,0}, {0,2,5,3}};

double PrismLinearLgnUnitElement::unit_face_normals[5][3] =
  { {0.000000e+00, 0.000000e+00, 1.000000e+00},
    {0.000000e+00, 0.000000e+00, -1.000000e+00},
    {-7.071068e-01, -7.071068e-01, 0.000000e+00},
    {0.000000e+00, 1.000000e+00, 0.000000e+00},
    {1.000000e+00, 0.000000e+00, 0.000000e+00} };

double PrismLinearLgnUnitElement::unit_face_areas[5] =
  { .5, .5, 1.41421356, 1., 1.};

double PrismLinearLgnUnitElement::unit_center[3] =
  { 1.0/3.0, 1.0/3.0, 0.5};
